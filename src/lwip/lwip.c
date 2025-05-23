/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : LWIP.c
 * Description        : This file provides initialization code for LWIP
 *                      middleWare.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "task.h"
#if defined(__CC_ARM) /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
static void ethernet_link_status_updated(struct netif *netif);
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */

/* Variables Initialization */
struct netif gnetif;
uint8_t useDHCP = 1;
ip4_addr_t ipaddr = {0};
ip4_addr_t netmask = {0};
ip4_addr_t gw = {0};
/* USER CODE BEGIN OS_THREAD_ATTR_CMSIS_RTOS_V2 */
#define INTERFACE_THREAD_STACK_SIZE (1024)
osThreadAttr_t attributes;
/* USER CODE END OS_THREAD_ATTR_CMSIS_RTOS_V2 */

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

sys_sem_t *LWIP_NETCONN_THREAD_SEM_GET()
{
    sys_sem_t *sem = (sys_sem_t *)pvTaskGetThreadLocalStoragePointer(NULL, 0);
    if (sem != NULL)
        return sem;

    sem = (sys_sem_t *)pvPortMalloc(sizeof(sys_sem_t));

    sys_sem_new(sem, 0);
    vTaskSetThreadLocalStoragePointer(NULL, 0, sem);
    return sem;
}
void LWIP_NETCONN_THREAD_SEM_ALLOC()
{
    sys_sem_t *sem = (sys_sem_t *)pvTaskGetThreadLocalStoragePointer(NULL, 0);
    if (sem != NULL)
        return;
    sys_sem_new(sem, 0);
    vTaskSetThreadLocalStoragePointer(NULL, 0, sem);
}
void LWIP_NETCONN_THREAD_SEM_FREE()
{
    sys_sem_t *sem = (sys_sem_t *)pvTaskGetThreadLocalStoragePointer(NULL, 0);
    if (sem == NULL)
        return;
    sys_sem_free(sem);
    vPortFree(sem);
    vTaskSetThreadLocalStoragePointer(NULL, 0, NULL);
}
/**
 * LwIP initialization function
 */
void MX_LWIP_Init(void)
{
    /* Initilialize the LwIP stack with RTOS */
    tcpip_init(NULL, NULL);

    /* IP addresses initialization with DHCP (IPv4) */
    if(useDHCP)
    {
        ipaddr.addr = 0;
        netmask.addr = 0;
        gw.addr = 0;
    }

    LOCK_TCPIP_CORE();
    /* add the network interface (IPv4/IPv6) with RTOS */
    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

    /* Registers the default network interface */
    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif))
    {
        /* When the netif is fully configured this function must be called */
        netif_set_up(&gnetif);
    }
    else
    {
        /* When the netif link is down this function must be called */
        netif_set_down(&gnetif);
    }

    /* Set the link callback function, this function is called on change of link status*/
    netif_set_link_callback(&gnetif, ethernet_link_status_updated);

    /* Create the Ethernet link handler thread */
    /* USER CODE BEGIN H7_OS_THREAD_NEW_CMSIS_RTOS_V2 */
    memset(&attributes, 0x0, sizeof(osThreadAttr_t));
    attributes.name = "EthLink";
    attributes.stack_size = INTERFACE_THREAD_STACK_SIZE;
    attributes.priority = osPriorityBelowNormal;
    osThreadNew(ethernet_link_thread, &gnetif, &attributes);
    /* USER CODE END H7_OS_THREAD_NEW_CMSIS_RTOS_V2 */

    /* Start DHCP negotiation for a network interface (IPv4) */
    if (useDHCP && netif_is_link_up(&gnetif))
        dhcp_start(&gnetif);
    //   dhcp_stop()

    UNLOCK_TCPIP_CORE();
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
#endif

/**
 * @brief  Notify the User about the network interface config status
 * @param  netif: the network interface
 * @retval None
 */
static void ethernet_link_status_updated(struct netif *netif)
{
    if (netif_is_up(netif))
    {
        /* USER CODE BEGIN 5 */
        printf("eth is up\n");
        if (useDHCP && netif_dhcp_data(netif_default) == NULL)
            dhcp_start(netif_default);
        /* USER CODE END 5 */
    }
    else /* netif is down */
    {
        /* USER CODE BEGIN 6 */
        printf("eth is down\n");
        // dhcp_stop(netif_default);
        // if(listener >= 0)
        // {
        //     // lwip_shutdown(listener, SHUT_RDWR);
        //     lwip_close(listener);
        //     listener = -2;
        // }
        /* USER CODE END 6 */
    }
}

#if defined(__CC_ARM) /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
    sio_fd_t sd;

    /* USER CODE BEGIN 7 */
    sd = 0; // dummy code
            /* USER CODE END 7 */

    return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
    /* USER CODE BEGIN 8 */
    /* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
    u32_t recved_bytes;

    /* USER CODE BEGIN 9 */
    recved_bytes = 0; // dummy code
                      /* USER CODE END 9 */
    return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
    u32_t recved_bytes;

    /* USER CODE BEGIN 10 */
    recved_bytes = 0; // dummy code
                      /* USER CODE END 10 */
    return recved_bytes;
}
#endif /* MDK ARM Compiler */
