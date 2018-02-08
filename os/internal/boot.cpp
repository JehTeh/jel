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
 *      -MCU Reset vector called, _resetVector().
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
/** jel Library Headers */


extern "C"
{
  void _resetVector(void) __attribute((noreturn));
}

extern volatile const uint32_t _sidata; /**< Start of initialization for .data, located in ROM. */
extern volatile uint32_t _sdata; /**< Start of .data section, located in RAM. */
extern volatile uint32_t _edata; /**< End of .data section, located in RAM. */
extern volatile uint32_t _sbss; /**< Start of .bss, located in RAM. */
extern volatile uint32_t _ebss; /**< End of .bss, located in RAM. */

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


void _resetVector(void)
{
  //TODO:
  //ANY SAFETY STARTUP/RESET TYPE DISPATCH
  //-CPU/CLOCKS INIT
  //-FPU INIT
  initBss();
  initData();
  //-ISR INIT
  //-CORE PERIPHERALS INIT
  //-OS START
  //This statement should never be reached under normal operation.
  while(true);
}
