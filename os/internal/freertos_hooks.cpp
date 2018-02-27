/** @file os/internal/freertos_hooks.cpp
 *  @brief Various FreeRTOS hook functions are implemented here.
 *
 *  @detail
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

/** C/C++ Standard Library Headers */
#include <cstdint>
/** jel Library Headers */
#include "os/internal/indef.hpp"
#include "os/api_threads.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t **tcbSpace, StackType_t **stackSpace, 
  uint32_t *stackSize);
void jel_threadCreate(volatile void* handle);
void jel_threadEntry(volatile void* handle);
void jel_threadExit(volatile void* handle);
#ifdef __cplusplus
}
#endif

void vApplicationTickHook()
{

}

void vApplicationIdleHook()
{

}

void vApplicationGetIdleTaskMemory(StaticTask_t **tcbSpace, StackType_t **stackSpace, 
  uint32_t *stackSize) 
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
  *tcbSpace = &xIdleTaskTCB;
  *stackSpace  = &uxIdleTaskStack[0];
  *stackSize = configMINIMAL_STACK_SIZE;
#ifdef ENABLE_THREAD_STATISTICS
  using namespace jel::os;
  static Thread::ThreadInfo ti =
  {
    Thread::Priority::minimum,
    Thread::ExceptionHandlerPolicy::terminate,
    &xIdleTaskTCB,
    nullptr, nullptr, 
    "idle",
    configMINIMAL_STACK_SIZE * 4,
    nullptr,
    nullptr,
    true,
    jel::Duration::zero(),
    jel::SteadyClock::zero()
  };
  Thread::schedulerAddIdleTask(&xIdleTaskTCB, &ti);
#endif
}

void jel_threadCreate(volatile void* handle)
{
  jel::os::Thread::schedulerThreadCreation(const_cast<void*>(handle));
}

void jel_threadEntry(volatile void* handle)
{
  jel::os::Thread::schedulerEntry(const_cast<void*>(handle));
}

void jel_threadExit(volatile void* handle)
{
  jel::os::Thread::schedulerExit(const_cast<void*>(handle));
}
