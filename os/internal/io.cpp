/** @file os/internal/io.cpp
 *  @brief Implementation of system I/O handling objects.
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
/** jel Library Headers */
#include "os/api_io.hpp"
#include "os/api_time.hpp"
#include "os/internal/indef.hpp"

namespace jel
{

MtWriter::MtWriter(std::unique_ptr<SerialWriterInterface> writer) :
  stream_(std::move(writer))
{
  
}

Status MtWriter::write(const char* cStr, size_t length, const Duration& timeout) 
{
  assert(cStr != nullptr); //Cannot print a nulltpr.
  if(length == 0) { length = std::strlen(cStr); }
  Timestamp start = SteadyClock::now();
  LockGuard lg{lock_, timeout};
  if(lg.isLocked())
  { 
    if(stream_->isBusy(timeout - (SteadyClock::now() - start)) == false)
    {
      stream_->write(cStr, length);
      stream_->isBusy(timeout - (SteadyClock::now() - start));
      return Status::success;
    }
    else
    {
      return Status::failure;
    }
  }
  else
  {
    return Status::failure;
  }
}

Status MtWriter::write(const String& string, const Duration& timeout)
{
  return write(string.c_str(), string.length(), timeout);
}

Status MtWriter::write(const char c)
{
  stream_->write(c);
  return Status::success;
}

AsyncLock MtWriter::lockOutput(const Duration& timeout)
{
  return AsyncLock{lock_, timeout};
}

MtReader::MtReader(std::unique_ptr<SerialReaderInterface> reader) :
  stream_(std::move(reader))
{

}

size_t MtReader::read(char* buffer, const size_t length, const Duration& timeout)
{
  if(length == 0) { return 0; }
  assert(buffer != nullptr); //Cannot receive data into a nullptr.
  Timestamp start = SteadyClock::now();
  LockGuard lg{lock_, timeout};
  if(lg.isLocked())
  {
    //Read one character less than the buffer size so we can null terminate it.
    stream_->read(buffer, length - 1);
    size_t readLen = stream_->waitForChars(timeout - (SteadyClock::now() - start));
    buffer[readLen] = '\0';
    return readLen;
  }
  return 0;
}

AsyncLock MtReader::lockInput(const Duration& timeout)
{
  return AsyncLock{lock_, timeout};
}

AsyncIoStream::AsyncIoStream(std::unique_ptr<SerialReaderInterface> reader, 
  std::unique_ptr<SerialWriterInterface> writer, bool sharedInterface) : 
  MtReader(std::move(reader)), MtWriter(std::move(writer)), shared_(sharedInterface)
{

}

AsyncIoStream::~AsyncIoStream() noexcept
{
  if(shared_)
  {
    MtWriter::stream_.release(); //Release the ownership of the shared base class from one of the 
    //two streams
    MtReader::stream_ = nullptr; //Delete the base class.
  }
  //If they are not shared, then the respective destructors for MtReader and MtWriter will be called
  //and delete their derived objects accordingly.
}

const PrettyPrinter::Config PrettyPrinter::defaultConfig;

PrettyPrinter::PrettyPrinter(const std::shared_ptr<MtWriter>& output, const Config& config) :
  out_(output), cfg_(config), clen_(0), cidnt_(0)
{
  
}

Status PrettyPrinter::print(const String& string)
{
  return print(string.c_str(), string.length());
}

Status PrettyPrinter::print(const char* cStr, size_t length)
{
  assert(cStr); //Cannot print a nullptr.
  if(length == 0) { length = std::strlen(cStr); }
  size_t bpos = 0;
  size_t epos = 0;
  /** Print a newline, optionally with carriage return, and indent to tabLevel. */
  auto printAutoNewline = [&](const bool addCr, const size_t tabLevel)
  {
    if(!cfg_.automaticNewline) { clen_ = 0; return; }
    if(addCr) { out_->write("\r\n", 2); }
    else { out_->write("\n", 1); }
    clen_ = 0;
    for(size_t i = 0; i < tabLevel; i++)
    {
      out_->write("\t", 1);
      clen_ += cfg_.indentDepth_chars; 
    }
    return;
  };
  /** Print from bpos to epos, then advance bpos to epos. */
  auto printToEpos = [&]()
  {
    out_->write(&cStr[bpos], epos - bpos);
    //TODO add throw on error
    bpos = epos;
    return;
  };
  /** Advance epos to the next visible character. */
  auto toNextVisible = [&]()
  {
    while(true)
    {
      if(cStr[epos] == '\t')
      {
        epos++;
        if(cidnt_ < cfg_.maxIndentDepth)
        {
          cidnt_++; //track current indentation depth.
        }
      }
      else if(cStr[epos] == '\n')
      {
        if(&cStr[0] <= &cStr[epos - 1])
        {
          if((cStr[epos - 1] != '\r') && cfg_.carriageReturnNewline)
          {
            printToEpos(); //Print up to newline character.
            out_->write("\r", 1); //Print a carriage return.
          }
        }
        epos++;
        cidnt_ = 0;
        clen_ = 0; //Reset current line length and indentation.
      }
      else if(cStr[epos] == AnsiFormatter::escSeqPrefix[0])
      {
        //If this is an escape sequence, we need to advance to epos past the end of it. When
        //printed, an escape sequence needs to remain contiguous, and is invisible, so it does not
        //count towards the line length.
        constexpr size_t csilen = constStringLen(AnsiFormatter::escSeqPrefix);
        bool isEscapeSeq = true;
        //Verify this is an actual escape sequence, not just an escape character. To do this,
        //check that this matches the escape sequence prefix.
        for(size_t i = 0; i < csilen; i++)
        {
          if(cStr[epos + i] != AnsiFormatter::escSeqPrefix[i])
          {
            isEscapeSeq = false;
            break;
          }
        }
        if(isEscapeSeq)
        {
          size_t escSeqStartPos = epos;
          while(true)
          {
            if((cStr[epos] >= 'A') && (cStr[epos] <= 'Z'))
            {
              epos++; //End of escape sequence.
              break;
            }
            if((cStr[epos] >= 'a') && (cStr[epos] <= 'z'))
            {
              epos++; //End of escape sequence.
              break;
            }
            epos++; //Not the end of escape sequence, keep advancing.
          }
          //If we are ignoring ANSI formatters, 'jump over' this escape sequence.
          if(cfg_.stripFormatters)
          {
            size_t savedEpos = epos;
            epos = escSeqStartPos;
            printToEpos();
            bpos = savedEpos; epos = savedEpos;
          }
        }
      }
      else if(cStr[epos] == ' ')
      {
        epos++;
        clen_++;
        if(clen_ >= cfg_.lineLen)
        {
          printToEpos();
          printAutoNewline(cfg_.carriageReturnNewline, cidnt_);
        }
      }
      else if(cStr[epos] == '\0')
      {
        printToEpos(); //End of string.
        return;
      }
      else if ((cStr[epos] < ' ') || (cStr[epos] == 0x7F))
      {
        epos++; //Other control character.
      }
      else
      {
        //Character is a visible character. We are done advancing.
        return;
      }
    }
  };
  auto toNextNonAlphanumeric = [&]()
  {
    size_t wlen = 0;
    while(true)
    {
      if((cStr[epos + wlen] > ' ') && (cStr[epos + wlen] < 0x7F))
      {
        wlen++;
      }
      else if((cStr[epos + wlen] > 0x7F))
      {
        wlen++; //Character is an extended ASCII code, we treat these like regular characters.
      }
      else
      {
        //Non-visible character, word is over.
        if((clen_ + wlen) >= cfg_.lineLen)
        {
          //Word would exceed the line length. We need to flush to current epos then print a newline
          //character (+ any indentation).
          printToEpos();
          printAutoNewline(cfg_.carriageReturnNewline, cidnt_);
          epos += wlen;
          clen_ += wlen;
          break;
        }
        else
        {
          //Word would not exceed line length. Simply increment epos and current line length by word
          //length.
          epos += wlen;
          clen_ += wlen;
          break;
        }
      }
    }
  };
  auto outLock(out_->lockOutput());
  while(cStr[epos] != '\0')
  {
    toNextVisible();
    toNextNonAlphanumeric();
  }
  if(bpos < epos)
  {
    printToEpos();
  }
  return Status::success;
}

void PrettyPrinter::nextLine()
{
  if(cfg_.carriageReturnNewline)
  {
    out_->write("\r\n");
  }
  else
  {
    out_->write("\n");
  }
  clen_ = 0;
}

constexpr char AnsiFormatter::reset[];
constexpr char AnsiFormatter::escSeqPrefix[];
constexpr char AnsiFormatter::Bold::enable[];
constexpr char AnsiFormatter::Bold::disable[];
constexpr char AnsiFormatter::Underline::enable[];
constexpr char AnsiFormatter::Underline::disable[];
constexpr char AnsiFormatter::SlowBlink::enable[];
constexpr char AnsiFormatter::SlowBlink::disable[];
constexpr char AnsiFormatter::ColorCode::black[];
constexpr char AnsiFormatter::ColorCode::red[];
constexpr char AnsiFormatter::ColorCode::green[];
constexpr char AnsiFormatter::ColorCode::yellow[];
constexpr char AnsiFormatter::ColorCode::blue[];
constexpr char AnsiFormatter::ColorCode::magenta[];
constexpr char AnsiFormatter::ColorCode::cyan[];
constexpr char AnsiFormatter::ColorCode::white[];
constexpr char AnsiFormatter::ColorCode::default_[];
constexpr char AnsiFormatter::BrightColorCode::black[];
constexpr char AnsiFormatter::BrightColorCode::red[];
constexpr char AnsiFormatter::BrightColorCode::green[];
constexpr char AnsiFormatter::BrightColorCode::yellow[];
constexpr char AnsiFormatter::BrightColorCode::blue[];
constexpr char AnsiFormatter::BrightColorCode::magenta[];
constexpr char AnsiFormatter::BrightColorCode::cyan[];
constexpr char AnsiFormatter::BrightColorCode::white[];
constexpr char AnsiFormatter::BackgroundColorCode::black[];
constexpr char AnsiFormatter::BackgroundColorCode::red[];
constexpr char AnsiFormatter::BackgroundColorCode::green[];
constexpr char AnsiFormatter::BackgroundColorCode::yellow[];
constexpr char AnsiFormatter::BackgroundColorCode::blue[];
constexpr char AnsiFormatter::BackgroundColorCode::magenta[];
constexpr char AnsiFormatter::BackgroundColorCode::cyan[];
constexpr char AnsiFormatter::BackgroundColorCode::white[];
constexpr char AnsiFormatter::BackgroundColorCode::default_[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::black[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::red[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::green[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::yellow[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::blue[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::magenta[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::cyan[];
constexpr char AnsiFormatter::BrightBackgroundColorCode::white[];
constexpr char AnsiFormatter::Erase::toEndOfLine[];
constexpr char AnsiFormatter::Erase::toStartOfLine[];
constexpr char AnsiFormatter::Erase::entireLine[];
constexpr char AnsiFormatter::Erase::toEndOfScreen[];
constexpr char AnsiFormatter::Erase::toStartOfScreen[];
constexpr char AnsiFormatter::Erase::entireScreen[];
constexpr char AnsiFormatter::Erase::entireScreenAndScrollback[];
constexpr char AnsiFormatter::Cursor::up[];
constexpr char AnsiFormatter::Cursor::down[];
constexpr char AnsiFormatter::Cursor::forward[];
constexpr char AnsiFormatter::Cursor::back[];
constexpr char AnsiFormatter::Cursor::nextLine[];
constexpr char AnsiFormatter::Cursor::previousLine[];
constexpr char AnsiFormatter::Cursor::savePosition[];
constexpr char AnsiFormatter::Cursor::restorePosition[];
constexpr char AnsiFormatter::Cursor::pageUp[];
constexpr char AnsiFormatter::Cursor::pageDown[];
constexpr char AnsiFormatter::Input::upArrowKey[];
constexpr char AnsiFormatter::Input::downArrowKey[];
constexpr char AnsiFormatter::Input::rightArrowKey[];
constexpr char AnsiFormatter::Input::leftArrowKey[];
constexpr char AnsiFormatter::Input::shiftUpArrowKey[];
constexpr char AnsiFormatter::Input::shiftDownArrowKey[];
constexpr char AnsiFormatter::Input::shiftRightArrowKey[];
constexpr char AnsiFormatter::Input::shiftLeftArrowKey[];
constexpr char AnsiFormatter::Input::homeKey[];
constexpr char AnsiFormatter::Input::insertKey[];
constexpr char AnsiFormatter::Input::deleteKey[];
constexpr char AnsiFormatter::Input::endKey[];
constexpr char AnsiFormatter::Input::pageUpKey[];
constexpr char AnsiFormatter::Input::pageDownKey[];

} /** namespace jel */

