/** @file os/internal/newlib_port.cpp
 *  @brief All libc/c++ porting functions are supplied here.
 *
 *  @detail
 *    To provide significant additonal functionality, newlib support is built into the jel. The jel
 *    leverages various portions of the C/C++ standard libraries and also provides the low level
 *    porting functionality needed to run the C/C++ STL on various embedded targets.
 *
 *    The following functionality is supported:
 *      -Newlib printf/scanf, both via stdout and stdin. These are redirected to the same interface
 *      the jel uses for the CLI and logging.
 *      -Malloc/New overrides. All allocations are performed via the System Allocator by default,
 *      with the option to specify a custom allocator that supports the jel api_allocator interface.
 *      -Filesystem functionality. On targets with hardware support, such as a microSD card, the
 *      fatfs filesystem is exposed via the standard fopen()/fclose() functions and their C++
 *      equivalents. 
 *      -Other required newlib porting functions (for compilation with -lnosys). 
 *
 *    The jel is built to make minimal use of newlib functions that have a large memory impact. This
 *    means that printf floats are not used, nor is the std::string or std::cout library components
 *    and associated functionality. This functionality may be used by the target if desired, in
 *    which case the linker will include the needed functions in the final binary.
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
#include <cstdint>
#include <cstring>
/** jel Library Headers */
#include "os/api_allocator.hpp"

//For use with clang static analysis tools - these functions do not need a 'dllimport' attribute.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-dllimport"
#endif
//These libc functions require C bindings.
#ifdef __cplusplus
extern "C" {
#endif
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void* calloc(size_t ptr, size_t size);
void free(void* ptr);
void _exit(int);
int _kill(int, int);
int _getpid();
int _stat(const char*, struct stat*);
int _fstat(int, struct stat*);
int _isatty(int);
int _open(const char*, int, int);
int _close(int);
int _link(const char*, const char*);
int _unlink(const char*);
int _lseek(int, int, int);
int _read(int, char*, int);
int _write(int file, char *ptr, int len);
void* _sbrk(int increment);
#ifdef __cplusplus
}
#endif
void* operator new(size_t size)
{
  return jel::os::SystemAllocator::systemAllocator()->allocate(size);
}

void* operator new[](size_t size)
{
  return jel::os::SystemAllocator::systemAllocator()->allocate(size);
}

void operator delete(void* ptr) noexcept
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
}

void operator delete[](void* ptr) noexcept
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
}

void operator delete(void* ptr, size_t) noexcept
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
}

void operator delete[](void* ptr, size_t) noexcept
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
}

void* operator new(size_t size, char const*, int)
{
  return jel::os::SystemAllocator::systemAllocator()->allocate(size);
}

void operator delete(void* ptr, char const*, int ) 
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
}

auto mallocBase = [](size_t size)
{
  void* ptr = jel::os::SystemAllocator::systemAllocator()->allocate(size);
  return ptr;
};

auto reallocBase = [](void* ptr, size_t size)
{
  void* newPtr = malloc(size);
  if(newPtr == nullptr) { return newPtr; }
  std::memcpy(newPtr, ptr, size);
  free(ptr);
  return newPtr;
};

auto callocBase = [](size_t num, size_t size)
{
  void* ptr = malloc(size * num);
  if(ptr == nullptr) { return ptr; }
  std::memset(ptr, 0, size * num);
  return ptr;
};

auto freeBase = [](void* ptr)
{
  jel::os::SystemAllocator::systemAllocator()->deallocate(ptr);
};

void* malloc(size_t size)
{
  return mallocBase(size);
}

void* realloc(void* ptr, size_t size)
{
  return reallocBase(ptr, size);
}

void* calloc(size_t num, size_t size)
{
  return callocBase(num, size);
}

void free(void* ptr)
{
  return freeBase(ptr);
}

void* __wrap_malloc(size_t size)
{
  return mallocBase(size);
}

void* __wrap_realloc(void* ptr, size_t size)
{
  return reallocBase(ptr, size);
}

void* __wrap_calloc(size_t num, size_t size)
{
  return callocBase(num, size);
}

void __wrap_free(void* ptr)
{
  return freeBase(ptr);
}

void* __wrap__malloc_r(void*, size_t size)
{
  return mallocBase(size);
}

void* __wrap__realloc_r(void*, void* ptr, size_t size)
{
  return reallocBase(ptr, size);
}

void* __wrap__calloc_r(void*, size_t num, size_t size)
{
  return callocBase(num, size);
}

void __wrap__free_r(void*, void* ptr)
{
  return freeBase(ptr);
}

void __malloc_lock()
{

}

void __malloc_unlock()
{

}

void _exit(int)
{
  while(true) {};
}

int _kill(int, int)
{
  return -1;
}

int _getpid()
{
  return 1;
}

int _isatty(int)
{
  return 1;
}

int _stat(const char* name, struct stat* st)
{
  (void)name; (void)st;
  return 0;
}

int _fstat(int file, struct stat* st)
{
  (void)file; (void)st;
  return 0;
}

int _open(const char* name, int flags, int mode)
{
  (void)name; (void)flags; (void)mode;
  return 0;
}

int _close(int file)
{
  (void)file;
  return 0;
}

int _link(const char* from, const char* to)
{
  (void)from; (void)to;
  return 0;
}

int _unlink(const char* name)
{
  (void)name;
  return 0;
}

int _lseek(int file, int offset, int smode)
{
  (void)file; (void)offset; (void)smode;
  return 0;
}

int _read(int file, char* buffer, int len)
{
  (void)file; (void)buffer; (void)len;
  return 0;
}

int _write(int file, char *ptr, int len)
{
  (void)file; (void)ptr; (void)len;
  return len;
}

void* _sbrk(int increment)
{
  (void)increment;
  return nullptr;
}


namespace __cxxabiv1
{
std::terminate_handler __terminate_handler = abort;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
