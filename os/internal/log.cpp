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

/** C/C++ Standard Library Headers */
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cstdarg>
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

StreamLoggerHelper Logger::operator<<(MessageType type)
{
  StreamLoggerHelper slh(*this, *pool_);
  slh = std::move(slh << type);
  return slh;
}

StreamLoggerHelper Logger::operator<<(char c)
{
  StreamLoggerHelper slh(*this, *pool_);
  return std::move(slh << c);
}

StreamLoggerHelper Logger::operator<<(const char* cString)
{
  StreamLoggerHelper slh(*this, *pool_);
  slh = std::move(slh << cString);
  return slh;
}

StreamLoggerHelper Logger::operator<<(int64_t int64)
{
  StreamLoggerHelper slh(*this, *pool_);
  return std::move(slh << int64);
}

StreamLoggerHelper Logger::operator<<(uint64_t uint64)
{
  StreamLoggerHelper slh(*this, *pool_);
  return std::move(slh << uint64);
}

StreamLoggerHelper Logger::operator<<(double fpDouble)
{
  StreamLoggerHelper slh(*this, *pool_);
  return std::move(slh << fpDouble);
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
    str.assign(" ", str.capacity() - 1);
    str.resize(str.capacity() - 1);
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
  uint8_t tempType = static_cast<uint8_t>(msg_.type);
  tempType |= 0x7F & static_cast<uint8_t>(type);
  msg_.type = static_cast<Logger::MessageType>(tempType);
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(char c)
{
  (void)c;
  if(msgLen_)
  {
    
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
    char* cp;
    for(cp = &str[msgLen_]; cp != &str[str.capacity()]; cp++)
    {
      *cp = *cString++;
      msgLen_++;
      if(!*cString)
      {
        str.resize(msgLen_);
        flush();
        return *this;
      }
    }
    str.resize(msgLen_);
  }
  else
  {

  }
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(int64_t int64)
{
  (void)int64;
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(uint64_t uint64)
{
  (void)uint64;
  return *this;
}

StreamLoggerHelper& StreamLoggerHelper::operator<<(double fpDouble)
{
  (void)fpDouble;
  return *this;
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

