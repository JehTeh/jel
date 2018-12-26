/** @file hw/targets/rm57/sysclock.cpp
 *  @brief Implementation of the RM57 system steady clock source.
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
#include "hw/api_irq.hpp"
#include "os/api_system.hpp"
/** TI Halcogen Headers */
#include "HL_system.h"
#include "HL_rti.h"

namespace jel
{
namespace hw
{
namespace sysclock
{
void SystemSteadyClockSource::startClock()
{
  void rtiInit(void);
  /** Configure the Compare Up Counter to overflow and increment the FRC when at the full 32b count. This should result
   * in an FRC value of RTI_FREQ*(2^32)+1. */
  rtiREG1->CNT->CPUCx = 0xFFFF'FFFF;
  rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);
}

uint64_t SystemSteadyClockSource::readClock() noexcept
{
  uint64_t count;
  {
    jel::CriticalSection cs;
    /** Note: The read of the Free Running Counter (FRC) triggers a capture in hardware of the current Up Counter value.
     * By setting the COMPARE register to 0xFFFFFFFF we effectively get a 64b wide counter at the RTI clock frequency.
     * The FRC ends up store the upper 32b of the counter, and the UC stores the lower 32b. */
    count = rtiREG1->CNT[rtiCOUNTER_BLOCK1].FRCx << 31;
    count |= rtiREG1->CNT[rtiCOUNTER_BLOCK1].UCx;
  }
  return count;
}

} /** namespace sysclock */
} /** namespace hw */
} /** namespace jel */
