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
#include "os/api_allocator.hpp"
#include "os/api_io.hpp"
#include "os/api_log.hpp"
#include "os/api_config.hpp"
/** RTOS Library Headers */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"

#ifdef HW_TARGET_RM57L843
extern "C" 
{
  extern volatile uint32_t ulPortYieldRequired;	
}
#ifdef portEND_SWITCHING_ISR
#undef portEND_SWITCHING_ISR
#define portEND_SWITCHING_ISR( xSwitchRequired )\
{												\
	if( xSwitchRequired != pdFALSE )			\
	{											\
		::ulPortYieldRequired = pdTRUE;			\
	}											\
}
#endif
#endif

namespace jel
{

inline TickType_t toTicks(const Duration& d) noexcept
{
  //Assumption is made that TickType_t is an integer type.
  constexpr uint64_t usPerTick = 1'000'000 / configTICK_RATE_HZ;
  if(d < Duration::zero()) { return 0; }
  uint64_t t_us = d.toMicroseconds();
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

typedef ObjectPool<String> JelStringPool;
extern std::shared_ptr<JelStringPool> jelStringPool;
/** This is intentionally not just a shared pointer - this pointer is set before libc constructor
 * calls and must persist through them. To avoid requiring a double de-reference, objects should
 * cache their own shared_ptr copy. */
extern std::shared_ptr<AsyncIoStream> jelStandardIo;
extern std::shared_ptr<Logger> jelLogger;

extern const char* jelBuildDateString;
extern const char* jelBuildTimeString;
extern const char* jelCompilerVersionString;

} /** namespace jel */
