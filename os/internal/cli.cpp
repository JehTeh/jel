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
  os::BlockAllocator<sizeof(Argument) + 16, config::cliMaximumArguments>;

std::unique_ptr<CliArgumentPool> argumentPool;

int32_t cliCmdHelp(CommandIo& io);
int32_t cliCmdLogin(CommandIo& io);

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
  return CliInstance::registerLibrary(library);
}

Vtt::Vtt(const std::shared_ptr<os::AsyncIoStream>& ios) : ios_(ios), printer_(ios),
  wb_(), rxs_(), wrtbuf_(new char[cfg_.maxEntryLength]), fmts_(new char[formatScratchBufferSize])
{
  assert(cfg_.maxEntryLength > 80);
  assert(cfg_.receiveBufferLength > 16);
  wb_.reserve(cfg_.maxEntryLength);
  rxs_.reserve(cfg_.receiveBufferLength);
  pfx_ = "";
}

Vtt::~Vtt() noexcept
{

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
  if((ret < static_cast<int>(config::cliMaximumStringLength - 1)) && (ret > 0))
  {
    return Status::success;
  }
  return Status::failure;
}

Status Vtt::colorizedWrite(const os::AnsiFormatter::Color color, const char* format, ...)
{
  auto lg = ios_->lockOutput();
  write(os::AnsiFormatter::setForegroundColor(color));
  va_list args;
  va_start(args, format);
  auto stat = write(format, args);
  va_end(args);
  write(os::AnsiFormatter::setForegroundColor(os::AnsiFormatter::Color::default_));
  return stat;
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
        auto lg = ios_->lockOutput();
        ios_->write(os::AnsiFormatter::Erase::entireLine);
        ios_->write('\r');
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
  auto lg = ios_->lockOutput();
  ios_->write(os::AnsiFormatter::Erase::entireLine);
  ios_->write('\r');
  buffer[0] = '\0'; return 0;
}

Status Vtt::prefix(const char* cStr)
{
  if(cStr != nullptr) { pfx_ = cStr; }
  else { pfx_ = ""; }
  return Status::success;
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
  using efmt = os::AnsiFormatter::Input;
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
    ios_->write(os::AnsiFormatter::Cursor::pageUp);
  }
  else if(rxs_.find(efmt::pageDownKey, sp) != npos)
  {
    ios_->write(os::AnsiFormatter::Cursor::pageDown);
  }
  else if((rxs_.find(efmt::upArrowKey, sp) != npos) ||
    (rxs_.find(efmt::shiftUpArrowKey, sp) != npos))
  {
    if(bufedtd_)
    {
      hbuf_.currentViewPos() = wb_;
    }
    hbuf_.prevViewPos();
    wb_ = hbuf_.currentViewPos();
    cpos_ = wb_.length();
    smode_ = false; imode_ = false;
  }
  else if((rxs_.find(efmt::downArrowKey, sp) != npos) ||
    (rxs_.find(efmt::shiftDownArrowKey, sp) != npos))
  {
    if(bufedtd_)
    {
      hbuf_.currentViewPos() = wb_;
    }
    hbuf_.nextViewPos();
    wb_ = hbuf_.currentViewPos();
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
    hbuf_.currentWritePos() = wb_;
    hbuf_.nextWritePos();
    hbuf_.resetViewPos();
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
  ios_->write(fmt::Erase::entireLine); //Erase the line.
  ios_->write('\r'); //Erase the line.
  ios_->write(fmt::reset); //Clear any active formatting.
  assert(pfx_); //Prefix string cannot be null!
  if(pfx_[0] != '\0') { ios_->write(pfx_); } //Print the prefix string.  
  if(imode_)
  {
    ios_->write(fmt::setBackgroundColor(fmt::Color::brightBlack)); //Highlight line background if 
    //in insert mode.
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
        ios_->write(fmt::Underline::enable);
        ios_->write(wb_[i]);
        ios_->write(fmt::Underline::disable);
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
          ios_->write(fmt::SlowBlink::enable);
          ios_->write(wb_[i]);
        }
        else if(i == cpos_)
        {
          ios_->write(wb_[i]);
          ios_->write(fmt::SlowBlink::disable);
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
          ios_->write(fmt::SlowBlink::enable);
        }
        else if(i == sst_)
        {
          ios_->write(wb_[i]);
          ios_->write(fmt::SlowBlink::disable);
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
          ios_->write(fmt::SlowBlink::enable);
          ios_->write(wb_[i]);
          ios_->write(fmt::SlowBlink::disable);
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
  ios_->write(fmt::reset); //Clear any active formatting.
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

String& Vtt::HistoryBuffer::currentViewPos()
{
  return *buffers_[vpos_].stored();
}

String& Vtt::HistoryBuffer::currentWritePos()
{
  return *buffers_[bpos_].stored();
}

String& Vtt::HistoryBuffer::oldestWritePos()
{
  size_t sBpos = bpos_;
  nextBpos();
  while(buffers_[bpos_].stored()->length() == 0)
  {
    nextBpos();
  }
  if(bpos_ == sBpos)
  {
    prevBpos();
  }
  String* sptr = buffers_[bpos_].stored();
  bpos_ = sBpos;
  return *sptr;
}

void Vtt::HistoryBuffer::resetViewPos()
{
  vpos_ = bpos_;
}

void Vtt::HistoryBuffer::nextViewPos()
{
  size_t sVpos = vpos_;
  nextVpos();
  while(buffers_[vpos_].stored()->length() == 0)
  {
    nextVpos();
  }
  if(sVpos == vpos_)
  {
    nextVpos();
  }
}

void Vtt::HistoryBuffer::prevViewPos()
{
  size_t sVpos = vpos_;
  prevVpos();
  while(buffers_[vpos_].stored()->length() == 0)
  {
    prevVpos();
  }
  if(sVpos == vpos_)
  {
    prevVpos();
  }
}

void Vtt::HistoryBuffer::nextWritePos()
{
  nextBpos();
}

void Vtt::HistoryBuffer::nextBpos()
{
  bpos_++;
  if(bpos_ >= config::cliHistoryDepth)
  {
    bpos_ = 0;
  }
}

void Vtt::HistoryBuffer::prevBpos()
{
  bpos_--;
  if(bpos_ >= config::cliHistoryDepth)
  {
    bpos_ = config::cliHistoryDepth - 1;
  }
}

void Vtt::HistoryBuffer::nextVpos()
{
  vpos_++;
  if(vpos_ >= config::cliHistoryDepth)
  {
    vpos_ = 0;
  }
}

void Vtt::HistoryBuffer::prevVpos()
{
  vpos_--;
  if(vpos_ >= config::cliHistoryDepth)
  {
    vpos_ = config::cliHistoryDepth - 1;
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

ArgumentContainer::ArgumentContainer() : argListValid_(false), numOfArgs_(0), cli_(nullptr),
  firstArg{nullptr, nullptr}
{
}

ArgumentContainer::ArgumentContainer(CliInstance* cli, const Tokenizer& tokens,
  const size_t discardThreshold, const char* params) : argListValid_(false), numOfArgs_(0), 
  cli_(cli), firstArg{nullptr, nullptr}
{
  generateArgumentList(cli, tokens, discardThreshold, params);
}

ArgumentContainer::~ArgumentContainer() noexcept
{
}

ArgumentContainer::Status ArgumentContainer::generateArgumentList(CliInstance* cli,
  const Tokenizer& tokens, const size_t discardThreshold, const char* params)
{
  cli_ = cli;
  ParameterString ps(params);
  if(discardThreshold > tokens.count())
  {
    cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
      "Insufficient arguments passed to command.\r\n");
    return Status::insufficientArguments;
  }
  if((tokens.count() - discardThreshold) > ps.totalCount())
  {
    cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
      "Too many arguments passed to command.\r\n");
    return Status::tooManyArguments;
  }
  if((tokens.count() - discardThreshold) < (ps.totalCount() - ps.optionalCount()))
  {
    cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
      "Insufficient arguments passed to command.\r\n");
    return Status::insufficientArguments;
  }
  if((tokens.count() - discardThreshold) >= config::cliMaximumArguments)
  {
    cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
      "The global maximum argument limit has been exceeded processing this command.\r\n");
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
          if(std::strpbrk(tokens[i + discardThreshold], ".eE") != nullptr)
          {
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into a signed integer. Argument appears to be a "
              "float.\r\n", discardThreshold + i, tokens[i + discardThreshold]);
            return Status::argumentTypeMismatch;
          }
          if(std::sscanf(tokens[i + discardThreshold], p.formatString, &temp) != 1)
          {
            //Currently this error reporting is a dirty hack. Will fix after tidying exceptions so
            //a more descriptive error can be pushed up to the CliInstance for reporting there.
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into a signed integer.\r\n", discardThreshold + i,
              tokens[i + discardThreshold]);
            return Status::argumentTypeMismatch;
          }
          appendListItem(temp);
        }
        break;
      case Argument::Type::uint64_t_:
        {
          uint64_t temp;
          if(std::strpbrk(tokens[i + discardThreshold], ".eE") != nullptr)
          {
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into an unsigned integer. Argument appears to be a "
              "float.\r\n", discardThreshold + i, tokens[i + discardThreshold]);
            return Status::argumentTypeMismatch;
          }
          if(std::strchr(tokens[i + discardThreshold], '-') != nullptr)
          {
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into an unsigned integer. Argument appears to be a "
              "negative number.\r\n", discardThreshold + i, tokens[i + discardThreshold]);
            return Status::argumentTypeMismatch;
          }
          if(std::sscanf(tokens[i + discardThreshold], p.formatString, &temp) != 1)
          {
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into an unsigned integer.\r\n",
              discardThreshold + i, tokens[i + discardThreshold]);
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
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed to parse argument %d [%s] into a floating point value.\r\n",
              discardThreshold + i, tokens[i + discardThreshold]);
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
            cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
              "Failed while parsing string argument. No free string memory available.\r\n");
            return Status::noFreeStringsAvailable;
          }
          String& s = *scont.stored();
          s.replace(0, config::stringPoolStringSize - 1, tokens[i + discardThreshold]);
          appendListItem(scont);
        }
        break;
      case Argument::Type::invalid:
      default:
        cli_->vtt_->colorizedWrite(CliInstance::defaultErrorColor,
          "The parameter string for this command is invalid and cannot be parsed.\r\n");
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

CliInstance* CliInstance::activeCliInstance;

CliInstance::CliInstance(std::shared_ptr<os::AsyncIoStream>& io) 
{
  if(activeCliInstance == nullptr)
  {
    //Initialize the argument memory pool used by the CLI.
    argumentPool = std::make_unique<CliArgumentPool>("CLI_Arg_Pool");
    activeCliInstance = this;
    libList_.libptr = &cliCmdLib;
    tptr_ = new os::Thread(reinterpret_cast<os::Thread::FunctionSignature>(&cliThreadDispatcher), 
      &io, "CLI", cliThreadStackSize_Words, cliThreadPriority);
  }
  else
  {
    assert(false); //Only one CLI should be active.
  }
}

Status CliInstance::registerLibrary(const Library& lib)
{
  if(activeCliInstance == nullptr) { return Status::failure; }
  LibrariesListItem* lliPtr = &activeCliInstance->libList_;
  while(lliPtr != nullptr)
  {
    if(std::strcmp(lliPtr->libptr->name, lib.name) == 0)
    {
      //Library is already registered.
      return Status::failure;
    }
    if(lliPtr->next == nullptr)
    {
      lliPtr->next = std::make_unique<LibrariesListItem>(lib);
      return Status::success;
    }
    lliPtr = lliPtr->next.get();
  }
  return Status::failure;
}

void CliInstance::cliThreadDispatcher(std::shared_ptr<os::AsyncIoStream>* io)
{
  activeCliInstance->cliThread(io);
} 

void CliInstance::cliThread(std::shared_ptr<os::AsyncIoStream>* io)
{
  istr_ = std::make_unique<String>();
  istr_->reserve(config::cliMaximumStringLength);
  //Instantiate a visual text terminal on the I/O interface. This will be used for all I/O performed
  //by the CLI.
  vtt_ = std::make_unique<Vtt>(*io);
  //On startup, the CLI will always default to the minimum permission level.
  aplvl_ = AccessPermission::unrestricted;
  while(true)
  {
    //Wait for some argument input.
    while(true)
    {
      vtt_->write("CLI awaiting input.\r\n");
      if(vtt_->read(*istr_) > 0)
      {
        break;
      }
    }
    //Tokenize the input.
    Tokenizer tokens(*istr_);
    //Search for and handle any special commands.
    if(handleSpecialCommands(tokens)) { continue; }
    if(tokens.count() < 1)
    {
      vtt_->colorizedWrite(defaultErrorColor, 
        "Commands must include a library and command name. Enter 'cli help' for more "
        "information.\r\n"); continue;
    }
    //Search for the library name. If not found, restart loop and await new input.
    if(!lookupLibrary(tokens[0])) { continue; }
    if(tokens.count() < 2)
    {
      vtt_->colorizedWrite(defaultErrorColor, 
        "Commands must include a library and command name. Enter 'cli help %s' for more "
        "information", alptr_->name);
      vtt_->colorizedWrite(defaultErrorColor, 
        " about the commands in the '%s' library.\r\n", alptr_->name);
      continue;
    }
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
  alptr_ = nullptr;
  LibrariesListItem* lli = &libList_;
  while(lli != nullptr)
  {
    if(std::strcmp(lli->libptr->name, name) == 0) { break; }
    else { lli = lli->next.get(); }
  }
  if(lli == nullptr)
  {
    vtt_->colorizedWrite(defaultErrorColor, 
      "Failed to find library '%s'. Try 'cli help' to list available libraries.\r\n", name);
    return false;
  }
  alptr_ = lli->libptr;
  return true;
}

bool CliInstance::lookupCommand(const char* name)
{
  //Lookup the command name in the active library.
  assert(alptr_); assert(name);
  acptr_ = nullptr;
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
    vtt_->colorizedWrite(defaultErrorColor, 
      "Failed to find command '%s' in library '%s'. Try 'cli help %s' to list available commands"
      " within the '%s' library.\r\n", name, alptr_->name, alptr_->name, alptr_->name);
    return false;
  }
  acptr_ = cptr;
  return true;
}

int CliInstance::executeCommand(Tokenizer& tokens)
{
  //If we have the command matched, we need to construct a CommandIo object, including an argument
  //container, and execute the command if the IO object is valid.
  assert(acptr_); 
  CommandIo cmdIo(this, tokens, *acptr_, *vtt_);
  if(!cmdIo.isValid_)
  {
    return 1;
  }
  int32_t cmdRet = 1;
  try
  {
    cmdRet = acptr_->function(cmdIo);
    if(cmdRet != 0)
    {
      vtt_->colorizedWrite(os::AnsiFormatter::Color::yellow,
        "Warning: Command returned status code %ld\r\n", cmdRet);
    }
  }
  catch(...)
  {
    vtt_->colorizedWrite(os::AnsiFormatter::Color::yellow,
      "Warning: Command threw an exception!\r\n");
  }
  return cmdRet;
}

bool CliInstance::doesAplvlMeetSecRequirment(const AccessPermission& lvlToCheckAgainst)
{
  auto reqLvl = static_cast<uint8_t>(lvlToCheckAgainst);
  auto curLvl = static_cast<uint8_t>(aplvl_);
  if(reqLvl <= curLvl)
  {
    return true;
  }
  return false;
}

CommandIo::CommandIo(CliInstance* cli, const Tokenizer& tokens, const CommandEntry& cmd, Vtt& vtt) :
  fmt(), cmdptr(&cmd), args(cli, tokens, 2, cmd.parameters), vtt_(vtt), cli_(cli),
  isValid_(args.isArgListValid())
{
  preIoPpConfig_.disableAllFormatting = vtt_.printer().editConfig().stripFormatters;
  preIoPpConfig_.automaticNewline = vtt_.printer().editConfig().automaticNewline;
}

CommandIo::~CommandIo() noexcept
{
  vtt_.printer().editConfig().stripFormatters = preIoPpConfig_.disableAllFormatting;
  vtt_.printer().editConfig().automaticNewline = preIoPpConfig_.automaticNewline;
}

Status CommandIo::print(const char* format, ...)
{
  printFormatters(); 
  va_list args;
  va_start(args, format);
  Status st = vtt_.write(format, args);
  va_end(args);
  vtt_.write(os::AnsiFormatter::reset);
  return st;
}

size_t CommandIo::scan(char* buffer, size_t bufferLen, const Duration& timeout)
{
  (void)buffer; (void)bufferLen; (void)timeout;
  return 0;
}

bool CommandIo::getConfirmation(const char* prompt, const Duration& timeout)
{
  (void)prompt; (void)timeout;
  return false;
}

bool CommandIo::waitForContinue(const char* prompt, const Duration& timeout)
{
  constexpr size_t bufLen = 8;
  Timestamp startT = SteadyClock::now();
  char buf[bufLen];
  auto sg = ToScopeGuard([&](){ vtt_.prefix(prompt); }, [&](){vtt_.prefix(nullptr);});
  while((SteadyClock::now() - startT) < timeout)
  {
    vtt_.read(buf, bufLen, timeout - (SteadyClock::now() - startT));
    if(std::strpbrk(buf, "\r\n"))
    {
      return true;
    }
  }
  return false;
}

int64_t CommandIo::readSignedInt(const char* prompt, const Duration& timeout)
{
  constexpr size_t bufLen = 32;
  Timestamp startT = SteadyClock::now();
  char buf[bufLen];
  auto sg = ToScopeGuard([&](){ vtt_.prefix(prompt); }, [&](){vtt_.prefix(nullptr);});
  while((SteadyClock::now() - startT) < timeout)
  {
    int64_t rval;
    vtt_.read(buf, bufLen, timeout - (SteadyClock::now() - startT));
    if(std::strpbrk(buf, ".") != nullptr) { continue; }
    if(std::sscanf(buf, "%lld", &rval) == 1)
    {
      return rval;
    }
  }
  throw os::Exception{os::ExceptionCode::cliArgumentReadTimeout,
    "Failed to read a signed integer within the specified timeout.\r\n"};
}

uint64_t CommandIo::readUnsignedInt(const char* prompt, const Duration& timeout)
{
  constexpr size_t bufLen = 32;
  Timestamp startT = SteadyClock::now();
  char buf[bufLen];
  auto sg = ToScopeGuard([&](){ vtt_.prefix(prompt); }, [&](){vtt_.prefix(nullptr);});
  while((SteadyClock::now() - startT) < timeout)
  {
    uint64_t rval;
    vtt_.read(buf, bufLen, timeout - (SteadyClock::now() - startT));
    if(std::strpbrk(buf, "-.") != nullptr) { continue; }
    if(std::sscanf(buf, "%llu", &rval) == 1)
    {
      return rval;
    }
  }
  throw os::Exception{os::ExceptionCode::cliArgumentReadTimeout,
    "Failed to read an unsigned integer within the specified timeout.\r\n"};
}

double CommandIo::readDouble(const char* prompt, const Duration& timeout)
{
  constexpr size_t bufLen = 32;
  Timestamp startT = SteadyClock::now();
  char buf[bufLen];
  auto sg = ToScopeGuard([&](){ vtt_.prefix(prompt); }, [&](){vtt_.prefix(nullptr);});
  while((SteadyClock::now() - startT) < timeout)
  {
    double rval;
    vtt_.read(buf, bufLen, timeout - (SteadyClock::now() - startT));
    if(std::sscanf(buf, "%lf", &rval) == 1)
    {
      return rval;
    }
  }
  throw os::Exception{os::ExceptionCode::cliArgumentReadTimeout,
    "Failed to read a double within the specified timeout.\r\n"};
}

void CommandIo::printFormatters()
{
  using afmtr = os::AnsiFormatter;
  vtt_.printer().editConfig().stripFormatters= fmt.disableAllFormatting;
  if(fmt.disableAllFormatting) { return; }
  vtt_.printer().editConfig().automaticNewline = fmt.automaticNewline;
  vtt_.write(afmtr::setForegroundColor(fmt.color));
  if(fmt.isBold) { vtt_.write(afmtr::Bold::enable); }
  else { vtt_.write(afmtr::Bold::disable); }
  if(fmt.isUnderlined) { vtt_.write(afmtr::Underline::enable); }
  else { vtt_.write(afmtr::Underline::disable); }
}

int32_t cliCmdHelp(CommandIo& io)
{
  io.print("TODO HELP\r\n");
  return 0;
}

int32_t cliCmdLogin(CommandIo& io)
{
  io.print("TODO LOGIN\r\n");
  return 0;
}

} /** namespace cli */
} /** namespace jel */

