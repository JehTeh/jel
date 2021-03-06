/** @file os/api_allocator.hpp
 *  @brief Provides the interface components used for all allocators compatabile with jel.
 *
 *  @detail
 *    The allocator API classes provide various dynamic memory related objects. These include:
 *      -A generic heap allocator statistics interface. Heaps that implement this interface are
 *      tracked in the global heap registry, allowing for easy access to detailed information about
 *      the memory consumption of a given heap.
 *      -An interface to the primary system allocator. The system allocator exposes the heap that is
 *      used by the new/delete and malloc/free functions. Generally speaking, allocations should be
 *      made via calls to new/delete, but because these calls are piped through the system
 *      allocator, it is acceptable to use the SystemAllocator::systemAllocator()->allocate or
 *      ->deallocate() calls interchangebly with memory returned by operator new() and malloc().
 *      -An 'object pool' template class, which provides a generic threadsafe container for storing
 *      re-usable objects. This is useful for cases where a shared resource is required between
 *      multiple threads and is commonly grabbed and released. Instead of system allocator calls to
 *      create and destroy new objects every time they are needed, which are considerably more
 *      expensive, a object pool can be used. Objects are retrieved from the pool inside of an
 *      ObjectContainer; the container class provides RAII gaurantees ensuring that objects grabbed
 *      from the pool will be returned when the container is destroyed.
 *      -A block allocator class. This is a specalized allocator (that is *not* inherently
 *      threadsafe) that can be used when multiple allocations of objects in multiples of the block
 *      size are expected. It is generally considerably faster than the system heap implementation
 *      using a linked list; large differences between object size and block size can, however,
 *      waste significant amounts of memory.
 *
 *  @todo
 *    -Implement a specialized heap fragmentation visualizer. This will be primarily a debug tool
 *    used to 'see' heap fragmentation (ideally via the CLI). Multiple tool implementations will
 *    likely be required for different heap types (i.e. muli-linked list vs block allocator) but
 *    should share a common abstracted interface that the visualizer can use to draw the in-use and
 *    free memory, as well as provide statistics on allocated and free block distributions and
 *    sizes.
 *    -Match STL allocator implementation. This will likely be done by expanding upon the
 *    AllocatorInterface class. This needs to be done to allow use of std::string/std::vector and
 *    other containers but with a custom allocator (for example, using external SDRAM memory).
 *    -Override STL/g++ exception allocation scheme. A custom/separate heap with fallback to the
 *    system one is more desirable here; this is to significantly reduce uncertainty in exception
 *    throw() times. Essentially, instead of relying on a seperate shared storage block only for
 *    fallback on OOM exceptions, rely on a threadsafe shared block normally with fallback on the
 *    system heap or system terminate(). 
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
#include <cassert>
/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_queues.hpp"

namespace jel
{

/** @Class AllocatorStatisticsInterface
 *  @brief An optional interface that can be implemented by allocator components and is used for
 *  tracking memory usage at the system level. 
 *  
 *  When implementing the statistics interface, an allocator should insert calls to
 *  recordAllocation/Deallocation at the appropriate points. The get remaining/totalFreeSpace
 *  functions must be implemented manually. 
 *  @note The allocator statistics are primarily designed as a debug tool, used to extend any built
 *  in RTOS functionality. Note that because of this implementations may not always be highly
 *  efficient or even entirely thread-safe. This is deemed acceptable, so long as generally speaking
 *  calls to AllocatorStatisticsInterface functions produce correct results and do not interfere
 *  with system behavior beyond the cost in time required to maintain and update the statistics. 
 *
 *  @todo implement begin()/end() iterators for AllocatorsTableEntry objects.
 *  */
class AllocatorStatisticsInterface
{
public:
  /** @struct AllocatorsTableEntry
   *  @brief Linked list structure that can be iterated through to view all registered system
   *  allocators.
   * */
  struct AllocatorsTableEntry
  {
    /** The next allocator stats entry. A nullptr indicates the end of the list. */
    AllocatorsTableEntry* next;
    /** A pointer to the allocator statistics. */
    AllocatorStatisticsInterface* statsIf;
  };
  /** Allocator names longer than this (including a NULL terminator) will be truncated. */
  static constexpr size_t maxNameLength_chars = 32;
  /** Registers the allocator within the system allocators table. */
  AllocatorStatisticsInterface(const char* allocatorName);
  /** Removes the allocator from the system allocators table. */
  virtual ~AllocatorStatisticsInterface() noexcept;
  /** Returns the free space available within the allocator (in bytes). Note: This may not be the
   * largest contiguous amount of free space, it is simply the total amount of space available. */
  virtual size_t freeSpace_Bytes() const noexcept = 0;
  /** Returns the minimum ever (since system boot time, SteadyClock = 0) free space available within
   * the allocator (in bytes). This is not the minimum ever contiguous free space. */
  virtual size_t minimumFreeSpace_Bytes() const noexcept = 0;
  /** Returns the total size of the allocator (in bytes). If an allocator implementation can change
   * its total available size, this number may change. */
  virtual size_t totalSpace_Bytes() const noexcept = 0;
  /** Returns the number of allocations that have been made by the allocator since system boot.
   * @TODO evaluate whether to move to uint64_t. */
  virtual size_t totalAllocations() const noexcept { return totalAllocations_; }
  /** Returns the number of deallocations that have been made by the allocator since system boot.
   * @TODO evaluate whether to move to uint64_t. */
  virtual size_t totalDeallocations() const noexcept { return totalDeallocations_; }
  /** Returns a null terminated C-string that indicates the name of the allocator. */
  virtual const char* name() const noexcept { return name_; }
  /** Provides a reference to the beginning of the linked list that includes all allocators
   * implementing a statistics interface. The first item in this list is always the
   * systemAllocator(). */
  static const AllocatorsTableEntry* systemAllocator() { return allocatorTableStart_; }
protected:
  void recordAllocation() noexcept { totalAllocations_++; };
  void recordDeallocation() noexcept { totalDeallocations_++; };
private:
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
  /** Acquire some memory. On failure, either a nullptr is returned or an OOM exception may be
   * thrown, depending on the allocator implementation. */
  virtual void* allocate(size_t size) = 0;
  /** De-allocate some memory. ptr Must be a valid, non-null pointer returned by the allocate()
   * function for this same allocator instance. */
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
  /** The system allocator does not support move or copy operations. */
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
  /** The constructSystemAllocator() function should only ever be called by the jel during startup,
   * and never the application. Repeated calls to this function will have no effect. */
  static void constructSystemAllocator() noexcept;
  /** Attempts to allocate memory for an exception to the internal thread-safe exception page. If
   * this fails, it will be allocated on the heap instead. */
  static void* allocateException(size_t size) noexcept;
  static void deallocateException(void* except) noexcept;
private:
  static constexpr size_t exceptionPageSize_bytes = 64;
  static std::atomic<bool> excpLocked_;
  static __attribute__((aligned(4))) uint8_t excpPage_[exceptionPageSize_bytes];
  static SystemAllocator* systemAllocator_;
};

/** @class ObjectPool
 *  @brief A 'pool' of a given object that allows RAII acquisition and release of an object stored
 *  in a container.
 *
 *  The ObjectPool provides a threadsafe manner of managing any generic objects. Objects are stored
 *  internally in the pool in a queue, and when acquired, are stored in a RAII container that will
 *  ensure the object is returned to the pool after it is no longer in use. This can be useful for
 *  items that need to be shared across multiple threads, and when frequent
 *  allocations/de-allocations are not desired. For example, internally all jel strings are stored
 *  in an object pool (technically, std::unique_ptr<String> are stored) allowing shared use by the
 *  CLI and logging threads as needed. This allows the jel to pre-allocate string buffers of an
 *  appropriate size and not require multiple expensive system allocator calls whenever printing to
 *  a logging buffer or using CLI functionality.
 *  
 *  Example of object pool use:
 *  @code
 *    ObjectPool<String, 10> sPool(); //Create a pool of 10 Strings
 *    ... <Do other things>
 *    {
 *      auto sContainer = sPool.acquire(); //Returns a ObjectContainer storing a String.
 *      std::sprintf(sContainer.stored().data(), "test %d",7); //Write a generic test to the string.
 *      std::printf(sContainer.stored().c_str()); //Print the string.
 *    } //The String automatically returns to the pool here when it falls out of scope.
 *  @endcode
 *  */
template<typename ObjectT>
class ObjectPool
{
public:
  using ObjQ = Queue<std::unique_ptr<ObjectT>>;
  /** A container storing objects retrieved from the pool. A valid container always holds a non-null
   * item pointer. */
  class ObjectContainer
  {
  public:
    ObjectContainer() : item_{nullptr}, q_{nullptr} {}
    ~ObjectContainer() noexcept { empty(); }
    ObjectContainer(const ObjectContainer&) = delete;
    ObjectContainer(ObjectContainer&& other) noexcept : item_{std::move(other.item_)}, q_{other.q_}
    { other.q_ = nullptr; }
    ObjectContainer& operator=(const ObjectContainer&) = delete;
    ObjectContainer& operator=(ObjectContainer&& other) noexcept 
    {
      empty();
      item_ = std::move(other.item_); q_ = other.q_; other.q_ = nullptr; 
      return *this;
    }
    ObjectT* stored() { return item_.get(); };
    const ObjectT* stored() const { return item_.get(); };
  private:
    friend ObjectPool;
    ObjectContainer(std::unique_ptr<ObjectT>& obj, ObjQ& q) : item_{std::move(obj)}, q_{&q} {}
    std::unique_ptr<ObjectT> item_;
    ObjQ* q_;
    void empty()
    {
      if(item_) { assert(q_); }
      if(item_ && q_) 
      { 
        q_->push(std::move(item_)); 
      } 
    }
  };
  /** Construct a new ObjectPool. Note that the constructor accepts generic arguments, which are
   * forwarded to each object when it is created and added to the pool. For example, using 
   *  @code
   *    ObjectPool<String, 10> sPool("Test123"); 
   *  @endcode
   *  will create a pool of 10 strings all storing "Test123". Perhaps more usefully, using
   *  @code
   *    ObjectPool<String, 10> sPool(127,' '); 
   *  @endcode
   *  will create a pool of 10 strings all at least 127 characters long, allowing up-front onetime
   *  memory allocation if all future writes to said strings are less than 128 characters. Functions
   *  can also be passed, so using
   *  @code
   *    size_t index = 0;
   *    ObjectPool<String, 10> sPool([&](){ return std::to_string(index++); }); 
   *  @endcode
   *  would, for example, initialize each string with an incrementing index starting at 0.
   * */
  template<typename ...Args>
  ObjectPool(const size_t count, Args&& ...args) : maxItems_(count), pool_(count)
  {
    minItms_ = count;
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
      if(pool_.size() < minItms_) { minItms_ = pool_.size(); }
      return ObjectContainer(objPtr, pool_);
    }
    return ObjectContainer();
  }
  size_t itemsInPool() const { return pool_.size(); }
  size_t minimumItemsInPool() const { return minItms_; };
  size_t maxItemsInPool() const { return maxItems_; }
private:
  const size_t maxItems_;
  size_t minItms_;
  ObjQ pool_;
};

/** @class BlockAllocator
 *  @brief The BlockAllocator is a basic implementation of a fixed size allocator. It is not
 *    inherently threadsafe.
 *
 *  The BlockAllocator is recommended where multiple allocations in multiples of the blockSize_Bytes
 *  need to be made. It is significantly faster than a dual linked-list heap, but does generally
 *  require careful matching of block size and desired allocation size. Any multiple of the block
 *  size is ideal (with some small, 4 byte overhead) for an allocation. 
 *  Note that the BlockAllocator is not immune to fragmentation issues; allocations are placed in
 *  the first found set of contiguous blocks that is large enough.
 * */
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

} /** namespace jel */
