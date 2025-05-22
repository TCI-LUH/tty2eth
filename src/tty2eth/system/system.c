#include <FreeRTOS.h>
#include <errno.h>
#include <cmsis_os2.h>
#include <task.h>
#include "utils/assert.h"
#include <sys/types.h>
#include <board.h>
#include <time.h>

__attribute__((section(".ucHeap"))) uint8_t ucHeap[configTOTAL_HEAP_SIZE];


// #define MALLOCS_INSIDE_ISRs

#if !defined(configUSE_NEWLIB_REENTRANT) ||  (configUSE_NEWLIB_REENTRANT!=1)
  #warning "#define configUSE_NEWLIB_REENTRANT 1 // Required for thread-safety of newlib sprintf, dtoa, strtok, etc..."
#endif

#ifdef MALLOCS_INSIDE_ISRs // STM code to avoid malloc within ISR (USB CDC stack)
    // We can't use vTaskSuspendAll() within an ISR.
    #define DRN_ENTER_CRITICAL_SECTION(_usis) { _usis = taskENTER_CRITICAL_FROM_ISR(); } // Disables interrupts (after saving prior state)
    #define DRN_EXIT_CRITICAL_SECTION(_usis)  { taskEXIT_CRITICAL_FROM_ISR(_usis);     } // Re-enables interrupts (unless already disabled prior taskENTER_CRITICAL)
#else
    #define DRN_ENTER_CRITICAL_SECTION(_usis) vTaskSuspendAll(); // Note: safe to use before FreeRTOS scheduler started, but not in ISR
    #define DRN_EXIT_CRITICAL_SECTION(_usis)  xTaskResumeAll();  // Note: safe to use before FreeRTOS scheduler started, but not in ISR
#endif

extern char __HeapBase;
extern char __HeapLimit;
extern char __HeapSize;
static size_t heapBytesRemaining = (size_t)&__HeapSize;


void * _sbrk_r(struct _reent *pReent, int incr) 
{
	(void)pReent;
    #ifdef MALLOCS_INSIDE_ISRs // block interrupts during free-storage use
      UBaseType_t usis; // saved interrupt status
    #endif

    static char *currentHeapEnd = &__HeapBase;

    
    DRN_ENTER_CRITICAL_SECTION(usis);
    if (currentHeapEnd + incr > &__HeapLimit) 
    {
        // Ooops, no more memory available...
        #if( configUSE_MALLOC_FAILED_HOOK == 1 )
        {
            extern void vApplicationMallocFailedHook( void );
            DRN_EXIT_CRITICAL_SECTION(usis);
            vApplicationMallocFailedHook();
        }
        #elif defined(configHARD_STOP_ON_MALLOC_FAILURE)
            // If you want to alert debugger or halt...
            // WARNING: brkpt instruction may prevent watchdog operation...
            while(1) { __asm("bkpt #0"); } // Stop in GUI as if at a breakpoint (if debugging, otherwise loop forever)
        #else
            // Default, if you prefer to believe your application will gracefully trap out-of-memory...
            pReent->_errno = ENOMEM; // newlib's thread-specific errno
            DRN_EXIT_CRITICAL_SECTION(usis);
        #endif
        return (char *)-1; // the malloc-family routine that called sbrk will return 0
    }
    // 'incr' of memory is available: update accounting and return it.
    char *previousHeapEnd = currentHeapEnd;
    currentHeapEnd += incr;
    heapBytesRemaining -= incr;
    DRN_EXIT_CRITICAL_SECTION(usis);
    return (char *) previousHeapEnd;
}


//! non-reentrant sbrk uses is actually reentrant by using current context
// ... because the current _reent structure is pointed to by global _impure_ptr
char * sbrk(int incr) { return _sbrk_r(_impure_ptr, incr); }
//! _sbrk is a synonym for sbrk.
char * _sbrk(int incr) { return sbrk(incr); }

#ifdef MALLOCS_INSIDE_ISRs // block interrupts during free-storage use
  static UBaseType_t malLock_uxSavedInterruptStatus;
#endif

void __malloc_lock(struct _reent *r)   {
  (void)(r);
  #if defined(MALLOCS_INSIDE_ISRs)
    DRN_ENTER_CRITICAL_SECTION(malLock_uxSavedInterruptStatus);
  #else
    int insideAnISR = xPortIsInsideInterrupt();
    if(assert(!insideAnISR, "malloc detected inside of an interrupt!!" )) // Make damn sure no more mallocs inside ISRs!!
        HAL_NVIC_SystemReset();
  vTaskSuspendAll();
  #endif
}

void __malloc_unlock(struct _reent *r) {
  (void)(r);
  #if defined(MALLOCS_INSIDE_ISRs)
    DRN_EXIT_CRITICAL_SECTION(malLock_uxSavedInterruptStatus);
  #else
  (void)xTaskResumeAll();
  #endif
}

void __env_lock()
{       
    vTaskSuspendAll(); 
}

void __env_unlock()  
{ 
    xTaskResumeAll(); 
}

time_t time (time_t *t)
{
    
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    HAL_RTC_GetTime(&hrtc, &time, FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, FORMAT_BIN);

    struct tm datetime = {
        .tm_sec = time.Seconds,
        .tm_min = time.Minutes,
        .tm_hour = time.Hours,
        .tm_mday = date.Date,
        .tm_mon = date.Month,
        .tm_year = date.Year+100,
    };
    time_t result = mktime(&datetime);

    if ( t )
    {
        *t = result;
    }

    return result;
}

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if(tp) {
        tp->tv_sec = time(NULL);
        tp->tv_nsec = 0;
    }
    return 0;
}

void _close() {}
void _fstat() {}
void _getpid() {}
void _gettimeofday() {}
void _isatty() {}
void _kill() {}
void _lseek() {}
void _read() {}