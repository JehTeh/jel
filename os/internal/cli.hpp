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
#include <exception>
#include <cstdarg>
/** jel Library Headers */
#include "os/api_cli.hpp"
#include "os/api_threads.hpp"
#include "os/internal/indef.hpp"

namespace jel
{
namespace cli
{

void initCliPool();
os::AllocatorStatisticsInterface& cliPoolIf();

/** @class Vtt
 *  @brief The Visual Text Terminal (VTT) provides input/output functionality.
 *
 *  The Vtt is designed to take over an I/O interface and provide standard CLI interface features,
 *  which includes supporting control sequences like home/delete, selection emulation, and command
 *  history buffering. 
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
  Vtt(const std::shared_ptr<os::AsyncIoStream>& ios);
  ~Vtt() noexcept;
  Status write(const char* cStr, size_t length = 0);
  Status write(const char* format, va_list args);
  size_t read(char* buffer, size_t bufferSize, const Duration& timeout = Duration::max()); 
  size_t read(String& string, const Duration& timeout = Duration::max()); 
  Status prefix(const char* cStr);
  os::PrettyPrinter& printer() { return printer_; }
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
  std::unique_ptr<char[]> wrtbuf_;
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

/** @class Tokenizer 
 *  @brief Splits the provided input string into discrete tokens. The input string must not be
 *    modified while the Tokenizer is extant.
 *  
 *  @note
 *    Eventually refactor this to take std::unique_ptr<String>, but not until CLI/String class is
 *    using custom deleter/allocator scheme.
 * */
class Tokenizer
{
public:
  Tokenizer(String& str, const char delimiter = ' ');
  const char* operator[](size_t index) const;
  size_t count() const { return tc_; }
private:
  size_t tc_;
  String& s_;
};

struct ParamaterStringComponent
{
};

class ParameterString
{
public:
  struct Symbols
  {
    static constexpr char delimiters[] = "%";
    static constexpr char optionals[] = "?";
    static constexpr char ignored[] = " hljztL";
    static constexpr char specifiers_char[] = "c";
    static constexpr char specifiers_strings[] = "s";
    static constexpr char specifiers_signedInts[] = "id";
    static constexpr char specifiers_unsignedInts[] = "u";
    static constexpr char specifiers_float[] = "f";
  };
  struct Parameter
  {
    static constexpr size_t maxFormatStringLength = 8;
    bool isOptional;
    Argument::Type type;
    char formatString[maxFormatStringLength];
  };
  ParameterString(const char* pstr);
  Parameter operator[](size_t index);
  size_t optionalCount() { return optcnt_; }
  size_t totalCount() { return pcnt_; }
private:
  size_t pcnt_;
  size_t optcnt_;
  const char* const s_;
};

class CliInstance
{
public:
  static constexpr size_t cliThreadStackSize_Words = 512;
  static constexpr os::Thread::Priority cliThreadPriority = os::Thread::Priority::low;
  static CliInstance* activeCliInstance;
  CliInstance(std::shared_ptr<os::AsyncIoStream>& io);
  ~CliInstance() noexcept;
  static Status registerLibrary(const Library& lib);
private:
  friend ArgumentContainer;
  struct LibrariesListItem
  {
    const Library* libptr;
    std::unique_ptr<LibrariesListItem> next;
    LibrariesListItem() : libptr(nullptr), next(nullptr) {}
    LibrariesListItem(const Library& lib) : libptr(&lib), next(nullptr) {}
  };
  AccessPermission aplvl_ = AccessPermission::unrestricted;
  os::Thread* tptr_;
  std::unique_ptr<String> istr_;
  std::unique_ptr<Vtt> vtt;
  LibrariesListItem libList_;
  const Library* alptr_;
  const CommandEntry* acptr_;
  bool handleSpecialCommands(Tokenizer& tokens);
  bool lookupLibrary(const char* name);
  bool lookupCommand(const char* name);
  int executeCommand(Tokenizer& tokens);
  bool doesAplvlMeetSecRequirment(const AccessPermission& lvlToCheckAgainst);
  void cliThread(std::shared_ptr<os::AsyncIoStream>* io);
  void printError(const os::AnsiFormatter::Color color, const char* format, ...)
    __attribute__((format(printf, 3, 4)));
  static void cliThreadDispatcher(std::shared_ptr<os::AsyncIoStream>* io);
};

} /** namespace cli */
} /** namespace jel */
