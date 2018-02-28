/** @file hw/targets/tm4c/vectorTable_tm4c123gh6pm.cpp
 *  @brief The default flash ISR vector table for the Tiva 123GH6PM CPU
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

/** Tivaware Library Headers */

extern "C"
{
extern void _jelEntry(void) __attribute__((noreturn));
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
}

extern void isrEntry_Uart0() noexcept __attribute__((interrupt ("IRQ")));

namespace jel
{
namespace hw
{
namespace irq 
{

extern void phantomIsr();
extern void faultIsr();

using IsrPtr = void(*)(void);

static volatile uint32_t bootStack[192] __attribute__((aligned(8), section (".noinit"), used));

static IsrPtr __attribute__((section (".vectorTable"), used)) vectorTable[] =
{
  reinterpret_cast<IsrPtr>(reinterpret_cast<uint32_t>(bootStack + (sizeof(bootStack) / 4))),
  &_jelEntry,                                            // The reset handler
  &faultIsr,                                          // The NMI handler
  &faultIsr,                                          // The hard fault handler
  &faultIsr,                                          // The MPU fault handler
  &faultIsr,                                          // The bus fault handler
  &faultIsr,                                          // The usage fault handler
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  &vPortSVCHandler,                                   // SVCall handler
  phantomIsr,                                         // Debug monitor handler
  nullptr,                                            // Reserved
  &xPortPendSVHandler,                                // The PendSV handler
  &xPortSysTickHandler,                               // The SysTick handler
  phantomIsr,                                         // GPIO Port A
  phantomIsr,                                         // GPIO Port B
  phantomIsr,                                         // GPIO Port C
  phantomIsr,                                         // GPIO Port D
  phantomIsr,                                         // GPIO Port E
  isrEntry_Uart0,                                     // UART0 Rx and Tx
  phantomIsr,                                         // UART1 Rx and Tx
  phantomIsr,                                         // SSI0 Rx and Tx
  phantomIsr,                                         // I2C0 Master and Slave
  phantomIsr,                                         // PWM Fault
  phantomIsr,                                         // PWM Generator 0
  phantomIsr,                                         // PWM Generator 1
  phantomIsr,                                         // PWM Generator 2
  phantomIsr,                                         // Quadrature Encoder 0
  phantomIsr,                                         // ADC Sequence 0
  phantomIsr,                                         // ADC Sequence 1
  phantomIsr,                                         // ADC Sequence 2
  phantomIsr,                                         // ADC Sequence 3
  phantomIsr,                                         // Watchdog timer
  phantomIsr,                                         // Timer 0 subtimer A
  phantomIsr,                                         // Timer 0 subtimer B
  phantomIsr,                                         // Timer 1 subtimer A
  phantomIsr,                                         // Timer 1 subtimer B
  phantomIsr,                                         // Timer 2 subtimer A
  phantomIsr,                                         // Timer 2 subtimer B
  phantomIsr,                                         // Analog Comparator 0
  phantomIsr,                                         // Analog Comparator 1
  phantomIsr,                                         // Analog Comparator 2
  phantomIsr,                                         // System Control (PLL, OSC, BO)
  phantomIsr,                                         // FLASH Control
  phantomIsr,                                         // GPIO Port F
  phantomIsr,                                         // GPIO Port G
  phantomIsr,                                         // GPIO Port H
  phantomIsr,                                         // UART2 Rx and Tx
  phantomIsr,                                         // SSI1 Rx and Tx
  phantomIsr,                                         // Timer 3 subtimer A
  phantomIsr,                                         // Timer 3 subtimer B
  phantomIsr,                                         // I2C1 Master and Slave
  phantomIsr,                                         // Quadrature Encoder 1
  phantomIsr,                                         // CAN0
  phantomIsr,                                         // CAN1
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // Hibernate
  phantomIsr,                                         // USB0
  phantomIsr,                                         // PWM Generator 3
  phantomIsr,                                         // uDMA Software Transfer
  phantomIsr,                                         // uDMA Error
  phantomIsr,                                         // ADC1 Sequence 0
  phantomIsr,                                         // ADC1 Sequence 1
  phantomIsr,                                         // ADC1 Sequence 2
  phantomIsr,                                         // ADC1 Sequence 3
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // GPIO Port J
  phantomIsr,                                         // GPIO Port K
  phantomIsr,                                         // GPIO Port L
  phantomIsr,                                         // SSI2 Rx and Tx
  phantomIsr,                                         // SSI3 Rx and Tx
  phantomIsr,                                         // UART3 Rx and Tx
  phantomIsr,                                         // UART4 Rx and Tx
  phantomIsr,                                         // UART5 Rx and Tx
  phantomIsr,                                         // UART6 Rx and Tx
  phantomIsr,                                         // UART7 Rx and Tx
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // I2C2 Master and Slave
  phantomIsr,                                         // I2C3 Master and Slave
  phantomIsr,                                         // Timer 4 subtimer A
  phantomIsr,                                         // Timer 4 subtimer B
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // Timer 5 subtimer A
  phantomIsr,                                         // Timer 5 subtimer B
  phantomIsr,                                         // Wide Timer 0 subtimer A
  phantomIsr,                                         // Wide Timer 0 subtimer B
  phantomIsr,                                         // Wide Timer 1 subtimer A
  phantomIsr,                                         // Wide Timer 1 subtimer B
  phantomIsr,                                         // Wide Timer 2 subtimer A
  phantomIsr,                                         // Wide Timer 2 subtimer B
  phantomIsr,                                         // Wide Timer 3 subtimer A
  phantomIsr,                                         // Wide Timer 3 subtimer B
  phantomIsr,                                         // Wide Timer 4 subtimer A
  phantomIsr,                                         // Wide Timer 4 subtimer B
  phantomIsr,                                         // Wide Timer 5 subtimer A
  phantomIsr,                                         // Wide Timer 5 subtimer B
  phantomIsr,                                         // FPU
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // I2C4 Master and Slave
  phantomIsr,                                         // I2C5 Master and Slave
  phantomIsr,                                         // GPIO Port M
  phantomIsr,                                         // GPIO Port N
  phantomIsr,                                         // Quadrature Encoder 2
  nullptr,                                            // Reserved
  nullptr,                                            // Reserved
  phantomIsr,                                         // GPIO Port P (Summary or P0)
  phantomIsr,                                         // GPIO Port P1
  phantomIsr,                                         // GPIO Port P2
  phantomIsr,                                         // GPIO Port P3
  phantomIsr,                                         // GPIO Port P4
  phantomIsr,                                         // GPIO Port P5
  phantomIsr,                                         // GPIO Port P6
  phantomIsr,                                         // GPIO Port P7
  phantomIsr,                                         // GPIO Port Q (Summary or Q0)
  phantomIsr,                                         // GPIO Port Q1
  phantomIsr,                                         // GPIO Port Q2
  phantomIsr,                                         // GPIO Port Q3
  phantomIsr,                                         // GPIO Port Q4
  phantomIsr,                                         // GPIO Port Q5
  phantomIsr,                                         // GPIO Port Q6
  phantomIsr,                                         // GPIO Port Q7
  phantomIsr,                                         // GPIO Port R
  phantomIsr,                                         // GPIO Port S
  phantomIsr,                                         // PWM 1 Generator 0
  phantomIsr,                                         // PWM 1 Generator 1
  phantomIsr,                                         // PWM 1 Generator 2
  phantomIsr,                                         // PWM 1 Generator 3
  phantomIsr                                          // PWM 1 Fault
};
}
}
}

