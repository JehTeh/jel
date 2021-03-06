/** @file os/internal/queues.cpp
 *  @brief Implementation of threadsafe queuing primitives
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
#include <cassert>

/** jel Library Headers */
#include "os/api_queues.hpp"
#include "os/api_exceptions.hpp"
#include "os/api_system.hpp"
#include "os/internal/indef.hpp"

namespace jel
{

GenericCopyQueue_Base::GenericCopyQueue_Base(const size_t maxLength, const size_t itemSize, uint8_t* memory) :
  itemStoragePtr_(memory)
{
  static_assert(sizeof(CbStorage) == sizeof(StaticQueue_t), 
    "Static queue storage size must be equal to underlying OS primitive size.");
  handle_ = xQueueCreateStatic(maxLength, itemSize, itemStoragePtr_, reinterpret_cast<StaticQueue_t*>(cbMemory_));
  if(handle_ == nullptr)
  {
    throw Exception{ExceptionCode::queueConstructionFailed,
      "Failed while constructing queue."};
  }
}

GenericCopyQueue_Base::~GenericCopyQueue_Base() noexcept
{
  if(handle_ != nullptr)
  {
    vQueueDelete(handle_);
  }
}

Status GenericCopyQueue_Base::genericPush(const void* item, const Duration& timeout) noexcept
{
  assert(item);  
  if(System::inIsr())  
  {  
    auto wakeHpTask = pdFALSE;  
    if(xQueueSendToBackFromISR(handle_, item, &wakeHpTask) == pdTRUE)  
    {  
      portYIELD_FROM_ISR(wakeHpTask);  
      return Status::success;  
    }  
    return Status::failure;  
  }  
  else  
  {  
    if(xQueueSendToBack(handle_, item, toTicks(timeout)) == pdTRUE)  
    {  
      return Status::success;  
    }  
    else  
    {  
      return Status::failure;  
    }  
  }

}

Status GenericCopyQueue_Base::genericPushToFront(const void* item, const Duration& timeout) noexcept
{
  assert(item);  
  if(System::inIsr())  
  {  
    auto wakeHpTask = pdFALSE;  
    if(xQueueSendToFrontFromISR(handle_, item, &wakeHpTask) == pdTRUE)  
    {  
      portYIELD_FROM_ISR(wakeHpTask);  
      return Status::success;  
    }  
    return Status::failure;  
  }  
  else  
  {  
    if(xQueueSendToFront(handle_, item, toTicks(timeout)) == pdTRUE)  
    {  
      return Status::success;  
    }  
    else  
    {  
      return Status::failure;  
    }  
  }
}

Status GenericCopyQueue_Base::genericPop(void* item, const Duration& timeout) noexcept
{
  assert(item);  
  if(System::inIsr())  
  {  
    auto wakeHpTask = pdFALSE;  
    if(xQueueReceiveFromISR(handle_, item, &wakeHpTask) == pdTRUE)  
    {  
      portYIELD_FROM_ISR(wakeHpTask);  
      return Status::success;  
    }  
    return Status::failure;  
  }  
  else  
  {  
    if(xQueueReceive(handle_, item, toTicks(timeout)) == pdTRUE)  
    {  
      return Status::success;  
    }  
    else  
    {  
      return Status::failure;  
    }  
  }
}

size_t GenericCopyQueue_Base::genericGetSize() const noexcept
{
  if(System::inIsr()) { return uxQueueMessagesWaitingFromISR(handle_); }
  else { return uxQueueMessagesWaiting(handle_); }
}

size_t GenericCopyQueue_Base::genericGetFreeSpace() const noexcept
{
  return uxQueueSpacesAvailable(handle_);
}

void GenericCopyQueue_Base::genericErase() noexcept
{
  xQueueReset(handle_);
}

#ifdef TARGET_SUPPORTS_CPPUTEST
TEST_GROUP(JEL_TestGroup_Queues_Pod)
{
  static constexpr size_t QueueSize_Items = 16;
  struct PodStruct
  {
    int16_t i16;
    int32_t i32;
    uint64_t ui64;
    PodStruct() : i16(0xADAD), i32(0x01234'5678), ui64(0xA5A5'DEAD'BEEF'A5A5) {}
    bool operator==(const PodStruct& rhs) 
      { return((i16 == rhs.i16) && (i32 == rhs.i32) && (ui64 == rhs.ui64)); }
  };
  std::unique_ptr<Queue<PodStruct>> queuePtr;
  void setup()
  {
    queuePtr = std::make_unique<Queue<PodStruct>>(16);
  }
  void teardown()
  {
    queuePtr.reset();
  }
};
TEST(JEL_TestGroup_Queues_Pod, PushPop)
{
  PodStruct s;
  for(size_t i = 0; i < QueueSize_Items; i++)
  {
    CHECK(queuePtr->push(s, jel::Duration::zero()) == Status::success);
    PodStruct defS;
    CHECK(s == defS);
  }
  CHECK(queuePtr->push(s, jel::Duration::zero()) != Status::success);
  CHECK(queuePtr->push(s, jel::Duration::milliseconds(5)) != Status::success);
  for(size_t i = 0; i < QueueSize_Items; i++)
  {
    CHECK(queuePtr->pop(s, jel::Duration::zero()) == Status::success);
  }
  CHECK(queuePtr->pop(s, jel::Duration::zero()) != Status::success);
  CHECK(queuePtr->pop(s, jel::Duration::milliseconds(5)) != Status::success);
}
#endif
} /** namespace jel */
