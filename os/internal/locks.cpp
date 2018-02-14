/** @file os/internal/locks.cpp
 *  @brief Implementation of locking primitives
 *
 *  @detail
 *    Locks are all implemented via the Lock base class. This provides the RTOS abstraction
 *    functionality. Note that all locks are designed using the static types supported by FreeRTOS.
 *    This allows for fine control over memory management; locks may be allocated via new() if
 *    desired but may also be stack allocated or a member of other objects. The older style malloc()
 *    calling semaphores, mutexes etc. are not implemented by the locks API and are not intended for
 *    use in the jel. Emulating the older style locks is possible by simply using the RTOS heap.
 *
 *  @code
 *    //Create a new Mutex lock on the standard RTOS heap (i.e. heap used by 'new()'.)
 *    std::unique_ptr<jel::os::Mutex> mtx = std::make_unique<jel::os::Mutex>();
 *  @endcode
 *
 *    Use of 'static' objects allows for simple, thread safe RAII that does not require access to the
 *    global system heap. For example, a stack allocated mutex will not require a free()/delete call
 *    and therefore reduces system blocking time when created or destroyed only to the RTOS
 *    teardown.
 *  @note
 *    A static assertion is used to verify that the memory reserved in the Lock definition is the
 *    same size as the memory required by the RTOS primitive. Changes in the RTOS configuration may
 *    require a change in this object size as well. This method is preferred as a way to isolate the
 *    RTOS implementation from the application, at the cost of some tedium when reconfiguring the
 *    RTOS (and application level testing to ensure stack sizes and object sizes are still
 *    acceptable).
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
#include "os/api_locks.hpp"
#include "os/api_exceptions.hpp"
#include "os/api_system.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

Lock::Lock(const Type type, const size_t maxCount, const size_t initialCount) :
  type_(type)
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
        break;
      case Type::recursiveMutex:
        xSemaphoreGiveRecursive(handle_);
        break;
      default:
        assert(false); //Illegal lock type.
        return;
    }
  }
}

} /** namespace os */
} /** namespace jel */
