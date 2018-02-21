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
#include "hw/api_exceptions.hpp"

namespace jel
{
namespace os 
{

int32_t cliCmdMemuse(cli::CommandIo& io);

const cli::CommandEntry cliCommandArray[] =
{
  {
    "memuse", cliCmdMemuse, "",
    "Reports the current memory usage of various heaps and memory pools in the system.\n",
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

} /** namespace os */
} /** namespace jel */

