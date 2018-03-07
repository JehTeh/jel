/** @file hw/generic/wdt.cpp
 *  @brief Generalized MCU hardware watchdog definitions. These are weakly-linked stubs that should
 *  be overridden in the target specific hardware implementation where appropriate.
 *  Note that the only WDT related functions implemented have to do with forcing system reset, as
 *  these are required by the jel whereas full WDT functionality is not.
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
#include "hw/api_wdt.hpp"

namespace jel
{
namespace hw
{
namespace wdt
{
__attribute__((weak)) void WdtController::systemReset()
{
  volatile uint32_t* aircr = reinterpret_cast<uint32_t*>(0xE000ED04);
  *aircr = 0x5FA0'0004;
}

} /** namespace wdt */
} /** namespace hw */
} /** namespace jel */

