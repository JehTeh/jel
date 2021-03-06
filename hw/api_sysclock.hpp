/** @file hw/api_sysclock.hpp
 *  @brief System steady clock interface definitions.
 *
 *  @detail
 *    The system steady clock source is used by jel for all basic timing purposes. It is required
 *    that the steady clock be a free running counter that is always incrementing, with at least 1
 *    microsecond of resolution. 
 *    It is acceptable for the steady clock to 'leap forward' in certain explicit cases, such as
 *    when the system exits a low power mode but before full scheduler functionality is re-enabled.
 *    It is not acceptable for the steady clock to ever run, or appear to run, backwards. Successive
 *    calls to readClock() must always return either the same or greater value.
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
namespace sysclock
{

class SystemSteadyClockSource
{
public:
  static void startClock();
  static uint64_t readClock() noexcept;
};

} /** namespace startup */
} /** namespace hw */
} /** namespace jel */

