/** @file os/internal/boot.cpp
 *  @brief jel system startup.
 *
 *  @detail
 *    The jel performs all low level initialization, replacing routines normally found in crt0 and
 *    the newlib initialization code. This is done to provide complete control over bootup and
 *    initialization order - the jel can ensure system resources are initialized in a manner that
 *    allows for static constructors to use RTOS and appropriate system calls.
 *    Note that this code is heavily dependent on the hw drivers for startup. It also requires that
 *    the external linker section symbols are correctly defined.
 *
 *    The typical boot process is as follows:
 *      -MCU Reset vector called and stack is setup, then _resetVector() is called.
 *      -CPU clocks and the FPU (if present) are enabled and initialized.
 *      -The .bss and .data sections are initialized.
 *      -The interrupt controller is configured and interrupts are enabled.
 *      -Minimal hardware peripherals are enabled and initialized, such as the system clock and GPIO
 *      drivers (including pinmuxing).
 *      -A maximum priority thread is created (the BOOT thread) and the OS is started.
 *      -----
 *      -The BOOT thread performs various common system component setup, including stdio, any
 *      filesystems, external RAM, MPU, etc.
 *      -C++ constructors are called and libc/newlib initialization is performed.
 *      -The startup thread tags itself for deletion and control is handed over to any threads
 *      instantiated in a static constructor.
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
#include <cstdint>
#include <cstring>
#include <cassert>
#include <clocale>
/** jel Library Headers */
#include "os/internal/indef.hpp"
#include "os/api_threads.hpp"
#include "os/api_allocator.hpp"
#include "os/api_io.hpp"
#include "hw/api_startup.hpp"
#include "hw/api_irq.hpp"
#include "hw/api_gpio.hpp"
#include "hw/api_sysclock.hpp"
#include "hw/api_uart.hpp"

#include "os/internal/cli.hpp"

extern "C"
{
/** Called by the driver layer _start function, which ensures stack is setup. */
void _resetVector(void) __attribute((noreturn));
/** libc constructor tables. These are manually called at the appropriate time to ensure static
 * construction occurs after certain core system primitives are available. */
extern void (*__preinit_array_start []) (void);
extern void (*__preinit_array_end []) (void);
extern void (*__init_array_start []) (void);
extern void (*__init_array_end []) (void);
extern void _init(void);

extern volatile const uint32_t _sidata; /**< Start of initialization for .data, located in ROM. */
extern volatile uint32_t _sdata; /**< Start of .data section, located in RAM. */
extern volatile uint32_t _edata; /**< End of .data section, located in RAM. */
extern volatile uint32_t _sbss; /**< Start of .bss, located in RAM. */
extern volatile uint32_t _ebss; /**< End of .bss, located in RAM. */
}


int main(int, char**);
namespace jel
{
namespace os
{
void bootThread(void*);

int32_t cliCmdReadAdc(cli::CommandIo& io);
const cli::CommandEntry cliTestCommandArray[] =
{
  {
    "readadc1", cliCmdReadAdc, "%u%?u",
    "Reads the value of the specified ADC1 pin. The command accepts up to two arguments:\n"
    "\t[0]: Positive integer in the range 1-15: The ADC1 channel to sample.\n"
    "\t[1]: (Optional) a positive integer. This is the polling time in milliseconds, and defaults "
    "to 1000ms.\n",
    cli::AccessPermission::unrestricted, nullptr
  },
};
extern const cli::Library cliTestCmdLib =
{
  "test",
  "The test library features various commands for system testing and validation.",
  sizeof(cliTestCommandArray)/sizeof(cli::CommandEntry),
  cliTestCommandArray
};

}
}

inline static void initBss()
{
  for(volatile uint32_t* bss = &_sbss; bss <= &_ebss;)
  {
    *bss++ = 0x0000'0000;
  }
}

inline static void initData()
{
  volatile const uint32_t* init = &_sidata;
  for(volatile uint32_t* data = &_sdata; data <= &_edata;)
  {
    *data++ = *init++;
  }
}

/** 
 *  @brief Pre-main initialization function, typically called immediately after stack setup is
 *  complete from the crt0 style ASM initializer, or directly on some architectures such as the M4
 *  where the stack is automatically configured.
 *
 *  The _resetVector function performs certain core system and libc initialization before calling
 *  main(). This initialization includes CPU and system basic clock configuration (i.e CPU
 *  frequency, bus clocks, etc). It also enables the FPU if it is present and performs .bss and
 *  .data section initialization. Once all this is completed, it instantiates the global default
 *  system allocator which translates all calls to malloc, new, free, delete, etc to the appropriate
 *  heap. After allocation setup is complete, core system peripherals that are required for jel
 *  operation are also enabled; these typically include GPIO drivers used for the heartbeat signal
 *  and the system clock peripheral used for timekeeping (see os/api_time.hpp). Afterwards, C++
 *  pre-init functions are also called, if any exist. main() is then called, which is expected to
 *  start the RTOS.  
 *
 *  @note
 *    On systems requiring additional low level handling at reset, a special dispatcher is called
 *    before all other initialization. This dispatcher can be implemented at the driver layer for a
 *    specific target and perform any actions required, including memory or system tests and
 *    bootloader routines.
 *
 *  */

void _resetVector(void)
{
  using namespace jel;
  hw::startup::customDispatcher();
  hw::startup::defaultInitializeClocks();
  hw::startup::enableFpu();
  initBss();
  initData();
  std::lconv* lptr = localeconv();
  (void)lptr;
  hw::startup::customDispatcherPostDataInit();
  hw::irq::InterruptController::enableGlobalInterrupts();
  os::SystemAllocator::constructSystemAllocator();
  hw::sysclock::SystemSteadyClockSource::startClock();
  //-heartbeat IO init
  //Newlib pre-os initialization. This does everything except call C++ static constructors, as such
  //most of the libc (not C++) functionality is available during pre-c++ initialization.
  for(int32_t i = 0; i < __preinit_array_end - __preinit_array_start; i++)
  {
    __init_array_start[i]();
  }
  _init();
  main(0, nullptr);
  std::terminate();
}

/** 
 *  @brief main(), the application entry point.
 *
 *  Registers a single, maximum priority thread named 'BOOT' that will begin executing once the RTOS
 *  is started, then starts the RTOS.
 *
 *  @note
 *    At this point, it is expected the system is in the following state:
 *      -CPU clocking and stack is setup and valid.
 *      -The .bss and .data sections have been initialized.
 *      -Global interrupts are enabled.
 *      -The system allocator is extant (i.e. calls to new/malloc() will not fail).
 *      -C++ static constructors have *not* been called.
 * */
int main(int, char**)
{
  std::unique_ptr<jel::os::Thread> btThread = 
    std::make_unique<jel::os::Thread>(&jel::os::bootThread, nullptr, "BOOT", 4096);
  btThread->detach();
  vTaskStartScheduler();
  return 1;
}

namespace jel
{
namespace os
{

/** Note: This must be nullptr/unitialized. If it is not, GCC will call a static constructor and
 * blow away the Io pointer that is assigned before C++ static initialization. */
std::shared_ptr<AsyncIoStream> jelStandardIo;
std::shared_ptr<JelStringPool> jelStringPool;

extern const cli::Library cliCmdLib;
/** 
 *  @brief The jel bootup thread, responsible for the majority of system initialization and starting
 *  the user application thread(s).
 *
 *  ...todo
 *
 * */


void initializeStandardIo()
{
  using namespace hw::uart;
  BasicUart_Base::Config uartConfig = config::jelRuntimeConfiguration.stdioUartConfiguration;
  BasicUart* uart = nullptr;
  switch(config::jelRuntimeConfiguration.stdioPortType)
  {
    case config::SerialPortType::uart0:
      {
        //Note: The ownership semantics may be a touch confusing here: Two unique pointers are
        //created, both effectively pointing to the same UART object. This is because the
        //AsyncIoStream accepts Reader and Writer interfaces that are not required to be part of the
        //same object, but still wants exclusive ownership over both. The AsyncIoStream does support
        //both base interfaces being the same object, however, and will deal with the underlying
        //memory appropriately if destroyed to avoid a double deletion. The sharedInterface flag
        //must be set to true to ensure this occurs.
        uart = new BasicUart(uartConfig);
        std::unique_ptr<SerialWriterInterface> writerIf(uart);
        std::unique_ptr<SerialReaderInterface> readerIf(uart);
        std::shared_ptr<AsyncIoStream> 
          io(new AsyncIoStream(std::move(readerIf), std::move(writerIf), true));
        jelStandardIo = io;
      }
      break;
    default:
      assert(false);
      break;
  }
  jelStandardIo->write(AnsiFormatter::reset);
  jelStandardIo->write("\r\n"
                       "╔══════════════════════════════════════╗\r\n"
                       "║                 BOOT                 ║\r\n"
                       "╚══════════════════════════════════════╝\r\n");
  jelStandardIo->write("System standard I/O initialization complete.\r\n");
  jelStandardIo->write("Runtime configuration '"); 
  jelStandardIo->write(config::jelRuntimeConfiguration.name);
  jelStandardIo->write("' has been loaded successfully.\r\n");
};

void bootThread(void*)
{
  //Initialize GPIO hardware and the serial I/O port
  hw::gpio::GpioController::initializeGpio();
  initializeStandardIo();
  jelStringPool = std::make_shared<ObjectPool<String, config::stringPoolStringCount>>();
  cli::startSystemCli(jelStandardIo);
  cli::registerLibrary(cliCmdLib);
  cli::registerLibrary(cliTestCmdLib);
  /** C++ Static object constructors are called here. */
  for(int32_t i = 0; i < __init_array_end - __init_array_start; i++)
  {
    __init_array_start[i]();
  }
  ThisThread::deleteSelf();
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister" 
#include "adc.h"
#pragma GCC diagnostic pop

int32_t cliCmdReadAdc(cli::CommandIo& io)
{
  ADC_HandleTypeDef* adc = &hadc1;
  MX_ADC1_Init();
  size_t channel = io.args[0].asUInt();
  switch(channel)
  {
    case 1:
      channel = ADC_CHANNEL_1;
      break;
    case 2:
      channel = ADC_CHANNEL_2;
      break;
    case 5:
      channel = ADC_CHANNEL_5;
      break;
    case 6:
      channel = ADC_CHANNEL_6;
      break;
    case 7:
      channel = ADC_CHANNEL_7;
      break;
    case 8:
      channel = ADC_CHANNEL_8;
      break;
    case 9:
      channel = ADC_CHANNEL_9;
      break;
    default:
      io.fmt.color = AnsiFormatter::Color::brightRed;
      io.print("This channel (%u) is not supported by the hardware configuration.\n", channel);
      return 1;
  }
  Duration pollRate = Duration::milliseconds(1000);
  if(io.args.totalArguments() > 1)
  {
    pollRate = Duration::milliseconds(io.args[1].asUInt());
    if(pollRate < Duration::milliseconds(100))
    {
      io.fmt.color = AnsiFormatter::Color::yellow;
      io.print("Caution: Polling rates faster than 100ms may cause issues when displayed via "
        "the CLI.\n");
      io.fmt.color = AnsiFormatter::Color::default_;
    }
  }
  io.print("Polling ADC channel every %lldms. Press enter to exit.\r\n", pollRate.toMilliseconds());
  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if(HAL_ADC_ConfigChannel(adc, &sConfig) != HAL_OK)
  {
    return 2;
  }
  io.fmt.automaticNewline = false;
  while(true)
  {
    HAL_ADC_Start(adc);
    if(HAL_ADC_PollForConversion(adc, HAL_MAX_DELAY) != HAL_OK)
    {
      return 3;
    }
    uint32_t cres = HAL_ADC_GetValue(adc);
    {
      auto lg = io.lockOuput();
      io.print("ADC: %lu                   \r", cres);
    }
    if(io.waitForContinue("", pollRate))
    {
      io.print("ADC: %lu                   \r", cres);
      break;
    }
  }
  return 0;
} 



} /** namespace os */
} /** namespace jel */
