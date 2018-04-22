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
#include "os/api_allocator.hpp"
#include "os/api_threads.hpp"
#include "os/api_io.hpp"
#include "os/api_time.hpp"
#include "os/api_locks.hpp"
#include "os/api_system.hpp"

namespace jel
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
    bool prefixTimestamp;
    bool prefixThreadName;
    bool prefixLoggerName;
    bool prefixType;
    bool colorize;
    MessageFormatting() : prefixTimestamp(true), prefixThreadName(true), prefixLoggerName(false),
      prefixType(true), colorize(true) {}
  };
  struct Config
  {
    /** Non-zero lengths result in an asynchronous logger. */
    int32_t asyncQueueLength;
    const char* name;
    MessageType defaultMessageType;
    MessageFormatting fmt;
    Config() : asyncQueueLength(10), name("lggr"), defaultMessageType(MessageType::default_) {}
  };
  Logger(const std::shared_ptr<MtWriter>& output, Config = Config());
  Status fprintInfo(const char* cStr)
  {
    return fastPrint(MessageType::info, cStr);
  }
  Status fprintDebug(const char* cStr)
  {
    return fastPrint(MessageType::debug, cStr);
  }
  Status fprintWarning(const char* cStr)
  {
    return fastPrint(MessageType::warning, cStr);
  }
  Status fprintError(const char* cStr)
  {
    return fastPrint(MessageType::error, cStr);
  }
  template<typename ...Args>
  Status printInfo(const char* format, Args&& ...args) 
  {
    if(System::cpuExceptionActive()) { return fprintInfo(format); }
    return print(MessageType::info, format, args...);
  }
  template<typename ...Args>
  Status printDebug(const char* format, Args&& ...args) 
  {
    if(System::cpuExceptionActive()) { return fprintDebug(format); }
    return print(MessageType::debug, format, args...);
  }
  template<typename ...Args>
  Status printWarning(const char* format, Args&& ...args) 
  {
    if(System::cpuExceptionActive()) { return fprintWarning(format); }
    return print(MessageType::warning, format, args...);
  }
  template<typename ...Args>
  Status printError(const char* format, Args&& ...args) 
  {
    if(System::cpuExceptionActive()) { return fprintError(format); }
    return print(MessageType::error, format, args...);
  }
private:
  struct PrintableMessage
  {
    Timestamp timestamp;
    const char* thread;
    bool isConst;
    MessageType type;
    const char* cStr;
    ObjectPool<String>::ObjectContainer poolString;
    PrintableMessage();
    PrintableMessage(const PrintableMessage&) = delete;
    PrintableMessage(PrintableMessage&&) = delete;
    PrintableMessage(const MessageType type, const char* constString);
    PrintableMessage(const MessageType type, ObjectPool<String>::ObjectContainer&& nonConstString);
    ~PrintableMessage() noexcept;
    PrintableMessage& operator=(PrintableMessage&& rhs);
    PrintableMessage& operator=(const PrintableMessage&) = delete;
  };
  using MsgQueue = Queue<PrintableMessage>;
  Status fastPrint(MessageType type, const char* cStr);
  Status __attribute__((format(printf, 3, 4))) print(MessageType type, const char* cStr, ...);
  PrettyPrinter pp_;
  Config cfg_;
  std::unique_ptr<Thread> tptr_;
  std::unique_ptr<MsgQueue> mq_;
  Status internalPrint(const PrintableMessage& msg);
  void printerThreadImpl();
  static void printerThread(Logger* instance);
};

} /** namespace jel */
