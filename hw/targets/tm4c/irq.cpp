/** @file hw/targets/tm4c/irq.cpp
 *  @brief Tiva C IRQ controller and vector table implementation.
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
/** Tivaware Library Headers */
#include "inc/hw_nvic.h"
#include "driverlib/interrupt.h"
/** Vector Table Include */
#ifdef HW_TARGET_TM4C123GH6PM
#include "hw/targets/tm4c/vectorTable_tm4c123gh6pm.hpp"
#endif
#ifdef HW_TARGET_TM4C1294NCPDT
#include "hw/targets/tm4c/vectorTable_tm4c1294ncpdt.hpp"
#endif

extern "C" void ISR_readFaultAddress(uint32_t* fStack);

namespace jel
{
namespace hw
{
namespace irq 
{

void phantomIsr() __attribute__((naked));
void faultIsr() __attribute__((naked));

void InterruptController::enableGlobalInterrupts()
{
  IntMasterEnable();
}

void InterruptController::disableGlobalInterrupts()
{
  IntMasterDisable();
}

void InterruptController::enableInterrupt(const Index channel, const IrqType)
{
  std::size_t channelOffset = static_cast<std::size_t>(channel);
  assert(channelOffset < sizeof(vectorTable)/sizeof(IsrPtr));
  //This must match the freeRTOS configMAX_SYSCALL_INTERRUPT_PRIORITY
  IntPrioritySet(channelOffset, 0xA0);
  IntEnable(channelOffset);
}

void InterruptController::disableInterrupt(const Index channel, const IrqType)
{
  std::size_t channelOffset = static_cast<std::size_t>(channel);
  assert(channelOffset < sizeof(vectorTable)/sizeof(IsrPtr));
  IntDisable(channelOffset);
}

void phantomIsr()
{
  /** Do nothing. */
}


void faultIsr()
{
  __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word ISR_readFaultAddress        \n"
    );
}

}
}
}

void ISR_readFaultAddress(uint32_t* fStack)
{
  volatile uint32_t r0; volatile uint32_t r1; volatile uint32_t r2;
  volatile uint32_t r3; volatile uint32_t r12; volatile uint32_t lr; 
  volatile uint32_t pc; volatile uint32_t psr;
  r0 = fStack[0]; r1 = fStack[1]; r2 = fStack[2]; r3 = fStack[3];
  r12 = fStack[4]; lr = fStack[5]; pc = fStack[6]; psr = fStack[7];
  (void)r0; (void)r1; (void)r2; (void)r3; (void)r12; (void)lr;
  (void)pc; (void)psr;
  while(true);
}

