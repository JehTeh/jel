/** @file os/internal/cli_cmds_testing.cpp
 *  @brief os module specific testing commands, in addition to CppuTest functionality integration for supported targets
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
#include "os/api_common.hpp"
#include "os/internal/indef.hpp"
#include "os/api_cli.hpp"
#include "os/api_allocator.hpp"
#include "os/api_threads.hpp"
#include "hw/api_exceptions.hpp"
#include "hw/api_wdt.hpp"

#ifdef TARGET_SUPPORTS_CPPUTEST
/** C/C++ Standard Library Headers */
#include <cmath>
#include <csetjmp>
#include <memory>
#include <cassert>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
/** CPPU Test Headers */
#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/PlatformSpecificFunctions.h"
#include "CppUTest/PlatformSpecificFunctions_c.h"
#endif

namespace jel
{

int32_t cliCmdTest_CppuTest(cli::CommandIo& io);
int32_t cliCmdTest_Logger(cli::CommandIo& io);
int32_t cliCmdTest_Exceptions(cli::CommandIo& io);

const cli::CommandEntry cliCommandArray_tests[] =
{
  {
    "cpputest", cliCmdTest_CppuTest, "%?s",
    "Calls the CppUTest command line test runner. Note: This feature is only supported on targets with sufficient "
    "memory (typically 64kB or greater) on a target by target basis.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "log", cliCmdTest_Logger, "",
    "Tests the integrated OS logging subsystem. Useful for validating different logging "
    "configurations across targets.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
  {
    "except", cliCmdTest_Exceptions, "",
    "Test the system exception allocation scheme.\n",
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

int32_t cliCmdTest_CppuTest(cli::CommandIo& io)
{
#ifndef TARGET_SUPPORTS_CPPUTEST
  io.fmt.color = AnsiFormatter::Color::brightRed;
  io.print("This target build does not support CppUTest integration.\n");
  return 1;
#else
  size_t totalArgs = 3;
  const char* argString[] = { "exe", "-v", "-c" };
  io.print("Executing CppUTest runner...");
  if(io.args.totalArguments() != 0)
  {
    io.print("TODO: Custom arg handling");
  }
  return RUN_ALL_TESTS(totalArgs, argString);
#endif
}

int32_t cliCmdTest_Logger(cli::CommandIo& io)
{
  constexpr Duration sleepTime = Duration::milliseconds(50);
  io.print("Testing CLI logger. This is done by printing multiple log statements. "
    "Each logging event is seperated by a ~%lldms delay.\n", sleepTime.toMilliseconds());
  Logger& log = Logger::sysLogChannel();
  io.print("Overridding logger mask level to hidden (i.e. display all).\n");
  log.config().maskLevel = Logger::MessageType::hidden;
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
  io.print("Test: Info print call (this should flush the print queue).\n");
  ThisThread::sleepfor(sleepTime);
  io.print("Testing LoggerStreamHelper (operator<< functionality).\n");
  log << "This is a test string" << flush;
  log << "Printing unsigned integer (systime): " << 
    Duration(SteadyClock::now().time_since_epoch()).toMicroseconds() << flush;
  log << "Printing float(123.456): " << 69.69 << flush;
  log << "Printing float(-9000.1): " << -9000.1 << flush;
  log << "Printing float(0.0001): " << 0.0001 << flush;
  log << "Printing float(systime): " << 
    static_cast<float>(Duration(SteadyClock::now().time_since_epoch()).toMicroseconds()) / 1'000'000
    << flush;
  log << "Multiprint (int/float/string): " << -123456789 << " " << 17.777 <<
    " final string" << flush;
  log << Logger::MessageType::error << "This message should be an error!" << flush;
  log << Logger::MessageType::warning << "This message should be a warning!" << flush;
  log << Logger::MessageType::info << "This message should be informational!" << flush;
  ThisThread::sleepfor(sleepTime);
  return 0;
}

int32_t cliCmdTest_Exceptions(cli::CommandIo& io)
{
  io.print("Testing throwing an int... (the CLI should catch this)\n");
  throw int(5);
  return 0;
}

} /** namespace jel */

#ifdef TARGET_SUPPORTS_CPPUTEST
/** CppUTest Integration/Porting Functions */

static jmp_buf test_exit_jmp_buf[10];
static int jmp_buf_index = 0;

TestOutput::WorkingEnvironment PlatformSpecificGetWorkingEnvironment()
{
  return TestOutput::eclipse;
}

static void DummyPlatformSpecificRunTestInASeperateProcess(UtestShell* shell, TestPlugin*, TestResult* result)
{
  result->addFailure(TestFailure(shell, "-p doesn't work on this platform, as it is lacking fork.\b"));
}

static int DummyPlatformSpecificFork(void)
{
  return 0;
}

static int DummyPlatformSpecificWaitPid(int, int*, int)
{
  return 0;
}

void (*PlatformSpecificRunTestInASeperateProcess)(UtestShell* shell, TestPlugin* plugin, TestResult* result) =
        DummyPlatformSpecificRunTestInASeperateProcess;
int (*PlatformSpecificFork)(void) = DummyPlatformSpecificFork;
int (*PlatformSpecificWaitPid)(int, int*, int) = DummyPlatformSpecificWaitPid;

extern "C" 
{

static int PlatformSpecificSetJmpImplementation(void (*function) (void* data), void* data)
{
  if (0 == setjmp(test_exit_jmp_buf[jmp_buf_index])) 
  {
    jmp_buf_index++;
    function(data);
    jmp_buf_index--;
    return 1;
  }
  return 0;
}

static void PlatformSpecificLongJmpImplementation()
{
  jmp_buf_index--;
  longjmp(test_exit_jmp_buf[jmp_buf_index], 1);
}

static void PlatformSpecificRestoreJumpBufferImplementation()
{
  jmp_buf_index--;
}

void (*PlatformSpecificLongJmp)() = PlatformSpecificLongJmpImplementation;
int (*PlatformSpecificSetJmp)(void (*)(void*), void*) = PlatformSpecificSetJmpImplementation;
void (*PlatformSpecificRestoreJumpBuffer)() = PlatformSpecificRestoreJumpBufferImplementation;

///////////// Time in millis
/*
*  In Keil MDK-ARM, clock() default implementation used semihosting.
*  Resolutions is user adjustable (1 ms for now)
*/
static long TimeInMillisImplementation()
{
  jel::Timestamp ts = jel::SteadyClock::now();
  return ts.toDuration().toMilliseconds();
}

///////////// Time in String

static const char* DummyTimeStringImplementation()
{
  time_t tm = 0;
  return ctime(&tm);
}

long (*GetPlatformSpecificTimeInMillis)() = TimeInMillisImplementation;
const char* (*GetPlatformSpecificTimeString)() = DummyTimeStringImplementation;

int (*PlatformSpecificVSNprintf)(char *str, size_t size, const char* format, va_list args) = vsnprintf;

static PlatformSpecificFile PlatformSpecificFOpenImplementation(const char* filename, const char* flag)
{
  return fopen(filename, flag);
}

static void PlatformSpecificFPutsImplementation(const char* str, PlatformSpecificFile file)
{
  fputs(str, (FILE*)file);
}

static void PlatformSpecificFCloseImplementation(PlatformSpecificFile file)
{
  std::fclose((FILE*)file);
}

static void PlatformSpecificFlushImplementation()
{
  std::fflush(stdout);
}

PlatformSpecificFile (*PlatformSpecificFOpen)(const char*, const char*) = PlatformSpecificFOpenImplementation;
void (*PlatformSpecificFPuts)(const char*, PlatformSpecificFile) = PlatformSpecificFPutsImplementation;
void (*PlatformSpecificFClose)(PlatformSpecificFile) = PlatformSpecificFCloseImplementation;

int PlatformSpecificPutcharImpl(int c)
{
  if(c == '\n')
  {
    std::putchar('\r');
  }
  return std::putchar(c);
}

int (*PlatformSpecificPutchar)(int) = PlatformSpecificPutcharImpl;


void (*PlatformSpecificFlush)() = PlatformSpecificFlushImplementation;

void* (*PlatformSpecificMalloc)(size_t size) = malloc;
void* (*PlatformSpecificRealloc)(void*, size_t) = realloc;
void (*PlatformSpecificFree)(void* memory) = free;
void* (*PlatformSpecificMemCpy)(void*, const void*, size_t) = memcpy;
void* (*PlatformSpecificMemset)(void*, int, size_t) = memset;

static int IsNanImplementation(double d)
{
  return std::isnan(d);
}

static int IsInfImplementation(double d)
{
  return std::isinf(d);
}

static int AtExitImplementation(void(*func)(void))
{
  return atexit(func);
}

double (*PlatformSpecificFabs)(double) = fabs;
int (*PlatformSpecificIsNan)(double) = IsNanImplementation;
int (*PlatformSpecificIsInf)(double) = IsInfImplementation;
int (*PlatformSpecificAtExit)(void(*func)(void)) = AtExitImplementation;

static PlatformSpecificMutex DummyMutexCreate(void)
{
  return 0;
}

static void DummyMutexLock(PlatformSpecificMutex)
{
}

static void DummyMutexUnlock(PlatformSpecificMutex)
{
}

static void DummyMutexDestroy(PlatformSpecificMutex)
{
}

PlatformSpecificMutex (*PlatformSpecificMutexCreate)(void) = DummyMutexCreate;
void (*PlatformSpecificMutexLock)(PlatformSpecificMutex) = DummyMutexLock;
void (*PlatformSpecificMutexUnlock)(PlatformSpecificMutex) = DummyMutexUnlock;
void (*PlatformSpecificMutexDestroy)(PlatformSpecificMutex) = DummyMutexDestroy;

}
#endif
