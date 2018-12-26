/** @file hw/generic/startup.cpp
 *  @brief Generalized MCU hardware startup definitions. These are weakly-linked stubs that should
 *  be overridden in the target specific hardware implementation where appropriate.
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
#include "hw/api_startup.hpp"

extern "C"
{
extern void _resetVector(void) __attribute__((noreturn));
extern void _jelEntry(void) __attribute__((noreturn));
}

__attribute__((weak)) void _jelEntry()
{
  _resetVector();
}

namespace jel
{
namespace hw
{
namespace startup
{
  void defaultInitializeClocks() __attribute__((weak));
  void enableFpu() __attribute__((weak));
  void enableMpu() __attribute__((weak));
  void customDispatcher() noexcept __attribute__((weak));
  void customDispatcherPostDataInit() noexcept __attribute__((weak));

  void defaultInitializeClocks()
  {
    /** Nothing can be done here in the generic function. */
  }

  void enableFpu()
  {
    /** Nothing can be done here in the generic function. */
  }

  void enableMpu()
  {
    /** Nothing can be done here in the generic function. */
  }

  void customDispatcher() noexcept
  {
    /** Nothing can be done here in the generic function. */
  }

  void customDispatcherPostDataInit() noexcept
  {
    /** Nothing can be done here in the generic function. */
  }
namespace reset
{
  ResetSourceType getResetSource() noexcept __attribute__((weak));
  const char* resetSourceToString(const ResetSourceType source) noexcept __attribute__((weak));
  ResetSourceType lastResetSource;

  ResetSourceType getResetSource() noexcept
  {
    return lastResetSource;
  }

  const char* resetSourceToString(const ResetSourceType source) noexcept
  {
    switch(source)
    {
      case ResetSourceType::unknown: return "Unknown";
      default: return "Invalid Reset Source";
    }
  }

}
} /** namespace startup */
} /** namespace hw */
} /** namespace jel */

