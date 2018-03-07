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
namespace os 
{

int32_t cliCmdMemuse(cli::CommandIo& io);
int32_t cliCmdCpuuse(cli::CommandIo& io);
int32_t cliCmdStackuse(cli::CommandIo& io);
int32_t cliCmdReadclock(cli::CommandIo& io);
int32_t cliCmdReboot(cli::CommandIo& io);

const cli::CommandEntry cliCommandArray[] =
{
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
    "readclock", cliCmdReadclock, "%?d",
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
};

extern const cli::Library cliCmdLib =
{
  "os",
  "The os library includes commands relating to system resource monitoring and control.\n",
  sizeof(cliCommandArray)/sizeof(cli::CommandEntry),
  cliCommandArray
};

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
      io.fmt.isBold = true;
      io.constPrint(
        " Handle         | Thread Name             | Total Time (ms)         | CPU(%)    \r\n");
      io.fmt.isBold = false;
      lc++;
      for(const auto& tip : Thread::registry())
      {
        Thread::ThreadInfo& ti = *tip.second;
        if(tip.second == nullptr)
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
        std::sprintf(pBuf, "%lld", ti.totalRuntime_.toMilliseconds());
        io.print(" %-24s|", pBuf);
        std::sprintf(pBuf, "%.2f",
          static_cast<float>(ti.totalRuntime_.toMilliseconds()) / 
          static_cast<float>(Timestamp{SteadyClock::now()}.toDuration().toMilliseconds()) * 100.0);
        io.print(" %-10s", pBuf);
        io.print("\r\n");
        lc++;
      }
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
    Thread::ThreadInfo& ti = *tip.second;
    if(tip.second == nullptr)
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

} /** namespace os */
} /** namespace jel */

