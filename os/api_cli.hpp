/** @file os/api_cli.hpp
 *  @brief The jel system CLI interface.
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

typedef os::ObjectPool<String, config::stringPoolStringCount> JelStringPool;
typedef os::Status Status;
typedef std::basic_string<char> CliString;

/** Forward declarations. */
class Tokenizer;
class Vtt;
class ArgumentContainer;

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

class ArgumentContainer
{
public:
  size_t totalArguments() const { return numOfArgs_; }
  const Argument& operator[](size_t idx) const; 
  bool isArgListValid() const { return argListValid_; };
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
  std::unique_ptr<ArgListItem, void(*)(ArgListItem*)> firstArg;
  ArgumentContainer();
  ~ArgumentContainer() noexcept;
  Status generateArgumentList(const Tokenizer& tokens, const size_t discardThreshold, 
    const char* params);
  template<typename T>
  ArgListItem& appendListItem(T&& argval);
};

struct FormatSpecifer
{
  os::AnsiFormatter::Color color;
  bool isBold;
  bool isUnderlined;
  bool automaticNewline;
  bool automaticIndent;
  bool enablePrefixes;
};

struct CommandEntry;
class CliInstance;

class CommandIo
{
public:
  ~CommandIo() noexcept;
  CommandIo(const CommandIo&) = delete;
  CommandIo(CommandIo&&) = delete;
  CommandIo& operator=(const CommandIo&) = delete;
  CommandIo& operator=(CommandIo&&) = delete;
  Status print(const char* format, ...) __attribute__((format(printf, 2, 3)));
  size_t scan(char* buffer, size_t bufferLen, const Duration& timeout = Duration::max());
  bool getConfirmation(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  bool waitForContinue(const char* prompt = nullptr, const Duration& timeout = Duration::max());
  bool ioObjectisValid() const;
  FormatSpecifer fmt;
  const ArgumentContainer args;
private:
  friend CliInstance;
  Vtt& vtt;
  CommandIo(const Tokenizer& tokens, const CommandEntry& cmd, Vtt& vtt);
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

void startSystemCli(std::shared_ptr<os::AsyncIoStream>& io);
Status registerLibrary(const Library& library);

} /** namespace cli */
} /** namespace jel */
