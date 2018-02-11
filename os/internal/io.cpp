/** @file os/internal/io.cpp
 *  @brief Implementation of system I/O handling objects.
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
#include <cstring>
#include <cstdio>
#include <cinttypes>
/** jel Library Headers */
#include "os/api_io.hpp"
#include "os/api_time.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

MtWriter::MtWriter(std::unique_ptr<SerialWriterInterface> writer) :
  stream_(std::move(writer))
{
  
}

Status MtWriter::write(const char* cStr, size_t length, const Duration& timeout) 
{
  assert(cStr != nullptr); //Cannot print a nulltpr.
  if(length == 0) { length = std::strlen(cStr); }
  Timestamp start = SteadyClock::now();
  LockGuard lg{lock_, timeout};
  if(lg.isLocked())
  { 
    if(stream_->isBusy(timeout - (SteadyClock::now() - start)) == false)
    {
      stream_->write(cStr, length);
      return Status::success;
    }
    else
    {
      return Status::failure;
    }
  }
  else
  {
    return Status::failure;
  }
}

Status MtWriter::write(const String& string, const Duration& timeout)
{
  return write(string.c_str(), string.length(), timeout);
}

AsyncLock MtWriter::lockStream(const Duration& timeout)
{
  return AsyncLock{lock_, timeout};
}

MtReader::MtReader(std::unique_ptr<SerialReaderInterface> reader) :
  stream_(std::move(reader))
{

}

size_t MtReader::read(char* buffer, const size_t length, const Duration& timeout)
{
  if(length == 0) { return 0; }
  assert(buffer != nullptr); //Cannot receive data into a nullptr.
  Timestamp start = SteadyClock::now();
  LockGuard lg{lock_, timeout};
  if(lg.isLocked())
  {
    size_t readLen = stream_->read(buffer, length - 1, timeout - (SteadyClock::now() - start));
    buffer[readLen] = '\0';
    return readLen;
  }
  return 0;
}

AsyncLock MtReader::lockStream(const Duration& timeout)
{
  return AsyncLock{lock_, timeout};
}

AsyncIoStream::AsyncIoStream(std::unique_ptr<SerialReaderInterface> reader, 
  std::unique_ptr<SerialWriterInterface> writer) :
  MtReader{std::move(reader)}, 
  MtWriter{std::move(writer)}
{

}

} /** namespace os */
} /** namespace jel */

