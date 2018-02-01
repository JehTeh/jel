/** @file os/api_locks.hpp
 *  @brief Interface for all asynchronous locking primitives.
 *
 *  @detail
 *    Multiple locking primitives are provided. These are typically directly backed by the
 *    underlying RTOS primitives. The following primitives are available:
 *      -A base implementation class. This should not be instantiated in the application directly.
 *      -A generic lock class. This can be instantiated, but is typically not preferred.
 *      -Specalized lock classes, including:
 *        -A basic semaphore. This is ISR safe.
 *        -A counting semaphore. This is also ISR safe.
 *        -A mutex. This is not ISR safe.
 *        -A fully recursive mutex. This is not ISR safe.
 *      -A generic RAII guard class. Supports all locking primitives and provides RAII capture and
 *      release capabilities.
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

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"

namespace jel
{
namespace os
{

class Lock_Base
{
public:
/** The supported different types of locks. */
  enum class Type
  {
    /** Signaling flag type lock appropriate for thread synchronization and usage in ISRs. */
    semaphore,
    /** Counting version of a regular semaphore appropriate for tracking available resource usage. 
     * Appropriate for usage in ISRs. */
    countingSemaphore,
    /** Mutual exclusion primitive appropriate for protecting data. NOTE: Not appropriate for use in
     * ISRs. */
    mutex,
    /** Recursive style mutex appropriate for ensuring a single thread can repeatedly 'acquire' a 
     * given resource. NOTE: Not usable in ISRs. */
    recursiveMutex,
  };
  Lock_Base() { } /**< The Lock_Base parent constructor performs no actions. The lock must 
                             be created via the Lock classes. */
  ~Lock_Base() noexcept; /**< Parent destructor will destroy the lock via the OS when the child lock
                           type is destroyed. */
  Lock_Base(Lock_Base& other) = delete; /**< Locks cannot be copy constructed. */
  Lock_Base(Lock_Base&& other) noexcept; /**< Locks may be move constructed. */
  Lock_Base& operator=(Lock_Base& rhs) = delete; /**< Locks cannot be copy assigned. */
  Lock_Base& operator=(Lock_Base&& rhs) noexcept; /**< Locks may be moved. */
  /** The calling thread will attempt to acquire the lock within the specified timeout. This is the 
   * same as attempting to decrement the count for a counting semaphore, which will only succeed if 
   * the count is greater than zero. By default, the timeout is specified to be the maximum possible
   * in the system. */
  Status lock(const Duration& timeout = Duration::max());
  /** The calling thread will release the lock if it currently holds it. This is the same as 
   * increasing the count for a counting semaphore. If the lock is
  * currently free or at its maximum count, this function will have no effect. */
  Status unlock();
  /** Returns true if two locks are identical. */
  bool operator==(const Lock_Base& other)
  {
    if(this->handle == other.handle)
    {
      return true;
    }
    return false;
  }
  /** Returns true if two locks are not identical. */
  bool operator!=(const Lock_Base& other)
  {
    if(*this == other)
    {
      return false;
    }
    return true;
  }
protected:
  using Handle = void*; 
  Handle handle; 
  Type type;
  Status initializeLock() noexcept;
};

/** @class Lock
 *  @brief Provides a thread-safe locking primitive.
 *
 *  Lock implements a generic lock object. Typically it is recommended to use one of the specialized
 *  child variants instead, such as Mutex or Semaphore. 
 *  @note Lock objects do not support copy construction, but may be moved.
 *  */
class Lock : public Lock_Base
{
public:
  Lock(const Type type) : Lock(type, 1, 1) {}
  Lock(const Type, const size_t maxSemaphoreCount, const size_t initialSemaphoreCount); 
  Lock(Lock& other) = delete;
  Lock(Lock&& other) noexcept : Lock_Base(std::move(other)) { }
  Lock& operator=(Lock& rhs) = delete; 
  Lock& operator=(Lock&& rhs) noexcept 
    { Lock::operator=(std::move(rhs)); return *this; } 
private:
  static constexpr size_t LockMemorySize_Bytes = 80;
  using Storage = uint8_t[LockMemorySize_Bytes]; 
  Storage lockData __attribute__((aligned(4))); 
};

class Semaphore : public Lock
{
public:
  Semaphore() : Lock(Type::semaphore) { } 
  Semaphore(Semaphore& other) = delete; 
  Semaphore(Semaphore&& other) noexcept : Lock(std::move(other)) { } 
  Semaphore& operator=(Semaphore& rhs) = delete; 
  Semaphore& operator=(Semaphore&& rhs) noexcept 
    { Lock::operator=(std::move(rhs)); return *this; } 
};

class CountingSemaphore : public Lock
{
public:
  CountingSemaphore(const size_t maxSemaphoreCount, const size_t initialSemaphoreCount) : 
    Lock(Type::countingSemaphore, maxSemaphoreCount, initialSemaphoreCount) { }
  CountingSemaphore(CountingSemaphore& other) = delete; 
  CountingSemaphore(CountingSemaphore&& other) noexcept : Lock(std::move(other)) { } 
  CountingSemaphore& operator=(CountingSemaphore& rhs) = delete; 
  CountingSemaphore& operator=(CountingSemaphore&& rhs) noexcept 
    { Lock::operator=(std::move(rhs)); return *this; } 
  size_t getCount() noexcept;
};

class Mutex : public Lock
{
public:
  Mutex() : Lock(Type::mutex) { }
  Mutex(Mutex& other) = delete; 
  Mutex(Mutex&& other) noexcept : Lock(std::move(other)) { } 
  Mutex& operator=(Mutex& rhs) = delete; 
  Mutex& operator=(Mutex&& rhs) noexcept { Lock::operator=(std::move(rhs)); return *this; } 
  bool isLocked() noexcept;
protected:
  Mutex(const Type recursiveMutex) noexcept : Lock(Type::recursiveMutex) 
    { (void)recursiveMutex; };
};

class RecursiveMutex : public Mutex
{
public:
  RecursiveMutex() : Mutex(Type::recursiveMutex) { } 
  RecursiveMutex(RecursiveMutex& other) = delete; 
  RecursiveMutex(RecursiveMutex&& other) noexcept : Mutex(std::move(other)) { } 
  RecursiveMutex& operator=(RecursiveMutex& rhs) = delete; 
  RecursiveMutex& operator=(RecursiveMutex&& rhs) noexcept 
    { Mutex::operator=(std::move(rhs)); return *this; } 
};

class LockGuard
{
public:
  LockGuard(Lock_Base* lockPtr, const Duration& timeout = Duration::max()) noexcept;
  LockGuard(Lock_Base& lockRef, const Duration& timeout = Duration::max()) noexcept : 
    LockGuard(&lockRef, timeout) {}
  ~LockGuard() noexcept; 
  LockGuard(const LockGuard& other) = delete;  
  LockGuard(LockGuard&& other) 
    { 
      this->lock = other.lock; this->locked = other.locked; 
      other.lock = nullptr; other.locked = false; 
    }  
  LockGuard& operator=(const LockGuard& rhs) = delete;  
  LockGuard& operator=(LockGuard&& other) 
  {
    this->lock = other.lock; this->locked = other.locked;
    other.lock = nullptr; other.locked = false;
    return *this;
  }
  /** Determines whether the held lock is the same as another lock object. */
  bool operator==(const Lock_Base& comp)
  {
    if(lock == &comp) { return true; } else { return false; }
  }
  /** If the LockGaurd acquired the lock successfully, this will return true. */
  bool isLocked() noexcept; 
  /** If a lock was not acquired on instantiation, calling this will attempt to capture it. Caution:
   * When used with a CountingSemaphore or RecursiveMutex, this can succeed multiple times. The
   * Guard will only perform a single RAII release() operation, however. */
  Status retryLock(const Duration& timeout) noexcept; 
  /** Manually release the underlying lock. */
  Status release() noexcept; 
protected:
  Lock_Base* lock; 
  bool locked;
};

} /** namespace os */
} /** namespace jel */
