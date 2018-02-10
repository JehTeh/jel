/** @file hw/targets/tm4c/sysclock.cpp
 *  @brief Implementation of the TM4C system steady clock source.
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
#include "hw/api_sysclock.hpp"
/** Tivaware Library Headers */
#include "driverlib/timer.h"

namespace jel
{
namespace hw
{
namespace sysclock
{
#ifdef HW_TARGET_TM4C123GH6PM
void SystemSteadyClockSource::startClock()
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_WTIMER0));
  TimerConfigure(WTIMER0_BASE, TIMER_CFG_ONE_SHOT_UP);
  TimerClockSourceSet(WTIMER0_BASE, TIMER_CLOCK_SYSTEM); //Using system clock as source.
  TimerLoadSet64(WTIMER0_BASE, UINT64_MAX);
  TimerEnable(WTIMER0_BASE, TIMER_A);
}

uint64_t SystemSteadyClockSource::readClock() noexcept
{
  //Convert timer value from system clock (max 80Mhz) to 1us.
  return TimerValueGet64(WTIMER0_BASE) / (systemClockFrequency_Hz() / 1'000'000);
}
#endif

} /** namespace sysclock */
} /** namespace hw */
} /** namespace jel */

