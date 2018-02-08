/** @file os/internal/threads.cpp
 *  @brief Implementation of the threading wrappers for jel.
 *
 *  @detail
 *    The Thread wrappers used by the jel perform two functions: Abstracting the C API of the RTOS
 *    and providing some additional setup and teardown capability, such as exception capture. This
 *    is done by wrapping the user supplied function and void* arg pointer in a custom dispatcher
 *    function that accepts a pointer to the Thread object itself, where the user supplied function
 *    and arg pointers are stored. The call to this function is then wrapped by the dispatcher with
 *    all needed functionality, including global try/catch guards and any instrumentation that is
 *    enabled.
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
#include <cassert>

/** jel Library Headers */
#include "os/api_threads.hpp"
#include "os/api_exceptions.hpp"
#include "os/api_system.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

GenericThread_Base::GenericThread_Base()
{
  /** Nothing is done here. All initialization is performed in the startThread() function to ensure
   * the Thread* object has completed construction. */
}

void GenericThread_Base::startThread(Thread* threadObject, const char* threadName, 
  const uint8_t priority, const size_t stackSize, uint8_t* cbMemory, uint8_t* stackMemory) 
{
  cbMemory_ = reinterpret_cast<ThreadControlStructureMemory*>(cbMemory);
  stackMemory_ = stackMemory;
  Thread::Handle thHandle = nullptr;
  if(cbMemory_ && stackMemory_)
  {
    assert(false); //Static thread construction is currently TODO 
  }
  else
  {
    if(xTaskCreate(reinterpret_cast<void(*)(void*)>(&dispatcher), threadName, stackSize, 
         threadObject, priority, &thHandle) != pdPASS)
    {
      throw Exception{ExceptionCode::threadConstructionFailed, 
        "Failed to allocate the required memory when constructing a new Thread."};
    }
  }
}

void GenericThread_Base::dispatcher(Thread* thread)
{
  thread->handle_ = xTaskGetCurrentTaskHandle();
  try
  {
    thread->userFunc_(thread->userArgPtr_);
  }
  catch(const std::exception e)
  {
    switch(thread->ehPolicy_)
    {
      case Thread::ExceptionHandlerPolicy::haltThread:
        {
          while(true)
          {
            ThisThread::sleepfor(Duration::seconds(1));
          }
        }
        break;
      case Thread::ExceptionHandlerPolicy::terminate:
        std::terminate();
    }
  }
  catch(...)
  {
    switch(thread->ehPolicy_)
    {
      case Thread::ExceptionHandlerPolicy::haltThread:
        {
          while(true)
          {
            ThisThread::sleepfor(Duration::seconds(1));
          }
        }
        break;
      case Thread::ExceptionHandlerPolicy::terminate:
        std::terminate();
    }
  }
}

void ThisThread::sleepfor(const Duration& time) noexcept
{
  vTaskDelay(toTicks(time));
}

void ThisThread::yield() noexcept
{
  taskYIELD();
}

void ThisThread::deleteSelf() noexcept
{
  vTaskDelete(nullptr);
}

} /** namespace os */
} /** namespace jel */
