/** @file os/internal/cli.cpp
 *  @brief Implementation of the jel CLI.
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
#include "os/internal/cli.hpp"
#include "os/api_allocator.hpp"

namespace jel
{
namespace cli 
{

using CliArgumentPool = 
  os::BlockAllocator<sizeof(ArgumentContainer::Argument) + 8, config::cliMaximumArguments>;

CliArgumentPool* argumentPool;

Vtt::Vtt(const std::shared_ptr<os::AsyncIoStream>& ios) :
  ios_(ios), printer_(std::static_pointer_cast<os::MtWriter>(ios)),
  wbWrapper_(os::jelStringPool->acquire()), rxWrapper_(os::jelStringPool->acquire()),
  wb_(*wbWrapper_.stored()), rxs_(*wbWrapper_.stored())
{

}

Status Vtt::write(const char* cStr, size_t length)
{
  return Status::success;
}

size_t Vtt::read(char* buffer, size_t bufferSize, const Duration& timeout)
{
  //Reset current cursor position, selection and insert mode parameters.
  cpos_ = 0; selpos_ = 0; selend_ = 0; imode_ = false;
  wb_ = "";
  while(true)
  {
    
  }
}

} /** namespace cli */
} /** namespace jel */

