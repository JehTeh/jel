/** @file os/api_allocator.hpp
 *  @brief Provides the interface components used for all allocators compatabile with jel.
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
#include <memory>
#include <atomic>

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"

namespace jel
{

namespace os
{

/** @Class AllocatorStatisticsInterface
 *  @brief An optional interface that can be implemented by allocator components and is used for
 *  tracking memory usage at the system level. 
 *  
 *  When implementing the statistics interface, an allocator should insert calls to
 *  recordAllocation/Deallocation at the appropriate points. The get remaining/totalFreeSpace
 *  functions must be implemented manually. 
 *  */
class AllocatorStatisticsInterface
{
public:
  /** Registers the allocator within the system allocators table. */
  AllocatorStatisticsInterface(const char* allocatorName);
  /** Removes the allocator from the system allocators table. */
  ~AllocatorStatisticsInterface() noexcept;
  virtual size_t freeSpace_Bytes() const noexcept = 0;
  virtual size_t minimumFreeSpace_Bytes() const noexcept = 0;
  virtual size_t totalSpace_Bytes() const noexcept = 0;
  virtual size_t totalAllocations() const noexcept { return totalAllocations_; }
  virtual size_t totalDeallocations() const noexcept { return totalDeallocations_; }
protected:
  void recordAllocation() noexcept { totalAllocations_++; };
  void recordDeallocation() noexcept { totalDeallocations_++; };
private:
  std::atomic<size_t> totalAllocations_;
  std::atomic<size_t> totalDeallocations_;
};

/** @class AllocatorInterface
 *  @brief A purely virtual interface class that is used throughout jel for aquiring and releasing
 *  dynamic memory. */
class AllocatorInterface
{
public:
  virtual void* allocate(size_t size) = 0;
  virtual void deallocate(void* ptr) = 0;
};

/** @SystemAllocator
 *  @brief The SystemAllocator is a singleton class that takes over all normal allocations
 *  (including calls to the default new/delete operators).
 *  */
class SystemAllocator : public AllocatorInterface, public AllocatorStatisticsInterface
{
public:
  SystemAllocator();
  ~SystemAllocator() noexcept;
  SystemAllocator(const SystemAllocator&) = delete;
  SystemAllocator(SystemAllocator&&) = delete;
  SystemAllocator& operator=(const SystemAllocator&) = delete;
  SystemAllocator& operator=(SystemAllocator&&) = delete;
  void* allocate(size_t size) override final;
  void deallocate(void* ptr) override final;
  size_t freeSpace_Bytes() const noexcept override final;
  size_t minimumFreeSpace_Bytes() const noexcept override final;
  size_t totalSpace_Bytes() const noexcept override final;
private:
  static SystemAllocator* singletonPtr;
};

extern SystemAllocator* const systemAllocator;

} /** namespace os */
} /** namespace jel */
