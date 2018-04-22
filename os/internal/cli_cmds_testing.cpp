/** @file os/internal/cli_cmds_testing.cpp
 *  @brief os module specific testing commands.
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

int32_t cliCmdTest_Logger(cli::CommandIo& io);

const cli::CommandEntry cliCommandArray_tests[] =
{
  {
    "test_logger", cliCmdTest_Logger, "",
    "Tests the integrated OS logging subsystem. Useful for validating different logging "
    "configurations across targets.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
};

extern const cli::Library cliCmdLib_tests =
{
  "os_tst",
  "The os_tst library includes a series of commands dedicated to testing various aspects of os "
  "module functionality. These are designed to be significantly lighter weight than CppuTest and "
  "as such are generally suitable when porting the jel.\n",
  sizeof(cliCommandArray_tests)/sizeof(cli::CommandEntry),
  cliCommandArray_tests
};

int32_t cliCmdTest_Logger(cli::CommandIo& io)
{
  constexpr Duration sleepTime = Duration::milliseconds(50);
  io.print("Testing CLI logger. This is done by printing multiple log statements. "
    "Each logging event is seperated by a ~%lldms delay.\n", sleepTime.toMilliseconds());
  Logger& log = Logger::sysLogChannel();
  log.fprintInfo("Test: fprintInfo");
  ThisThread::sleepfor(sleepTime);
  log.fprintDebug("Test: fprintDebug");
  ThisThread::sleepfor(sleepTime);
  log.fprintWarning("Test: fprintWarning");
  ThisThread::sleepfor(sleepTime);
  log.fprintError("Test: fprintError");
  ThisThread::sleepfor(sleepTime);
  log.printInfo("Test: printInfo");
  ThisThread::sleepfor(sleepTime);
  log.printDebug("Test: printDebug");
  ThisThread::sleepfor(sleepTime);
  log.printWarning("Test: printWarning");
  ThisThread::sleepfor(sleepTime);
  log.printError("Test: printError");
  ThisThread::sleepfor(sleepTime);
  Duration dur = SteadyClock::now() - SteadyClock::zero();
  log.printInfo("Test: printInfo(integer, float): %lld, %f", dur.toMicroseconds(), 
    static_cast<float>(dur.toMicroseconds()));
  log.printDebug("Test: printDebug(integer, float): %lld, %f", dur.toMicroseconds(), 
    static_cast<float>(dur.toMicroseconds()));
  log.printWarning("Test: printWarning(integer, float): %lld, %f", dur.toMicroseconds(), 
    static_cast<float>(dur.toMicroseconds()));
  log.printError("Test: printError(integer, float): %lld, %f", dur.toMicroseconds(), 
    static_cast<float>(dur.toMicroseconds()));
  ThisThread::sleepfor(sleepTime);
  io.print("Testing bulk fast print operation (five consecutive fp calls)...\n");
  log.fpInf("Test: fpInfo");
  log.fpDbg("Test: fpDbg");
  log.fpWrn("Test: fpWrn");
  log.fpErr("Test: fpErr");
  log.fp(Logger::MessageType::default_, "Test: fp (default_)");
  log.printInfo("Test: Info print call (this should flush the print queue).");
  return 0;
}

} /** namespace jel */
