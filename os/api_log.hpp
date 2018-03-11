/** @file os/api_log.hpp
 *  @brief Logging primitives for use by the jel and application code.
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
#include <string>
#include <cstring>
#include <cassert>
/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_io.hpp"
#include "os/api_time.hpp"
#include "os/api_locks.hpp"

namespace jel
{
namespace os
{

/** @class Logger
 *  @brief Provides formatted, asynchronous logging style output to application threads.
 *  */
class Logger
{
public:
  enum class MessageType : uint8_t
  {
    hidden = 0,
    debug,
    info,
    warning,
    error,
    default_ = info
  };
  struct MessageFormatting 
  {
    bool disablePrefixes;
  };
  struct Config
  {
    bool useAsyncPrintThread;
    MessageType defaultMessageType;
    Config() : useAsyncPrintThread(true), defaultMessageType(MessageType::default_) {}
  };
  Logger(const std::shared_ptr<MtWriter>& output, Config = Config());
  Status printInfo(const char* cStr);
  Status printDebug(const char* cStr);
  Status printWarning(const char* cStr);
  Status printError(const char* cStr);
  template<typename ...Args>
  Status printInfo(const char* format, Args&& ...args) __attribute__((format(printf, 2, 3)));
  template<typename ...Args>
  Status printDebug(const char* format, Args&& ...args) __attribute__((format(printf, 2, 3)));
  template<typename ...Args>
  Status printWarning(const char* format, Args&& ...args) __attribute__((format(printf, 2, 3)));
  template<typename ...Args>
  Status printError(const char* format, Args&& ...args) __attribute__((format(printf, 2, 3)));
private:
  struct PrintableMessage
  {
    Timestamp timestamp;
    union Msg
    {
      
    };
  };
  Status fastPrint(const char* cStr);
  Status print(const char* cStr, ...);
  PrettyPrinter pp;
};

} /** namespace os */
} /** namespace jel */
