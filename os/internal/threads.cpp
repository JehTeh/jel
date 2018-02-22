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

void GenericThread_Base::startThread(Thread* threadObject) 
{
  Thread::ThreadInfo* inf = threadObject->inf_.get();
  inf->handle_ = nullptr;
  if(inf->cbMem_ && inf->stackMem_)
  {
    assert(false); //Static thread construction is currently TODO - not simple with freeRTOS, self
    //deleting tasks run into issues with memory ownership when idle task attempts to delete them.
  }
  else
  {
    if(xTaskCreate(reinterpret_cast<void(*)(void*)>(&dispatcher), 
         inf->name_, inf->maxStack_, inf, static_cast<uint32_t>(inf->priority_),
         &inf->handle_) != pdPASS)
    {
      throw Exception{ExceptionCode::threadConstructionFailed, 
        "Failed to allocate the required memory when constructing a new Thread."};
    }
  }
}

void GenericThread_Base::dispatcher(void* threadInf)
{
  Thread::ThreadInfo* inf = reinterpret_cast<Thread::ThreadInfo*>(threadInf);
  inf->handle_ = xTaskGetCurrentTaskHandle();
  {
    LockGuard lg(ThreadStatistics::mapLock.get());
    void* hdl = inf->handle_;
    ThreadStatistics::map->insert({hdl, {hdl, inf->name_}});
  }
  try
  {
    inf->userFunc_(inf->userArgPtr_);
  }
  catch(const std::exception e)
  {
    switch(inf->ehPolicy_)
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
    switch(inf->ehPolicy_)
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
  if(inf->isDetached_)
  {
    delete inf;
  }
}

Thread::Thread(FunctionSignature userFunction, void* args, const char* name, const size_t stackSize,
  const Priority priority, const ExceptionHandlerPolicy ehPolicy)
{
  //Allocate and configure inf_ structure. This needs to be seperate from the thread object so that
  //if the thread is detached the inf_ still exists.
  inf_ = std::make_unique<ThreadInfo>(); 
  inf_->userFunc_ = userFunction; inf_->userArgPtr_ = args; inf_->name_ = name;
  inf_->maxStack_ = stackSize; inf_->priority_ = priority; inf_->ehPolicy_ = ehPolicy;
  inf_->isDetached_ = false; inf_->cbMem_ = nullptr; inf_->stackMem_ = nullptr;
  startThread(this);
}

Thread::~Thread() noexcept
{
  if(inf_ != nullptr)
  {
    if(inf_->handle_ != nullptr)
    {
      vTaskDelete(inf_->handle_);
    }
    inf_.reset();
  }
}

void Thread::detach()
{
  inf_->isDetached_ = true;
  inf_.release();
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

#ifdef ENABLE_THREAD_STATISTICS
std::unique_ptr<RecursiveMutex> ThreadStatistics::mapLock;
std::unique_ptr<std::unordered_map<void*, ThreadStatistics>> ThreadStatistics::map;
#endif

void ThreadStatistics::initializeThreadStats()
{
#ifdef ENABLE_THREAD_STATISTICS
  mapLock = std::make_unique<RecursiveMutex>();
  map = std::make_unique<std::unordered_map<void*, ThreadStatistics>>();
#endif
}

void ThreadStatistics::threadEntry()
{
#ifdef ENABLE_THREAD_STATISTICS
  void* hdl = xTaskGetCurrentTaskHandle();
  try
  {
    LockGuard lg(mapLock.get());
    map->at(hdl).lastEntry = SteadyClock::now();
  }
  catch(const std::out_of_range& e)
  {
    assert(false);
  }
#endif
}

void ThreadStatistics::threadExit()
{
#ifdef ENABLE_THREAD_STATISTICS
  void* hdl = xTaskGetCurrentTaskHandle();
  try
  {
    LockGuard lg(mapLock.get());
    map->at(hdl).totalRuntime += SteadyClock::now() - map->at(hdl).lastEntry;
  }
  catch(const std::out_of_range& e)
  {
    assert(false);
  }
#endif
}

} /** namespace os */
} /** namespace jel */
