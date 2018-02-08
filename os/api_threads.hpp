/** @file os/api_threads.hpp
 *  @brief Interface for all RTOS backed threading primtives.
 *
 *  @detail
 *    RTOS multithreading functionality is wrapped and implemented here. Two thread objects are
 *    available: One is a wrapper around the FreeRTOS task object and requires a call to malloc/new,
 *    and another wraps the FreeRTOS static task object. Note that in the case of the static task
 *    type, it must never be deleted to avoid issues with the idle task and memory ownership. This
 *    does not make it suitable for RAII.
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

/** C/C++ Standard Library Headers */

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_locks.hpp"

namespace jel
{
namespace os
{

class Thread;

class GenericThread_Base
{
protected:
  using ThreadControlStructureMemory = uint8_t[1176];
  GenericThread_Base();
  void startThread(Thread* threadObject, const char* threadName, 
    const uint8_t priority, const size_t stackSize, uint8_t* cbMemory, uint8_t* stackMemory);
  static void dispatcher(Thread* thread);
private:
  ThreadControlStructureMemory* cbMemory_;
  uint8_t* stackMemory_;
};
 

class Thread : private GenericThread_Base
{
public:
  using FunctionSignature = void (*)(void*);
  /** @enum Priority
   *  @brief Possible priorities for thread execution. 
   * */
  enum class Priority : uint8_t
  {
    /** The maximum supported thread priority. Should be used only for critical routines with very
     * low latency allowances. */
    maximum = 8,
    /** High priority threads should be scheduled here. Generally, this is appropriate for driver
     * TX/RX handlers and similar. */
    high = 6,
    /** All regular runtime threads default to this priority. This is the recommended priority for
     * system tasks. */
    normal = 4,
    /** Low priority operations without a meaningful realtime requirement should be scheduled at
     * this level. This includes items such as human interface processing or logging thread output.
     * */
    low = 2,
    /** This is the priority of the system idle thread. No other thread should share this priority.
     * */
    minimum = 0,
  };
  /** @enum ExceptionHandlerPolicy 
   *  @brief This policy is used to determine what action is taken if an uncaught exception is
   *  thrown in a user thread.
   *  */
  enum class ExceptionHandlerPolicy
  {
    /** The thread will stop executing and enter into an infinite sleep loop, waking up once every
     * second. The exception object will be printed (if possible) and preserved and can be easily
     * inspected via a debugger. */
    haltThread,
    /** std::terminate is called. This typically results in a full system shutdown. No attempt will
     * be made to signal that an uncaught exception occurred. */
    terminate
  };
  Thread(FunctionSignature userFunction, void* args, const char* name, const size_t stackSize = 256,
    const Priority priority = Priority::normal, 
    const ExceptionHandlerPolicy ehPolicy = ExceptionHandlerPolicy::haltThread) : 
    GenericThread_Base(), priority_(priority), ehPolicy_(ehPolicy),
    userFunc_(userFunction), userArgPtr_(args), name_(name), maxStack_(stackSize)
    {
      startThread(this, name_, static_cast<uint8_t>(priority_), maxStack_, 
        nullptr, nullptr);
    }
protected:
  friend GenericThread_Base;
  using Handle = void*;
  Priority priority_;
  ExceptionHandlerPolicy ehPolicy_;
  Handle handle_;
  void(*userFunc_)(void*);
  void* userArgPtr_;
  const char* name_;
  size_t maxStack_;
};

class ThisThread
{
public:
  static void sleepfor(const Duration& time) noexcept;
  static void yield() noexcept;
  static void deleteSelf() noexcept;
};

} /** namespace os */
} /** namespace jel */