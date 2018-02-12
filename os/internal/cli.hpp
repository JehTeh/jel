/** @file os/internal/cli.hpp
 *  @brief Internal definitions used by the jel CLI.
 *
 *  @detail
 *    Due to the complexity of the CLI implementation, prototypes and declarations for CLI objects
 *    are located in this header file.
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
#include <cassert>
/** jel Library Headers */
#include "os/api_cli.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace cli
{

class VttIo 
{
public:
  Status write(const char* cStr, size_t length);
  size_t read(char* buffer, size_t bufferSize, const Duration& timeout); 
  Status prefix(const char* cStr);
  VttIo(const std::shared_ptr<os::AsyncIoStream>& ios);
  ~VttIo();
private:
  std::shared_ptr<os::AsyncIoStream> ios_;
  os::PrettyPrinter printer_;
};

} /** namespace cli */
} /** namespace jel */
