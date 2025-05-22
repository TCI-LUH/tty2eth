#include "main.h"
#include "board.h"
#include "sshd.h"
#include "storage.h"
#include "configuration.h"
#include "lwip.h"
#include "lwip/apps/sntp.h"
#include "ksz9563r.h"
#include "system/stm32h7xx_it.h"
#include "metric.h"

#include <wolfssh/ssh.h>
#include <stdio.h>
#include <string.h>

#include "lwip/sockets.h"

#include "FreeRTOS.h"

#define HELLO_TEST 0
#define RTC_TEST 0

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES - 1;

static int hasSystemTimeInitialized = FALSE;

__attribute__((section(".noinit"))) int hasHardFault;

__attribute__((section(".ram_d3_section"))) char serialReadBuffer[REDIRECT_BUFFER_SIZE];

// void mainLoop();
void wolfCryptDemo(const void *argument);
void defaultTask(void *argument);

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTaskAttributes = {
    .name = "defaultTask",
    .stack_size = 2 * 1024,
    .priority = (osPriority_t)osPriorityNormal,
};

void setSystemTime(time_t t, uint32_t us)
{
    printf("sntp update: %lu.%lu\n", (uint32_t)t, us);

    struct tm *now = gmtime(&t);
    RTC_TimeTypeDef time;
    time.Hours = now->tm_hour;
    time.Minutes = now->tm_min;
    time.Seconds = now->tm_sec;
    time.SubSeconds = us;

    RTC_DateTypeDef date;
    date.Year = now->tm_year % 100;
    date.Month = now->tm_mon + 1;
    date.WeekDay = now->tm_wday + 1;
    date.Date = now->tm_mday;

    HAL_RTC_SetTime(&hrtc, &time, FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &date, FORMAT_BIN);

    hasSystemTimeInitialized = TRUE;
}

static void helloTest()
{
#if HELLO_TEST
    int count = 0;
    while(1)
    {
        HAL_GPIO_TogglePin(LED_USR_GPIO_Port, LED_USR_Pin);
        printf("hello test %d\n", count++);
        osDelay(100);
    }
#endif
}

static void rtcTest()
{
#if RTC_TEST
    while (1)
    {
        RTC_TimeTypeDef time;
        RTC_DateTypeDef date;

        /* must get time and date here due to STM32 HW bug */
        HAL_RTC_GetTime(&hrtc, &time, FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date, FORMAT_BIN);
        printf("datetime: %d.%d.%d %d:%d:%d.%03d\n",
               date.Date, date.Month, date.Year,
               time.Hours, time.Minutes, time.Seconds, time.SubSeconds);
        osDelay(100);
    }
#endif
}

void initTime()
{
    /* Configure and start the SNTP client */
    LOCK_TCPIP_CORE();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "0.europe.pool.ntp.org");
    ip_addr_t sntpServer;
    IP_ADDR4(&sntpServer, 10, 42, 2, 1);
    sntp_setserver(2, &sntpServer);
    IP_ADDR4(&sntpServer, 10, 0, 0, 1);
    sntp_setserver(3, &sntpServer);
    sntp_init();
    UNLOCK_TCPIP_CORE();

    rtcTest();

    printf("wait for time initialization...\n");
    while (hasSystemTimeInitialized == FALSE)
        osDelay(100);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    MPU_Config();

    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

    SystemClock_Config();
    PeriphCommonClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_UART5_Init();
    printf("init\n");
    MX_I2C2_Init();
    MX_RTC_Init();
    MX_HASH_Init();
    MX_RNG_Init();
    wolfSSH_Init();

    initStorage();
    initCpuHighPerformanceZone();

    /* Init scheduler */
    osKernelInitialize();
    defaultTaskHandle = osThreadNew(defaultTask, NULL, &defaultTaskAttributes);

    osKernelStart();

    /* Infinite loop */
    //   wolfCryptDemo(NULL);
    //   mainLoop();
    // mainSSHD();
    while (1)
    {
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin != PHYTEC_READY_Pin)
        return;
    
    
    GPIO_PinState ready = HAL_GPIO_ReadPin(PHYTEC_READY_GPIO_Port, PHYTEC_READY_Pin);
    if(ready)
    {
        printf("phytec ready detected\n");
        osDelay(300);
        printf("init uart 4\n");
        MX_UART4_Init();
        printf("dma inited\n");
        SCB_InvalidateDCache_by_Addr((uint32_t *)(((uint32_t)serialReadBuffer) & ~(uint32_t)0x1F), sizeof(serialReadBuffer) + 32);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, serialReadBuffer, sizeof(serialReadBuffer));
        // __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
    }
    else
    {
        printf("phytec stop detected\n");
        printf("stop uart 4\n");
        HAL_UART_DeInit(&huart4);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    char* uartName = huart == &huart4 ? "uart4" : (huart == &huart5 ? "uart5" : "unknown uart"); 
    int err = HAL_UART_GetError(huart);
    printf("uart error %s %d!!\n", uartName, err);
    printf("restart uart 4\n");
    HAL_UART_DeInit(&huart4);
    MX_UART4_Init();
    SCB_InvalidateDCache_by_Addr((uint32_t *)(((uint32_t)serialReadBuffer) & ~(uint32_t)0x1F), sizeof(serialReadBuffer) + 32);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, serialReadBuffer, sizeof(serialReadBuffer));
    // __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t endOffset)
{
    static int oldPos = 0;
    if (huart->Instance != UART4)
        return;

    int pos = oldPos;
    
    // printf("%.*s", (int)(endOffset - pos), serialReadBuffer + pos);
    if(endOffset >= pos)
        sshdRedirectOutput(serialReadBuffer + pos, (endOffset - pos));
    oldPos = endOffset % sizeof(serialReadBuffer);

    SCB_InvalidateDCache_by_Addr((uint32_t *)(((uint32_t)serialReadBuffer) & ~(uint32_t)0x1F), sizeof(serialReadBuffer) + 32);
    // HAL_UARTEx_ReceiveToIdle_DMA(&huart4, serialReadBuffer, sizeof(serialReadBuffer));
    // __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
}

void dumpI2CMem(I2C_HandleTypeDef *hi2c, uint16_t devAddr, uint16_t memStartAddr, uint16_t memEndAddr)
{
    uint16_t count = memEndAddr - memStartAddr;   
    uint8_t mem[0xFF];

    uint16_t chunkCount = count / sizeof(mem);
    uint16_t chunkReminder = count % sizeof(mem);


    printf("dump i2c mem, dev: %02X, start: %04X, end: %04X, count: %d\n", devAddr, memStartAddr, memEndAddr, count);
    
    for(int chunkID = 0; chunkID < chunkCount; chunkID++)
    {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, devAddr<<1, memStartAddr, I2C_MEMADD_SIZE_16BIT, &mem[0], sizeof(mem), 100);
        memStartAddr += sizeof(mem);
        if(status != HAL_OK)
        {
            printf("mem read error\n");
            return;
        }

        for(int i = 0; i < sizeof(mem); i++)
            printf("%02X", mem[i]);
    }
    
    if(chunkReminder > 0)
    {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, devAddr<<1, memStartAddr, I2C_MEMADD_SIZE_16BIT, &mem[0], chunkReminder, 100);
        if(status != HAL_OK)
        {
            printf("mem read error\n");
            return;
        }

        for(int i = 0; i < chunkReminder; i++)
            printf("%02X", mem[i]);
    }
    printf("\ndump compleat\n");
}

void listingUDP()
{
    printf("start udp listener..\n"); 
    char buf[128];
    int fd;



    if((fd = lwip_socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("cannot create socket");
        return;
    }

    struct sockaddr_in myaddr = {0};
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(4444);

    if(lwip_bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0){
        perror("cannot bind");
        return;
    }

    struct sockaddr_in claddr = {0};
    socklen_t clientlen = sizeof(claddr);
    while (TRUE) {
        ssize_t recvlen = lwip_recvfrom(fd, buf, sizeof(buf)-1, 0, (struct sockaddr *)&claddr, &clientlen);
        if (recvlen < 0) {
            perror("cannot recvfrom()");
            return;
        }
        printf("Received %d bytes\n",recvlen);
        buf[recvlen] = 0;
        printf("Received message: \"%s\"\n",buf);

    }
}

void startUDPsender()
{
    printf("start udp sender..\n"); 
    char buf[128] = "this is a test!!";
    int fd;



    if((fd = lwip_socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("cannot create socket");
        return;
    }

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = htonl(LWIP_MAKEU32(10, 0, 0, 1));
    dest.sin_port = htons(4444);
    socklen_t destlen = sizeof(dest);
    while (TRUE) {
        printf("send udp: %s\n", buf);
        ssize_t n = lwip_sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *)&dest, destlen);
        if (n < 0) {
            printf("cannot lwip_sendto(): %d\n",n);
            return;
        }
        osDelay(100);
    }
}

void startSendTestOnUart()
{
    printf("start uart4 test sender..\n"); 
    char buf4[] = "this is a test: uart4!!\n";
    char buf5[] = "this is a test: uart5!!\n";
    while (TRUE) 
    {
        HAL_UART_Transmit(&huart4, buf4, strlen(buf4), 100);
        HAL_UART_Transmit(&huart5, buf5, strlen(buf5), 100);
        HAL_GPIO_TogglePin(LED_USR_GPIO_Port, LED_USR_Pin);
        osDelay(500);
    }
}

void startReadTestOnUart4()
{
    printf("start phytec\n");
    HAL_GPIO_WritePin(DEV_RST_GPIO_Port, DEV_RST_Pin, GPIO_PIN_SET);
    printf("start uart4 test reader..\n"); 
    while (TRUE) 
    {
    char buf[4] = {0};
        
        volatile HAL_StatusTypeDef status = HAL_UART_Receive(&huart4, buf, sizeof(buf), 100);
        HAL_UART_Transmit(&huart5, buf, sizeof(buf), 100);
    }
}

void defaultTask(void *argument)
{
    osDelay(100);
    printf("start\n");
    initConfigFiles();
    MX_LWIP_Init();
    printf("lwip inited\n");
    // mainSSHD();
    // wolfCryptDemo(NULL);

    if (hasHardFault == 1)
    {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!!! Hard Fault detected after reset !!!\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    hasHardFault = 0;   

    helloTest();
    // startReadTestOnUart4();
    // startSendTestOnUart();

    initSSHD();
    printf("start phytec\n");
    if(configIsAutoBoot())
        HAL_GPIO_WritePin(DEV_RST_GPIO_Port, DEV_RST_Pin, GPIO_PIN_SET);

    
    osDelay(100);
    storageTest();



    // while (TRUE) 
    // {
    //     printf("debug\n");
    //     HAL_GPIO_TogglePin(LED_USR_GPIO_Port, LED_USR_Pin);
    //     osDelay(500);
    // }

#ifdef TEST_HEAP
    size_t count = 1;
    char buffer[512];
    while (1)
    {
        void *p = malloc(1024);
        snprintf(buffer, sizeof(buffer), "test alloc: %dk @ %p\n", count++, p);
        printf(buffer);
        if (p == NULL)
        {
            printf("out of mem\n");
            while (1)
                osDelay(1000);
        }
    }
#endif

start:
    printf("wait for link\n");
    osDelay(500);
    // dumpI2CMem(&hi2c2, KSZ9563_I2C_ADDR, 0x00, 0x43FF);
    while (netif_is_link_up(netif_default) == FALSE)
    {
        HAL_GPIO_TogglePin(LED_USR_GPIO_Port, LED_USR_Pin);
        osDelay(100);
    }
    printf("link up\n");
    const ip_addr_t *local_addr = ip4_netif_get_local_ip(netif_default);

    while (local_addr->addr == 0)
    {
        HAL_GPIO_TogglePin(LED_USR_GPIO_Port, LED_USR_Pin);
        local_addr = ip4_netif_get_local_ip(netif_default);
        osDelay(100);
    }

    HAL_GPIO_WritePin(LED_USR_GPIO_Port, LED_USR_Pin, GPIO_PIN_RESET);

    
    ETH_MACConfigTypeDef macConf = {0};
    HAL_ETH_GetMACConfig(&heth, &macConf);
    printf("local ip: %s, speed: %d Mbits, duplex: %s\n", ip4addr_ntoa(local_addr), macConf.Speed == ETH_MACCR_FES ? 100 : 10, 
        macConf.DuplexMode == ETH_MACCR_DM ? "full" : "half");
    // startUDPsender();
    // listingUDP();
    // HAL_NVIC_SystemReset();

    initTime();

    initMetric();
    cpuLowPerformance();
    bool done = listenSSHD();
    if (done == false)
        HAL_NVIC_SystemReset();
    return;

    
    struct sockaddr_in addr;
    addr.sin_len = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(22);

    printf("start listener..\n");
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    volatile int status = bind(listener, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (status < 0)
    {
        printf("bind error: %d\n", status);
        goto start;
    }
    status = listen(listener, 0);
    if (status < 0)
    {
        printf("listen error: %d\n", status);
        goto start;
    }

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);

    // const char *message = "Hello UDP message!\n\r";
    // uint8_t buffer[100];

    // ip_addr_t PC_IPADDR;
    // IP_ADDR4(&PC_IPADDR, 10, 42, 2, 1);

    // struct udp_pcb *my_udp = udp_new();
    //  udp_connect(my_udp, , 4444);
    // struct pbuf *udp_buffer = NULL;

    for (;;)
    {
        // udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);

        // if (udp_buffer != NULL)
        // {
        //     printf("broadcast: %s\n", message);
        //     memcpy(udp_buffer->payload, message, strlen(message));
        //     udp_sendto_if(my_udp, udp_buffer, &ip_addr_broadcast, 4444, netif_default);
        //     pbuf_free(udp_buffer);
        // }
        char msg[32];

        printf("wait for connection..\n");
        int client = accept(listener, (struct sockaddr *)&clientAddr, &addrlen);
        if (client < 0 || listener < 0)
        {
            printf("listener disconnected\n");
            goto start;
        }

        printf("client connected: %s\n", inet_ntoa(clientAddr.sin_addr));

        while (1)
        {

            int count = recv(client, &msg, sizeof(msg) - sizeof(char), 0);
            if (count <= 0)
                break;
            msg[count] = 0;

            printf("read: %s", &msg[0]);

            send(client, msg, count, 0);
            send(client, msg, count, 0);
        }
        closesocket(client);

        osDelay(100);
    }
    /* USER CODE END 5 */
}

void vApplicationStackOverflowHook(TaskHandle_t *pxTask, char *pcTaskName)
{
    printf("stack overflow: %s\n", pcTaskName);

    hasHardFault = 1;
    HAL_NVIC_SystemReset();
}
// int _write(int32_t file, uint8_t *ptr, int32_t len)
// {
// /* Implement your write code here, this is used by puts and printf for example */
// int i=0;
// for(i=0 ; i<len ; i++)
// ITM_SendChar((*ptr++));
// return len;

// }
int _write(int file, const unsigned char *ptr, int len)
{

    HAL_UART_Transmit(&huart5, ptr, len, 100);
    return len;
}
