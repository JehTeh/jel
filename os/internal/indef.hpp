/** @file os/indef.hpp
 *  @brief Definitions used internally within the os layer. 
 *
 *  @detail
 *    This header should not be included by any application files, only jel os files.
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

/** jel Library Headers */
#include "os/api_common.hpp"
#include "os/api_time.hpp"
/** RTOS Library Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"

namespace jel
{
namespace os
{

inline TickType_t toTicks(const Duration& d) noexcept
{
  //Assumption is made that TickType_t is an integer type.
  constexpr int64_t usPerTick = 1'000'000 / configTICK_RATE_HZ;
  int64_t t_us = d.toMicroseconds();
  if(t_us > 0)
  {
    TickType_t ticks = t_us / usPerTick;
    if(ticks == 0) { return 1; }
    return ticks > UINT32_MAX ? UINT32_MAX : ticks;
  }
  else
  {
    return 0;
  }
}

}
}