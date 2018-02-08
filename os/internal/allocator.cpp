/** @file os/internal/allocator.cpp
 *  @brief System and allocator implementation.
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
#include <cstring>
#include <cassert>
/** jel Library Headers */
#include "os/api_allocator.hpp"
#include "os/api_exceptions.hpp"
#include "os/api_system.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

AllocatorStatisticsInterface::AllocatorsTableEntry*
  AllocatorStatisticsInterface::allocatorTableStart_ = nullptr; 

AllocatorStatisticsInterface::AllocatorStatisticsInterface(const char* allocatorName)
{
  //Add this allocator to the system allocators table. The table is a unidirectional linked list
  //structure, where the first entry is always the static systemTableStatsEntry_, which represents
  //the system allocator as it is always the first allocator. Each successive allocator instantiated
  //will follow the linked list to the end and insert itself and its corresponding table entry
  //there.
  statsTableEntry_.statsIf = this;
  statsTableEntry_.next = nullptr;
  if(allocatorTableStart_ == nullptr)
  {
    allocatorTableStart_ = &statsTableEntry_;
  }
  else
  {
    AllocatorsTableEntry* tablePtr = allocatorTableStart_;
    {
      SchedulerLock schLock; //Lock out other threads from potentially creating an allocator and 
      //editing this list while we walk it and update it.
      while(tablePtr->next != nullptr)
      {
        tablePtr = tablePtr->next;
      }
      tablePtr->next = &statsTableEntry_;
    }
  }
  std::strncpy(name_, allocatorName, maxNameLength_chars - 1);
  name_[maxNameLength_chars - 1] = '\0';
  totalAllocations_ = 0;
  totalDeallocations_ = 0;
}

AllocatorStatisticsInterface::~AllocatorStatisticsInterface() noexcept
{
  //When an allocator is deleted, it needs to be removed from the allocators listing table as well. 
  AllocatorsTableEntry* lastTable = nullptr;
  AllocatorsTableEntry* thisTable = allocatorTableStart_;
  {
    SchedulerLock schLock;
    while(thisTable->statsIf != this)
    {
      lastTable = thisTable;
      thisTable = thisTable->next;
    }
    AllocatorsTableEntry* nextTable = thisTable->next;
    lastTable->next = nextTable;
  }
}

/** @class System Allocator
 *
 * The memory used to store the global system allocator singleton. Because the system allocator
 * provides the new/malloc redirection implementation, it must be allocated through placement new
 * directly into a predefined storage region. By the time C++ constructors (or any allocation
 * functions) are called it must already be operational.
 *
 * @note
 *  The jel startup routine must manually construct a SystemAllocator object at the appropriate
 *  point in boot cycle.
 *
 * */
static uint8_t systemAllocatorStorage[sizeof(SystemAllocator)] __attribute__((aligned(4)));

SystemAllocator* SystemAllocator::systemAllocator_ = nullptr;

SystemAllocator::SystemAllocator() : AllocatorStatisticsInterface("SYSTEM")
{
  if(systemAllocator_ != nullptr)
  {
    throw Exception{ExceptionCode::allocatorConstructionFailed, 
      "The system allocator is already instantiated."};
  }
  systemAllocator_ = this;
}

void SystemAllocator::constructSystemAllocator() noexcept
{
  if(systemAllocator_ == nullptr)
  {
    //Do not assign the systemAllocator_ pointer here. That is done in the SystemAllocator
    //constructor, which is automatically called by placement new.
    new (systemAllocatorStorage) SystemAllocator;
  }
}

void* SystemAllocator::allocate(size_t size)
{
  void* ptr = pvPortMalloc(size);
  if(ptr == nullptr)
  {
    throw std::bad_alloc();
  }
  recordAllocation();
  return ptr;
}

void SystemAllocator::deallocate(void* ptr) 
{
  vPortFree(ptr);
  recordDeallocation();
}

size_t SystemAllocator::freeSpace_Bytes() const noexcept
{
  size_t fs = xPortGetFreeHeapSize();
  return fs;
}

size_t SystemAllocator::minimumFreeSpace_Bytes() const noexcept
{
  size_t mfs = xPortGetMinimumEverFreeHeapSize();
  return mfs;
}

size_t SystemAllocator::totalSpace_Bytes() const noexcept
{
  return configTOTAL_HEAP_SIZE;
}

} /** namespace os */
} /** namespace jel */

