/** @file hw/targets/stm32f3/irq.cpp
 *  @brief STM32 IRQ controller and vector table implementation.
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
#include <cassert>
/** jel Library Headers */
#include "hw/targets/tm4c/tiva_shared.hpp"
#include "hw/api_irq.hpp"
/** Vector Table Include */
#ifdef HW_TARGET_STM32F302RCT6
#include "hw/targets/stm32f3/vectorTable_stm32f302rct6.hpp"
#endif

namespace jel
{
namespace hw
{
namespace irq 
{

void phantomIsr() __attribute__((naked));
void faultIsr() __attribute__((naked, noreturn));


void InterruptController::enableGlobalInterrupts()
{
}

void InterruptController::disableGlobalInterrupts()
{
}

void InterruptController::enableInterrupt(const Index channel, const IrqType)
{
}

void InterruptController::disableInterrupt(const Index channel, const IrqType)
{
}

void phantomIsr()
{
  /** Do nothing. */
}

void faultIsr()
{
  while(true);
}

}
}
}

