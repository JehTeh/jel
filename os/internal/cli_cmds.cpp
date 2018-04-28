/** @file os/internal/cli_cmds.cpp
 *  @brief os module specific CLI commands.
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

//Define this to disable some warnings from linting tools.
#define _CRT_SECURE_NO_WARNINGS

/** C/C++ Standard Library Headers */
#include <cassert>
#include <cstring>
/** jel Library Headers */
#include "os/internal/indef.hpp"
#include "os/api_cli.hpp"
#include "os/api_allocator.hpp"
#include "os/api_threads.hpp"
#include "hw/api_exceptions.hpp"
#include "hw/api_wdt.hpp"

namespace jel
{

size_t printCpuUse(cli::CommandIo& io, char* pBuf, const size_t pBufLen, const bool showStack);
size_t printMemUse(cli::CommandIo& io, char* pBuf, const size_t pBufLen);

int32_t cliCmdBuildInfo(cli::CommandIo& io);
int32_t cliCmdMemuse(cli::CommandIo& io);
int32_t cliCmdCpuuse(cli::CommandIo& io);
int32_t cliCmdStackuse(cli::CommandIo& io);
int32_t cliCmdReadclock(cli::CommandIo& io);
int32_t cliCmdReboot(cli::CommandIo& io);
int32_t cliCmdRmon(cli::CommandIo& io);
int32_t cliCmdEnableTestLib(cli::CommandIo& io);

const cli::CommandEntry cliCommandArray[] =
{
  {
    "buildinfo", cliCmdBuildInfo, "",
    "Prints jel system build information.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "memuse", cliCmdMemuse, "",
    "Reports the current memory usage of various heaps and memory pools in the system.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "cpuuse", cliCmdCpuuse, "%?u",
    "Reports the current CPU usage and other thread statistics. By default, the output is "
    "refreshed every 3 seconds. A custom refresh rate, in seconds, can optionally be included to "
    "change this behaviour.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "stackuse", cliCmdStackuse, "%?s",
    "Takes a snapshot of the current thread stack usage. Note that this can cause issues in "
    "systems that require precision timing, as the scheduler may be paused for a while. To "
    "ensure that you have actually read this message, call this command with a '-c' parameter.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "time", cliCmdReadclock, "%?d",
    "Reads the current system clock. Automatically refreshes at a default rate of once per "
    "second.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "reboot", cliCmdReboot, "%?u%?s",
    "Restarts the processor/MCU. Depending on the hardware platform, this is at minimum a software "
    "reset but if at all possible a full system reset. Two optional arguments can be specified:\n"
    "\t[0] (unsigned integer): Time in seconds to delay before restarting. This defaults to five "
    "seconds and allows the countdown to be aborted if desired.\n"
    "\t[1] (string): If '-f' (force) is passed, reset is performed immediately without confirmation"
    ". It is recommended this option not be used on systems sensitive to an immediate shutdown.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "rmon", cliCmdRmon, "%?s%?u",
    "Displays the resource monitoring utility. The Resource MONitor (RMON) provides information "
    "about all registered system resources, such as memory heaps/pools, thread statistics, etc. "
    "Two parameters are optionally accepted by the command. These are:\n"
    "\t[0] String: If a '-s' flag is provided, stack usage information will be included. If '-n' "
    "is provided, no stack usage is included. '-n' is the default.\n"
    "\t[1] Unsigned integer: Refresh time in seconds. Defaults to 3.\n"
    "Note that monitoring thread stack usage can have a significant impact on the RTOS scheduler "
    "and should likely be avoided when the system is under hard real-time constraints and heavy "
    "CPU load.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "etl", cliCmdEnableTestLib, "",
    "Enables the os module testing CLI command library.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
};

extern const cli::Library cliCmdLib =
{
  "os",
  "The os library includes commands relating to system resource monitoring and control.\n",
  sizeof(cliCommandArray)/sizeof(cli::CommandEntry),
  cliCommandArray
};

int32_t cliCmdBuildInfo(cli::CommandIo& io)
{
  io.fmt.isBold = true;
  io.constPrint("JEL (JT's Embedded Libraries) Info:\r\n");
  io.constPrint("Build Date: "); io.fmt.isBold = false;
  io.constPrint(jelBuildDateString); 
  io.constPrint("@"); io.constPrint(jelBuildTimeString); io.constPrint("\r\n"); 
  io.fmt.isBold = true;
  io.constPrint("GCC/G++ Version: \r\n\t"); io.fmt.isBold = false;
  io.constPrint(jelCompilerVersionString); io.constPrint("\r\n");
  io.fmt.isBold = true;
#ifdef __OPTIMIZE__
  io.fmt.color = AnsiFormatter::Color::green;
  io.constPrint("This build is an optimized build (-O1 or greater).\r\n");
#else
  io.fmt.color = AnsiFormatter::Color::yellow;
  io.constPrint("This build is not an optimized build (-O0).\r\n");
#endif
  io.fmt.isBold = false;
  io.fmt.color = AnsiFormatter::Color::default_;
#if defined(HW_TARGET_RM57L843)
  io.constPrint("Built for the RM57L843 processor.\r\n");
#elif defined(HW_TARGET_STM32F302RCT6)
  io.constPrint("Built for the STM32F302RCT6 processor.\r\n");
#elif defined(HW_TARGET_TM4C1294NCPDT)
  io.constPrint("Built for the TM4C1294NCPDT processor.\r\n");
#elif defined(HW_TARGET_TM4C123GH6PM)
  io.constPrint("Built for the TM4C123GH6PM processor.\r\n");
#else
  io.constPrint("This jel has not been built for a supported/recognized processor.\r\n");
#error "No Target defined."
#endif
  io.fmt.isBold = true;
  io.constPrint("Runtime Config: "); io.fmt.isBold = false;
  io.constPrint(config::jelRuntimeConfiguration.name); io.constPrint("\r\n");
  return 0;
}

int32_t cliCmdMemuse(cli::CommandIo& io)
{
  const auto *aeptr = AllocatorStatisticsInterface::systemAllocator();
  io.fmt.automaticNewline = false;
  while(aeptr != nullptr)
  {
    const auto* sptr = aeptr->statsIf;
    io.print(
      "Allocator %s:\r\n\tFree Space: %uB\r\n\tMin. Free Space: %uB\r\n\tTotal Size: %uB\r\n",
      sptr->name(), sptr->freeSpace_Bytes(), sptr->minimumFreeSpace_Bytes(), 
      sptr->totalSpace_Bytes());
    io.print("\tAllocations: %u\r\n\tDeallocations: %u\r\n",
      sptr->totalAllocations(), sptr->totalDeallocations());
    aeptr = aeptr->next;
  }
  io.print(
    "jel String pool use:\r\n\tFree items: %u\r\n\tMin. Free Items: %u\r\n\tTotal Items: %u\r\n", 
    jelStringPool->itemsInPool(), jelStringPool->minimumItemsInPool(), 
    jelStringPool->maxItemsInPool());
  return 0;
}

int32_t cliCmdCpuuse(cli::CommandIo& io)
{
#ifndef ENABLE_THREAD_STATISTICS
  io.print("Thread statistics are not enabled on this build.");
#else
  io.fmt.automaticNewline = false;
  Duration pollPeriod = Duration::seconds(3);
  if(io.args.totalArguments() > 0)
  {
    if(Duration::seconds(io.args[0].asUInt()) > Duration::zero())
    {
      pollPeriod = Duration::seconds(io.args[0].asUInt());
    }
    else
    {
      pollPeriod = Duration::seconds(1);
    }
  }
  io.print("Displaying system CPU usage (%llds refresh). Press enter to exit.\r\n", 
    pollPeriod.toSeconds());
  constexpr size_t pBufLen = 32;
  char pBuf[pBufLen];
  while(true)
  {
    {
      auto lg = io.lockOuput();
      size_t lc = 0;
      io.constPrint(AnsiFormatter::Erase::toEndOfScreen);
      lc += printCpuUse(io, pBuf, pBufLen, false);
      if(io.waitForContinue("Press 'enter' to exit.", pollPeriod))
      {
        break;
      }
      for(size_t i = 0; i < lc; i++)
      {
        io.constPrint(AnsiFormatter::Cursor::up);
      }
    }
  }
#endif
  return 0;
}

size_t printCpuUse(cli::CommandIo& io, char* pBuf, const size_t pBufLen, const bool showStack)
{
  size_t lc = 0;
  io.fmt.isBold = true;
  if(showStack)
  {
    io.constPrint(
      " Handle       | Thread Name          | Total Time (ms) | CPU(%) | Min. Stack (B)\r\n");
  }
  else
  {
    io.constPrint(
      " Handle         | Thread Name             | Total Time (ms)         | CPU(%)    \r\n");
  }
  io.fmt.isBold = false;
  lc++;
  for(const auto& tip : Thread::registry())
  {
    const Thread::ThreadInfo& ti = *tip;
    if(tip == nullptr)
    {
      return lc;
    }
    std::sprintf(pBuf, "%p", ti.handle_);
    if(showStack) { io.print(" %-13s|", pBuf); }
    else { io.print(" %-15s|", pBuf); }
    if(ti.isDeleted_)
    {
      constexpr char ds[] = " (deleted)";
      constexpr size_t dsl = constStringLen(ds);
      std::strncpy(pBuf, ti.name_, pBufLen - dsl);
      std::strcat(pBuf, ds);
      if(showStack) { io.print(" %-21s|", pBuf); }
      else { io.print(" %-24s|", pBuf); }
    }
    else
    {
      if(showStack) { io.print(" %-21s|", ti.name_); }
      else { io.print(" %-24s|", ti.name_); }
    }
    std::sprintf(pBuf, "%lld", ti.totalRuntime_.toMilliseconds());
    if(showStack) { io.print(" %-16s|", pBuf); }
    else { io.print(" %-24s|", pBuf); }
    std::sprintf(pBuf, "%.2f",
      static_cast<float>(ti.totalRuntime_.toMilliseconds()) / 
      static_cast<float>(Timestamp{SteadyClock::now()}.toDuration().toMilliseconds()) * 100.0);
    if(showStack) { io.print(" %-7s|", pBuf); }
    else { io.print(" %-10s", pBuf); }
    if(showStack)
    {
      if(ti.isDeleted_)
      {
        std::sprintf(pBuf, "%u", ti.minStackBeforeDeletion_bytes_);
        io.print(" %-14s", pBuf);
      }
      else
      {
        std::sprintf(pBuf, "%lu", uxTaskGetStackHighWaterMark(ti.handle_) * 4);
        io.print(" %-14s", pBuf);
      }
    }
    io.print("\r\n");
    lc++;
  }
  return lc;
}

size_t printMemUse(cli::CommandIo& io, char* pBuf, const size_t pBufLen)
{
  size_t lc = 0;
  io.fmt.isBold = true;
  io.constPrint(
    " Heap           | Free (B)   | Min. Free (B) | Size (B)   | Allocs.  | Deallocs.\r\n");
  io.fmt.isBold = false;
  lc++;
  const auto *alloc = AllocatorStatisticsInterface::systemAllocator();
  while(alloc != nullptr)
  {
    const auto* stats = alloc->statsIf;
    io.print(" %-15s|", stats->name());
    std::snprintf(pBuf, pBufLen, "%u", stats->freeSpace_Bytes());
    io.print(" %-11s|", pBuf);
    std::snprintf(pBuf, pBufLen, "%u", stats->minimumFreeSpace_Bytes());
    io.print(" %-14s|", pBuf);
    std::snprintf(pBuf, pBufLen, "%u", stats->totalSpace_Bytes());
    io.print(" %-11s|", pBuf);
    std::snprintf(pBuf, pBufLen, "%u", stats->totalAllocations());
    io.print(" %-9s|", pBuf);
    std::snprintf(pBuf, pBufLen, "%u", stats->totalDeallocations());
    io.print(" %-9s", pBuf);
    io.print("\r\n");
    lc++;
    alloc = alloc->next;
  }
  return lc;
}

int32_t cliCmdStackuse(cli::CommandIo& io)
{
  if(io.args.totalArguments() < 1)
  {
    io.print("Please read the command help before using this command.\r\n");
    return 1;
  }
  if(io.args[0].asString() != "-c")
  {
    io.print("Please read the command help before using this command.\r\n");
    return 2;
  }
#ifndef ENABLE_THREAD_STATISTICS
  io.print("Thread statistics tracking must be enabled to use this command.\r\n");
  return 3;
#else
  io.fmt.isBold = true;
  io.constPrint(
    " Handle         | Thread Name             | Min Stack Free (B) | Stack Size (B)\r\n");
  io.fmt.isBold = false;
  constexpr size_t pBufLen = 32;
  char pBuf[pBufLen];
  for(const auto& tip : Thread::registry())
  {
    Thread::ThreadInfo& ti = *tip;
    if(tip == nullptr)
    {
      return 1;
    }
    std::sprintf(pBuf, "%p", ti.handle_);
    io.print(" %-15s|", pBuf);
    if(ti.isDeleted_)
    {
      constexpr char ds[] = " (deleted)";
      constexpr size_t dsl = constStringLen(ds);
      std::strncpy(pBuf, ti.name_, pBufLen - dsl);
      std::strcat(pBuf, ds);
      io.print(" %-24s|", pBuf);
    }
    else
    {
      io.print(" %-24s|", ti.name_);
    }
    if(ti.isDeleted_)
    {
      std::sprintf(pBuf, "%d", ti.minStackBeforeDeletion_bytes_);
      io.print(" %-19s|", pBuf);
    }
    else
    {
      size_t minFree_Words = uxTaskGetStackHighWaterMark(ti.handle_);
      std::sprintf(pBuf, "%d", minFree_Words * 4);
      io.print(" %-19s|", pBuf);
    }
    std::sprintf(pBuf, "%d", ti.maxStack_bytes_);
    io.print(" %-14s", pBuf);
    io.print("\r\n");
  }
  return 0;
#endif
}

int32_t cliCmdReadclock(cli::CommandIo& io)
{
  io.fmt.automaticNewline = false;
  Duration pollPeriod = Duration::seconds(1);
  if(io.args.totalArguments() > 0)
  {
    if(Duration::seconds(io.args[0].asUInt()) > Duration::zero())
    {
      pollPeriod = Duration::seconds(io.args[0].asUInt());
    }
    else
    {
      pollPeriod = Duration::seconds(1);
    }
  }
  io.print("Displaying system time (%llds refresh). Press enter to exit.\r\n", 
    pollPeriod.toSeconds());
  while(true)
  {
    {
      auto lg = io.lockOuput();
      io.print(AnsiFormatter::Erase::entireLine);
      auto d = Duration{SteadyClock::now() - SteadyClock::zero()};
      io.print("System Clock: %llus (%lldus)\r", d.toSeconds(), d.toMicroseconds());
    }
    if(io.waitForContinue("", pollPeriod))
    {
      break;
    }
  }
  return 0;
}

int32_t cliCmdReboot(cli::CommandIo& io)
{
  Duration countdown{Duration::seconds(5)};
  bool forceRestart = false;
  for(const auto& i : io.args)
  {
    if(i.type == cli::Argument::Type::uint64_t_)
    {
      countdown = Duration::seconds(i.asUInt());
    }
    else if(i.type == cli::Argument::Type::string_)
    {
      if(i.asString() == "-f")
      {
        forceRestart = true;
      }
      else
      {
        io.fmt.color = AnsiFormatter::Color::yellow;
        io.print("'%s' is not a supported argument.\n", i.asString().c_str());
        return 1;
      }
    }
    else
    {
      io.fmt.color = AnsiFormatter::Color::red;
      io.print("Illegal argument detected.\n");
      return 2;
    }
  }
  if(forceRestart)
  {
    hw::wdt::WdtController::systemReset();
  }
  else
  {
    io.fmt.automaticNewline = false;
    io.print("The system will reboot in %llu seconds. Continue (y/n)?\r\n",
      countdown.toSeconds());
    if(!io.getConfirmation(" "))
    {
      io.fmt.color = AnsiFormatter::Color::brightBlue;
      io.print("Reset aborted.\r\n");
      return 0;
    }
    while(true)
    {
      if(countdown.toSeconds() <= 0)
      {
        break;
      }
      if(countdown.toSeconds() <= 7) { io.fmt.color = AnsiFormatter::Color::brightYellow; }
      if(countdown.toSeconds() <= 3) { io.fmt.color = AnsiFormatter::Color::brightRed; }
      io.print("Restarting system in %llu seconds (press enter to abort)...\r", 
        countdown.toSeconds());
      if(io.waitForContinue("", Duration::seconds(1)))
      {
        io.fmt.color = AnsiFormatter::Color::brightBlue;
        io.print("Reset aborted.\r\n");
        return 0;
      }
      io.print(AnsiFormatter::Erase::entireLine);
      countdown -= Duration::seconds(1);
    }
    io.print("\n");
    hw::wdt::WdtController::systemReset();
  }
  return 0;
}

int32_t cliCmdRmon(cli::CommandIo& io)
{
  bool printStack = false;
  Duration pollPeriod = Duration::seconds(3);
  if(io.args.totalArguments() >= 1)
  {
    if(io.args[0].asString() == "-s")
    {
      printStack = true;
    }
    else if(io.args[0].asString() == "-n")
    {
      printStack = false;
    }
    else
    {
      io.print("'%s' is not a supported parameter. See command help for details.\n", 
        io.args[0].asString().c_str());
    }
  }
  if(io.args.totalArguments() == 2)
  {
    pollPeriod = Duration::seconds(io.args[1].asUInt());
  }
  io.fmt.automaticNewline = false;
  constexpr size_t pBufLen = 32;
  char pBuf[pBufLen];
  while(true)
  {
    {
      auto lg = io.lockOuput();
      size_t lc = 0;
      io.constPrint(AnsiFormatter::Erase::toEndOfScreen);
      lc += printCpuUse(io, pBuf, pBufLen, printStack);
      lc += printMemUse(io, pBuf, pBufLen);
      if(io.waitForContinue("Press 'enter' to exit.", pollPeriod))
      {
        break;
      }
      for(size_t i = 0; i < lc; i++)
      {
        io.constPrint(AnsiFormatter::Cursor::up);
      }
    }
  }
  return 0;
}

extern const cli::Library cliCmdLib_tests;

int32_t cliCmdEnableTestLib(cli::CommandIo& io)
{
#ifndef __OPTIMIZE__
  int32_t stat = 0;
  io.fmt.automaticNewline = false;
  io.print("Registering '%s' library... ", cliCmdLib_tests.name);
  if(cli::registerLibrary(cliCmdLib_tests) == Status::success)
  {
    io.fmt.color = AnsiFormatter::Color::brightGreen;
    io.print("Registration successful.\n");
  }
  else
  {
    io.fmt.color = AnsiFormatter::Color::brightRed;
    io.print("Registration failed!\n");
    stat = 1;
  }
  return stat;
#else
  io.fmt.color = AnsiFormatter::Color::brightRed;
  io.print("The '%s' library is available only on debug builds.\n", cliCmdLib_tests.name);
  return 0;
#endif
}

} /** namespace jel */
