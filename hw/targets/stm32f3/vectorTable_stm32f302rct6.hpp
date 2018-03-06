/** @file hw/targets/stm32f3/vectorTable_stm32f302rct6.cpp
 *  @brief The default flash ISR vector table for the STM32F302RCT6 CPU
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

extern "C"
{
void faultIsrWrapper(void);
 
extern void _jelEntry(void) __attribute__((noreturn));
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void DebugMon_Handler(void);
extern void WWDG_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void PVD_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TAMP_STAMP_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void RTC_WKUP_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void FLASH_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void RCC_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI0_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI2_TSC_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI4_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel4_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel5_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel6_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA1_Channel7_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void ADC1_2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USB_HP_CAN_TX_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USB_LP_CAN_RX0_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void CAN_RX1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void CAN_SCE_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI9_5_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM1_BRK_TIM15_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM1_UP_TIM16_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM1_CC_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM4_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void I2C1_EV_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void I2C1_ER_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void I2C2_EV_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void I2C2_ER_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void SPI1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void SPI2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USART1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USART2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USART3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void EXTI15_10_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void RTC_Alarm_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USBWakeUp_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void SPI3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void UART4_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void UART5_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void TIM6_DAC_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA2_Channel1_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA2_Channel2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA2_Channel3_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA2_Channel4_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void DMA2_Channel5_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void COMP1_2_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void COMP4_6_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USB_HP_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USB_LP_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void USBWakeUp_RMP_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
extern void FPU_IRQHandler(void) __attribute__((weak, alias ("faultIsrWrapper")));
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
  &_jelEntry,         
  &faultIsr,          
  &faultIsr,          
  &faultIsr,          
  &faultIsr,          
  &faultIsr,          
  0,
  0,
  0,
  0,
  vPortSVCHandler,
  DebugMon_Handler,
  0,
  xPortPendSVHandler,
  xPortSysTickHandler,
  WWDG_IRQHandler,
  PVD_IRQHandler,
  TAMP_STAMP_IRQHandler,
  RTC_WKUP_IRQHandler,
  FLASH_IRQHandler,
  RCC_IRQHandler,
  EXTI0_IRQHandler,
  EXTI1_IRQHandler,
  EXTI2_TSC_IRQHandler,
  EXTI3_IRQHandler,
  EXTI4_IRQHandler,
  DMA1_Channel1_IRQHandler,
  DMA1_Channel2_IRQHandler,
  DMA1_Channel3_IRQHandler,
  DMA1_Channel4_IRQHandler,
  DMA1_Channel5_IRQHandler,
  DMA1_Channel6_IRQHandler,
  DMA1_Channel7_IRQHandler,
  ADC1_2_IRQHandler,
  USB_HP_CAN_TX_IRQHandler,
  USB_LP_CAN_RX0_IRQHandler,
  CAN_RX1_IRQHandler,
  CAN_SCE_IRQHandler,
  EXTI9_5_IRQHandler,
  TIM1_BRK_TIM15_IRQHandler,
  TIM1_UP_TIM16_IRQHandler,
  TIM1_TRG_COM_TIM17_IRQHandler,
  TIM1_CC_IRQHandler,
  TIM2_IRQHandler,
  TIM3_IRQHandler,
  TIM4_IRQHandler,
  I2C1_EV_IRQHandler,
  I2C1_ER_IRQHandler,
  I2C2_EV_IRQHandler,
  I2C2_ER_IRQHandler,
  SPI1_IRQHandler,
  SPI2_IRQHandler,
  USART1_IRQHandler,
  USART2_IRQHandler,
  USART3_IRQHandler,
  EXTI15_10_IRQHandler,
  RTC_Alarm_IRQHandler,
  USBWakeUp_IRQHandler,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  SPI3_IRQHandler,
  UART4_IRQHandler,
  UART5_IRQHandler,
  TIM6_DAC_IRQHandler,
  0,
  DMA2_Channel1_IRQHandler,
  DMA2_Channel2_IRQHandler,
  DMA2_Channel3_IRQHandler,
  DMA2_Channel4_IRQHandler,
  DMA2_Channel5_IRQHandler,
  0,
  0,
  0,
  COMP1_2_IRQHandler,
  COMP4_6_IRQHandler,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  USB_HP_IRQHandler,
  USB_LP_IRQHandler,
  USBWakeUp_RMP_IRQHandler,
  0,
  0,
  0,
  0,
  FPU_IRQHandler
};


}
}
}

void faultIsrWrapper(void)
{
  jel::hw::irq::faultIsr();
}


