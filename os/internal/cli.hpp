/** @file os/internal/cli.hpp
 *  @brief Internal definitions used by the jel CLI.
 *
 *  @detail
 *    Due to the complexity of the CLI implementation, prototypes and declarations for CLI objects
 *    are located in this header file.
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
#include <cassert>
/** jel Library Headers */
#include "os/api_cli.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace cli
{

/** @class Vtt
 *  @brief The Visual Text Terminal (VTT) provides input/output functionality.
 *
 * */
class Vtt 
{
public:
  struct Config
  {
    size_t historyDepth = config::cliHistoryDepth;
    size_t maxEntryLength = 128;
    size_t receiveBufferLength = 32;
    Duration pollingPeriod =  Duration::milliseconds(50);
  };
  Status write(const char* cStr, size_t length);
  size_t read(char* buffer, size_t bufferSize, const Duration& timeout); 
  Status prefix(const char* cStr);
  Vtt(const std::shared_ptr<os::AsyncIoStream>& ios);
  ~Vtt();
private:
  static constexpr size_t formatScratchBufferSize = 16; 
  class HistoryBuffer
  {
  public:
    HistoryBuffer();
    String& currentBuffer();
    String& operator++();
    String& operator--();
    String& operator++(int index);
    String& operator--(int index);
  private:
    size_t bpos_;
    os::JelStringPool::ObjectContainer buffers_[config::cliHistoryDepth];
    void nextpos();
    void prevpos();
  };
  std::shared_ptr<os::AsyncIoStream> ios_;
  os::PrettyPrinter printer_;
  Config cfg_;
  String wb_;
  String rxs_;
  const char* pfx_;
  size_t cpos_;
  size_t sst_;
  bool imode_;
  bool smode_;
  bool cshandled_;
  bool terminated_;
  bool bufedtd_;
  std::unique_ptr<char[]> fmts_; 
  HistoryBuffer hbuf_;
  size_t loadRxs(const Duration& timeout);
  size_t handleControlCharacters();
  bool parseEscapeSequence(const size_t csbeg);
  bool parseAsciiControl(const size_t  csbeg);
  bool terminateInput(const size_t csbeg);
  void regenerateOuput();
  void eraseSelection();
};

} /** namespace cli */
} /** namespace jel */
