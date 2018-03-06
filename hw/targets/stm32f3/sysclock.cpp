/** @file hw/targets/stm32f3/sysclock.cpp
 *  @brief Implementation of the STM32F3 system steady clock source.
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
#include "hw/api_sysclock.hpp"
#include "os/api_system.hpp"
/** stm HAL Headers */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister"
#pragma GCC diagnostic ignored "-Wuninitialized"
#include "tim.h"
#include "stm32f3xx_ll_tim.h"
#pragma GCC diagnostic pop

namespace jel
{
namespace hw
{
namespace sysclock
{

TIM_HandleTypeDef& sclk = htim2;
uint32_t sclkCount;

void SystemSteadyClockSource::startClock()
{
  MX_TIM2_Init();
  HAL_TIM_Base_Start_IT(&sclk);
  sclkCount = 0;
}

uint64_t SystemSteadyClockSource::readClock() noexcept
{
  CriticalSection cs; 
  uint32_t low, high;
  {
    CriticalSection cs;
    uint32_t flagStatusBefore = LL_TIM_IsActiveFlag_UPDATE(sclk.Instance);
    low = LL_TIM_GetCounter(sclk.Instance);
    uint32_t flagStatusAfter = LL_TIM_IsActiveFlag_UPDATE(sclk.Instance);
    high = sclkCount;
    //Check if overflow occured.
    if(flagStatusAfter != flagStatusBefore)
    {
      low = LL_TIM_GetCounter(sclk.Instance);
      //Increment copy of tick count here, actual value will be incremented by ISR function after
      //critical section is exited.
      high++;
    }
  }
  return (static_cast<uint64_t>(high) << 32) | low;
}

} /** namespace sysclock */
} /** namespace hw */
} /** namespace jel */

extern "C" void jel_hw_sysclock_isr(void)
{
  using namespace jel::hw::sysclock;
  sclkCount++;
  __HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_UPDATE);
}
