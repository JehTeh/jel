/** @file FreeRTOSConfig.h
 *  @brief FreeRTOS configuration paramaters.
 *
 *  @author Jonathan Thomson 
 */
/**
 * MIT License
 * 
 * Copyright 2018, Jonathan Thomson 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#define ENABLE_THREAD_STATISTICS
#ifdef ENABLE_THREAD_STATISTICS
extern void jel_threadExit(volatile void*);
extern void jel_threadEntry(volatile void*);
extern void jel_threadCreate(volatile void*);
#endif

#include <stdint.h>

#ifdef HW_TARGET_TM4C123GH6PM
/** CPU clock speed. */
#define configCPU_CLOCK_HZ                          ((uint32_t)80000000)
/** Heap size in bytes. The Tiva 123 jel implementation only uses a single heap. */
#define configTOTAL_HEAP_SIZE                       ((size_t)(24576))
/** Minimum task stack size, in 32b words. A value of  (1024 bytes) was found sufficient in
 * testing for most tasks, including the idle task. */
#define configMINIMAL_STACK_SIZE                    ((unsigned short)128)
/** Interrupt priority configuration, from TI. NOTE: When enabling hardware interrupts, ensure they
 * are mapped to a priority equal to or below this to avoid corrupting RTOS routines! The only time
 * an interrupt can be mapped to a higher priority is when it explicitly avoids using any and all
 * RTOS related primitives and doesn't affect data in a critical section. */
/* Priority 7, or 0xE0 as only the top three bits are implemented.  This is the lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY             (7 << 5)    
/* Priority 5, or 0xA0 as only the top three bits are implemented. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY        (5 << 5)  

#elif defined(HW_TARGET_STM32F302RCT6)
#define configCPU_CLOCK_HZ                          ((uint32_t)64000000)
#define configTOTAL_HEAP_SIZE                       ((size_t)(24576))
#define configMINIMAL_STACK_SIZE                    ((unsigned short)256)
#define configKERNEL_INTERRUPT_PRIORITY             (7 << 5)    
#define configMAX_SYSCALL_INTERRUPT_PRIORITY        (5 << 5)  
#else
#error "No hardware target has been defined. JEL must be built for an explicit hardware target."
#endif

#ifdef ENABLE_THREAD_STATISTICS
#define traceTASK_CREATE(x) jel_threadCreate(x)
#define traceTASK_SWITCHED_IN() jel_threadEntry(pxCurrentTCB)
#define traceTASK_SWITCHED_OUT() jel_threadExit(pxCurrentTCB)
#endif

#define configTICK_RATE_HZ                          ((portTickType)100)
#define configMAX_TASK_NAME_LEN                     (24)
#define configMAX_PRIORITIES                        (12)
#define configMAX_CO_ROUTINE_PRIORITIES             (2)
#define configSUPPORT_STATIC_ALLOCATION             1
#define configUSE_PREEMPTION                        1
#define configUSE_IDLE_HOOK                         1
#define configUSE_TICK_HOOK                         1
#define configUSE_TRACE_FACILITY                    1
#define configIDLE_SHOULD_YIELD                     1
#define configUSE_MUTEXES                           1
#define configUSE_RECURSIVE_MUTEXES                 1
#define INCLUDE_xSemaphoreGetMutexHolder            1
#define configUSE_COUNTING_SEMAPHORES               1
#define INCLUDE_xTaskGetCurrentTaskHandle           1
#define configUSE_NEWLIB_REENTRANT                  1
#define configUSE_16_BIT_TICKS                      0
#define configUSE_CO_ROUTINES                       0
#define configGENERATE_RUN_TIME_STATS               0
#define configCHECK_FOR_STACK_OVERFLOW              0
#define configUSE_STATS_FORMATTING_FUNCTIONS        0
#define configQUEUE_REGISTRY_SIZE                   0
#define INCLUDE_vTaskPrioritySet                    1
#define INCLUDE_uxTaskPriorityGet                   1
#define INCLUDE_vTaskDelete                         1
#define INCLUDE_vTaskCleanUpResources               0
#define INCLUDE_vTaskSuspend                        1
#define INCLUDE_vTaskDelayUntil                     1
#define INCLUDE_vTaskDelay                          1
#define INCLUDE_uxTaskGetStackHighWaterMark         1
