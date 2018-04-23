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
#include <cstdarg>
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
 *
 *  The logger provides a simplified interface to log application activity. It supports multiple
 *  message types (debug, info, warning and error) in addition to integrating timestamp
 *  functionality (when the statement was pushed, not when it was printed) and source thread
 *  tracking functionality.
 *
 *  Two types of print functionality are available - fast print functions which support only
 *  printing fixed strings and traditional printf style print functions which support the full array
 *  of printf specifiers. Generally speaking, the application layer should make use of the print
 *  functions, except in the following cases:
 *    -It is required to print a log statement from an interrupt.
 *    -Precision timing with no blocking is required in the calling thread.
 *    -A string longer than the configured print buffer length needs to be printed.
 *  In these cases, make use of the fastPrint functions as required.
 *
 *  @note: Benchmarking on the TM4C1294NCPDT (120MHz) resulted in fast print speeds of roughly
 *   ~10us/print statement (GCC, -O2).
 *  */
class Logger
{
public:
  /** The type of logger message. Note that only 127 types are supported - The upper nibble of the
   * MessageType enumeration is reserved for internal implementation features. */
  enum class MessageType : uint8_t
  {
    hidden = 0,
    debug = 16,
    info = 32,
    warning = 48,
    error = 64,
    default_ = info
  };
  /** struct MessageFormatting
   * @brief Controls the output formatting of Logger messages when they are printed. */
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
  /** struct Config
   * @brief The Config structure is passed to the Logger on creation and is used to set various
   * Logger parameters. */
  struct Config
  {
    /** The maximum number of logging messages that can be buffered in the print queue. Note that
     * there is differing behavior depending on the type of operation - fast print functions will
     * simply fail on a full queue whereas regular print functions will block until the queue is
     * sufficiently empty. With an asynchronous thread doing the printing, this can cause
     * significant delays in the calling thread, so it is recommended to keep the queue size
     * relatively large. */
    int32_t maxPrintQueueLength;
    /** If enabled, a low priority thread will be responsible for printing all log messages. */
    bool useAsyncPrintThread;
    /** The name of the logger. */
    const char* name;
    /** The masking level for messages. Messages with an enum index lower than or equal to this type
     * will not be displayed. */
    MessageType maskLevel;
    /** Formatting controls when printing log statements. */
    MessageFormatting fmt;
    Config() : maxPrintQueueLength(10), useAsyncPrintThread(true), name("jel::log"), 
      maskLevel(MessageType::default_) {}
  };
  Logger(const std::shared_ptr<MtWriter>& output, Config = Config());
  /** Fast print functions
   *  Fast print functions (prefixed with 'fp') are useful for precision logging statements or
   *  printing data from an ISR. There are some limitations with their usage, however:
   *    -No argument parsing (printf style) is supported.
   *    -The cStr argument (the actual logging statement) must exist in const memory for the
   *    duration of the logging statements existence in the print buffer, as it is referenced only
   *    by a pointer.
   *    -If the print queue is full, the message is lost.
   *  Practically speaking, these limitations mean that fp statements should only use const-char
   *  strings in constant memory:
   *  @code
   *    jel::log().fpInf("Const string"); //OK
   *    ...
   *    char str[32] = "my temporary string";
   *    jel::log().fpInf(str); //Bad, memory will almost certainly be released before printing
   *    actually occurs.
   *  @endcode
   * */
  Status fprint(const MessageType type, const char* cStr) noexcept { return fastPrint(type, cStr); }
  Status fprintInfo(const char* cStr) noexcept { return fastPrint(MessageType::info, cStr); }
  Status fprintDebug(const char* cStr) noexcept { return fastPrint(MessageType::debug, cStr); }
  Status fprintWarning(const char* cStr) noexcept { return fastPrint(MessageType::warning, cStr); }
  Status fprintError(const char* cStr) noexcept { return fastPrint(MessageType::error, cStr); }
  Status fpInf(const char* cStr) noexcept { return fastPrint(MessageType::info, cStr); }
  Status fpDbg(const char* cStr) noexcept { return fastPrint(MessageType::debug, cStr); }
  Status fpWrn(const char* cStr) noexcept { return fastPrint(MessageType::warning, cStr); }
  Status fpErr(const char* cStr) noexcept { return fastPrint(MessageType::error, cStr); }
  Status fp(const MessageType type, const char* cStr) noexcept { return fastPrint(type, cStr); }
  /** Regular print functions
   * The regular print functions can be used to output generic messages with printf style
   * formatting. Messages including printf style formatters are limited in length to the underlying
   * logger string pool string buffer size.
   * Note that in synchronous print modes any messages in the print queue (such as from previous
   * fast print calls) will be flushed when calling a regular print function. This can significantly
   * increase the call time. Furthermore, in asynchronous printing mode, the calling thread may
   * sleep while waiting for room in the print queue.
   * */
#define LOGGER_PRINT_FOR_TYPE( mType ) \
    if(System::cpuExceptionActive()) { return fp(mType, format); } \
    va_list vargs; \
    va_start(vargs, format); \
    Status stat = print(mType, format, vargs); \
    va_end(vargs); \
    return stat;
  Status __attribute__((format(printf, 2, 3))) printInfo(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::info); }
  Status __attribute__((format(printf, 2, 3))) printDebug(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 2, 3))) printWarning(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 2, 3))) printError(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 3, 4))) 
    print(const MessageType type, const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(type); }
  Status __attribute__((format(printf, 2, 3))) pInf(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 2, 3))) pDbg(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 2, 3))) pWrn(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  Status __attribute__((format(printf, 2, 3))) pErr(const char* format, ...) 
    { LOGGER_PRINT_FOR_TYPE(MessageType::debug); }
  /** Exposes the internal logger configuration settings. Useful for adjusting logger configuration
   * at runtime. */
  Config& config() noexcept { return cfg_; }
  /** Returns a reference to the system logging channel integrated into the jel. Useful on targets
   * where instantiating multiple loggers is too expensive. */
  static Logger& sysLogChannel();
private:
  struct PrintableMessage
  {
    Timestamp timestamp;
    Thread::Handle threadHandle;
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
  Status fastPrint(MessageType type, const char* cStr) noexcept;
  Status  print(MessageType type, const char* format, va_list list);
  PrettyPrinter pp_;
  Config cfg_;
  std::unique_ptr<Thread> tptr_;
  std::unique_ptr<MsgQueue> mq_;
  Status internalPrint(const PrintableMessage& msg);
  void printerThreadImpl();
  static void printerThread(Logger* instance);
};

/** log()
 * @brief Access the jel system logging channel.
 * This function simply wraps the call to jel::Logger::sysLogChannel(). It is provided for
 * convenience when using the system logger in an external application. Usage example:
 * @code
 * jel::log().printDebug("Debug message"); //Easier to type than
 * jel::Logger::sysLogChannel().printDebug("Debug message"); //but functionally equivalent. 
 * @endcode
 * */
inline Logger& log() { return Logger::sysLogChannel(); }

} /** namespace jel */
