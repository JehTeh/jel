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
  if(cfg_.asyncQueueLength)
  {
    mq_ = std::make_unique<MsgQueue>(cfg_.asyncQueueLength);
    tptr_ = std::make_unique<Thread>(reinterpret_cast<void(*)(void*)>(printerThread), this, 
      cfg_.name, 1024, Thread::Priority::low);
  }
}

Status Logger::fastPrint(const MessageType type, const char* cStr)
{
  PrintableMessage msg(type, cStr);
  if(cfg_.asyncQueueLength)
  {
    return mq_->push(std::move(msg), Duration::zero());
  }
  else
  {
    return internalPrint(msg);
  }
}

Status Logger::print(const MessageType type, const char* format, ...)
{
  va_list vargs;
  va_start(vargs, format);
  auto strContainer = jelStringPool->acquire();
  String& string = *strContainer.stored();
  string.assign(" ", string.capacity() - 1);
  int32_t printStat = std::vsnprintf(&string[0], string.length(), format, vargs);
  va_end(vargs);
  if(printStat < 0)
  {
    return Status::failure;
  }
  string.resize(printStat);
  PrintableMessage msg(type, std::move(strContainer));
  if(cfg_.asyncQueueLength)
  {
    return mq_->push(std::move(msg));
  }
  else
  {
    return internalPrint(msg);
  }
}

Status Logger::internalPrint(const PrintableMessage& msg)
{
  auto lg = pp_.writerBase().lockOutput();
  if(pp_.currentLength())
  {
    pp_.nextLine();
  }
  if(cfg_.fmt.prefixType)
  {
    switch(msg.type)
    {
      case MessageType::debug:
        if(cfg_.fmt.colorize)
        {
          pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightBlue));
        }
        pp_.print("[DBG]");
        break;
      case MessageType::info:
        if(cfg_.fmt.colorize)
        {
          pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::white));
        }
        pp_.print("[INF]");
        break;
      case MessageType::warning:
        if(cfg_.fmt.colorize)
        {
          pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightYellow));
        }
        pp_.print("[WRN]");
        break;
      case MessageType::error:
        if(cfg_.fmt.colorize)
        {
          pp_.print(AnsiFormatter::setForegroundColor(AnsiFormatter::Color::brightRed));
        }
        pp_.print("[ERR]");
        break;
      case MessageType::hidden:
        break;
    }
    if(cfg_.fmt.colorize)
    {
      pp_.print(AnsiFormatter::reset);
    }
  }
  if(cfg_.fmt.prefixTimestamp)
  {
    constexpr size_t bSize = 32;
    char buf[bSize];
    int64_t tus = msg.timestamp.toDuration().toMicroseconds();
    std::snprintf(buf, bSize, "[%lld.%lld,%lld]", tus / 1'000'000, (tus / 1'000) % 1'000,
      tus % 1000);
    pp_.print(buf);
  }
  if(cfg_.fmt.prefixThreadName)
  {
    pp_.print("["); pp_.print(msg.thread); pp_.print("]");
  }
  if(cfg_.fmt.prefixLoggerName)
  {
    pp_.print("["); pp_.print(cfg_.name); pp_.print("]");
  }
  if(msg.isConst)
  {
    if(msg.cStr)
    {
      pp_.print(msg.cStr);
    }
  }
  else
  {
    pp_.print(msg.poolString.stored()->c_str());
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
  timestamp(SteadyClock::now()), thread(ThisThread::name()), isConst(true), 
  type(MessageType::hidden), cStr(nullptr), poolString()
{

}

Logger::PrintableMessage::PrintableMessage(const MessageType type_, const char* constString) :
  timestamp(SteadyClock::now()), thread(ThisThread::name()), isConst(true), 
  type(type_), cStr(constString), poolString()
{

}

Logger::PrintableMessage::PrintableMessage(const MessageType type_, 
  ObjectPool<String>::ObjectContainer&& nonConstString) :
  timestamp(SteadyClock::now()), thread(ThisThread::name()), isConst(false), 
  type(type_), cStr(nullptr), poolString(std::move(nonConstString))
{

}

Logger::PrintableMessage& Logger::PrintableMessage::operator=(PrintableMessage&& rhs)
{
  timestamp = rhs.timestamp;
  thread = rhs.thread;
  isConst = rhs.isConst;
  type = rhs.type;
  if(isConst)
  {
    cStr = rhs.cStr;
  }
  else
  {
    poolString = std::move(rhs.poolString);
  }
  return *this;
}

Logger::PrintableMessage::~PrintableMessage() noexcept
{
}

} /** namespace jel */

