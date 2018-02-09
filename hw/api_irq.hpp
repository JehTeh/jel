/** @file hw/api_irq.hpp
 *  @brief General interrupt controller interface routines are exposed via this interface.
 *
 *  @detail
 *    
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

#pragma once

/** C/C++ Standard Library Headers */
#include <cstdint>
/** jel Library Headers */

namespace jel
{
namespace hw
{
namespace irq
{

/** @enum Index
 *  @brief Generic indexes used for enabling or disabling specific interrupt vectors in the
 *  interrupt controller. 
 *
 *  IRQ/FIQ indexes are considered generic across hardware, with the different mappings for different
 *  processors. While it is possible to explicitly enable and disable indexes in the application
 *  code, it is recommended this functionality generally remain at the driver level, and where
 *  necessary that the application controls a specific interrupt vector the driver implementation
 *  provides a function that returns the correct index for that target. For example,
 *  @code
 *    //Generic Uart API example, visible to the application 
 *    class OutputUart : public os::SerialWriterInterface
 *    {
 *      ...
 *      irq::Index getUartIsrIndex() noexcept;
 *      ...
 *    };
 *
 *    //Implementation in the uart source file for one target
 *    irq::Index OutputUart::getUartIsrIndex() { return isr::Index::chan7; }
 *    //Implementation in the uart source file for another target
 *    irq::Index OutputUart::getUartIsrIndex() { return isr::Index::chan17; }
 *  @endcode
 *  This allows for the application to ensure it always controls the correct interrupt line for that
 *  peripheral. It also allows for interrupt remapping to be done in the background, either at
 *  compile time due to design changes or at runtime.
 *  */
enum class Index : uint32_t
{
  chan0 = 0, chan1, chan2, chan3, chan4, chan5, chan6, chan7, chan8, chan9, chan10, chan11, chan12, 
  chan13, chan14, chan15, chan16, chan17, chan18, chan19, chan20, chan21, chan22, chan23, chan24,
  chan25, chan26, chan27, chan28, chan29, chan30, chan31, chan32, chan33, chan34, chan35, chan36,
  chan37, chan38, chan39, chan40, chan41, chan42, chan43, chan44, chan45, chan46, chan47, chan48,
  chan49, chan50, chan51, chan52, chan53, chan54, chan55, chan56, chan57, chan58, chan59, chan60,
  chan61, chan62, chan63, chan64, chan65, chan66, chan67, chan68, chan69, chan70, chan71, chan72,
  chan73, chan74, chan75, chan76, chan77, chan78, chan79, chan80, chan81, chan82, chan83, chan84,
  chan85, chan86, chan87, chan88, chan89, chan90, chan91, chan92, chan93, chan94, chan95, chan96,
  chan97, chan98, chan99, chan100, chan101, chan102, chan103, chan104, chan105, chan106, chan107, chan108,
  chan109, chan110, chan111, chan112, chan113, chan114, chan115, chan116, chan117, chan118, chan119, chan120,
  chan121, chan122, chan123, chan124, chan125, chan126, chan127 
};

enum class IrqType
{
  irq, fiq
};

class InterruptController
{
public:
  /** Sets the global interrupt enabled state to true. On MCUs with additional interrupt signals,
   * such as the Cortex-R series FIQs, these are also enabled. */
  static void enableGlobalInterrupts();
  /** Disable all disable-able interrupts. On MCUs such as the Cortex-R series, this does *not*
   * disable FIQs, as they can not be disabled short of a POR. */
  static void disableGlobalInterrupts();
  /** Enables a specific interrupt channel, of either the FIQ or IRQ type. On MCUs without FIQ
   * support all requests are treated as IRQs. */
  static void enableInterrupt(const Index channel, const IrqType type = IrqType::irq);
  /** Disables a specific interrupt channel, of either the FIQ or IRQ type. On MCUs without FIQ
   * support all requests are treated as IRQs. */
  static void disableInterrupt(const Index channel, const IrqType type = IrqType::irq);
};

}
} /** namespace hw */
} /** namespace jel */

