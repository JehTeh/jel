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
/** jel Library Headers */
#include "os/api_io.hpp"
#include "os/api_time.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace os
{

AsyncWriter::AsyncWriter(std::unique_ptr<SerialWriterInterface> writer) :
  stream_(std::move(writer))
{
  
}

Status AsyncWriter::write(const char* cStr, size_t length, const Duration& timeout) 
{
  assert(cStr != nullptr); //Cannot print a nulltpr.
  if(length == 0) { length = std::strlen(cStr); }
  Timestamp start = SteadyClock::now();
  LockGuard lg{lock_, timeout};
  if(lg.isLocked())
  { 
    if(stream_->isBusy(SteadyClock::now() - start) == false)
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

Status AsyncWriter::write(const String& string, const Duration& timeout)
{
  return write(string.c_str(), string.length(), timeout);
}

AsyncLock AsyncWriter::lockStream(const Duration& timeout)
{
  return AsyncLock{lock_, timeout};
}


AsyncWriter& AsyncWriter::operator<<(bool value)
{
  if(value)
  {
    constexpr size_t trueLen = constStringLen("true");
    write("true", trueLen);
  }
  else
  {
    constexpr size_t falseLen = constStringLen("false");
    write("false", falseLen);
  }
}

AsyncWriter& AsyncWriter::operator<<(int64_t value)
{

}

AsyncWriter& AsyncWriter::operator<<(uint64_t value)
{

}

AsyncWriter& AsyncWriter::operator<<(float value)
{

}

AsyncWriter& AsyncWriter::operator<<(double value)
{

}

AsyncWriter& AsyncWriter::operator<<(const String& string)
{

}

AsyncWriter& AsyncWriter::operator<<(const char* cStr)
{

}

AsyncWriter& AsyncWriter::operator<<(void*& ptr)
{

}

} /** namespace os */
} /** namespace jel */

