#include "metric.h"

#include "utils/assert.h"
#include "configuration.h"
#include "board.h"
#include "max31875.h"
#include "ina4235.h"

#include "lwip.h"
#include <lwip/sockets.h>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <time.h>


osThreadId_t metricListenTaskHandle;
const osThreadAttr_t metricListenTaskAttributes = {
  .name = "metricListenTask",
  .stack_size = 16 * 1024,
  .priority = (osPriority_t) osPriorityNormal,
};

Metric currentMetric;

float metricGetTemp()
{
    const float convertion_temp_table[12] = {0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
    uint16_t config = 0;
    HAL_StatusTypeDef status = I2CReadReg16(TEMP_SENSOR_ADDR, MAX31875_CONF_REG_ADDR, &config);
    if(assert(status == HAL_OK, "metric get temp read config reg error: %d", status))
        return 0;
    config = __builtin_bswap16(config);

    uint16_t tempBits = 0;
    status = I2CReadReg16(TEMP_SENSOR_ADDR, MAX31875_TEMP_REG_ADDR, &tempBits);
    if(assert(status == HAL_OK, "metric get temp read temp reg error: %d", status))
        return 0;
    tempBits = __builtin_bswap16(tempBits);

    float result = 0;

    if(config & MAX31875_DATAFORMAT_EXTENDED)
	{
		for(uint8_t n = 3 ; n<15 ; n++)		
			result += ((tempBits >> n) & 1) * convertion_temp_table[n-3];
	} 
    else 
    {
		for(uint8_t n = 4 ; n<15 ; n++)		
			result += ((tempBits >> n) & 1) * convertion_temp_table[n-4];
	}

	if( ((tempBits >> 15) & 1) == 1 )		
        return -result;
    return result;
}

PowerMetric metricGetPowerOfCH(int ch)
{
    PowerMetric results = {0};


    uint16_t voltage = 0;
    HAL_StatusTypeDef status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_BUS_VOLTAGE_CHn_REG(ch), &voltage);
    if(assert(status == HAL_OK, "metric get power of ch%d, read voltage reg error: %d", ch, status))
        return (PowerMetric){0};

    uint16_t shuntVoltage = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_SHUNT_VOLTAGE_CHn_REG(ch), &shuntVoltage);
    if(assert(status == HAL_OK, "metric get power of ch%d, read shunt voltage reg error: %d", ch, status))
        return (PowerMetric){0};

    uint16_t current = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_CURRENT_CHn_REG(ch), &current);
    if(assert(status == HAL_OK, "metric get power of ch%d, read current reg error: %d", ch, status))
        return (PowerMetric){0};

    uint16_t power = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_POWER_CHn_REG(ch), &power);
    if(assert(status == HAL_OK, "metric get power of ch%d, read power reg error: %d", ch, status))
        return (PowerMetric){0};

    uint16_t flags = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_FLAGS_REG, &flags);
    if(assert(status == HAL_OK, "metric get flags, reg error: %d", status))
        return (PowerMetric){0};

    uint16_t ti = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_MANUFACTURER_ID_REG, &ti);
    if(assert(status == HAL_OK, "metric get manufacturer id, reg error: %d", status))
        return (PowerMetric){0};
    uint16_t did = 0;
    status = I2CReadReg16(POWER_SENSOR_ADDR, INA4235_DEVICE_ID_REG, &did);
    if(assert(status == HAL_OK, "metric get dev id, reg error: %d", status))
        return (PowerMetric){0};
    
    flags = __builtin_bswap16(flags);
    ti = __builtin_bswap16(ti);
    did = __builtin_bswap16(did);
    printf("DEBUG: INA4235 flags: 0x%x, mid: %.2s (0x%x), devID: 0x%x\n", flags, (char*)&ti, ti, did);
    results.voltage = __builtin_bswap16(voltage)*1.6*0.001;
    results.shuntVoltage = abs((int16_t)__builtin_bswap16(shuntVoltage))*2.5*0.001;
    results.current = abs((int16_t)__builtin_bswap16(current)) * POWER_SENSOR_CURRENT_LSB_MIN * 1000;
    results.power = abs((int16_t)__builtin_bswap16(power)) * POWER_SENSOR_CURRENT_LSB_MIN * 32;
    return results;
}

void metricCalibratePowerSensor()
{
    double current_lsb = POWER_SENSOR_CURRENT_LSB_MIN;
    uint16_t shunt_cal = (uint16_t)(INA4235_CALIBRATE_CONST / (current_lsb * POWER_SENSOR_R_SHUNT));
    
    for(int ch = 1; ch <= 4; ch++)
    {
        HAL_StatusTypeDef status = I2CWriteReg16(POWER_SENSOR_ADDR, INA4235_CALIBRATION_CHn_REG(ch), __builtin_bswap16(shunt_cal));
        if(assert(status == HAL_OK, "metric calibrate power ch%d, write calibration reg error: %d", ch, status))
            return;
    }
}

Metric* getCurrentMetric()
{
    ETH_MACConfigTypeDef macConf = {0};
    HAL_ETH_GetMACConfig(&heth, &macConf);
    currentMetric.ip = *ip4_netif_get_local_ip(netif_default);
    currentMetric.netDuplexMode = macConf.DuplexMode ,
    currentMetric.netSpeed = macConf.Speed == ETH_MACCR_FES ? 100 : 10;
    currentMetric.temp = metricGetTemp();
    currentMetric.power5v = metricGetPowerOfCH(2);
    currentMetric.power3v3 = metricGetPowerOfCH(1);
    currentMetric.power1v2 = metricGetPowerOfCH(4);
    currentMetric.hasPhytecPower = (bool)HAL_GPIO_ReadPin(PHYTEC_READY_GPIO_Port, PHYTEC_READY_Pin);

    return &currentMetric;
}

void metricListenHTTPThread(void*);
void metricListenHTTPSThread(void*);


bool metricIsEnable()
{
    cJSON* enable = cJSONUtils_GetPointer(mainConfig, "/metrics/enable");
    if(enable)
        return cJSON_IsTrue(enable);
    return false;
}
bool metricIsListenHTTPS()
{
    cJSON* enable = cJSONUtils_GetPointer(mainConfig, "/metrics/listenHTTPS");
    if(enable)
        return cJSON_IsTrue(enable);
    return false;
}

void initMetric()
{
    currentMetric = (Metric){0};
    currentMetric.uptimeSince = time(NULL);

    metricCalibratePowerSensor();
    
    if(metricIsEnable() == false)
        return;

    if(metricIsListenHTTPS())
    {
        wolfSSL_Init();
        // wolfSSL_Debugging_ON();
        metricListenTaskHandle = osThreadNew(metricListenHTTPSThread, NULL, &metricListenTaskAttributes);
    }
    else
    {
        metricListenTaskHandle = osThreadNew(metricListenHTTPThread, NULL, &metricListenTaskAttributes);
    }
}

void metricSend404(void* ctx, int (*sendFunc)(void*, const void*, int))
{
    char message[] = R"(
HTTP/1.1 404 Not Found

Not Found
)";

    printf("send 404 response\n");
    int status = sendFunc(ctx, message, sizeof(message));
}

void metricSend(void* ctx, int (*sendFunc)(void*, const void*, int))
{
    char message[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; version=0.0.4; charset=utf-8\r\n"
        "\r\n"
        "tty2eth_start_time{ip=\"%s\", identifier=\"%s\"} %llu\n"
        "tty2eth_uptime{ip=\"%s\", identifier=\"%s\"} %llu\n"
        "tty2eth_network_speed{ip=\"%s\", identifier=\"%s\"} %d\n"
        "tty2eth_temperature{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_5v_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_5v_shunt_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_5v_current{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_5v_power{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_3v3_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_3v3_shunt_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_3v3_current{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_3v3_power{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_1v2_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_1v2_shunt_voltage{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_1v2_current{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_1v2_power{ip=\"%s\", identifier=\"%s\"} %f\n"
        "tty2eth_has_phytec_power{ip=\"%s\", identifier=\"%s\"} %d\n";

    time_t now = time(NULL);
    getCurrentMetric();
    char* identifier = configGetMetricIdentifier();
    char* ipStr = ip4addr_ntoa(&currentMetric.ip);

    char buffer[2048];
    int count = snprintf(buffer, sizeof(buffer), message,
        ipStr, identifier, currentMetric.uptimeSince, 
        ipStr, identifier, now - currentMetric.uptimeSince,
        ipStr, identifier, currentMetric.netSpeed,
        ipStr, identifier, currentMetric.temp,

        ipStr, identifier, currentMetric.power5v.voltage,
        ipStr, identifier, currentMetric.power5v.shuntVoltage,
        ipStr, identifier, currentMetric.power5v.current,
        ipStr, identifier, currentMetric.power5v.power,

        ipStr, identifier, currentMetric.power3v3.voltage,
        ipStr, identifier, currentMetric.power3v3.shuntVoltage,
        ipStr, identifier, currentMetric.power3v3.current,
        ipStr, identifier, currentMetric.power3v3.power,

        ipStr, identifier, currentMetric.power1v2.voltage,
        ipStr, identifier, currentMetric.power1v2.shuntVoltage,
        ipStr, identifier, currentMetric.power1v2.current,
        ipStr, identifier, currentMetric.power1v2.power,

        ipStr, identifier, currentMetric.hasPhytecPower
    );

    printf("send response\n");
    int status = sendFunc(ctx, buffer, count);

}

int metricListenHTTPSSend(void* ctx, const void* data, int sz)
{
    return wolfSSL_write((WOLFSSL*)ctx, data, sz);
}
int metricListenHTTPSend(void* ctx,  const void* data, int sz)
{
    return lwip_write(*(int*)ctx, data, sz);
}


bool checkEndpoint(char* endpoint, char* request)
{
    return strncmp(endpoint, request, strlen(endpoint)) == 0;
}

void metricListenHTTPSThread(void*)
{
    WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_3_server_method());
    if(assert(ctx, "metric listener, cannot create ssl ctx"))
        return;


    volatile int status = wolfSSL_CTX_use_certificate_buffer(ctx, metricCertFile->start,
                metricCertFile->fileSize, WOLFSSL_FILETYPE_PEM);
    if(assert(status == WOLFSSL_SUCCESS, "ERROR: failed to load client certificate"))
        return;
    status = wolfSSL_CTX_use_PrivateKey_buffer(ctx, metricKeyFile->start,
                metricKeyFile->fileSize, WOLFSSL_FILETYPE_PEM);
    if(assert(status == WOLFSSL_SUCCESS, "ERROR: failed to load client key"))
        return;

    struct sockaddr_in addr;
    addr.sin_len = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(443);

    printf("start metric listener on %d..\n", ntohs(addr.sin_port));
    int metricListener = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(metricListener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    status = bind(metricListener, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));

    if(assert(status == 0, "metric listen, can not bind listener"))
        return;

    status = listen(metricListener, 0);
    if(assert(status == 0, "metric listen, can not start listener"))
        return;
    
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    
    while(1)
    {
        printf("wait for connection..\n");
        int client = accept(metricListener, (struct sockaddr*)&clientAddr, &addrlen);
        if(assert(client >= 0, "metric client accept error: %d", client))
            return;

        printf("client connected, fd: %d: ip: %s\n", client, inet_ntoa(clientAddr.sin_addr));


        WOLFSSL* ssl = wolfSSL_new(ctx);
        if(assert(ssl, "metric ssl error"))
            return;

        wolfSSL_set_fd(ssl, client);

        status = wolfSSL_accept(ssl);
        if(assert(status == WOLFSSL_SUCCESS, "metric client, cannot accept ssl: %d",  wolfSSL_get_error(ssl, status), lwip_strerr))
            goto cleanup;


        WOLFSSL_CIPHER* cipher = wolfSSL_get_current_cipher(ssl);
        printf("SSL cipher suite is %s\n", wolfSSL_CIPHER_get_name(cipher));

        char buffer[1024];
        int count = wolfSSL_read(ssl, buffer, sizeof(buffer));
        printf("request: %.*s", count, buffer);

        if(checkEndpoint("GET /metrics HTTP/1.1", buffer))
            metricSend(ssl, metricListenHTTPSSend);
        else
            metricSend404(ssl, metricListenHTTPSSend);

    cleanup:
        printf("disconnect client\n");
        wolfSSL_shutdown(ssl);
        wolfSSL_free(ssl);
        ssl = NULL;
        close(client);
    }
}


void metricListenHTTPThread(void*)
{
    struct sockaddr_in addr;
    addr.sin_len = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(80);

    printf("start metric listener on %d..\n", ntohs(addr.sin_port));
    int metricListener = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(metricListener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    int status = bind(metricListener, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));

    if(assert(status == 0, "metric listen, can not bind listener"))
        return;

    status = listen(metricListener, 0);
    if(assert(status == 0, "metric listen, can not start listener"))
        return;
    
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    
    while(1)
    {
        printf("wait for connection..\n");
        int client = accept(metricListener, (struct sockaddr*)&clientAddr, &addrlen);
        if(assert(client >= 0, "metric client accept error: %d", client))
            return;
        enterCpuHighPerformanceZone();
        printf("client connected, fd: %d: ip: %s\n", client, inet_ntoa(clientAddr.sin_addr));

        char buffer[1024];
        int count = lwip_read(client, buffer, sizeof(buffer));
        printf("request: %.*s", count, buffer);

        if(checkEndpoint("GET /metrics HTTP/1.1", buffer))
            metricSend(&client, metricListenHTTPSend);
        else
            metricSend404(&client, metricListenHTTPSend);

    cleanup:
        printf("disconnect client\n");
        close(client);
        exitCpuHighPerformanceZone();
    }
}

void metricCollectionThread(void*)
{
    while(1)
    {

    }
}