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
#include <cstring>
/** jel Library Headers */
#include "os/internal/cli.hpp"
#include "os/api_allocator.hpp"
#include "hw/api_exceptions.hpp"

namespace jel
{
namespace cli 
{

using CliArgumentPool = 
  os::BlockAllocator<sizeof(Argument) + 8, config::cliMaximumArguments>;

CliArgumentPool* argumentPool;


Vtt::Vtt(const std::shared_ptr<os::AsyncIoStream>& ios) : ios_(ios), printer_(ios),
  wb_(), rxs_(), fmts_(new char[formatScratchBufferSize])
{
  assert(cfg_.maxEntryLength > 80);
  assert(cfg_.receiveBufferLength > 16);
  wb_.reserve(cfg_.maxEntryLength);
  rxs_.reserve(cfg_.receiveBufferLength);
  pfx_ = "";
}


Status Vtt::write(const char* cStr, size_t length)
{
  printer_.print(cStr, length);
  return Status::success;
}

size_t Vtt::read(String& string, const Duration& timeout)
{
  string.assign(" ", string.capacity() - 1);
  size_t sz = read(&string[0], string.length(), timeout);
  string.resize(sz);
  return sz;
}

size_t Vtt::read(char* buffer, size_t bufferSize, const Duration& timeout)
{
  assert(bufferSize);
  wb_ = ""; cpos_ = 0; cshandled_ = false; imode_ = false; smode_ = false; terminated_ = false;
  bufedtd_ = false;
  const Timestamp tStart = SteadyClock::now();
  while((SteadyClock::now() - tStart) < timeout)
  {
    //Read characters into the receive scratch buffer.
    while(loadRxs(timeout - (SteadyClock::now() - tStart)) == 0)
    {
      if((SteadyClock::now() - tStart) >= timeout)
      {
        ios_->write(os::AnsiFormatter::FixedControlSequences::eraseLine);
        buffer[0] = '\0'; return 0;
      }
    }
    //Scan the receive scratch for any special control sequences or characters.
    if(handleControlCharacters()) { cshandled_ = true; } else { cshandled_ = false; }
    regenerateOuput();
    if(terminated_)
    {
      ios_->write("\r\n");
      std::strncpy(buffer, wb_.c_str(), bufferSize - 1);
      buffer[bufferSize - 1] = '\0'; //Ensure buffer is null terminated.
      return wb_.length() > bufferSize ? bufferSize : wb_.length();
    }
  }
  ios_->write(os::AnsiFormatter::FixedControlSequences::eraseLine);
  buffer[0] = '\0'; return 0;
}

size_t Vtt::loadRxs(const Duration& timeout)
{
  const Timestamp tStart = SteadyClock::now();
  rxs_.assign(cfg_.receiveBufferLength, ' ');
  while(true)
  {
    try
    {
      //Use &rxs_[0] here. Could use .data(), but this avoids requiring C++17.
      size_t charsRx = ios_->read(&rxs_[0], rxs_.size(), cfg_.pollingPeriod);
      if(charsRx > 0)
      {
        rxs_.resize(charsRx);
        return rxs_.size();
      }
      if((SteadyClock::now() - tStart) >= timeout)
      {
        return 0;
      }
    }
    catch(const hw::Exception& e)
    {
      //Ignore error with clang, YCM/libclang has issues with completion and inherited from
      //templates.
      #ifndef __clang__
      if(e.error == hw::ExceptionCode::receiveOverrun)
      {
        //If the RX hardware was overrun, we will just retry the receive operation and throw out the
        //buffer.
        continue;
      }
      else
      {
        throw e;
      }
      #endif
    }
  }
}

size_t Vtt::handleControlCharacters()
{
  using fmt = os::AnsiFormatter;
  //Search for any special escape or control characters (i.e. non-visible ASCII)
  for(size_t pos = 0; pos != rxs_.length(); pos++)
  {
    if((rxs_[pos] < ' ') || (rxs_[pos] == 0x7F))
    {
      switch(rxs_[pos])
      {
        case fmt::ControlCharacters::escape:
          if(parseEscapeSequence(pos)) { return 1; }
          break;
        case fmt::ControlCharacters::newline:
        case fmt::ControlCharacters::carriageReturn:
          if(terminateInput(pos)) { return 1; }
          break;
        default:
          if(parseAsciiControl(pos)) { return 1; }
      }
    }
  }
  return 0;
}

bool Vtt::parseEscapeSequence(const size_t sp)
{
  using efmt = os::AnsiFormatter::FixedControlSequences;
  constexpr size_t npos = std::string::npos; 
  if(rxs_.find(efmt::leftArrowKey, sp) != npos)
  {
    if(cpos_ > 0) { cpos_--; }
    smode_ = false;
  }
  else if(rxs_.find(efmt::rightArrowKey, sp) != npos)
  {
    if(cpos_ < wb_.length()) { cpos_++; }
    smode_ = false;
  }
  else if(rxs_.find(efmt::shiftLeftArrowKey, sp) != npos)
  {
    if(!smode_)
    {
      smode_ = true;
      if(cpos_ > 0) { sst_ = cpos_; }
      else { sst_ = 0; }
    }
    if(cpos_ > 0) { cpos_--; }
  }
  else if(rxs_.find(efmt::shiftRightArrowKey, sp) != npos)
  {
    if(!smode_)
    {
      smode_ = true;
      if(cpos_ < wb_.length()) { sst_ = cpos_; }
      else { sst_ = wb_.length() - 1; }
    }
    if(cpos_ < wb_.length()) { cpos_++; }
  }
  else if(rxs_.find(efmt::insertKey, sp) != npos)
  {
    smode_ = false;
    imode_ = !imode_;
  }
  else if(rxs_.find(efmt::homeKey, sp) != npos)
  {
    cpos_ = 0;
  }
  else if(rxs_.find(efmt::deleteKey, sp) != npos)
  {
    //Erase the key under the cursor.
    if(smode_)
    {
      eraseSelection();
    }
    else
    {
      if(cpos_ < wb_.length())
      {
        wb_.erase(cpos_, 1);
      } 
      bufedtd_ = true;
    }
  }
  else if(rxs_.find(efmt::endKey, sp) != npos)
  {
    cpos_ = wb_.length();
  }
  else if(rxs_.find(efmt::pageUpKey, sp) != npos)
  {
    ios_->write(efmt::pageUp);
  }
  else if(rxs_.find(efmt::pageDownKey, sp) != npos)
  {
    ios_->write(efmt::pageDown);
  }
  else if((rxs_.find(efmt::upArrowKey, sp) != npos) ||
    (rxs_.find(efmt::shiftUpArrowKey, sp) != npos))
  {
    if(bufedtd_)
    {
      ++hbuf_ = wb_;
      hbuf_--;
    }
    auto* hbptr = &hbuf_.currentBuffer();
    while((--hbuf_).length() == 0)
    {
      if(hbptr == &(hbuf_.currentBuffer())) { break; }
    };
    wb_ = hbuf_.currentBuffer();
    cpos_ = wb_.length();
    smode_ = false; imode_ = false;
  }
  else if((rxs_.find(efmt::downArrowKey, sp) != npos) ||
    (rxs_.find(efmt::shiftDownArrowKey, sp) != npos))
  {
    if(bufedtd_)
    {
      --hbuf_ = wb_;
      hbuf_++;
    }
    auto* hbptr = &hbuf_.currentBuffer();
    while((++hbuf_).length() == 0)
    {
      if(hbptr == &(hbuf_.currentBuffer())) { break; }
    };
    wb_ = hbuf_.currentBuffer();
    cpos_ = wb_.length();
    smode_ = false; imode_ = false;
  }
  else
  {
    //Some unused control sequence.
  }
  return true;
}

bool Vtt::parseAsciiControl(const size_t sp)
{
  using fmt = os::AnsiFormatter::ControlCharacters;
  if((rxs_[sp] == fmt::backspace) || (rxs_[sp] == fmt::del))
  {
    if(smode_)
    {
      eraseSelection();
      return true;
    }
    if(cpos_ > 0)
    {
      if(wb_.length() > 0)
      {
        //Erase character before the cursor.
        wb_.erase(cpos_ - 1, 1);
        cpos_--;
      }
      bufedtd_ = true;
    }
  }
  return true;
}

bool Vtt::terminateInput(const size_t sp)
{
  using fmt = os::AnsiFormatter::ControlCharacters;
  if((rxs_[sp] == fmt::carriageReturn) || (rxs_[sp] == fmt::newline))
  {
    terminated_ = true;
    hbuf_++ = wb_;
  }
  return true;
}

void Vtt::regenerateOuput()
{
  //If a non-control-sequence is in the rxs_, we need to insert it into the working buffer.
  if(!cshandled_ && !((wb_.length() + rxs_.length()) >= cfg_.maxEntryLength))
  {
    bufedtd_ = true;
    if(imode_) //Insert mode logic is actually inverted, i.e. if insert key was pressed then 
      //overwrite, not insert.
    {
      wb_.replace(cpos_, rxs_.length(), rxs_);
      cpos_ += rxs_.length();
    }
    else if(smode_)
    {
      eraseSelection();
      if(wb_.length() > 0)
      {
        wb_.insert(cpos_, rxs_);
        cpos_ += rxs_.length();
      }
      else
      {
        wb_ = rxs_;
        cpos_ = wb_.length();
      }
    }
    else if(wb_.length() == 0)
    {
      wb_ = rxs_;
      cpos_ = wb_.length();
    }
    else
    {
      wb_.insert(cpos_, rxs_);
      cpos_ += rxs_.length();
    }
  }
  using fmt = os::AnsiFormatter;
  auto lg = ios_->lockOutput();
  ios_->write(fmt::FixedControlSequences::eraseLine); //Erase the line.
  ios_->write(fmt::FixedControlSequences::resetFormatting); //Clear any active formatting.
  assert(pfx_); //Prefix string cannot be null!
  if(pfx_[0] != '\0') { ios_->write(pfx_); } //Print the prefix string.  
  if(imode_)
  {
    std::sprintf(fmts_.get(), "%s100%c", fmt::EscapeSequences::csi, 
      static_cast<char>(fmt::Csi::SGR));
    ios_->write(fmts_.get()); //Highlight line background if in insert mode.
  }
  else
  {
    ios_->write("\e[49m"); //Disable any highlighting outside of insert mode.
  }
  for(size_t i = 0; i < wb_.length(); i++) //Print the buffer.
  {
    if(imode_)
    {
      if(i == cpos_)
      {
        ios_->write(fmt::FixedControlSequences::underlineEnable);
        ios_->write(wb_[i]);
        ios_->write(fmt::FixedControlSequences::underlineDisable);
      }
      else
      {
        ios_->write(wb_[i]);
      }
    }
    else if(smode_)
    {
      if(cpos_ > sst_)
      {
        if(i == sst_)
        {
          ios_->write(fmt::FixedControlSequences::highlightEnable);
          ios_->write(wb_[i]);
        }
        else if(i == cpos_)
        {
          ios_->write(wb_[i]);
          ios_->write(fmt::FixedControlSequences::highlightDisable);
        }
        else
        {
          ios_->write(wb_[i]);
        }
      }
      else if(cpos_ < sst_)
      {
        if(i == cpos_)
        {
          ios_->write(wb_[i]);
          ios_->write(fmt::FixedControlSequences::highlightEnable);
        }
        else if(i == sst_)
        {
          ios_->write(wb_[i]);
          ios_->write(fmt::FixedControlSequences::highlightDisable);
        }
        else
        {
          ios_->write(wb_[i]);
        }
      }
      else
      {
        if(i == sst_)
        {
          ios_->write(fmt::FixedControlSequences::highlightEnable);
          ios_->write(wb_[i]);
          ios_->write(fmt::FixedControlSequences::highlightDisable);
        }
        else
        {
          ios_->write(wb_[i]);
        }
      }
    }
    else
    {
      ios_->write(wb_[i]);
    }
  }
  ios_->write(fmt::FixedControlSequences::resetFormatting); //Clear any active formatting.
  std::sprintf(fmts_.get(), "\e[%dG", cpos_ + std::strlen(pfx_) + 1);
  ios_->write(fmts_.get()); //Set cursor position.
}

void Vtt::eraseSelection()
{
  if(cpos_ > sst_)
  {
    smode_ = false;
    wb_.erase(sst_, cpos_ - sst_ + 1);
    cpos_ = sst_;
  }
  else
  {
    smode_ = false;
    wb_.erase(cpos_, sst_ - cpos_ + 1);
  }
  bufedtd_ = true;
}

Vtt::HistoryBuffer::HistoryBuffer()
{
  for(size_t i = 0; i < config::cliHistoryDepth; i++)
  {
    buffers_[i] = os::jelStringPool->acquire();
    buffers_[i].stored()->reserve(config::cliMaximumStringLength);
    *buffers_[i].stored() = "";
  }
  bpos_ = 0;
}

String& Vtt::HistoryBuffer::currentBuffer()
{
  return *buffers_[bpos_].stored();
}

String& Vtt::HistoryBuffer::operator++()
{
  nextpos();
  return *buffers_[bpos_].stored();
}

String& Vtt::HistoryBuffer::operator--()
{
  prevpos();
  return *buffers_[bpos_].stored();
}

String& Vtt::HistoryBuffer::operator++(int)
{
  String& s = *buffers_[bpos_].stored();
  nextpos();
  return s;
}

String& Vtt::HistoryBuffer::operator--(int)
{
  String& s = *buffers_[bpos_].stored();
  prevpos();
  return s;
}

void Vtt::HistoryBuffer::nextpos()
{
  bpos_++;
  if(bpos_ >= config::cliHistoryDepth)
  {
    bpos_ = 0;
  }
}

void Vtt::HistoryBuffer::prevpos()
{
  bpos_--;
  if(bpos_ >= config::cliHistoryDepth)
  {
    bpos_ = config::cliHistoryDepth - 1;
  }
}

Tokenizer::Tokenizer(String& str, const char delimiter) : tc_(0), s_(str)
{
  if(s_.length() == 0) { return; }
  for(size_t i = 0; i < s_.length();)
  {
    if(s_[i] == delimiter)
    {
      s_[i++] = '\0';
    }
    else
    {
      tc_++;
      while((s_[i] != delimiter) && (i < s_.length())) { i++; }
    }
  }
}

const char* Tokenizer::operator[](size_t index) const
{
  if(index >= tc_) { return nullptr; }
  size_t c = 0;
  for(size_t i = 0; i < s_.length();)
  {
    if(s_[i] != '\0')
    {
      if(c++ == index)
      {
        return &s_[i];
      }
      while((s_[i] != '\0') && (i < s_.length())) { i++; }
    }
    else
    {
      i++;
    }
  }
  return nullptr;
}

ParameterString::ParameterString(const char* pstr) : pcnt_(0), optcnt_(0), s_(pstr)
{
  if(s_ == nullptr) { return; }
  if(s_[0] == '\0') { return; }
  const char* pos = s_;
  while(pos != nullptr)
  {
    //Scan for a delimiter which marks the beginning of a parameter.
    //If we find one, increment parameter count and if it is also optional increment optional count.
    pos = std::strpbrk(pos, Symbols::delimiters);
    if(pos != nullptr) 
    {
      pos++;
      pcnt_++;
      //If the next character after delimiter is an optional specifier, also increment optional
      //count.
      if(std::strchr(Symbols::optionals, *pos) != nullptr)
      {
        optcnt_++;
      }
      if(*pos == '\0') { break; }
    }
  }
}

ParameterString::Parameter ParameterString::operator[](size_t index)
{
  size_t cp = 0;
  const char* pos = s_;
  pos = std::strpbrk(pos, Symbols::delimiters);
  while(pos != nullptr)
  {
    //If a delimiter is found
    if(cp == index)
    {
      //The current parameter matches the requested index. We need to construct a new Parameter
      //object.
      Parameter param;
      pos++;
      //Check if the parameter is optional.
      if(std::strchr(Symbols::optionals, *pos) != nullptr) { param.isOptional = true; pos++; }
      else { param.isOptional = false; }
      //Advance past any 'ignored' characters. These would typically include things like unused
      //scanf modifiers such as 'l' or 'h'. If a match to any of them is found at the current
      //position, just advance to to the next position.
      while(std::strchr(Symbols::ignored, *pos) != nullptr) { pos++; }
      if(*pos == '\0') { break; } //End of parameters list. 
      //Now we have to determine what type of parameter is excepted.
      if(std::strchr(Symbols::specifiers_signedInts, *pos) != nullptr)
      {
        param.type = Argument::Type::int64_t_;
      }
      else if(std::strchr(Symbols::specifiers_unsignedInts, *pos) != nullptr)
      {
        param.type = Argument::Type::uint64_t_;
      }
      else if(std::strchr(Symbols::specifiers_float, *pos) != nullptr)
      {
        param.type = Argument::Type::double_;
      }
      else if(std::strchr(Symbols::specifiers_strings, *pos) != nullptr)
      {
        param.type = Argument::Type::string_;
      }
      else
      {
        param.type = Argument::Type::invalid;
      }
      return param;
    }
    //This is not the parameter index we want. Advance to the next delimiter or the end of the
    //string.
    else { cp++; pos++; }
    pos = std::strpbrk(pos, Symbols::delimiters);
  }
  //No parameter found.
  return Parameter{true, Argument::Type::invalid };
}



ArgumentContainer::ArgumentContainer(const Tokenizer& tokens, const size_t discardThreshold, 
  const char* )
{
  if((tokens.count() - discardThreshold) >= config::cliMaximumArguments)
  {
    throw CliException("", CliException::Id::maxArgumentsExceeded);
  }
  //size_t ppos = 0;
  for(size_t i = discardThreshold; i < tokens.count(); i++)
  {
  }
}

} /** namespace cli */
} /** namespace jel */

