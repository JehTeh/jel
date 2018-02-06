/** @file os/api_queues.hpp
 *  @brief Interface for all asynchronous queue primitives. 
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

#pragma once

/** C/C++ Standard Library Headers */

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"

namespace jel
{
namespace os
{

class GenericCopyQueue_Base
{
protected:
  GenericCopyQueue_Base(const size_t maxLength, const size_t itemSize, uint8_t* memory);
  ~GenericCopyQueue_Base() noexcept;
  Status genericPush(const void* item, const Duration& timeout) noexcept;
  Status genericPushToFront(const void* item, const Duration& timeout) noexcept;
  Status genericPop(void* item, const Duration& timeout) noexcept;
  size_t genericGetSize() const noexcept;
  size_t genericGetFreeSpace() const noexcept;
  void genericErase() noexcept;
private:
  using CbStorage = uint8_t[80];
  using Handle = void*;
  Handle handle_;
  CbStorage cbMemory_ __attribute__((aligned(4)));
  uint8_t* itemStoragePtr_;
};

/** @class Queue
 *  @brief General Queue template for a threadsafe, RTOS backed Queue.
 *
 *
 *  @note: For 'smart' objects that make use of non-trivial move and copy constructors, some
 *  additional overhead is required. Furthermore, any objects that store their address or the
 *  address of their members/have this address stored somewhere will be corrupted by this queue.
 *  This is because a memcpy() is performed after the move constructor call when pushing or before
 *  the move constructor call when popping. Objects that can only be moved or have non-trivial copy
 *  constructors are allowable without issue however (i.e. std::unique_ptr is acceptable). Moving
 *  individual elements of a linked list onto and off of the queue, however, would corrupt the link
 *  ptrs as they refer to addresses of elements that get moved onto the queue, then silently
 *  memcpy()'d in the background. 
 *  @note Eventually this Queue object will be re-rolled such that it is tightly integrated with the
 *  RTOS and supports direct move and copy operations across MPU barriers.
 *  */
template<typename T, size_t maxNumberOfElements, bool isTrivial = std::is_trivially_copyable<T>::value>
class Queue : private GenericCopyQueue_Base
{
public:
  Queue() : GenericCopyQueue_Base(maxNumberOfElements, sizeof(T), itemMemory_) { }
  ~Queue() noexcept;
  template<typename = std::enable_if<isTrivial>>
  Status push(const T& item, const Duration& timeout = Duration::max()) noexcept
  {
    return genericPush(&item, timeout); 
  }
  template<typename = std::enable_if<isTrivial>>
  Status pushToFront(const T& item, const Duration& timeout = Duration::max()) noexcept
  {
    return genericPushToFront(&item, timeout); 
  }
  template<typename = std::enable_if<isTrivial>>
  Status pop(const T& item, const Duration& timeout = Duration::max()) noexcept
  {
    return genericPop(&item, timeout);
  }
  template<typename = std::enable_if<!isTrivial>>
  Status push(T&& item, const Duration& timeout = Duration::max()) 
  {
    uint8_t tempMemory[sizeof(T)] __attribute__((aligned(4)));
    T* tPtr = new (tempMemory) T(); //TODO: Check if default construction can be eliminated in favour of raw ptr.
    *tPtr = std::move(item);
    return genericPush(tPtr, timeout);
  }
  template<typename = std::enable_if<!isTrivial>>
  Status pushToFront(T&& item, const Duration& timeout = Duration::max())
  {
    uint8_t tempMemory[sizeof(T)] __attribute__((aligned(4)));
    T* tPtr = new (tempMemory) T(); //TODO: See push(T&&)
    *tPtr = std::move(item);
    return genericPushToFront(tPtr, timeout);
  }
  template<typename = std::enable_if<!isTrivial>>
  Status pop(T&& item, const Duration& timeout = Duration::max())
  {
    uint8_t tempMemory[sizeof(T)] __attribute__((aligned(4)));
    T* tPtr = new (tempMemory) T(); //TODO: See push(T&&)
    if(genericPop(tPtr, timeout) == Status::success)
    {
      item = std::move(*tPtr);
      return Status::success;
    }
    else
    {
      return Status::failure;
    }
  }
private:
  uint8_t itemMemory_[sizeof(T) * maxNumberOfElements];
};


} /** namespace os */
} /** namespace jel */
