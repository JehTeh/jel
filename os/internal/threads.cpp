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
    //Create with minimum priority. This allows us to insert thread into the ireg_ before it
    //executes.
    if(xTaskCreate(reinterpret_cast<void(*)(void*)>(&dispatcher), 
         inf->name_, inf->maxStack_bytes_ / 4 + (inf->maxStack_bytes_ % 4), inf, 
         static_cast<uint32_t>(Thread::Priority::minimum),
         &inf->handle_) != pdPASS)
    {
      throw Exception{ExceptionCode::threadConstructionFailed, 
        "Failed to allocate the required memory when constructing a new Thread."};
    }
#ifdef ENABLE_THREAD_STATISTICS
    Thread::ireg_->insert({inf->handle_, inf});
#endif
    vTaskPrioritySet(inf->handle_, static_cast<uint32_t>(inf->priority_));
  }
}

void GenericThread_Base::dispatcher(void* threadInf)
{
  Thread::ThreadInfo* inf = reinterpret_cast<Thread::ThreadInfo*>(threadInf);
  inf->handle_ = xTaskGetCurrentTaskHandle();
#ifdef ENABLE_THREAD_STATISTICS
  inf->totalRuntime_ = Duration::zero();
  inf->lastEntry_ = SteadyClock::now();
#endif
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
#ifdef ENABLE_THREAD_STATISTICS
    Thread::ireg_->erase(inf->handle_);
#endif
    delete inf;
  }
}

#ifdef ENABLE_THREAD_STATISTICS
std::unique_ptr<Thread::InfoRegistry> Thread::ireg_;
#endif

Thread::Thread(FunctionSignature userFunction, void* args, const char* name, 
  const size_t stackSize_bytes, const Priority priority, const ExceptionHandlerPolicy ehPolicy)
{
  //Allocate and configure inf_ structure. This needs to be seperate from the thread object so that
  //if the thread is detached the inf_ still exists.
  inf_ = std::make_unique<ThreadInfo>(); 
  inf_->userFunc_ = userFunction; inf_->userArgPtr_ = args; inf_->name_ = name;
  inf_->maxStack_bytes_ = stackSize_bytes; inf_->priority_ = priority; inf_->ehPolicy_ = ehPolicy;
  inf_->isDetached_ = false; inf_->isDeleted_ = false; 
  inf_->cbMem_ = nullptr; inf_->stackMem_ = nullptr;
#ifdef ENABLE_THREAD_STATISTICS
  inf_->totalRuntime_ = Duration::zero();
  inf_->lastEntry_ = Timestamp::min();
  if(Thread::ireg_ == nullptr)
  {
    Thread::ireg_ = std::make_unique<Thread::InfoRegistry>();
  }
#endif
  startThread(this);
}

Thread::~Thread() noexcept
{
  if(inf_ != nullptr)
  {
#ifdef ENABLE_THREAD_STATISTICS
    ireg_->erase(inf_->handle_);
#endif
    if(inf_->handle_ != nullptr)
    {
      vTaskDelete(inf_->handle_);
    }
    inf_.reset();
  }
}

#ifdef ENABLE_THREAD_STATISTICS
void Thread::schedulerEntry(Handle handle)
{
  //(*ireg_)[handle]->lastEntry_ = SteadyClock::now();
  try
  {
    ireg_->at(handle)->lastEntry_ = SteadyClock::now();
  }
  catch(const std::out_of_range& e)
  {
    //Nothing is purposely done here.
    asm("nop");
  }
}

void Thread::schedulerExit(Handle handle)
{
  //ThreadInfo* i = (*ireg_)[handle];
  try
  {
    ThreadInfo* i = ireg_->at(handle);
    i->totalRuntime_ += SteadyClock::now() - i->lastEntry_;
  }
  catch(const std::out_of_range& e)
  {
    //Nothing is purposely done here.
    asm("nop");
  }
}

void Thread::schedulerThreadCreation(Handle)
{
}

void Thread::schedulerAddIdleTask(Handle h, ThreadInfo* inf)
{
  if(ireg_ == nullptr)
  {
    ireg_ = std::make_unique<InfoRegistry>();
  }
  ireg_->insert({h, inf});
}
#endif

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

void ThisThread::deleteSelf(bool performCompleteErasure) noexcept
{
  if(performCompleteErasure)
  {
    Thread::ThreadInfo* iptr = Thread::ireg_->at(xTaskGetCurrentTaskHandle());
    delete(iptr);
  }
  else
  {
    Thread::ireg_->at(xTaskGetCurrentTaskHandle())->isDeleted_ = true;
    Thread::ireg_->at(xTaskGetCurrentTaskHandle())->minStackBeforeDeletion_bytes_ = 
      uxTaskGetStackHighWaterMark(nullptr) * 4;
  }
  vTaskDelete(nullptr);
}

} /** namespace os */
} /** namespace jel */
