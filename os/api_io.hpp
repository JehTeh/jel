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

/** jel Library Headers */
#include "os/api_common.hpp"

namespace jel
{

namespace os
{

class SerialWriterInterface
{
public:
  virtual void print(const char* cStr, const size_t length_chars);
  virtual void print(const char c);
  virtual bool pendUntilFree();
};

}
}
