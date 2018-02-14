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
#include <bitset>
/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_queues.hpp"

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
  /** Allocator names longer than this (including a NULL terminator) will be truncated. */
  static constexpr size_t maxNameLength_chars = 32;
  /** Registers the allocator within the system allocators table. */
  AllocatorStatisticsInterface(const char* allocatorName);
  /** Removes the allocator from the system allocators table. */
  virtual ~AllocatorStatisticsInterface() noexcept;
  virtual size_t freeSpace_Bytes() const noexcept = 0;
  virtual size_t minimumFreeSpace_Bytes() const noexcept = 0;
  virtual size_t totalSpace_Bytes() const noexcept = 0;
  virtual size_t totalAllocations() const noexcept { return totalAllocations_; }
  virtual size_t totalDeallocations() const noexcept { return totalDeallocations_; }
protected:
  void recordAllocation() noexcept { totalAllocations_++; };
  void recordDeallocation() noexcept { totalDeallocations_++; };
private:
  struct AllocatorsTableEntry
  {
    AllocatorsTableEntry* next;
    AllocatorStatisticsInterface* statsIf;
  };
  std::atomic<size_t> totalAllocations_;
  std::atomic<size_t> totalDeallocations_;
  char name_[maxNameLength_chars];
  AllocatorsTableEntry statsTableEntry_;
  static AllocatorsTableEntry* allocatorTableStart_;
};

/** @class AllocatorInterface
 *  @brief A purely virtual interface class that is used throughout jel for aquiring and releasing
 *  dynamic memory. */
class AllocatorInterface
{
public:
  virtual ~AllocatorInterface() noexcept {}
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
  ~SystemAllocator() {};
  SystemAllocator(const SystemAllocator&) = delete;
  SystemAllocator(SystemAllocator&&) = delete;
  SystemAllocator& operator=(const SystemAllocator&) = delete;
  SystemAllocator& operator=(SystemAllocator&&) = delete;
  void* allocate(size_t size) override final;
  void deallocate(void* ptr) override final;
  size_t freeSpace_Bytes() const noexcept override final;
  size_t minimumFreeSpace_Bytes() const noexcept override final;
  size_t totalSpace_Bytes() const noexcept override final;
  static SystemAllocator* systemAllocator() { return systemAllocator_; }
  /** The constructSystemAllocator function should only ever be called by the jel during startup,
   * and never the application. */
  static void constructSystemAllocator() noexcept;
private:
  static SystemAllocator* systemAllocator_;
};

template<typename ObjectT, size_t count>
class ObjectPool
{
public:
  using ObjQ = Queue<std::unique_ptr<ObjectT>, count>;
  /** A container storing objects retrieved from the pool. A valid container always holds a non-null
   * item pointer. */
  class ObjectContainer
  {
  public:
    ObjectContainer() : item_{nullptr}, q_{nullptr} {}
    ~ObjectContainer() noexcept { if(item_ != nullptr) { q_->push(std::move(item_)); } }
    ObjectContainer(const ObjectContainer&) = delete;
    ObjectContainer(ObjectContainer&& other) noexcept : item_{std::move(other.item_)}, q_{other.q_}
    { other.q_ = nullptr; }
    ObjectContainer& operator=(const ObjectContainer&) = delete;
    ObjectContainer& operator=(ObjectContainer&& other) noexcept 
    {
      item_ = std::move(other.item_); q_ = other.q_; other.q_ = nullptr; 
    }
    ObjectT* stored() { return item_.get(); };
  private:
    friend ObjectPool;
    ObjectContainer(std::unique_ptr<ObjectT>& obj, ObjQ& q) : item_{std::move(obj)}, q_{&q} {}
    std::unique_ptr<ObjectT> item_;
    ObjQ* q_;
  };
  template<typename ...Args>
  ObjectPool(Args&& ...args)
  {
    for(size_t i = 0; i < count; i++)
    {
      std::unique_ptr<ObjectT> newObj = std::make_unique<ObjectT>(std::forward<Args>(args)...);
      pool_.push(std::move(newObj));
    }
  }
  /** Acquire an object from the pool. If no object can be acquired before the timeout occurs, an
   * empty container will be returned. */
  ObjectContainer acquire(const Duration& timeout = Duration::max())
  {
    std::unique_ptr<ObjectT> objPtr;
    if(pool_.pop(objPtr, timeout) == Status::success)
    {
      return ObjectContainer(objPtr, pool_);
    }
    return ObjectContainer();
  }
private:
  ObjQ pool_;
};

template<size_t blockSize_Bytes, size_t totalBlocks>
class BlockAllocator : public AllocatorStatisticsInterface, public AllocatorInterface
{
public:
  BlockAllocator(const char* name = "Pool") : 
    AllocatorStatisticsInterface(name), AllocatorInterface(),
    nblk_{totalBlocks}, blksz_{blockSize_Bytes}, sz_{totalBlocks * blockSize_Bytes},
    fblkcnt_{totalBlocks}, minfblkcnt_{totalBlocks}
  {
    for(size_t i = 0; i < nblk_; i++)
    {
      iuf_[i] = false;
    }
  }
  BlockAllocator(const BlockAllocator&) = delete;
  BlockAllocator(BlockAllocator&&) = delete;
  BlockAllocator& operator=(const BlockAllocator&) = delete;
  BlockAllocator& operator=(BlockAllocator&&) = delete;
  size_t freeSpace_Bytes() const noexcept final override { return fblkcnt_ * blksz_; };
  size_t minimumFreeSpace_Bytes() const noexcept final override { return minfblkcnt_ * blksz_; };
  size_t totalSpace_Bytes() const noexcept final override { return sz_; };
  void* allocate(size_t size_Bytes) final override
  {
    if(size_Bytes == 0) { return nullptr; }
    size_Bytes += sizeof(size_t);
    if(size_Bytes >= sz_) { throw std::bad_alloc(); }
    size_t blksreq = size_Bytes / blksz_;
    if((size_Bytes % blksz_) != 0) { blksreq++; }
    size_t contBlocks = 0;
    size_t* poolMemPtr = nullptr;
    size_t firstFreeIdx;
    for(size_t i = 0; i < nblk_; i++)
    {
      if(!iuf_[i]) //Scan through in-use-flags for free blocks.
      {
        if(++contBlocks == blksreq)
        {
          //A chunk of free blocks long enough is found. Lets get a pointer to the beginning of the
          //memory chunk.
          poolMemPtr = reinterpret_cast<size_t*>
            (reinterpret_cast<size_t>(mem_) + (((i + 1) - contBlocks) * blksz_));
          firstFreeIdx = i; //Don't set block flags here, if an exception is raised due to OOM it 
          //will leave pool in an illegal state.
          break;
        }
      }
      else
      {
        contBlocks = 0; //This block isn't free, so our current free block length is reset.
      }
    }
    if(!poolMemPtr) { throw std::bad_alloc(); } //No string of free blocks long enough found.
    for(size_t j = contBlocks; j > 0;)
    {
      iuf_[(firstFreeIdx + 1) - j--] = true; //Set in-use-flags for each block.
    }
    *poolMemPtr = blksreq; //Store total blocks in this allocation. This is used when deallocating.
    //Update current free/min free block count for statistics interface.
    fblkcnt_ -= blksreq; if(fblkcnt_ < minfblkcnt_) { minfblkcnt_ = fblkcnt_; }
    recordAllocation();
    return poolMemPtr += 1; //Return pointer w/ 4B offset to hide our blksreq value.
  }
  void deallocate(void* itemPtr) final override
  {
    assert(itemPtr >= mem_); assert(itemPtr <= (mem_ + sz_));
    //cast to integer pointer for easy of use.
    size_t* mptr = reinterpret_cast<size_t*>(itemPtr);
    mptr--; //Apply negative offset so we now point at the blksreq saved when this was allocated.
    size_t btof = *mptr;
    size_t iufFirstFlag = 
      (reinterpret_cast<size_t>(mptr) - reinterpret_cast<size_t>(mem_)) / blksz_;
    assert(iufFirstFlag < iuf_.size());
    for(size_t i = iufFirstFlag; i < iufFirstFlag + btof;)
    {
      iuf_[i++] = false;
    }
    fblkcnt_ += btof; 
    recordDeallocation();
  }
private:
  const size_t nblk_; 
  const size_t blksz_; 
  const size_t sz_; 
  size_t fblkcnt_;
  size_t minfblkcnt_;
  std::bitset<totalBlocks> iuf_; 
  uint8_t mem_[blockSize_Bytes * totalBlocks] __attribute__((aligned(4)));
};

} /** namespace os */
} /** namespace jel */
