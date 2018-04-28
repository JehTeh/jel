/** @file os/internal/log.cpp
 *  @brief Implementation of system logging utility
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

//Prevent the cross compiler/linter from complaining.
#define _CRT_SECURE_NO_WARNINGS
/** C/C++ Standard Library Headers */
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cstdarg>
#include <cstring>
/** jel Library Headers */
#include "os/api_log.hpp"
#include "os/internal/indef.hpp"

namespace jel
{

FlushLineTag flush;

Logger::Logger(const std::shared_ptr<MtWriter>& output, Config cfg, 
  std::shared_ptr<ObjectPool<String>> pool) : pp_(output), cfg_(cfg)
{
  pool_ = pool;
  if(!pool_)
  {
    pool_ = jelStringPool;
  }
  mq_ = std::make_unique<MsgQueue>(cfg_.maxPrintQueueLength);
  if(cfg_.useAsyncPrintThread)
  {
    tptr_ = std::make_unique<Thread>(reinterpret_cast<void(*)(void*)>(printerThread), this, 
      cfg_.name, config::jelRuntimeConfiguration.loggerThreadStackSize_Bytes, 
      Thread::Priority::low);
  }
}

template<typename PrintType>
StreamLoggerHelper Logger::toStreamHelper(PrintType value)
{
  StreamLoggerHelper slh(*this, *pool_);
  return std::move(slh << value);
}

StreamLoggerHelper Logger::operator<<(MessageType type)
{
  return toStreamHelper(type);
}

StreamLoggerHelper Logger::operator<<(char c)
{
  return toStreamHelper(c);
}

StreamLoggerHelper Logger::operator<<(const char* cString)
{
  return toStreamHelper(cString);
}

StreamLoggerHelper Logger::operator<<(int16_t int16)
{
  return toStreamHelper(int16);
}

StreamLoggerHelper Logger::operator<<(int int32)
{
  return toStreamHelper(int32);
}

StreamLoggerHelper Logger::operator<<(int64_t int64)
{
  return toStreamHelper(int64);
}

StreamLoggerHelper Logger::operator<<(uint8_t uint8)
{
  return toStreamHelper(uint8);
}

StreamLoggerHelper Logger::operator<<(uint16_t uint16)
{
  return toStreamHelper(uint16);
}

StreamLoggerHelper Logger::operator<<(unsigned int uint32)
{
  return toStreamHelper(uint32);
}

StreamLoggerHelper Logger::operator<<(uint64_t uint64)
{
  return toStreamHelper(uint64);
}

StreamLoggerHelper Logger::operator<<(double fpDouble)
{
  return toStreamHelper(fpDouble);
}

StreamLoggerHelper Logger::operator<<(FlushLineTag)
{
  /** The logger instance itself doesn't actually need to do anything except create a new
   * StreamLoggerHelper and return that. */
  StreamLoggerHelper slh(*this, *pool_);
  return slh;
}

StreamLoggerHelper Logger::flush()
{
  /** The logger instance itself doesn't actually need to do anything except create a new
   * StreamLoggerHelper and return that. */
  StreamLoggerHelper slh(*this, *pool_);
  return slh;
}

Status Logger::fastPrint(const MessageType type, const char* cStr) noexcept
{
  if(cfg_.maskLevel >= type)
  {
    //Masked out messages are always considered successfully 'printed'.
    return Status::success;
  }
  return mq_->push({type, cStr}, Duration::zero());
}

Status Logger::print(const MessageType type, const char* format, va_list vargs) 
{
  if(cfg_.maskLevel >= type)
  {
    //Masked out messages are always considered successfully 'printed'.
    return Status::success;
  }
  auto strContainer = pool_->acquire();
  String& string = *strContainer.stored();
  string.assign(" ", string.capacity() - 1);
  int32_t printStat = std::vsnprintf(&string[0], string.length(), format, vargs);
  if(printStat < 0)
  {
    return Status::failure;
  }
  string.resize(printStat);
  PrintableMessage msg(type, std::move(strContainer));
  return messagePrint(msg);
}

Status Logger::messagePrint(PrintableMessage& msg)
{
  if(cfg_.useAsyncPrintThread)
  {
    return mq_->push(std::move(msg));
  }
  else
  {
    Status stat = internalPrint(msg);
    //Flush the print queue.
    while((stat == Status::success) && (mq_->pop(msg) == Status::success))
    {
      stat = internalPrint(msg);
    }
    return stat;
  }
}

Status Logger::internalPrint(const PrintableMessage& msg)
{
  auto lg = pp_.writerBase().lockOutput();
  if(pp_.currentLength())
  {
    pp_.nextLine();
  }
  if(cfg_.fmt.colorize)
  {
    switch(static_cast<MessageType>(static_cast<uint8_t>(msg.type) & 0x7F))
    {
      case MessageType::debug:
        pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightBlue));
        break;
      case MessageType::info:
        pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::white));
        break;
      case MessageType::warning:
        pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightYellow));
        break;
      case MessageType::error:
        pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightRed));
        break;
      case MessageType::hidden:
        break;
    }
  }
  if(cfg_.fmt.prefixTimestamp)
  {
    constexpr size_t bSize = 32;
    char buf[bSize];
    int64_t tus = msg.timestamp.toDuration().toMicroseconds();
    std::snprintf(buf, bSize, "[%lld.%03lld,%03lld", tus / 1'000'000, (tus / 1'000) % 1'000,
      tus % 1000);
    pp_.print(buf);
    if(cfg_.fmt.prefixType)
    {
      pp_.print("-");
    }
    else
    {
      pp_.print("]");
    }
  }
  if(cfg_.fmt.prefixType)
  {
    if(!cfg_.fmt.prefixTimestamp)
    {
      pp_.print("[");
    }
    switch(static_cast<MessageType>(static_cast<uint8_t>(msg.type) & 0x7F))
    {
      case MessageType::debug:
        pp_.print("DBG]");
        break;
      case MessageType::info:
        pp_.print("INF]");
        break;
      case MessageType::warning:
        pp_.print("WRN]");
        break;
      case MessageType::error:
        pp_.print("ERR]");
        break;
      case MessageType::hidden:
        break;
    }
  }
  if(cfg_.fmt.colorize)
  {
    pp_.print(AnsiFormatter::reset);
  }
  if(cfg_.fmt.prefixThreadName)
  {
    const char* name = Thread::lookupName(msg.threadHandle);
    if(name)
    {
      pp_.print("["); pp_.print(name); pp_.print("]");
    }
  }
  if(cfg_.fmt.prefixLoggerName)
  {
    pp_.print("["); pp_.print(cfg_.name); pp_.print("]");
  }
  pp_.print(" ");
  if(static_cast<uint8_t>(msg.type) & 0x80)
  {
    pp_.print(msg.poolString.stored()->c_str());
  }
  else
  {
    if(msg.cStr)
    {
      pp_.print(msg.cStr);
    }
  }
  if(pp_.currentLength())
  {
    pp_.nextLine();
  }
  return Status::success;
}

void Logger::printerThread(Logger* instance)
{
  instance->printerThreadImpl();
}

void Logger::printerThreadImpl()
{
  PrintableMessage msg(MessageType::hidden, nullptr);
  while(true)
  {
    if(mq_->pop(msg) == Status::success)
    {
      internalPrint(msg);
    }
  }
}

Logger::PrintableMessage::PrintableMessage() :
  timestamp(SteadyClock::now()), threadHandle(ThisThread::handle()), type(MessageType::hidden)
{
  //If type & 0x80 == 0, message is constant type and union contains cStr pointer. Otherwise,
  //message is non-const type and contains a pool object.
  uint8_t temp = static_cast<uint8_t>(type);
  temp &= ~0x80;
  type = static_cast<MessageType>(temp);
}

Logger::PrintableMessage::PrintableMessage(const MessageType type_, const char* constString) :
  timestamp(SteadyClock::now()), threadHandle(ThisThread::handle()), type(type_), cStr(constString)
{
  //If type & 0x80 == 0, message is constant type and union contains cStr pointer. Otherwise,
  //message is non-const type and contains a pool object.
  uint8_t temp = static_cast<uint8_t>(type);
  temp &= ~0x80;
  type = static_cast<MessageType>(temp);
}

Logger::PrintableMessage::PrintableMessage(const MessageType type_, 
  ObjectPool<String>::ObjectContainer&& nonConstString) :
  timestamp(SteadyClock::now()), threadHandle(ThisThread::handle()), type(type_), cStr(nullptr),
  poolString(std::move(nonConstString))
{
  //If type & 0x80 == 0, message is constant type and union contains cStr pointer. Otherwise,
  //message is non-const type and contains a pool object.
  uint8_t temp = static_cast<uint8_t>(type);
  temp |= 0x80;
  type = static_cast<MessageType>(temp);
}

Logger::PrintableMessage& Logger::PrintableMessage::operator=(PrintableMessage&& rhs)
{
  timestamp = rhs.timestamp;
  threadHandle = rhs.threadHandle;
  type = rhs.type;
  if(static_cast<uint8_t>(type) & 0x80)
  {
    poolString = std::move(rhs.poolString);
    rhs.type = MessageType::hidden;
  }
  else
  {
    cStr = rhs.cStr;
  }
  return *this;
}

Logger::PrintableMessage::~PrintableMessage() noexcept
{
}

Logger& Logger::sysLogChannel() 
{
  return *jelLogger;
}

StreamLoggerHelper::StreamLoggerHelper(Logger& logger, ObjectPool<String>& pool) :msgValid_(false),
  msgLen_(0), lgr_(&logger), pool_(&pool)
{
  ObjectPool<String>::ObjectContainer strContainer = pool_->acquire();
  if(strContainer.stored())
  {
    msg_ = Logger::PrintableMessage(lgr_->cfg_.defaultStreamLevel, std::move(strContainer));
    String& str = *msg_.poolString.stored();
    str = "";
    msgValid_ = true;
  }
}

StreamLoggerHelper::~StreamLoggerHelper() noexcept
{
  if(msgLen_ && msgValid_)
  {
    lgr_->messagePrint(msg_);
  }
}

StreamLoggerHelper::StreamLoggerHelper(StreamLoggerHelper&& other) : msgValid_(other.msgValid_),
  msgLen_(other.msgLen_), lgr_(other.lgr_), pool_(other.pool_) 
{
  if(this == &other) { return; }
  other.msgLen_ = 0;
  other.msgValid_ = false;
  msg_ = std::move(other.msg_);
}

StreamLoggerHelper& StreamLoggerHelper::operator=(StreamLoggerHelper&& rhs)
{
  if(&rhs == this) { return *this; }
  msgLen_ = rhs.msgLen_;
  msgValid_ = rhs.msgValid_;
  rhs.msgLen_ = 0;
  rhs.msgValid_ = false;
  lgr_ = rhs.lgr_;
  pool_ = rhs.pool_;
  msg_ = std::move(rhs.msg_);
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(Logger::MessageType type)
{
  uint8_t tempType = static_cast<uint8_t>(type);
  tempType |= 0x80 & static_cast<uint8_t>(msg_.type);
  msg_.type = static_cast<Logger::MessageType>(tempType);
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(char c)
{
  if(msgValid_)
  {
    //Temporary to store char when printed.
    constexpr size_t pBufSize = 8; 
    char pBuf[pBufSize];
    String& str = *msg_.poolString.stored();
    msgLen_ += std::snprintf(pBuf, pBufSize, "%c", c);
    assert(str.capacity());
    if((std::strlen(pBuf) + str.length()) >= (str.capacity() - 1))
    {
      return *this;
    }
    str += pBuf;
    msgLen_ = str.length();
    return *this;
  }
  else
  {
  }
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(const char* cString)
{
  if(msgValid_)
  {
    String& str = *msg_.poolString.stored();
    assert(str.capacity());
    if((std::strlen(cString) + str.length()) >= (str.capacity() - 1))
    {
      return *this;
    }
    str += cString;
    msgLen_ = str.length();
  }
  else
  {
  }
  return *this;
}

template<typename PrintType, size_t pBufSize>
StreamLoggerHelper& StreamLoggerHelper::streamLoggerToString(PrintType value)
{
  if(!msgValid_) { return *this; }

  char pBuf[pBufSize];
  String& str = *msg_.poolString.stored();
  int printLen = auto_snprintf(pBuf, pBufSize, value);
  if((printLen > 0) && (printLen < static_cast<int>(pBufSize)))
  {
    msgLen_ += printLen;
  }
  else
  {
    if(printLen < 0)
    {
      static constexpr char printErrorString[] = "PRINT_ERROR";
      std::strncpy(pBuf, printErrorString, pBufSize);
      msgLen_ += sizeof(printErrorString) / sizeof(char);
    }
    else
    {
      assert(pBufSize > 5);
      //Indicate truncation occured using ...
      msgLen_ += pBufSize - 1;
      pBuf[pBufSize - 1] = '.';
      pBuf[pBufSize - 2] = '.';
      pBuf[pBufSize - 3] = '.';
    }
  }
  assert(str.capacity());
  if((std::strlen(pBuf) + str.length()) >= (str.capacity() - 1))
  {
    return *this;
  }
  str += pBuf;
  return *this;
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, int16_t value)
{
  return std::snprintf(buffer, bufferLen, "%d", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, int value)
{
  return std::snprintf(buffer, bufferLen, "%d", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, int64_t value)
{
  return std::snprintf(buffer, bufferLen, "%lld", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, uint8_t value)
{
  return std::snprintf(buffer, bufferLen, "%u", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, uint16_t value)
{
  return std::snprintf(buffer, bufferLen, "%d", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, unsigned int value)
{
  return std::snprintf(buffer, bufferLen, "%u", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, uint64_t value)
{
  return std::snprintf(buffer, bufferLen, "%llu", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, float value)
{
  return std::snprintf(buffer, bufferLen, "%f", value);
}

template<>
int StreamLoggerHelper::auto_snprintf(char* buffer, size_t bufferLen, double value)
{
  return std::snprintf(buffer, bufferLen, "%f", value);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(int16_t int16)
{
  return (*this).streamLoggerToString<int64_t, 22>(int16);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(int int32)
{
  return (*this).streamLoggerToString<int64_t, 22>(int32);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(int64_t int64)
{
  return (*this).streamLoggerToString<int64_t, 22>(int64);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(uint8_t uint8)
{
  return (*this).streamLoggerToString<int64_t, 22>(uint8);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(uint16_t uint16)
{
  return (*this).streamLoggerToString<uint64_t, 22>(uint16);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(unsigned int uint32)
{
  return (*this).streamLoggerToString<uint64_t, 22>(uint32);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(uint64_t uint64)
{
  return (*this).streamLoggerToString<uint64_t, 22>(uint64);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(float fpFloat)
{
  return (*this).streamLoggerToString<double, 32>(fpFloat);
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(double fpDouble)
{
  return (*this).streamLoggerToString<double, 32>(fpDouble);
}

StreamLoggerHelper& StreamLoggerHelper::flush()
{
  if(msgLen_ && msgValid_)
  {
    lgr_->messagePrint(msg_);
    msgLen_ = false;
  }
  return *this;
}

} /** namespace jel */

