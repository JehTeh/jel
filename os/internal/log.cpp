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

Logger::Logger(const std::shared_ptr<MtWriter>& output, Config cfg) : pp_(output), cfg_(cfg)
{
  mq_ = std::make_unique<MsgQueue>(cfg_.maxPrintQueueLength);
  if(cfg_.useAsyncPrintThread)
  {
    tptr_ = std::make_unique<Thread>(reinterpret_cast<void(*)(void*)>(printerThread), this, 
      cfg_.name, config::jelRuntimeConfiguration.loggerThreadStackSize_Bytes, 
      Thread::Priority::low);
  }
}

Status Logger::fastPrint(const MessageType type, const char* cStr)
{
  return mq_->push({type, cStr}, Duration::zero());
}

Status Logger::print(const MessageType type, const char* format, va_list vargs) 
{
  auto strContainer = jelStringPool->acquire();
  String& string = *strContainer.stored();
  string.assign(" ", string.capacity() - 1);
  int32_t printStat = std::vsnprintf(&string[0], string.length(), format, vargs);
  if(printStat < 0)
  {
    return Status::failure;
  }
  string.resize(printStat);
  PrintableMessage msg(type, std::move(strContainer));
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

} /** namespace jel */

