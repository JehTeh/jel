/** @file os/api_cli.hpp
 *  @brief The jel system CLI interface.
 *
 *  @detail
 *    The jel Command Line Interface (CLI) is designed to provide a robust application interface for
 *    any system implementing the jel. At a high level, its features are as follows:
 *      -ANSI/VT100 terminal control and emulation. The user facing input side of the CLI performs
 *      the following when connected to an emulator supporting ANSI control sequences:
 *        -Full local (to microcontroller) line echoing and editing, including support for cursor
 *        movement, insertion, and multicharacter selection. Unfortunately does not support VIM. Use
 *        the left/right arrow keys (optionally plus shift) to move the cursor on the current input
 *        line. The Insert, Home, Delete, End and Page Up and Page Down keys are all supported.
 *        -Input history recollection and editing. Pressing the up/down arrow keys allows cycling
 *        through the available history buffers, which include the current line being edited and
 *        lines previously input (i.e., 'Enter' was pressed). 
 *        -Implements a PrettyPrinter class for all output to the user, allowing automatic
 *        line-break insertion, formatting removal, and selection between LFCR and LF style
 *        line-endings.
 *      -A simplified application facing interface allowing for easy command function creation and
 *      prebuilt argument parsing (see example). Each command accepts only a single argument, the
 *      CommandIo& object, which provides the following:
 *        -Output functionality, including printf formatter support for outputting via the
 *        standard CLI channel (internally buffered, so there is a maximum allowable printing
 *        length). Arbitrarily long messages can also be output but may not include printf style 
 *        formatters.
 *        -Output formatting can be adjusted on the fly by modifying the io.fmt object. This
 *        includes things like automatic newline insertion, ANSI code stripping, output color, etc.
 *        This is an RAII style object, when the command is finished executing formatting returns to
 *        the default (as such, all commands can assume the formatting is setup in a default manner
 *        upon entry).
 *        -Specialized and generic input functionality, i.e. raw input can be read directly from the
 *        terminal emulator or special functions for reading an integer, float, etc. can be used.
 *        Furthermore, common operations such as pending on user input/confirmation (like hitting
 *        the 'Enter' key, answering 'yes/no') are available.
 *        -Arguments received by the CLI are automatically parsed and containerized. These can be
 *        accessed inside the io.args object array style ([]) or via iterator. Each argument object
 *        includes information about its type and the value as parsed by the CLI.
 *      -Restricted and unrestricted permission levels for commands. Commands can be configured so
 *      only after 'logging in' to the CLI with the appropriate username and password can they be
 *      seen in the help menu and executed.
 *
 *  @todo
 *    -Improve parsing of optional sequences. Currently an optional command argument included after
 *    another optional command argument necessitates that the first argument be entered. This should
 *    not always be required; for example if an input argument does not positively match with the
 *    first optional argument, it should be evaluated against the next one, etc until either a
 *    possible match is found or no argument is matched. This is mostly so commands like 'os reboot'
 *    do not require a '0' argument for a timeout before a '-f' argument for forcing immediate
 *    shutdown.
 *    -Add escape key recognition to various commandIo functions (i.e., on waitForConfirmation(),
 *    instead of requiring a timeout to return a false an escape key can be hit.)
 *    -Implement login functionality, instead of current stubs. Passwords should be pre-hashed so
 *    they are not recognizable in the binary, need to figure out best way to do this.
 *    -Evaluate/prototype compile time command sorting and storage (possibly in a trie) to allow
 *    rapid lookups. Not strictly necessary, but would be superior to the current naive
 *    implementation. Need to ensure it is done at compile time, however, to maintain low memory
 *    footprint.
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
#include "os/api_common.hpp"
#include "os/api_time.hpp"
#include "os/api_io.hpp"
#include "os/api_allocator.hpp"
#include "os/api_config.hpp"

namespace jel
{
namespace cli
{

typedef ObjectPool<String> JelStringPool;
typedef Status Status;
typedef std::basic_string<char> CliString;

/** Forward declarations. */
class Tokenizer;
class Vtt;
class ArgumentContainer;

/** @class Argument
 *  @brief An Argument is a container for a single argument parsed by the CLI parser.
 *
 *  Arguments are automatically sorted and parsed based on a commands parameters string. Arguments
 *  stored in an Argument object include a type enumeration and a value enumeration, along with some
 *  access functions (although the value can be accessed directly). Generally it is recommended the
 *  stored value be accessed via the access functions in place of directly through the union, as
 *  this can catch a type mismatch in debug builds.
 *  */
struct Argument
{
public: 
  enum class Type
  {
    int64_t_,
    uint64_t_,
    double_,
    string_,
    invalid,
  };
  union Value
  {
    int64_t int64_t_;
    uint64_t uint64_t_;
    double double_;
    JelStringPool::ObjectContainer string_;
    Value(int64_t i) { int64_t_ = i; }
    Value(uint64_t ui) { uint64_t_ = ui; }
    Value(double dbl) { double_ = dbl; }
    Value(JelStringPool::ObjectContainer& strc) : string_{std::move(strc)} {  }
    ~Value() noexcept {};
  };
  const Type type;
  const Value value;
  ~Argument() noexcept { if(type == Type::string_) { value.string_.~ObjectContainer(); }}
  const int64_t& asInt() const { assert(type == Type::int64_t_); return value.int64_t_; }; 
  const uint64_t& asUInt() const { assert(type == Type::uint64_t_); return value.uint64_t_; };
  const double& asDouble() const { assert(type == Type::double_); return value.double_; };
  const String& asString() const { assert(type == Type::string_); return *value.string_.stored(); };
private:
  friend ArgumentContainer;
  Argument(const int64_t& int_) : type(Type::int64_t_), value{int_ } { }
  Argument(const uint64_t& uint_) : type(Type::uint64_t_), value{uint_ } { }
  Argument(const double& dbl_) : type(Type::double_), value{dbl_} { }
  Argument(JelStringPool::ObjectContainer& str_) : type(Type::string_), value{str_} { }
};

class CommandIo;
class CliInstance;

/** @class ArgumentContainer
 *  @brief The ArgumentContainer stores any parsed Argument objects that were read by the CLI before
 *  calling the command.
 *
 *  @note Only a total of totalArguments() can be accessed in the container. Accesses to an argument
 *  index via the [] operator greater than or equal to the totalArguments() value will trigger an
 *  assertion on debug builds, and result in undefined behavior on production builds.
 * */
class ArgumentContainer
{
private:
  friend CommandIo;
  enum class Status
  {
    success,
    tooManyArguments,
    insufficientArguments,
    maxGlobalArgsExceeded,
    argumentTypeMismatch,
    noFreeStringsAvailable,
  };
  struct ArgListItem
  {
    Argument arg;
    std::unique_ptr<ArgListItem, void(*)(ArgListItem*)> next;
    template<typename T>
    ArgListItem(T&& argval) : arg(argval), next{nullptr, nullptr} { }
  };
  bool argListValid_;
  size_t numOfArgs_;
  CliInstance* cli_;
  std::unique_ptr<ArgListItem, void(*)(ArgListItem*)> firstArg;
  ArgumentContainer();
  ArgumentContainer(CliInstance* cli, const Tokenizer& tokens, const size_t discThresh,
    const char* params);
  ~ArgumentContainer() noexcept;
  Status generateArgumentList(CliInstance* cli, const Tokenizer& tokens,
    const size_t discardThreshold, const char* params);
  template<typename T>
  ArgListItem& appendListItem(T&& argval);
public:
  class ConstIterator
  {
    public:
      ConstIterator& operator++() { this->ptr = ptr->next.get(); return *this; }
      ConstIterator operator++(int) { auto temp(*this); this->ptr = ptr->next.get(); return temp; }
      bool operator!=(ConstIterator& rhs) 
        { if(this->ptr != rhs.ptr) { return true; } return false; }
      const Argument& operator*() { return ptr->arg; }
      const Argument& operator->() { return ptr->arg; }
    private:
      friend ArgumentContainer;
      constexpr ConstIterator(const ArgListItem* baseItem) : ptr(baseItem)
        { ptr = baseItem; }
      const ArgListItem* ptr;
  };
  size_t totalArguments() const { return numOfArgs_; }
  const Argument& operator[](size_t idx) const; 
  bool isArgListValid() const { return argListValid_; };
  ConstIterator begin() const { return ConstIterator{firstArg.get()}; }
  ConstIterator end() const { return ConstIterator{nullptr}; };
};

/** @struct FormatSpecifier
 *  @brief Formatting object used to control output formatting for printing functions in the
 *  CommandIo object. */
struct FormatSpecifer
{
  AnsiFormatter::Color color = AnsiFormatter::Color::white;
  bool isBold = false;
  bool isUnderlined = false;
  bool automaticNewline = true;
  bool enablePrefixes = false;
  bool disableAllFormatting = false;
};

struct CommandEntry;
class CliInstance;

/** @class CommandIo
 *  @brief The CommandIo object is passed into a user defined CLI command and provides access to any
 *  parsed arguments and advanced printing functionality.
 * */
class CommandIo
{
public:
  ~CommandIo() noexcept;
  CommandIo(const CommandIo&) = delete;
  CommandIo(CommandIo&&) = delete;
  CommandIo& operator=(const CommandIo&) = delete;
  CommandIo& operator=(CommandIo&&) = delete;
  /** Locks the output channel against asynchronous access. */
  AsyncLock lockOuput(const Duration& timeout = Duration::max());
  /** Prints a printf style formatting string with arguments. Note that the total length of the
   * string, after argument expansions, must be less than the config::cliMaximumStringLength.
   * Multiple successive calls to print(...) are acceptable.
   * */
  Status print(const char* format, ...) __attribute__((format(printf, 2, 3)));
  /** Prints an arbitrary length string. Printf style formatters are not supported. */
  Status constPrint(const char* cString, const size_t length = 0);
  /** Prints an arbitrary length string. Printf style formatters are not supported. */
  Status constPrint(String& string);
  /** Returns the current length of the line being printed. */
  size_t currentLineLength() const;
  /** Returns a constant reference to the current printer configuration. */
  const PrettyPrinter::Config& printerConfig() const;
  /** Reads in data from the CLI input in a manner identical to scanf and returns either when data
   * has been successfully read or when the timeout expires. */
  size_t scan(char* buffer, size_t bufferLen, const Duration& timeout = Duration::max());
  /** Prompts the CLI user for confirmation. Confirmation can take the form 'y/Y n/N','Yes/No', etc.
   * This command will return either after user input is parsed successfully or a timeout occurs. If
   * the user confirmed (with 'yes') the true is returned, if they did not confirm or a timeout
   * occurs then false is returned. */
  bool getConfirmation(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  /** Waits for a 'continue' signal from the user. This typically means they must press enter. If no
   * continue signal is entered within the timeout specified, this function returns false. */
  bool waitForContinue(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  /** Prompts the user for a signed integer input. This function will only return once a signed
   * integer is successfully parsed. In the event of a timeout expiration, an exception will be
   * thrown.
   * @throws ExceptionCode::cliArgumentReadTimeout in the event no valid data is read before the
   * timeout occurs. */
  int64_t readSignedInt(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  /** Prompts the user for an unsigned integer input. This function will only return once an 
   * unsigned integer is successfully parsed. In the event of a timeout expiration, an exception
   * will be thrown.
   * @throws ExceptionCode::cliArgumentReadTimeout in the event no valid data is read before the
   * timeout occurs. */
  uint64_t readUnsignedInt(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  /** Prompts the user for a floating point input. This function will only return once a float 
   * is successfully parsed. In the event of a timeout expiration, an exception will be thrown.
   * @throws ExceptionCode::cliArgumentReadTimeout in the event no valid data is read before the
   * timeout occurs. */
  double readDouble(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  /** The current formatting configuration to use when printing. */
  FormatSpecifer fmt;
  /** A pointer to the CommandEntry value for this specific command. */
  const CommandEntry* cmdptr;
  /** The arguments parsed by the CLI parser. */
  const ArgumentContainer args;
private:
  static constexpr size_t formatBufferSize = 16;
  friend CliInstance;
  Vtt& vtt_;
  CliInstance* cli_;
  const bool isValid_;
  FormatSpecifer preIoPpConfig_;
  CommandIo(CliInstance* cli, const Tokenizer& tokens, const CommandEntry& cmd, Vtt& vtt);
  void printFormatters();
};

enum class AccessPermission : uint8_t
{
  unrestricted = 0,
  restricted
};

/** @struct CommandEntry
 *  @brief An entry for a single command into a library.
 *  
 *  A CommandEntry is used to interface an external command to the CLI module. They are composed of
 *  various paramaters required by the CLI for parsing the command and presenting it to the user.
 *  Typically, multiple CommandEntries are grouped together into a table which is referenced by a
 *  Library. For example,
 *  @code
 *    const CommandEntry moduleCmds[] =
 *    {
 *      {
 *        "command1", &cmd1Function, "%d",
 *        "This is the command 1 help string.",
 *        AccessPermission::unrestricted, nullptr
 *      },
 *      {
 *        "command2", &cmd2Function, "%d",
 *        "This is the command 2 help string.",
 *        AccessPermission::unrestricted, nullptr
 *      },
 *      ...
 *    };
 *  @endcode
 *  A CommandEntry is looked up within a Library by name. Because tokenization is performed on space
 *  characters, a CommandEntry name must not contain any spaces or whitespace characters. The
 *  permissible characters for a CommandEntry name include 0-9, A-Z, a-z, and visible special
 *  characters such as !,~,@,#,$,%,^,&,*,(,),-,+ etc (essentially any visible ASCII character in the
 *  range 0x21-0x7E).
 *
 *  When calling a command, the library name must be prefixed and seperated from the command name by
 *  one or more space characters.
 *
 *  @note
 *    Commands within a library cannot share the same name. If multiple commands share the same
 *    name, only the first command as it appears in the group will be executed.
 * */
struct CommandEntry
{
  using FunctionPointer = int32_t (*)(CommandIo& io);
  const char* name;
  const FunctionPointer function;
  const char* parameters;
  const char* helpString;
  const AccessPermission securityLevel;
  const void* extendedParamters;
};

/** @struct Library
 *  @brief A series of command entries collected into a single library. 
 *
 *  Libraries are used to group multiple CommandEntries into a single module, for presentation to
 *  the CLI. For example,
 *  @code
 *    const Library moduleLib =
 *    {
 *      "module1", 
 *      "Module 1 help string.",
 *      sizeof(moduleCmds)/sizeof(CommandEntry),
 *      moduleCmds
 *    };
 *  @endcode
 *  Command lookup is performed by the CLI on a per-library basis, which requires each library to
 *  have a unique name. Furthermore, library names must start only with alphanumeric characters
 *  (0-9, A-Z, a-z) and cannot contain any spaces or special characters such as *,&,-, etc. Library
 *  names containing special characters are reserved for the CLI implementation, as such they may be
 *  accepted but must not be present in application library names. Furthermore, the following
 *  library names are reserved and cannot be used by an application library:
 *    -tty -cli -os -hw -jel
 *  */
struct Library
{
  const char* name;
  const char* helpString;
  const size_t numberOfEntries;
  const CommandEntry* entries;
  using CIt = ConstIterator<CommandEntry>;
  CIt begin() const { return CIt{entries}; }
  CIt end() const { return CIt{entries, numberOfEntries}; }
};

/** This is called by the jel on startup. It should not be used by the application. */
void startSystemCli(std::shared_ptr<AsyncIoStream>& io);
/** Any application libraries must be registered with the CLI before use by calling this function.
 * */
Status registerLibrary(const Library& library);

} /** namespace cli */
} /** namespace jel */
