/** @file os/locks.cpp
 *  @brief Implementation of locking primitives
 *
 *  @detail
 *    Locks are all implemented via the Lock base class. This provides the RTOS abstraction
 *    functionality. 
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
#include "os/api_locks.hpp"
#include "os/api_exceptions.hpp"
#include "os/api_system.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

Lock::Lock(const Type type, const size_t maxCount, const size_t initialCount)
{
  //Ensure the memory required by the RTOS matches the size of the memory in the object.
  static_assert(sizeof(StaticMemoryBlock) == sizeof(StaticSemaphore_t), 
    "Lock storage size does not match that required by the RTOS");
  handle_ = nullptr;
  StaticQueue_t* mem = reinterpret_cast<StaticQueue_t*>(&staticMemory_);
  switch(type)
  {
    case Type::semaphore:
      handle_ = xSemaphoreCreateBinaryStatic(mem);
      break;
    case Type::countingSemaphore:
      handle_ = xSemaphoreCreateCountingStatic(maxCount, initialCount, mem);
      break;
    case Type::mutex:
      handle_ = xSemaphoreCreateMutexStatic(mem);
      break;
    case Type::recursiveMutex:
      handle_ = xSemaphoreCreateRecursiveMutexStatic(mem);
      break;
    default:
      assert(false); 
      break;
  }
  if(handle_ == nullptr)
  {
    throw Exception{ExceptionCode::lockConstructionFailed,
      "Failed to create lock."};
  }
  //Release the lock on creation, so it is free to grab. Complies with STL convention.
  switch(type)
  {
    case Type::countingSemaphore:
      break;
    case Type::recursiveMutex:
      xSemaphoreGiveRecursive(handle_);
      break;
    default:
      xSemaphoreGive(handle_);
      break;
  }
}

Lock::~Lock() noexcept
{
  if(handle_ != nullptr)
  {
    vSemaphoreDelete(handle_);
  }
}

Status Lock::lock(const Duration& timeout) noexcept
{
  assert(handle_);
  if(System::inIsr())
  {
    switch(type_)
    {
      case Type::semaphore:
      case Type::countingSemaphore:
        {
          auto wakeTask = pdFALSE;
          if(xSemaphoreTakeFromISR(handle_, &wakeTask) == pdTRUE)
          {
            portYIELD_FROM_ISR(wakeTask);
            return Status::success;
          }
          return Status::failure;
        }
      case Type::mutex:
      case Type::recursiveMutex:
      default:
        assert(false); //Cannot operate on a mutex in an ISR.
        break;
    }
  }
  else
  {
    switch(type_)
    {
      case Type::semaphore:
      case Type::countingSemaphore:
      case Type::mutex:
        {
          if(xSemaphoreTake(handle_, toTicks(timeout)) == pdTRUE)
          {
            return Status::success;
          }
          return Status::failure;
        }
      case Type::recursiveMutex:
        {
          if(xSemaphoreTakeRecursive(handle_, toTicks(timeout)) == pdTRUE)
          {
            return Status::success;
          }
          return Status::failure;
        }
      default:
        assert(false); //Illegal semaphore type.
        break;
    }
  }
}

void Lock::unlock() noexcept
{
  assert(handle_);
  if(System::inIsr())
  {
    switch(type_)
    {
      case Type::semaphore:
      case Type::countingSemaphore:
        {
          auto wakeTask = pdFALSE;
          if(xSemaphoreGiveFromISR(handle_, &wakeTask) == pdTRUE)
          {
            portYIELD_FROM_ISR(wakeTask);
            return;
          }
          return;
        }
      case Type::mutex:
      case Type::recursiveMutex:
      default:
        assert(false); //Illegal to operate on a mutex in an ISR.
        return;
    }
  }
  else
  {
    switch(type_)
    {
      case Type::semaphore:
      case Type::mutex:
      case Type::countingSemaphore:
        xSemaphoreGive(handle_);
      case Type::recursiveMutex:
        xSemaphoreGiveRecursive(handle_);
      default:
        assert(false); //Illegal lock type.
        return;
    }
  }
}

} /** namespace os */
} /** namespace jel */
