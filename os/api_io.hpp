/** @file os/api_io.hpp
 *  @brief Input and Output interfaces and related objects are exposed here.
 *
 *  @detail
 *    Multiple I/O components are built into jel. These include the following:
 *      -SerialReader and SerialWriter interface components. These interfaces must be implemented by
 *      a hardware peripheral or internal write to/read from buffer object and will be used for all
 *      standard I/O.
 *      -AsyncSerialReader and AsyncSerialWriter wrapper objects. These objects are constructed
 *      around a SerialReader/Writer interface and provide thread safety via a mutex. Note that this
 *      makes AsyncSerialReader/Writer objects illegible for use in ISRs.
 *      -IoController object comprised of at least one AsyncSerialReader/Writer object. The
 *      IoController can be used to redirect I/O or share I/O operations across various Reader and
 *      Writer objects. All system I/O is also performed through one of these objects which is
 *      instantiated on bootup.
 *      -AnsiFormatter objects used to generate ANSI/VT100 style formatting codes suitable for human
 *      readable output via a terminal such as Putty or Bash. Due to various slight differences in
 *      formatting behaviour between terminals, multiple formatter variations are available.
 *      -LinePrinter object used to enable automatic newline/linebreak insertion. Non-whitespace
 *      characters printed via a LinePrinter are tracked, in addition to indentation depth, to allow
 *      for automatic insertion of newlines ensuring printed data maintains an acceptable width.
 *      -PrettyPrinter object comprised of an AnsiFormatter and a LinePrinter, and wraps an
 *      IoController object for actual output. The PrettyPrinter provides a convenient interface for
 *      outputting human readable data via CLI. It also enables operations such as runtime
 *      reconfiguration of the underlying AnsiFormatter.
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
#include <memory>
#include <string>
#include <cstring>
#include <cassert>
/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_locks.hpp"

namespace jel
{
/** String class used by the jel.
 *  @note
 *    This is provided primarily to allow easy allocator changes or std::string replacement.
 * */
typedef std::basic_string<char> String;

/** 
 *  A constexpr c-string length function, allowing the evaluation of a string length at compile
 *  time.
 *  @note
 *    To force constexpr evaluation, the result of this function must be assigned to a constexpr
 *    value. For example,
 *    @code
 *      constexpr size_t testStringLen = constStringLen("test"); //Evaluated at compile time.
 *      SerialWriterInterface->write("test", constStringLen("test")); //May/may not be evaluated at
 *      //compile time.
 *    @endcode
 *    This is as per the C++14 standard.
 * */
constexpr size_t constStringLen(const char* cString)
{
  size_t len = 0;
  for(; cString[len] != '\0';len++) {}
  return len;
}

namespace os
{
/** @class SerialWriterInterface
 *  @brief The SerialWriterInterface is implemented by drivers that support serial transmission of
 *  data.
 *  
 *  SerialWriterInterfaces are designed to be implemented by any driver or other object that
 *  supports transmitting serial data. 
 *
 *  @note The SerialWriteInterface does not support multi-threaded operation. Simultaneous calls to
 *  Writer functions from multiple threads have undefined behavior.
 * */
class SerialWriterInterface
{
public:
  virtual ~SerialWriterInterface() noexcept {}
  /** Write a string of length_chars to the output. If the output is busy, transmission will be
   * overridden as soon as possible. */
  virtual void write(const char* cStr, const size_t length_chars) = 0;
  /** Write a single character to the output. If the output is busy, transmission will be overridden
   * as soon as possible (i.e., there is a space available in the buffer). */
  virtual void write(const char c) = 0;
  /** Check if the transmitter is currently busy. If a nonzero timeout parameter is specified, this
   * call will block until the transmitter is no longer busy or the timeout expires. */
  virtual bool isBusy(const Duration& timeout) = 0;
};

/** @class SerialReaderInterface
 *  @brief The SerialReaderInterface is implemented by drivers that support serial reception of
 *  data.
 *  @note The SerialReaderInterface does not support multi-threaded operation. Simultaneous calls to
 *  Reader functions from multiple threads have undefined behavior.
 * */
class SerialReaderInterface
{
public:
  virtual ~SerialReaderInterface() noexcept {}
  /** Starts a receive operation. If a receive operation is ongoing, it will be overridden. 
   *  @throws
   *    This function may throw if the hardware performing the reception encounters an overrun
   *    condition.
   * */
  virtual size_t read(char* buffer, const size_t bufferLen) = 0;
  /** If a receive operation is ongoing, this will block until it is complete or the timeout is
   * reached. */
  virtual size_t waitForChars(const Duration& timeout) = 0;
};

/** @class AsyncLock
 *  @brief Locking object that can be held to prevent asynchronous access to streams.
 *  
 *  For MtWriter and MtReader objects, it is possible that multiple threads will attempt to
 *  read/write data simultaneously. While this is allowed, it means that separate calls to
 *  write()/read() may be split by other threads when this is not desired. To prevent this from
 *  happening, the Async objects support locking, which when done returns an AsyncLock that prevents
 *  all other threads from reading or writing to the object until the owning thread has released the
 *  Lock.
 * */
class AsyncLock : public LockGuard
{
public:
  AsyncLock(Mutex& mtx, const Duration& timeout) : LockGuard{mtx, timeout} {}
  ~AsyncLock() noexcept {}
  AsyncLock(AsyncLock&& other) : LockGuard{std::move(other)} {  }
  AsyncLock(const AsyncLock&) = delete;
};

/** @class MtWriter
 *  @brief Provides a convenient interface around a SerialWriter that also provides thread-safety.
 *
 *  @note
 *    MtWriter objects are not capable of being used in interrupts.
 * */
class MtWriter
{
public:
  /** Construct a threadsafe writer object around a serial writer object. The writer object is
   * considered the output stream.
   *  @note
   *    Ownership of the output stream is taken by the MtWriter. 
   * */
  MtWriter(std::unique_ptr<SerialWriterInterface> writer);
  /** Writes to the output stream. A total of len characters will be printed, including null
   * characters, from the memory pointed to by data. A length of zero will result in a call to
   * std::strlen on the *data, in which case *data must be a valid, null terminated c-string.
   * */
  Status write(const char* data, size_t len = 0, const Duration& timeout = Duration::max());
  /** Writes a string to the output stream in the same manner was write(const char*...). */
  Status write(const String& string, const Duration& timeout = Duration::max());
  /** Writes a single character to the output stream. Note: This function is optimized for speed,
   * and does *not* assume the output stream is already locked. It must be manually locked with
   * lockStream() before calling! */
  Status write(const char c);
  /** Lock the output stream. An AsyncLock object will be returned, which will prevent other threads
   * from using the output stream so long as it is extant. */
  AsyncLock lockOutput(const Duration& timeout = Duration::max());
protected:
  std::unique_ptr<SerialWriterInterface> stream_;
  RecursiveMutex lock_;
};

/** @class MtReader
 *  @brief Provides a threadsafe interface around a SerialReader.
 *  
 *  */
class MtReader
{
public:
  /** Construct the reader around a serial reader object. The reader object is considered the input
   *  stream. 
   *  @note
   *    Ownership of the input stream is taken by the MtReader.
   * */
  MtReader(std::unique_ptr<SerialReaderInterface> reader);
  /** Read up to length - 1 characters into the buffer. The buffer is always null terminated. */
  size_t read(char* buffer, const size_t length, const Duration& timeout);
  AsyncLock lockInput(const Duration& timeout);
protected:
  std::unique_ptr<SerialReaderInterface> stream_;
  RecursiveMutex lock_;
};

/** @class MtIoStream 
 *  @brief Provides a threadsafe I/O interface comprised of an MtReader and MtWriter.
 *
 *  Combines a reader and writer interface under one asynchronous wrapper.
 *  @note
 *    If the reader and
 *
 *    writer pointers refer to the same base object (for example, a UART class that implements both
 *    reader and writer) then the sharedInterface flag must be set to true to ensure a double
 *    deletion does not occur on destruction of the AsyncIoStream.
 * */
class AsyncIoStream : public MtReader, public MtWriter
{
public:
  AsyncIoStream(std::unique_ptr<SerialReaderInterface> reader, 
    std::unique_ptr<SerialWriterInterface> writer, const bool sharedInterface);
  ~AsyncIoStream() noexcept;
private:
  bool shared_;
};

/** @struct AnsiFormatter
 *  @brief Provides functionality for creating ANSI terminal compatible formatting codes.
 *
 *  The formatting codes generated by the AnsiFormatter should be transmitted to the terminal
 *  interface as standard string characters. Note that not all terminals support all ANSI codes, or
 *  support all ANSI codes correctly or in an identical manner. The formatter is primarily tested
 *  and designed to work with PUTTY (https://www.putty.org/) and bash on the windows linux
 *  subsystem.
 * */
struct AnsiFormatter
{
  /** Definitions for special ASCII characters. */
  struct ControlCharacters
  {
    static constexpr char bell = '\007';
    static constexpr char backspace = '\010';
    static constexpr char tab = '\t';
    static constexpr char newline = '\n';
    static constexpr char carriageReturn = '\r';
    static constexpr char escape = '\033';
    static constexpr char del = '\177';
  };
  static constexpr char reset[] = "\e[0m";
  static constexpr char escSeqPrefix[] = "\e[";
  struct Bold
  {
    static constexpr char enable[] = "\e[1m";
    static constexpr char disable[] = "\e[21m";
  };
  struct Underline
  {
    static constexpr char enable[] = "\e[4m";
    static constexpr char disable[] = "\e[24m";
  };
  struct SlowBlink
  {
    static constexpr char enable[] = "\e[5m";
    static constexpr char disable[] = "\e[25m";
  };
  enum class Color 
  {
    black,
    brightBlack,
    red,
    brightRed,
    green,
    brightGreen,
    yellow,
    brightYellow,
    blue,
    brightBlue,
    magenta,
    brightMagenta,
    cyan,
    brightCyan,
    white,
    brightWhite,
    default_
  };
  struct ColorCode
  {
    static constexpr char black[] =                 "\e[30m";
    static constexpr char red[] =                   "\e[31m";
    static constexpr char green[] =                 "\e[32m";
    static constexpr char yellow[] =                "\e[33m";
    static constexpr char blue[] =                  "\e[34m";
    static constexpr char magenta[] =               "\e[35m";
    static constexpr char cyan[] =                  "\e[36m";
    static constexpr char white[] =                 "\e[37m";
    static constexpr char default_[] =              "\e[39m";
  };
  struct BrightColorCode
  {
    static constexpr char black[] =                 "\e[90m";
    static constexpr char red[] =                   "\e[91m";
    static constexpr char green[] =                 "\e[92m";
    static constexpr char yellow[] =                "\e[93m";
    static constexpr char blue[] =                  "\e[94m";
    static constexpr char magenta[] =               "\e[95m";
    static constexpr char cyan[] =                  "\e[96m";
    static constexpr char white[] =                 "\e[97m";
  };
  struct BackgroundColorCode
  {
    static constexpr char black[] =                 "\e[40m";
    static constexpr char red[] =                   "\e[41m";
    static constexpr char green[] =                 "\e[42m";
    static constexpr char yellow[] =                "\e[43m";
    static constexpr char blue[] =                  "\e[44m";
    static constexpr char magenta[] =               "\e[45m";
    static constexpr char cyan[] =                  "\e[46m";
    static constexpr char white[] =                 "\e[47m";
    static constexpr char default_[] =              "\e[49m";
  };
  struct BrightBackgroundColorCode
  {
    static constexpr char black[] =                 "\e[100m";
    static constexpr char red[] =                   "\e[101m";
    static constexpr char green[] =                 "\e[102m";
    static constexpr char yellow[] =                "\e[103m";
    static constexpr char blue[] =                  "\e[104m";
    static constexpr char magenta[] =               "\e[105m";
    static constexpr char cyan[] =                  "\e[106m";
    static constexpr char white[] =                 "\e[107m";
  };
  struct Erase
  {
    static constexpr char toEndOfLine[] = "\e[0K";
    static constexpr char toStartOfLine[] = "\e[1K";
    static constexpr char entireLine[] = "\e[2K";
    static constexpr char toEndOfScreen[] = "\e[0J";
    static constexpr char toStartOfScreen[] = "\e[1J";
    static constexpr char entireScreen[] = "\e[2J";
    static constexpr char entireScreenAndScrollback[] = "\e[3J";
  };
  struct Cursor
  {
    static constexpr char up[] = "\e[1A";
    static constexpr char down[] = "\e[1B";
    static constexpr char forward[] = "\e[1C";
    static constexpr char back[] = "\e[1D";
    static constexpr char nextLine[] = "\e[1E";
    static constexpr char previousLine[] = "\e[1F";
    static constexpr char savePosition[] = "\e[s";
    static constexpr char restorePosition[] = "\e[u";
    static constexpr char pageUp[] = "\e[S";
    static constexpr char pageDown[] = "\e[T";
  };
  struct Input
  {
    static constexpr char upArrowKey[] = "\e[A";
    static constexpr char downArrowKey[] = "\e[B";
    static constexpr char rightArrowKey[] = "\e[C";
    static constexpr char leftArrowKey[] = "\e[D";
    static constexpr char shiftUpArrowKey[] = "\eOA";
    static constexpr char shiftDownArrowKey[] = "\eOB";
    static constexpr char shiftRightArrowKey[] = "\eOC";
    static constexpr char shiftLeftArrowKey[] = "\eOD";
    static constexpr char homeKey[] = "\e[1~";
    static constexpr char insertKey[] = "\e[2~";
    static constexpr char deleteKey[] = "\e[3~";
    static constexpr char endKey[] = "\e[4~";
    static constexpr char pageUpKey[] = "\e[5~";
    static constexpr char pageDownKey[] = "\e[6~";
  };
  static const char* setCursorPosition(char* buffer, const size_t length, size_t hPos)
  {
    assert(length >= 8); assert(hPos < 999); if(hPos > 999) { hPos = 999; }
    std::snprintf(buffer, length, "\e[%uG", hPos);
    return buffer;
  }
  static const char* setForegroundColor(const Color color)
  {
    switch(color)
    {
      case Color::black: return ColorCode::black;
      case Color::brightBlack: return BrightColorCode::black;
      case Color::red: return ColorCode::red;
      case Color::brightRed: return BrightColorCode::red;
      case Color::green: return ColorCode::green;
      case Color::brightGreen: return BrightColorCode::green;
      case Color::yellow: return ColorCode::yellow;
      case Color::brightYellow: return BrightColorCode::yellow;
      case Color::blue: return ColorCode::blue;
      case Color::brightBlue: return BrightColorCode::blue;
      case Color::magenta: return ColorCode::magenta;
      case Color::brightMagenta: return BrightColorCode::magenta;
      case Color::cyan: return ColorCode::cyan;
      case Color::brightCyan: return BrightColorCode::cyan;
      case Color::white: return ColorCode::white;
      case Color::brightWhite: return BrightColorCode::white;
      case Color::default_: return ColorCode::default_;
    }
    return nullptr;
  }
  static const char* setBackgroundColor(const Color color)
  {
    switch(color)
    {
      case Color::black: return BackgroundColorCode::black;
      case Color::brightBlack: return BrightBackgroundColorCode::black;
      case Color::red: return BackgroundColorCode::red;
      case Color::brightRed: return BrightBackgroundColorCode::red;
      case Color::green: return BackgroundColorCode::green;
      case Color::brightGreen: return BrightBackgroundColorCode::green;
      case Color::yellow: return BackgroundColorCode::yellow;
      case Color::brightYellow: return BrightBackgroundColorCode::yellow;
      case Color::blue: return BackgroundColorCode::blue;
      case Color::brightBlue: return BrightBackgroundColorCode::blue;
      case Color::magenta: return BackgroundColorCode::magenta;
      case Color::brightMagenta: return BrightBackgroundColorCode::magenta;
      case Color::cyan: return BackgroundColorCode::cyan;
      case Color::brightCyan: return BrightBackgroundColorCode::cyan;
      case Color::white: return BackgroundColorCode::white;
      case Color::brightWhite: return BrightBackgroundColorCode::white;
      case Color::default_: return BackgroundColorCode::default_;
    }
    return nullptr;
  }
};
/** @class PrettyPrinter
 *  @brief Provides an automatic output formatting interface for printing to a CLI.
 *  
 *  The PrettyPrinter will take arbitrary, non-null character input data and write it to an output
 *  stream. Data that is printed can automatically have linebreaks inserted or ANSI Formatters
 *  stripped out as desired. Automatic linebreaks will only be inserted inside non-whitespace, non
 *  ANSI control characters.
 *  */
class PrettyPrinter
{
public:
  struct Config
  {
    size_t lineLen = 80;
    size_t indentDepth_chars = 4;
    size_t maxIndentDepth = 4;
    bool stripFormatters = false;
    bool carriageReturnNewline = true;
    bool automaticNewline = true;
  };
  static const Config defaultConfig;
  PrettyPrinter(const std::shared_ptr<MtWriter>& output, const Config& config = defaultConfig);
  /** Prints a string. */
  Status print(const String& string);
  /** Prints a null-terminated C-string. If a length parameter is provided, a call to std::strlen is
   * saved. The length parameter should be identical to what is returned by std::strlen(cStr); */
  Status print(const char* cStr, size_t length = 0);
  Config& editConfig() { return cfg_; }
  /** Automatically output a newline sequence and reset the current line length to zero. */
  void nextLine();
  size_t currentLength() const { return clen_; }
private:
  std::shared_ptr<MtWriter> out_;
  Config cfg_;
  size_t clen_;
  size_t cidnt_;
};

} /** namespace os */
} /** namespace jel */
