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
  virtual ~SerialWriterInterface() noexcept;
  /** Write a string of length_chars to the output. If the output is busy, transmission will be
   * overridden as soon as possible. */
  virtual void write(const char* cStr, const size_t length_chars) = 0;
  /** Write a single character to the output. If the output is busy, transmission will be overridden
   * as soon as possible. */
  virtual void write(const char c) = 0;
  /** Check if the transmitter is currently busy. If a nonzero timeout parameter is specified, this
   * call will block until the transmitter is no longer busy or the timeout expires. */
  virtual bool isBusy(const Duration& timeout) const = 0;
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
  virtual ~SerialReaderInterface() noexcept;
  /** Reads incoming data into the pointed to buffer, up to a maximum length (not including a null
   * terminator) of bufferLength_chars. Data is not null terminated, and is returned exactly as is
   * read from the input stream. A timeout must also be specified, if the timeout is exceeded then
   * the call will return regardless of the number of characters received.
   * @returns The number of characters received. */
  virtual size_t read(char* buffer, const size_t bufferLen, 
    const Duration& timeout) = 0;
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
  /** Lock the output stream. An AsyncLock object will be returned, which will prevent other threads
   * from using the output stream so long as it is extant. */
  AsyncLock lockStream(const Duration& timeout = Duration::max());
private:
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
  AsyncLock lockStream(const Duration& timeout);
private:
  std::unique_ptr<SerialReaderInterface> stream_;
  RecursiveMutex lock_;
};

/** @class MtIoStream 
 *  @brief Provides a threadsafe I/O interface comprised of an MtReader and MtWriter.
 * */
class AsyncIoStream : public MtReader, public MtWriter
{
public:
  AsyncIoStream(std::unique_ptr<SerialReaderInterface> reader, 
    std::unique_ptr<SerialWriterInterface> writer);
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
public:
  /** Escape sequences are prefixed to various formatter operations. */
  struct EscapeSequences
  {
    /** 'Control Sequence Introducer' - CSI escape sequence. */
    static constexpr char csi[] = "\e[";
  };
  /** @enum Csi
   *  @brief 'Control Sequence Introducer' - CSI command sequences. 
   *
   *  These sequences generally take the form 
   *  @code
   *    EscapeSequences::csi + [n] + Csi::[s]
   *  @endcode
   *  where:
   *    [n] is a numeric paramater modifying the sequence action.
   *    [s] is the CSI sequence.
   *  For example, a string composed of the members
   *  @code
   *    EscapeSequences::csi + 1 + Csi::CUD
   *  @endcode
   *  will send a cursor down (1 line) command.
   *
   *  It is recommended to use the csiComposer functions when producing CSI commands, although such
   *  operations can also be done manually by following the above form if desired. Note that not
   *  *all* CSI commands take exactly that form, some accept multiple numeric inputs. See the CSI
   *  section at https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_sequences for details.
   * */
  enum class Csi : char
  {
    CUU = 'A', /**< Cursor up. */
    CUD = 'B', /**< Cursor down. */
    CUF = 'C', /**< Cursor forward. */
    CUB = 'D', /**< Cursor back. */
    CNL = 'E', /**< Cursor next line. */
    CPL = 'F', /**< Cursor previous line. */
    CHA = 'G', /**< Cursor horizontal absolute. */
    /** Erase in display.
     * 0 = Erase from cursor to end of screen.
     * 1 = Erase from cursor to beginning of screen.
     * 2 = Clear entire screen. */
    ED = 'J', 
    /** Erase in line. Note - cursor position does not change.
     * 0 = Erase from cursor to EOL.
     * 1 = Erase from cursor to beginning of line.
     * 2 = Clear entire line. */
    EL = 'K',
    SU = 'S',  /**< Scroll up. */
    SD = 'T',  /**< Scroll down. */
    /** Set SGR parameter. See the Sgr structure definition for details. */
    SGR = 'm', 
    SCP = 's', /**< Save cursor position. */
    RCP = 'u' /**< Restore cursor position. */
  };
  /** @enum Sgr
   *  @brief 'Select Graphic Rendition' - SGR parameters used for coloring and formatting text
   *  output. 
   *  
   *  @note
   *    The SGR parameters should be converted from integer format to character format before use
   *    (i.e. Sgr::Reset should be printed as '0' not '\0'.)
   * */
  enum class Sgr : uint16_t
  {
    Reset = 0x30'00, /**< '0' */
    Bold = 0x31'00, /**< '1' */
    Faint = 0x32'00, /**< '2' */
    Italic = 0x33'00, /**< '3' */
    Underline = 0x34'00, /**< '4' */
    SlowBlink = 0x35'00, /**< '5' */
    RapidBlink = 0x36'00, /**< '6' */
    ReverseVideo = 0x37'00, /**< '7' */
    Conceal = 0x38'00, /**< '8' */
    DefaultFont = 0x31'30, /**< '10' */
    DefaultColorAndIntensity = 0x32'32, /**< '22' */
    UnderlineOff = 0x32'34, /**< '24' */
    BlinkOff = 0x32'35, /**< '25' */
    InverseOff = 0x32'37, /**< '27' */
    /** Must have the Color code overwrite the final character. */
    SetForegroundColor = 0x33'30, /**< '30' */
    DefaultForegroundColor = 0x33'39, /**< '39' */
    /** Must have the Color code overwrite the final character. */
    SetBackgroundColor = 0x34'30, /**< '40' */
    DefaultBackgroundColor = 0x34'39, /**< '49' */
    Framed = 0x35'31, /**< '51' */
    Encircled = 0x35'32, /**< '52' */
    Overlined = 0x35'33, /**< '53' */
    NotFramedOrEncircled = 0x35'34, /**< '54' */
    NotOverline = 0x35'35, /**< '55' */
  };
  /** @enum Color
   *  @brief ANSI color codes that can be used with SGR commands. 
   *  */
  enum class Color : char
  {
    black = '0',
    red = '1',
    green = '2',
    yellow = '3',
    blue = '4',
    magenta = '5',
    cyan = '6',
    white = '7' 
  };
  /** @enum EraseFunction
   *  @brief A list of erase functions for use with the eraseIn[xxx] functions.
   * */
  enum class EraseFunction : char 
  {
    toEndOf = '0',
    toBeginningOf = '1',
    all = '2'
  };
  /** 
   *  Writes an ANSI CSI sequence to a buffer that sets the cursor position using the given Csi
   *  cursor control sequence. Optionally, an offset value can be provided to move the cursor
   *  position multiple Csi operations worth of distance.
   * */
  String& cursorPosition(String& buffer, const Csi& controlSequence, uint8_t offset = 1)
  {
    buffer = EscapeSequences::csi;
    buffer.append(std::to_string(offset));
    buffer += static_cast<char>(controlSequence);
    return buffer;
  }
  /** 
   *  Writes an ANSI CSI sequence to a buffer that erases all or some of the screen from the current
   *  cursor position.
   * */
  String& eraseInScreen(String& buffer, const EraseFunction function)
  {
    buffer = EscapeSequences::csi;
    buffer += static_cast<char>(function);
    buffer += static_cast<char>(Csi::ED);
    return buffer;
  }
  /** 
   *  Writes an ANSI CSI sequence to a buffer that erases all or some of the current line the cursor
   *  is on. Erasing is done relative to the cursor position within the line.
   * */
  String& eraseInLine(String& buffer, const EraseFunction function)
  {
    buffer = EscapeSequences::csi;
    buffer += static_cast<char>(function);
    buffer += static_cast<char>(Csi::EL);
    return buffer;
  }
  /** 
   *  Writes an ANSI CSI sequence to a buffer that sets the foreground/text color to the given
   *  value.
   * */
  String& setForegroundColor(String& buffer, const Color color)
  {
    buffer = EscapeSequences::csi;
    char fgCode = static_cast<uint16_t>(Sgr::SetForegroundColor) >> 8;
    buffer += fgCode;
    buffer += static_cast<char>(color);
    buffer += static_cast<char>(Csi::SGR);
    return buffer;
  }
  /** 
   *  Writes an ANSI CSI sequence to a buffer that sets the background/screen color to the given
   *  value.
   * */
  String& setBackgroundColor(String& buffer, const Color color)
  {
    buffer = EscapeSequences::csi;
    char fgCode = static_cast<uint16_t>(Sgr::SetBackgroundColor) >> 8;
    buffer += fgCode;
    buffer += static_cast<char>(color);
    buffer += static_cast<char>(Csi::SGR);
    return buffer;
  }
  /** 
   *  Writes an ANSI CSI sequence to a buffer that either enables or disables a 'bold' effect on
   *  text.
   * */
  String& setBold(String& buffer, const bool enable)
  {
    char sgr[3];
    buffer = EscapeSequences::csi;
    if(enable) 
    { 
      sgr[0] = static_cast<uint16_t>(Sgr::Bold) >> 8; sgr[1] = '\0';
    }
    else 
    { 
      sgr[0] = static_cast<uint16_t>(Sgr::DefaultColorAndIntensity) >> 8; 
      sgr[1] = static_cast<uint16_t>(Sgr::DefaultColorAndIntensity) & 0xFF; sgr[2] = '\0';
    }
    buffer += sgr; buffer += static_cast<char>(Csi::SGR);
    return buffer;
  };
  /** 
   *  Writes an ANSI CSI sequence to a buffer that either enables or disables a 'underline' effect
   *  on text.
   * */
  String& setUnderline(String& buffer, const bool enable)
  {
    char sgr[3];
    buffer = EscapeSequences::csi;
    if(enable) 
    { 
      sgr[0] = static_cast<uint16_t>(Sgr::Underline) >> 8; sgr[1] = '\0';
    }
    else 
    { 
      sgr[0] = static_cast<uint16_t>(Sgr::UnderlineOff) >> 8; 
      sgr[1] = static_cast<uint16_t>(Sgr::UnderlineOff) & 0xFF; sgr[2] = '\0';
    }
    buffer += sgr; buffer += static_cast<char>(Csi::SGR);
    return buffer;
  };
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
  };
  static const Config defaultConfig;
  PrettyPrinter(const std::shared_ptr<MtWriter>& output, const Config& config = defaultConfig);
  /** Prints a string. */
  Status print(const String& string);
  /** Prints a null-terminated C-string. If a length parameter is provided, a call to std::strlen is
   * saved. The length parameter should be identical to what is returned by std::strlen(cStr); */
  Status print(const char* cStr, size_t length = 0);
  Config& editConfig() {return cfg_; }
private:
  std::shared_ptr<MtWriter> out_;
  Config cfg_;
  size_t clen_;
  size_t cidnt_;
};

} /** namespace os */
} /** namespace jel */
