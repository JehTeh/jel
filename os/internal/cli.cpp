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

std::unique_ptr<CliArgumentPool> argumentPool;

int cliCmdHelp(CommandIo& io);
int cliCmdLogin(CommandIo& io);

const CommandEntry cliCommandArray[] =
{
  {
    "help", cliCmdHelp, "%?s%?s",
    "The help command performs multiple functions, depending on the arguments passed. These"
    " include:\n"
    "\t(0 Arguments): Prints the generic CLI user instructions. Also lists all command libraries "
    "currently registered with the CLI. This command is called by using 'cli help'.\n"
    "\t(1 Argument): Prints detailed information about a specific library, including all commands "
    "included in that library. This command is called by using 'cli help [library_name]'.\n"
    "\t(2 Arguments): Prints detailed information about a specific command contained in a specific "
    "library. This command is called by using 'cli help [library_name] [command_name]'.\n"
    "Note: The help command requires at least 2 free strings in the system string pool to function "
    "correctly. Systems that do not support strings in commands cannot make use of the generic "
    "help command.",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "login", cliCmdLogin, "%s%s%?u",
    "The login command is used to elevate the current CLI access level. It requires both a "
    "username and password, which if correct will temporarily elevate the permission level.\n"
    "Usage: 'cli login [username] [password] {custom integer timeout, in seconds}'\n"
    "Note: A custom timeout of zero seconds will never expire and is not recommended. "
    "If the login command is performed again with a new timeout, the latest entered timeout will "
    "take precedence.",
    cli::AccessPermission::unrestricted, nullptr
  },
};

const Library cliCmdLib =
{
  "cli",
  "The CLI command library ('cli') is the default library registered with every CLI instance. "
  "It provides basic utilities such as command lookup and help functionality, security login to "
  "view and access secure commands, and some specialized testing functionality.",
  sizeof(cliCommandArray)/sizeof(CommandEntry),
  cliCommandArray
};

//Temporary used for debug
os::AllocatorStatisticsInterface& cliPoolIf() { return *argumentPool; }

void startSystemCli(std::shared_ptr<os::AsyncIoStream>& io)
{
  new CliInstance(io);
}

Status registerLibrary(const Library& library)
{

}


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

Status Vtt::write(const char* format, va_list args) 
{
  int ret = vsnprintf(wrtbuf_.get(), config::cliMaximumStringLength, format, args);
  printer_.print(wrtbuf_.get());
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
    if((s_[i] == delimiter) || (s_[i] == '"'))
    { 
      if(s_[i] == '"')
      {
        size_t slen = 0;
        s_[i++] = '\0';
        while((s_[i] != '\0') && (i < s_.length()))
        {
          if(s_[i] == '"')
          {
            s_[i++] = '\0';
            if(slen > 0) { tc_++; }
            break;
          }
          else { i++; slen++; }
        }
      }
      else
      {
        s_[i++] = '\0';
      }
    }
    else
    {
      tc_++;
      while((s_[i] != delimiter) && (s_[i] != '"') && (i < s_.length())) { i++; }
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
        std::strncpy(param.formatString, "%lli", Parameter::maxFormatStringLength);
      }
      else if(std::strchr(Symbols::specifiers_unsignedInts, *pos) != nullptr)
      {
        param.type = Argument::Type::uint64_t_;
        std::strncpy(param.formatString, "%llu", Parameter::maxFormatStringLength);
      }
      else if(std::strchr(Symbols::specifiers_float, *pos) != nullptr)
      {
        param.type = Argument::Type::double_;
        std::strncpy(param.formatString, "%llf", Parameter::maxFormatStringLength);
      }
      else if(std::strchr(Symbols::specifiers_strings, *pos) != nullptr)
      {
        param.type = Argument::Type::string_;
        std::strncpy(param.formatString, "%s", Parameter::maxFormatStringLength);
      }
      else
      {
        param.type = Argument::Type::invalid;
        param.formatString[0] = '\0';
      }
      return param;
    }
    //This is not the parameter index we want. Advance to the next delimiter or the end of the
    //string.
    else { cp++; pos++; }
    pos = std::strpbrk(pos, Symbols::delimiters);
  }
  //No parameter found.
  return Parameter{true, Argument::Type::invalid, "" };
}

ArgumentContainer::ArgumentContainer() : argListValid_(false), numOfArgs_(0), 
  firstArg{nullptr, nullptr}
{
}

ArgumentContainer::~ArgumentContainer() noexcept
{
}

ArgumentContainer::Status ArgumentContainer::generateArgumentList(const Tokenizer& tokens,
  const size_t discardThreshold, const char* params)
{
  ParameterString ps(params);
  if(discardThreshold > tokens.count())
  {
    return Status::insufficientArguments;
  }
  if((tokens.count() - discardThreshold) > ps.totalCount())
  {
    return Status::tooManyArguments;
  }
  if((tokens.count() - discardThreshold) < (ps.totalCount() - ps.optionalCount()))
  {
    return Status::insufficientArguments;
  }
  if((tokens.count() - discardThreshold) >= config::cliMaximumArguments)
  {
    return Status::maxGlobalArgsExceeded;
  }
  for(size_t i = 0; i < (tokens.count() - discardThreshold); i++)
  {
    ParameterString::Parameter p = ps[i];
    switch(p.type)
    {
      case Argument::Type::int64_t_:
        {
          int64_t temp;
          if(std::sscanf(tokens[i + discardThreshold], p.formatString, &temp) != 1)
          {
            return Status::argumentTypeMismatch;
          }
          appendListItem(temp);
        }
        break;
      case Argument::Type::uint64_t_:
        {
          uint64_t temp;
          if(std::sscanf(tokens[i + discardThreshold], p.formatString, &temp) != 1)
          {
            return Status::argumentTypeMismatch;
          }
          appendListItem(temp);
        }
        break;
      case Argument::Type::double_:
        {
          double temp;
          if(std::sscanf(tokens[i + discardThreshold], p.formatString, &temp) != 1)
          {
            return Status::argumentTypeMismatch;
          }
          appendListItem(temp);
        }
        break;
      case Argument::Type::string_:
        {
          auto scont = os::jelStringPool->acquire(Duration::zero());
          if(scont.stored() == nullptr)
          {
            //Failed to grab a string from the global pool.
            return Status::noFreeStringsAvailable;
          }
          String& s = *scont.stored();
          s.replace(0, config::stringPoolStringSize - 1, tokens[i + discardThreshold]);
          appendListItem(scont);
        }
        break;
      case Argument::Type::invalid:
      default:
        //
        break;
    }
  }
  argListValid_ = true;
  return Status::success;
}

template<typename T>
ArgumentContainer::ArgListItem& ArgumentContainer::appendListItem(T&& argval)
{
  std::unique_ptr<ArgListItem, void(*)(ArgListItem*)>* iptr = &firstArg;
  while(iptr->get() != nullptr)
  {
    iptr = &iptr->get()->next;
  }
  *iptr = std::unique_ptr<ArgListItem, void(*)(ArgListItem*)>
    (new (argumentPool->allocate(sizeof(ArgListItem))) ArgListItem(argval),
     [](ArgListItem* i){ i->~ArgListItem(); argumentPool->deallocate(i);});
  numOfArgs_++;
  return *iptr->get();
}

const Argument& ArgumentContainer::operator[](size_t index) const
{
  if((numOfArgs_ == 0) || (index >= numOfArgs_))
  {
    throw os::Exception{os::ExceptionCode::cliInvalidArgumentIndex, 
      "Invalid argument index requested (%d/%d).", index, numOfArgs_};
  }
  const Argument* aptr = &firstArg->arg;
  ArgListItem* alptr = firstArg.get();
  for(size_t i = 0; i < index; i++)
  {
    if(alptr->next.get() != nullptr)
    {
      alptr = alptr->next.get();
      aptr = &alptr->arg;
    }
    else
    {
      throw os::Exception{os::ExceptionCode::cliInvalidArgumentIndex, 
        "Corrupt CLI argument container detected."};
    }
  }
  return *aptr;
}

CliInstance::CliInstance(std::shared_ptr<os::AsyncIoStream>& io) 
{
  if(activeCliInstance == nullptr)
  {
    //Initialize the argument memory pool used by the CLI.
    argumentPool = std::make_unique<CliArgumentPool>();
    activeCliInstance = this;
    tptr_ = new os::Thread(reinterpret_cast<os::Thread::FunctionSignature>(&cliThreadDispatcher), 
      &io, "CLI", cliThreadStackSize_Words, cliThreadPriority);
  }
  else
  {

  }
}

void CliInstance::cliThreadDispatcher(std::shared_ptr<os::AsyncIoStream>* io)
{
  activeCliInstance->cliThread(io);
} 

void CliInstance::cliThread(std::shared_ptr<os::AsyncIoStream>* io)
{
  istr_ = std::make_unique<String>();
  istr_->reserve(config::cliMaximumStringLength);
  vtt = std::make_unique<Vtt>(*io);
  //Instantiate a visual text terminal on the I/O interface. This will be used for all I/O performed
  //by the CLI.
  //On startup, the CLI will always default to the minimum permission level.
  while(true)
  {
    //Wait for some argument input.
    while(true)
    {
      vtt->write("CLI awaiting input.\r\n");
      if(vtt->read(istr_) > 0)
      {
        break;
      }
    }
    //Tokenize the input.
    Tokenizer tokens(*istr_);
    //Search for and handle any special commands.
    if(handleSpecialCommands(tokens)) { continue; }
    //Search for the library name. If not found, restart loop and await new input.
    if(!lookupLibrary(tokens[0])) { continue; }
    //Search for the command name. If not found, restart loop and await new input.
    if(!lookupCommand(tokens[1])) { continue; }
    //Parse out arguments from tokens list and if successful execute the command.
    executeCommand(tokens);
  }
}

bool CliInstance::handleSpecialCommands(Tokenizer& tokens)
{
  (void)tokens;
  //TODO placeholder for now.
  return false;
}

bool CliInstance::lookupLibrary(const char* name)
{
  //Perform a lookup in the registered libraries list.
  assert(name);
  LibrariesListItem* lli = &libList_;
  while(lli != nullptr)
  {
    if(std::strcmp(lli->libptr->name, name) == 0) { break; }
    else { lli = lli->next; }
  }
  if(lli == nullptr)
  {
    //TODO err
    return false;
  }
  return true;
}

bool CliInstance::lookupCommand(const char* name)
{
  //Lookup the command name in the active library.
  assert(alptr_); assert(name);
  const CommandEntry* cptr = nullptr;
  for(const auto& cmd : *alptr_)
  {
    if(std::strcmp(cmd.name, name) == 0)
    {
      if(doesAplvlMeetSecRequirment(cmd.securityLevel))
      {
        cptr = &cmd;
      }
      break;
    }
  }
  if(cptr == nullptr)
  {
    //TODO err
    return false;
  }
  return true;
}

int CliInstance::executeCommand(Tokenizer& tokens)
{
  //If we have the command matched, we need to construct a CommandIo object, including an argument
  //container, and execute the command if the IO object is valid.
  assert(acptr_); 
  CommandIo cmdIo(tokens, *acptr_, *vtt);
  if(!cmdIo.ioObjectisValid())
  {
    //TODO err, io object creation failure
    return 1;
  }
  try
  {
    if(acptr_->function(cmdIo) != 0)
    {
      //TODO err, report command status code.
    }
  }
  catch(...)
  {
    //TODO catch specalized exceptions
  }
}

Status CommandIo::print(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vtt.write(format, args);
  va_end(args);
}

int cliCmdHelp(CommandIo& io)
{
  io.print("TODO HELP");
}

int cliCmdLogin(CommandIo& io)
{
  io.print("TODO LOGIN");
}

} /** namespace cli */
} /** namespace jel */

