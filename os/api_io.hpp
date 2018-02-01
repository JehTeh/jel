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

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"

namespace jel
{

namespace os
{

/** @class SerialWriterInterface
 *  @brief The SerialWriterInterface is implemented by drivers that support serial transmission of
 *  data.
 *  @note The SerialWriteInterface does not support multi-threaded operation. Simultaneous calls to
 *  Writer functions from multiple threads have undefined behavior.
 * */
class SerialWriterInterface
{
public:
  /** Write a string of length_chars to the output. If the output is busy, transmission will be
   * overridden as soon as possible. */
  virtual void write(const char* cStr, const size_t length_chars) const = 0;
  /** Write a single character to the output. If the output is busy, transmission will be overridden
   * as soon as possible. */
  virtual void write(const char c) const = 0;
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
  /** Reads incoming data into the pointed to buffer, up to a maximum length (not including a null
   * terminator) of bufferLength_chars. Data is not null terminated, and is returned exactly as is
   * read from the input stream. A timeout must also be specified, if the timeout is exceeded then
   * the call will return regardless of the number of characters received.
   * @returns The number of characters received. */
  virtual size_t read(char* buffer, const size_t bufferLength_chars, 
    const Duration& timeout) const = 0;
};

class AsyncWriter
{
  AsyncWriter(const std::unique_ptr<SerialWriterInterface> writer);
  Status write(const char* data, const size_t len) const;
};

class AsyncReader
{
  AsyncReader(const std::unique_ptr<SerialReaderInterface> reader);
};

} /** namespace os */
} /** namespace jel */
