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
 *  @todo
 *    -Perform more testing and possibly remove/refactor std::unordered_map use with regards to
 *    thread statistics tracking. In the majority of cases it is unlikely overhead (both in memory
 *    and lookup time) to a naive approach is worth it (i.e. < threads).
 *    -Test full thread destruction path. Unknown if current implementation leaks memory on thread
 *    destruction.
 *    -Test and optimize disabling thread statistics for release builds.
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
#include <memory>
#include <unordered_map>
/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_locks.hpp"

#define ENABLE_THREAD_STATISTICS

namespace jel
{
namespace os
{

class Thread;

/** @class ThisThread
 *  @brief 'Static Class' that provides actions that affect only the calling thread.
 *  */
class ThisThread
{
public:
  /** Requests the thread be placed into sleep mode for the given duration.
   * @note The duration is only as fine grained as the RTOS tick scheduler (typically 1-10ms). Sleep
   * calls for shorter periods than this may not have any effect. */
  static void sleepfor(const Duration& time) noexcept;
  /** Requests the current thread yeild() to any equal or higher priority threads. Depending on the
   * rest of the system, this may or may not have any effect. */
  static void yield() noexcept;
  /** Causes the thread to be deleted.
   *  If performCompleteErasure is set then this will also result in a removal of the associated
   *  ThreadInfo object. In systems with thread statistics enabled, this will completely remove all
   *  trace of the thread as if it never existed. It is recommended this only be done for temporary
   *  worker threads that will be constantly destroyed and re-created (if it was not done,
   *  eventually an OOM error would occur due to all the ThreadInfo objects).
   * */
  static void deleteSelf(bool performCompleteErasure = false) noexcept;
};

/** @class GenericThread_Base
 *  @brief Base class for threading primitives. Not for application use. */
class GenericThread_Base
{
protected:
  using ThreadControlStructureMemory = uint8_t[1176];
  GenericThread_Base();
  void startThread(Thread* threadObject);
  static void dispatcher(void* threadInf);
};

/** @class Thread
 *  @brief Thread objects provide separate execution threads that are scheduled by the RTOS.
 *  
 *  Threads require, at minimum, a function pointer and a null terminated C-string 'name' to be
 *  instantiated. Once created, a new task/thread will be created in the RTOS and the function
 *  pointer (plus additional thread information, such as any arguments) will be passed to a generic
 *  thread wrapper function. The wrapper will perform any extraneous setup then call the user
 *  provided function with the supplied argument pointer.
 *  By using the wrapper function, a Thread object can catch generic exceptions that the user
 *  implementation fails to catch and act accordingly. It also allows for some setup of various
 *  supporting utilities, such as thread statistics.
 *
 *  @note Application functionality can be implemented using static Thread objects. These objects
 *  will be constructed on startup after STL and jel initialization, and as such can make full use
 *  of jel::os resources. It is guaranteed that no thread with a priority less than maximum will
 *  begin execution before all static C++ constructor calls are completed, therefore it is perfectly
 *  acceptable to synchronize application startup using static jel::os primitives.
 * */
class Thread : private GenericThread_Base
{
public:
  /** User provided functions must follow this format (i.e. 'void foo(void* pointer) { }') */
  using FunctionSignature = void (*)(void*);
  using Handle = void*;
  /** @enum Priority
   *  @brief Possible priorities for thread execution. 
   * */
  enum class Priority : uint32_t
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
  /** @struct ThreadInfo
   *  @brief Contains detailed information about a threads state.
   *  
   *  ThreadInfo objects are accessible for each thread if thread statistics are compiled into the
   *  binary. They provide information about a given thread, including its name, handle, whether or
   *  not it has been deleted (With the exception of threads that are completely erased), its total
   *  runtime, the last time it was scheduled, etc.
   *  @note In the case of thread runtime stats, these numbers are intended for debugging purposes
   *  only. As such they are not necessarily thread safe and may be updated any time, resulting in
   *  erroneous values. If absolutely accurate information is required, the scheduler should be
   *  locked while operating with thread statistics. Furthermore, threads being deleted that include
   *  a ThreadInfo object may erase an in-use ThreadInfo object. Lock the scheduler when
   *  reading/searching for a ThreadInfo object to protect against this, in systems where threads
   *  may be deleted.
   *  */
  struct ThreadInfo
  {
    Priority priority_;
    ExceptionHandlerPolicy ehPolicy_;
    Handle handle_;
    void(*userFunc_)(void*);
    void* userArgPtr_;
    const char* name_;
    size_t maxStack_bytes_;
    std::unique_ptr<ThreadControlStructureMemory> cbMem_;
    std::unique_ptr<uint8_t[]> stackMem_;
    bool isDetached_;
    /** If true, the thread has been deleted and will no longer be scheduled. */
    bool isDeleted_;
    /** If the thread was deleted, this can be used to gain an idea of how much free stack was left
     * at the highest stack usage point during the threads lifetime. */
    size_t minStackBeforeDeletion_bytes_;
#ifdef ENABLE_THREAD_STATISTICS
    /** The total time the thread has spent scheduled. Note: Any interrupts occurring while a thread
     * is running will count against this time. */
    Duration totalRuntime_;
    /** The last time this thread was scheduled in. */
    Timestamp lastEntry_;
#endif
  };
#ifdef ENABLE_THREAD_STATISTICS
  using InfoRegistry = std::unordered_map<Handle, ThreadInfo*>;
#endif
  /** Construct a new thread. If no arguments are desired in the userFunction, simply pass a nullptr
   * args value. */
  Thread(FunctionSignature userFunction, void* args, const char* name, 
    const size_t stackSize_bytes = 256, const Priority priority = Priority::normal, 
    const ExceptionHandlerPolicy ehPolicy = ExceptionHandlerPolicy::haltThread); 
  ~Thread() noexcept;
  /** Detach the thread from the Thread object. Once this is done the Thread object may be destroyed
   * without affecting the created thread. */
  void detach();
#ifdef ENABLE_THREAD_STATISTICS
  /** Returns a reference to the thread statistics registry, which stores all ThreadInfo structures
   * for use by the application. */
  static const InfoRegistry& registry() { return *ireg_; }
  /** Used to indicate when a thread is scheduled for execution by the kernel. Not for application
   * use. */
  static void schedulerEntry(Handle handle);
  /** Used to indicate when a thread is un-scheduled for execution by the kernel. Not for
   * application use. */
  static void schedulerExit(Handle handle);
  /** Used to indicate when a thread is created by the kernel. Not for application use. */
  static void schedulerThreadCreation(Handle handle);
  /** Used to register the static idle task and associated information. Not for application use. */
  static void schedulerAddIdleTask(Handle handle, ThreadInfo* info);
#endif
protected:
  friend ThisThread;
  friend GenericThread_Base;
  std::unique_ptr<ThreadInfo> inf_;
#ifdef ENABLE_THREAD_STATISTICS
  static std::unique_ptr<InfoRegistry> ireg_;
#endif
};


} /** namespace os */
} /** namespace jel */
