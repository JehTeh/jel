/** @file hw/targets/rm57/irq.cpp
 *  @brief RM57 IRQ controller and vector table implementation.
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
#include <type_traits>
/** jel Library Headers */
#include "hw/api_irq.hpp"
/** TI Halcogen Headers */
#include "HL_sys_core.h"
#include "HL_sys_vim.h"
/** Vector Table Include */
#ifdef HW_TARGET_RM57L843
#include "hw/targets/rm57/vectorTable_rm57l843.hpp"
#endif

namespace jel
{
namespace hw
{
namespace irq 
{

void phantomIsr();
void faultIsr();

void InterruptController::enableGlobalInterrupts()
{
  _enable_interrupt_();
}

void InterruptController::disableGlobalInterrupts()
{
  _disable_IRQ_interrupt_();
}

void InterruptController::enableInterrupt(const Index channel, const IrqType type)
{
  assert((static_cast<uint32_t>(channel) > 2) && "Illegal attempt to enable hardwired IRQ channel on the RM57");
  assert((static_cast<uint32_t>(channel) < 127) && "Illegal attempt to enable out of bounds IRQ channel on the RM57");
  volatile uint32_t* setreg = &vimREG->REQMASKSET0 + (static_cast<size_t>(channel) % 32);
  volatile uint32_t* fiqreg = &vimREG->FIRQPR0 + (static_cast<size_t>(channel) % 32);
  *setreg |= 1 << static_cast<size_t>(channel);
  switch(type)
  {
    case IrqType::irq:
      *fiqreg &= ~(1 << static_cast<size_t>(channel));
      break;
    case IrqType::fiq:
      *fiqreg &= ~(1 << static_cast<size_t>(channel));
  }
}

void InterruptController::disableInterrupt(const Index channel, const IrqType type)
{ 
  assert((static_cast<uint32_t>(channel) > 2) && "Illegal attempt to disable hardwired IRQ channel on the RM57");
  assert((static_cast<uint32_t>(channel) < 127) && "Illegal attempt to disable out of bounds IRQ channel on the RM57");
  volatile uint32_t* clrreg = &vimREG->REQMASKCLR0 + (static_cast<size_t>(channel) % 32);
  volatile uint32_t* fiqreg = &vimREG->FIRQPR0 + (static_cast<size_t>(channel) % 32);
  *clrreg |= 1 << static_cast<size_t>(channel);
  switch(type)
  {
    case IrqType::irq:
      *fiqreg &= ~(1 << static_cast<size_t>(channel));
      break;
    case IrqType::fiq:
      *fiqreg &= ~(1 << static_cast<size_t>(channel));
      break;
  }
}

void phantomIsr()
{
  assert(!"PHANTOM ISR TRIGGERED");
  /** Do nothing. */
  while(true);
}


void faultIsr()
{
  /** Do nothing. */
  assert(!"FAULT ISR TRIGGERED");
  while(true);
}

}
}
}
