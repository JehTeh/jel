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

/** jel Library Headers */
#include "os/api_allocator.hpp"
#include "os/api_exceptions.hpp"
#include "os/internal/indef.hpp"


namespace jel
{
namespace os
{

static SystemAllocator defaultAllocator;

SystemAllocator* const systemAllocator = &defaultAllocator;

SystemAllocator* SystemAllocator::singletonPtr = nullptr;

SystemAllocator::SystemAllocator() : AllocatorStatisticsInterface("SYSTEM")
{
  if(singletonPtr != nullptr)
  {
    throw Exception{ExceptionCode::allocatorConstructionFailed, 
      "The system allocator is already instantiated."};
  }
  singletonPtr = this;
}

SystemAllocator::~SystemAllocator() noexcept
{
  singletonPtr = nullptr;
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

