#include "main.h"
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/sys.h"
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"

__attribute__((section(".LwipRAM"))) volatile u8_t lwip_ram[LWIP_MEM_ALIGN_SIZE(MEM_SIZE)];

// static osThreadId lwip_tcpip_thread;
// static u8_t lwip_core_lock_count;
// static osThreadId lwip_core_lock_holder_thread;
// extern sys_mutex_t lock_tcpip_core;

// void sys_lock_tcpip_core(void)
// {
//    sys_mutex_lock(&lock_tcpip_core);
//    if (lwip_core_lock_count == 0) {
//      lwip_core_lock_holder_thread = osThreadGetId();
//    }
//    lwip_core_lock_count++;
// }

// void sys_unlock_tcpip_core(void)
// {
// 	lwip_core_lock_count--;
// 	if (lwip_core_lock_count == 0) {
// 		lwip_core_lock_holder_thread = 0;
// 	}
// 	sys_mutex_unlock(&lock_tcpip_core);
// }

// void sys_mark_tcpip_thread(void)
// {
// 	lwip_tcpip_thread = osThreadGetId();
// }

// void sys_check_core_locking(void)
// {
// 	taskENTER_CRITICAL();
// 	taskEXIT_CRITICAL();

// 	if(lwip_tcpip_thread != 0)
// 	{
// 		osThreadId current_thread = osThreadGetId();
// 		LWIP_ASSERT("Function called without core lock",
// 				current_thread == lwip_core_lock_holder_thread && lwip_core_lock_count > 0);
// 	}
// }

#if LWIP_TCPIP_CORE_LOCKING

/** Flag the core lock held. A counter for recursive locks. */
static u8_t lwip_core_lock_count;
static TaskHandle_t lwip_core_lock_holder_thread;

void sys_lock_tcpip_core(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    LWIP_ASSERT("warning, sys_lock_tcpip_core lock it self\n", lwip_core_lock_holder_thread != task);

    sys_mutex_lock(&lock_tcpip_core);
    if (lwip_core_lock_count == 0)
    {
        lwip_core_lock_holder_thread = task;
    }
    lwip_core_lock_count++;
}

void sys_unlock_tcpip_core(void)
{
    lwip_core_lock_count--;
    if (lwip_core_lock_count == 0)
    {
        lwip_core_lock_holder_thread = 0;
    }
    sys_mutex_unlock(&lock_tcpip_core);
}

#endif /* LWIP_TCPIP_CORE_LOCKING */

#if !NO_SYS
static TaskHandle_t lwip_tcpip_thread;
#endif

void sys_mark_tcpip_thread(void)
{
#if !NO_SYS
    lwip_tcpip_thread = xTaskGetCurrentTaskHandle();
#endif
}

void sys_check_core_locking(void)
{
    /* Embedded systems should check we are NOT in an interrupt context here */
    /* E.g. core Cortex-M3/M4 ports:
           configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );

       Instead, we use more generic FreeRTOS functions here, which should fail from ISR: */
    taskENTER_CRITICAL();
    taskEXIT_CRITICAL();

#if !NO_SYS
    if (lwip_tcpip_thread != 0)
    {
        TaskHandle_t current_thread = xTaskGetCurrentTaskHandle();

#if LWIP_TCPIP_CORE_LOCKING
        LWIP_ASSERT("Function called without core lock",
                    current_thread == lwip_core_lock_holder_thread && lwip_core_lock_count > 0);
#else  /* LWIP_TCPIP_CORE_LOCKING */
        LWIP_ASSERT("Function called from wrong thread", current_thread == lwip_tcpip_thread);
#endif /* LWIP_TCPIP_CORE_LOCKING */
    }
#endif /* !NO_SYS */
}
