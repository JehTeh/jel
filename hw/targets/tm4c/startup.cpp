/** @file hw/targets/tm4c/startup.cpp
 *  @brief Hardware specific startup routines for the TIVA series MCUs.
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
#include <cstdint>
/** jel Library Headers */
#include "hw/targets/tm4c/tiva_shared.hpp"
#include "hw/api_startup.hpp"
/** Tivaware Library Headers */
#include "inc/hw_nvic.h"

extern "C"
{
extern void _resetVector(void) __attribute__((noreturn));

void _start(void) __attribute__((noreturn));
}

void _start(void) 
{
  /** No stack specific init is required on the tiva. */
  _resetVector();
}

namespace jel
{
namespace hw
{
namespace startup
{
  void defaultInitializeClocks();
  void enableFpu();

  void defaultInitializeClocks()
  {
    #ifdef HW_TARGET_TM4C123GH6PM
    //Configure the CPU clock for the maximum rated (80MHz). 
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    #endif
  }

  void enableFpu()
  {
    //As per tivaware library documentation...
    HWREG(NVIC_CPAC) = ((HWREG(NVIC_CPAC) & ~(NVIC_CPAC_CP10_M | NVIC_CPAC_CP11_M)) |
      NVIC_CPAC_CP10_FULL | NVIC_CPAC_CP11_FULL);
  }
}
}
}

