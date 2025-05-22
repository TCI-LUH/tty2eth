#pragma once

// #include <cmsis_os2.h>


#define osLOCK(mutex) for(int __state = (osMutexAcquire(mutex, 0), 1); __state > 0; osMutexRelease(mutex), __state = 0)