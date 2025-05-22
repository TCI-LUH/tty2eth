
#include "sshd.h"
#include "utils/assert.h"
#include "utils/ringbuffer.h"
#include "storage.h"
#include "utils/lock.h"
#include "board.h"
#include "metric.h"
#include "configuration.h"

#include <wolfssh/ssh.h>
#include <wolfssh/wolfscp.h>
#include <wolfssh/wolfsftp.h>
#include <wolfssl/wolfcrypt/hash.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <lwip/sockets.h>
#include "lwip.h"

#include <cmsis_os2.h>

#include <cJSON_Utils.h>

// #include <stdio.h>
#include <string.h>


typedef enum
{
    SSHConnectionTypeDisconnect,
    SSHConnectionTypeConnecting,
    SSHConnectionTypeSSH,
    SSHConnectionTypeSFTP,
} SSHConnectionType;

typedef enum
{
    SSHConnectionModeNone = 0,
    SSHConnectionModeReadOnly = 1,
    SSHConnectionModeShell = 2,
    SSHConnectionModeMenue = 4,
} SSHConnectionMode;

typedef struct SSHConnection
{
    WOLFSSH* ssh;
    SSHConnectionType type;
    SSHConnectionMode mode;
    struct SSHConnection* next;
} SSHConnection;

typedef struct
{
    char* type;
    size_t typeSize;

    char* key;
    size_t keySize;
} PubKey;

typedef struct
{
    char* password;
    cJSON* keys;
    bool noAuth;
    
} User;


osThreadId_t sshdReceiveTaskHandle;
const osThreadAttr_t sshdReceiveTaskAttributes = {
  .name = "sshdReceiveTask",
  .stack_size = 20 * 1024,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t sshdSendTaskHandle;
const osThreadAttr_t sshdSendTaskAttributes = {
  .name = "sshdSendTask",
  .stack_size = 12 * 1024,
  .priority = (osPriority_t) osPriorityNormal,
};

// osSemaphoreId_t sshdRedirectSem;
osEventFlagsId_t sshdRedirectEvent;
char sshdRedirectRingBuffer[4*1024];
Ring sshdRedirectRing;

byte hostKeyDer[256] = {0};
word32 hostKeyDerSize = 0;


SSHConnection* sshConnectionList;
osMutexId_t sshConnectionListMutex;
bool sshdInitialized = false;




void sshdSendMenu(SSHConnection* con);

bool findUserData(char* username, User* userdata)
{
    *userdata = (User){0};

    if(assert(username, "username is not set"))
        return false;
    if(assert(mainConfig, "no config"))
        return false;

    cJSON* users = cJSON_GetObjectItem(mainConfig, "users");
    if(assert(users, "no users found in sshd"))
        return false;

    cJSON* user = cJSON_GetObjectItem(users, username);
    if(assert(user, "user not found in sshd: %s", username))
        return false;
    
    cJSON* noAuth = cJSON_GetObjectItem(user, "no-auth");
    if(noAuth && cJSON_IsTrue(noAuth))
        return userdata->noAuth = true;

    cJSON* password = cJSON_GetObjectItem(user, "password");
    if(password)
        userdata->password = cJSON_GetStringValue(password);
    cJSON* keys = cJSON_GetObjectItem(user, "keys");
    if(keys && cJSON_IsArray(keys))
        userdata->keys = keys;

    return userdata->password || userdata->keys;
}

static bool parsePublicKey(char* buf, PubKey* pubKey)
{
    char* end = buf + strlen(buf);

    pubKey->type = buf;
    buf = strchr(buf, ' ');
    if(buf == NULL)
        return false;
    pubKey->keySize = (size_t)(buf - pubKey->type);
    buf++;
    if(buf >= end)
        return false;
    pubKey->key = buf;
    buf = strchr(buf, ' ');
    if(buf == NULL)
        return false;
    if(buf >= end)
        return false;
    pubKey->keySize = (size_t)(buf - pubKey->key);
    return true;
}

int userAuthInternal(byte authType, WS_UserAuthData* authData, void* ctx)
{
    byte authHashServer[WC_SHA256_DIGEST_SIZE];
    byte authHashClient[WC_SHA256_DIGEST_SIZE];
    User userData = {0};
    bool hasUser = findUserData(authData->username, &userData);
    if(hasUser == FALSE)
        return WOLFSSH_USERAUTH_INVALID_USER;

    if(userData.noAuth)
        return WOLFSSH_USERAUTH_SUCCESS;

    if (authType == WOLFSSH_USERAUTH_PASSWORD) 
    {
        wc_Sha256Hash(authData->sf.password.password,
                authData->sf.password.passwordSz,
                authHashServer);

        size_t pwLen = strlen(userData.password);
        word32 bufLen = sizeof(authHashClient);
        Base16_Decode((byte*)userData.password, pwLen, authHashClient, &bufLen);
        if(memcmp(authHashServer, authHashClient, MIN(bufLen, WC_SHA256_DIGEST_SIZE)) != 0)
            return WOLFSSH_USERAUTH_INVALID_PASSWORD;
    }

    else if (authType == WOLFSSH_USERAUTH_PUBLICKEY)
    {
        wc_Sha256Hash(authData->sf.publicKey.publicKey,
                authData->sf.publicKey.publicKeySz,
                authHashServer);

        cJSON* keyElement = NULL;
        cJSON_ArrayForEach(keyElement, userData.keys)
        {
            char* key = cJSON_GetStringValue(keyElement);
            if(key == NULL || key[0] == 0)
                continue;

            PubKey pubKey = {0};
            bool status = parsePublicKey(key, &pubKey);
            if(assert("key parsing error, for user: %s", authData->username))
                continue;
            
            {
                void* keyData = malloc(pubKey.keySize);
                if(assert(keyData, "cannot alloc key data, for user: {%s}", authData->username))
                    continue;

                word32 keyDataLen = pubKey.keySize;
                status = Base64_Decode(pubKey.key, pubKey.keySize, keyData, &keyDataLen);
                if(assert(status == 0, "key base64 decoding failed, for user: %s", authData->username))
                {
                    free(keyData);
                    continue;
                }

                status = wc_Sha256Hash(keyData, keyDataLen, authHashClient);
                free(keyData);
                if(status != 0)
                    continue;
            }

            if(memcmp(authHashServer, authHashClient, WC_SHA256_DIGEST_SIZE) == 0)
                return WOLFSSH_USERAUTH_SUCCESS;
        }

        return WOLFSSH_USERAUTH_INVALID_PUBLICKEY;
    }

    return WOLFSSH_USERAUTH_SUCCESS;
}

int userAuth(byte authType, WS_UserAuthData* authData, void* ctx)
{
    osMutexAcquire(mainConfigFile->mutex, 0);

    printf("auth: %s %s %s\n", authData->authName, authData->username, authData->serviceName);
    int result = userAuthInternal(authType, authData, ctx);

    osMutexRelease(mainConfigFile->mutex);
    return result;
}

void initKeys()
{

    cJSON* hostKey = cJSONUtils_GetPointer(mainConfig, "/sshd/hostKey");
    if(hostKey)
    {
        char* value = cJSON_GetStringValue(hostKey);
        hostKeyDerSize = sizeof(hostKeyDer);
        Base64_Decode((byte*)value, strlen(value), hostKeyDer, &hostKeyDerSize);
        return;
    }

    printf("init ssh keys: host key\n");
    ecc_key key;
    wc_ecc_init(&key);
    WC_RNG rng;
    wc_InitRng(&rng);
    int res = wc_ecc_make_key(&rng, 32, &key);
    if(assert(res == 0, "cannot create ssh host key: %d", res))
        return;

    res = wc_EccKeyToDer(&key, hostKeyDer, sizeof(hostKeyDer));
    if(assert(res > 0, "cannot export ssh host key: %d", res))
        return;

    hostKeyDerSize = res;

    if(mainConfig == NULL)
        mainConfig = cJSON_CreateObject();

    cJSON* sshd = cJSON_GetObjectItem(mainConfig, "sshd");
    if(sshd == NULL)
        sshd = cJSON_AddObjectToObject(mainConfig, "sshd");
    
    byte hostKeyB64[512];
    word32 hostKeyB64Size = sizeof(hostKeyB64);
    Base64_Encode_NoNl(hostKeyDer, res, hostKeyB64, &hostKeyB64Size);
    cJSON_AddStringToObject(sshd, "hostKey", (char*)hostKeyB64);

    bool status = cJSON_PrintPreallocated(mainConfig, mainConfigFile->start, mainConfigFile->bufSize, true);
    if(status)
    {
        mainConfigFile->fileSize = strlen(mainConfigFile->start);
        saveStorage();
    }
}

void createConnection(WOLFSSH* ssh, SSHConnectionType type)
{
    SSHConnection* con = (SSHConnection*)malloc(sizeof(SSHConnection));
    
    osLOCK(sshConnectionListMutex)
    {
        *con = (SSHConnection){
            .ssh = ssh,
            .type = type,
            .mode = SSHConnectionModeMenue,
            .next = sshConnectionList,
        };
        sshConnectionList = con;
    }
}

void sshdDisconnectClient(SSHConnection* con)
{
    if(con->type == SSHConnectionTypeDisconnect)
        return;
    int fd = wolfSSH_get_fd(con->ssh);
    char* err = wolfSSH_get_error_name(con->ssh);
    printf("disconnect ssh connection, fd: %d err: %s\n", fd, err);

    wolfSSH_shutdown(con->ssh);
    wolfSSH_free(con->ssh);
    lwip_close(fd);
    con->type = SSHConnectionTypeDisconnect;
}

void sshdSendStr(SSHConnection* con, char* str)
{
    size_t count = strlen(str);
    wolfSSH_stream_send(con->ssh, str, (word32) count);
}

void sshdSendLogBuffer(SSHConnection* con)
{
    wolfSSH_stream_send(con->ssh, sshdRedirectRingBuffer, (word32) sizeof(sshdRedirectRingBuffer));
}

void sshdSendMenu(SSHConnection* con)
{
    const static char menuMessage[] = 
        "tty2eth Managed Module, IP: %s\r\n"
        "1) show Status\r\n"
        "2) %s the Gateway-Module\r\n"
        "3) restart the Gateway-Module\r\n"
        "4) view Console (read only)\r\n"
        "5) enter Console Mode\r\n"
        "0) disconnect\r\n";

    Metric* currentMetric = getCurrentMetric();

    char buffer[1024];
    int count = snprintf(buffer, sizeof(buffer), menuMessage, 
        ip4addr_ntoa(&currentMetric->ip),
        currentMetric->hasPhytecPower ? "stop" : "start");
    wolfSSH_stream_send(con->ssh, buffer, (word32) count);
}

void sshdSendStatus(SSHConnection* con)
{
    const static char statusMessage[] = 
        "tty2eth Managed Module, Status:\r\n"
        "\tUptime: %lld:%02d:%02d\r\n"
        "\tNetwork IP: %s\r\n"
        "\tNetwork Duplex Mode: %s\r\n"
        "\tNetwork Speed: %d Mbits\r\n"
        "\tGateway Module Temperature: %f °C\r\n"
        "\t5 V Power: %.2f V, %.2f mA, %.2f W, (shunt %.2f mV)\r\n"
        "\t3.3 V Power: %.2f V, %.2f mA, %.2f W, (shunt %.2f mV)\r\n"
        "\t1.2 V Power: %.2f V, %.2f mA, %.2f W, (shunt %.2f mV)\r\n"
        "\tHas Phytec Power: %s\r\n";

    time_t now = time(NULL);
    Metric* currentMetric = getCurrentMetric();
    time_t uptime = now - currentMetric->uptimeSince;
    struct tm* uptimeTM = gmtime(&uptime);
    
    char buffer[1024];
    int count = snprintf(buffer, sizeof(buffer), statusMessage, 
        uptime / 3600, uptimeTM->tm_min, uptimeTM->tm_sec,
        ip4addr_ntoa(&currentMetric->ip),
        currentMetric->netDuplexMode == ETH_MACCR_DM ? "full" : "half",
        currentMetric->netSpeed,
        currentMetric->temp,
        currentMetric->power5v.voltage, currentMetric->power5v.current, currentMetric->power5v.power, currentMetric->power5v.shuntVoltage,
        currentMetric->power3v3.voltage, currentMetric->power3v3.current, currentMetric->power3v3.power, currentMetric->power3v3.shuntVoltage,
        currentMetric->power1v2.voltage, currentMetric->power1v2.current, currentMetric->power1v2.power, currentMetric->power1v2.shuntVoltage,
        currentMetric->hasPhytecPower ? "true" : "false"
        );
    wolfSSH_stream_send(con->ssh, buffer, (word32) count);
    sshdSendMenu(con);
}

void sshdHandleMenu(SSHConnection* con, char* readBuf, size_t count)
{
    if(count == 0)
        return;
    switch(readBuf[0])
    {
        case '1':
            sshdSendStatus(con);
        break;
        case '2':
            bool status = togglePhytec();
            if(status)
                sshdSendStr(con, "phytec is starting\n");
            else
                sshdSendStr(con, "phytec is stopping\n");
            sshdSendMenu(con);
        break;
        case '3':
            sshdSendStr(con, "phytec is restarting\n");
            restartPhytec();
            sshdSendMenu(con);
        break;
        case '4':
            con->mode = SSHConnectionModeReadOnly;
            sshdSendLogBuffer(con);
        break;
        case '5':
            con->mode = SSHConnectionModeShell;
            sshdSendLogBuffer(con);
        break;
        case '0':
            sshdSendStr(con, "disconnecting\r\n");
            sshdDisconnectClient(con);
        break;
    }
}
bool fdHasAny(fd_set const *fdset)
{
    static fd_set empty = {0};
    return memcmp(fdset, &empty, sizeof(fd_set)) != 0;
}

void sshdReceiveThread(void*)
{
    printf("sshdReceiveThread start...\n");
    while(1)
    {
        fd_set rfds, sfds, efds;
        FD_ZERO(&rfds);
        FD_ZERO(&sfds);
        FD_ZERO(&efds);
        int maxfd = 0;


        osLOCK(sshConnectionListMutex)
        {
            for(SSHConnection* con = sshConnectionList; con; con = con->next)
            {
                int fd = wolfSSH_get_fd(con->ssh);

                // printf("sshdReceiveThread, monitor fd: %d\n", fd);
                maxfd = MAX(maxfd, fd);
                FD_SET(fd, &rfds);
                // FD_SET(fd, &sfds);
                FD_SET(fd, &efds);
            }
        }

        // printf("sshdReceiveThread, select\n");
        struct timeval timeout = {.tv_sec = 0, .tv_usec = 100000};
        int status = select(maxfd + 1, &rfds, NULL, &efds, &timeout);
        // printf("sshdReceiveThread, select status: %d\n", status);

        if(fdHasAny(&rfds) == false && fdHasAny(&efds) == false)
            continue;

        osLOCK(sshConnectionListMutex)
        {
            enterCpuHighPerformanceZone();
            for(SSHConnection* con = sshConnectionList; con; con = con->next)
            {
                int fd = wolfSSH_get_fd(con->ssh);
                printf("sshdReceiveThread, process fd: %d, error: %s\n", fd, wolfSSH_get_error_name(con->ssh));
                if(FD_ISSET(fd, &efds))
                {
                    sshdDisconnectClient(con);

                    // if(con->type == SSHConnectionTypeConnecting)
                    //     exitCpuHighPerformanceZone();
                    continue;
                }
 
                if(FD_ISSET(fd, &rfds) || FD_ISSET(fd, &sfds))
                {
                    if(con->type == SSHConnectionTypeConnecting)
                    {
                        int status = wolfSSH_accept(con->ssh);
                        int err = wolfSSH_get_error(con->ssh);
                        if(status == WS_FATAL_ERROR && (err == WS_WANT_READ || err == WS_WANT_WRITE))
                            continue;
                        if(status == WS_SUCCESS)
                        {
                            con->type = SSHConnectionTypeSSH;
                            sshdSendMenu(con);
                            // exitCpuHighPerformanceZone();
                            continue;
                        }
                        else if(status == WS_SFTP_COMPLETE)
                        {
                            con->type = SSHConnectionTypeSFTP;
                            // exitCpuHighPerformanceZone();
                            continue;
                        }
                        
                        sshdDisconnectClient(con);
                        // exitCpuHighPerformanceZone();
                    }
                    else if(con->type == SSHConnectionTypeSSH)
                    {
                        byte readBuf[128];
                        int count = wolfSSH_stream_read(con->ssh, readBuf, sizeof(readBuf));
                        int err = wolfSSH_get_error(con->ssh);
                        if(count < 0)
                        {
                            if(err != WS_WANT_READ && err != WS_WANT_WRITE)
                                sshdDisconnectClient(con);
                            continue;
                        }
                        
                        switch(con->mode)
                        {
                            case SSHConnectionModeShell:
                                HAL_UART_Transmit(&huart4, readBuf, count, 100);
                            break;
                            case SSHConnectionModeReadOnly:
                                con->mode = SSHConnectionModeMenue;
                                sshdSendStr(con, "\r\n");
                                sshdSendMenu(con);
                            break;
                            case SSHConnectionModeMenue:
                                sshdHandleMenu(con, readBuf, count);
                            break;
                        }
                        // printf("ssh read, fd: %d: %.*s\n", fd, count, readBuf);
                    }
                    else if(con->type == SSHConnectionTypeSFTP)
                    {     
                        int ret = wolfSSH_SFTP_read(con->ssh);
                        if(ret < 0 && ret != WS_WANT_READ && ret != WS_WANT_WRITE)
                            sshdDisconnectClient(con);
                    }
                }
            }

            SSHConnection** preTarget = &sshConnectionList;
            for(SSHConnection* con = sshConnectionList; con;)
            {
                SSHConnection* cur = con;
                con = con->next;
                if(cur->type == SSHConnectionTypeDisconnect)
                {
                    *preTarget = cur->next;
                    free(cur);
                    cur = NULL;
                    continue;
                }
                preTarget = &cur->next;
            }
            exitCpuHighPerformanceZone();
        }
    }
}




void sshdSendThread(void*)
{
    printf("sshdSendThread start...\n");
    while(1)
    {
        // osSemaphoreAcquire(sshdRedirectSem, osWaitForever);
        osEventFlagsWait(sshdRedirectEvent, 1, osFlagsWaitAny, osWaitForever);
        osLOCK(sshConnectionListMutex)
        {
            enterCpuHighPerformanceZone();
            RingSlice data = ring_peek(&sshdRedirectRing, sshdRedirectRing.capacity);
            // printf("send ssh data count: %d\n", data.size[0] + data.size[1]);
            printf("\e[0;36m>>>\n%.*s%.*s\n<<<\e[0m\n", data.size[0],data.ptr[0], data.size[1],data.ptr[1]);
            for(SSHConnection* con = sshConnectionList; con; con = con->next)
            {
                if(con->type == SSHConnectionTypeSSH && (con->mode == SSHConnectionModeReadOnly || con->mode == SSHConnectionModeShell))
                {
                    wolfSSH_stream_send(con->ssh, data.ptr[0], (word32) data.size[0]);
                    if(data.ptr[1])
                        wolfSSH_stream_send(con->ssh, data.ptr[1], (word32) data.size[1]);
                }
            }
            ring_consume(&sshdRedirectRing, data.size[0] + data.size[1]);
            exitCpuHighPerformanceZone();
        }
    }
}


void sshdRedirectOutput(char* buffer, size_t size)
{
    ring_append(&sshdRedirectRing, buffer, size);
    // osSemaphoreRelease(sshdRedirectSem);
    osEventFlagsSet(sshdRedirectEvent, 1);


}

void initSSHD()
{
    if(sshdInitialized)
        return;
    // sshdRedirectSem = osSemaphoreNew(1, 1, NULL);
    sshdRedirectEvent = osEventFlagsNew(NULL);
    ring_init(&sshdRedirectRing, sshdRedirectRingBuffer, sizeof(sshdRedirectRingBuffer));

    sshConnectionListMutex = osMutexNew(NULL);
    initKeys();

    sshdSendTaskHandle = osThreadNew(sshdSendThread, NULL, &sshdSendTaskAttributes);

    sshdInitialized = true;
}
bool listenSSHD()
{
    initSSHD();
    wolfSSH_Debugging_ON();
    WOLFSSH_CTX* ctx = wolfSSH_CTX_new(WOLFSSH_ENDPOINT_SERVER, NULL );
    wolfSSH_SetUserAuth(ctx, userAuth);
    wolfSSH_CTX_SetBanner(ctx, "tty2eth ssh connection\r\n");
    wolfSSH_CTX_UsePrivateKey_buffer(ctx, hostKeyDer, hostKeyDerSize, WOLFSSH_FORMAT_ASN1);


    sshdReceiveTaskHandle = osThreadNew(sshdReceiveThread, NULL, &sshdReceiveTaskAttributes);

    struct sockaddr_in addr;
    addr.sin_len = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(22);

    printf("start listener..\n");
    int sshdListener = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sshdListener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    volatile int status = bind(sshdListener, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));

    if(assert(status == 0, "sshd listen, can not bind listener"))
        return false;

    status = listen(sshdListener, 0);
    if(assert(status == 0, "sshd listen, can not start listener"))
        return false;
    
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    
    printf("wait for connection..\n");
    while(1)
    {
        int client = accept(sshdListener, (struct sockaddr*)&clientAddr, &addrlen);
        if(assert(client >= 0, "sshd client accept error: %d", client))
            return false;
        enterCpuHighPerformanceZone();

        printf("client connected, fd: %d: ip: %s\n", client, inet_ntoa(clientAddr.sin_addr));


        // printf("start phytec\n");
        // HAL_GPIO_WritePin(DEV_RST_GPIO_Port, DEV_RST_Pin, GPIO_PIN_SET);

        WOLFSSH* ssh = wolfSSH_new(ctx);
        wolfSSH_set_fd(ssh, client);

        volatile int foo = lwip_fcntl(client, F_GETFL, 0);
        lwip_fcntl(client, F_SETFL,  O_NONBLOCK);
        createConnection(ssh, SSHConnectionTypeConnecting);
        exitCpuHighPerformanceZone();
    }
}