/** @file os/internal/system.cpp
 *  @brief Various system utility calls are implemented here.
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
#include <cassert>
/** jel Library Headers */
#include "os/api_system.hpp"
#include "os/api_threads.hpp"
#include "os/internal/indef.hpp"

namespace jel
{

bool System::cpuExceptionActive() noexcept
{
#if defined(HW_TARGET_RM57L843)
  uint32_t cpsr = _getCPSRValue_() & 0x1F;
  if((cpsrMbits == 0x11) || (cpsrMbits == 0x12))
  {
    return true;
  }
#elif defined(HW_TARGET_TM4C123GH6PM) || defined(HW_TARGET_TM4C129XNCZAD)
  uint32_t* scb_icsr = reinterpret_cast<uint32_t*>(0xE000ED04);
  if((*scb_icsr & 0x1FF) != 0)
  {
    return true;
  }
#else
#ifndef __clang__
#error "CPU architecture not supported."
#endif
#endif
  return false;
}

bool System::inIsr() noexcept
{
#if defined(HW_TARGET_RM57L843)
  uint32_t cpsr = _getCPSRValue_() & 0x1F;
  if((cpsrMbits == 0x12))
  {
    return true;
  }
#elif defined(HW_TARGET_TM4C123GH6PM) || defined(HW_TARGET_TM4C129XNCZAD)
  uint32_t* scb_icsr = reinterpret_cast<uint32_t*>(0xE000ED04);
  if((*scb_icsr & 0x1FF) != 0)
  {
    return true;
  }
#else
#ifndef __clang__
#error "CPU architecture not supported."
#endif
#endif
  return false;
}

CriticalSection::CriticalSection() noexcept
{
  if(System::cpuExceptionActive())
  {
    mask_ = taskENTER_CRITICAL_FROM_ISR();
  }
  else
  {
    taskENTER_CRITICAL();
  }
}

CriticalSection::~CriticalSection() noexcept
{
  if(System::cpuExceptionActive())
  {
    taskEXIT_CRITICAL_FROM_ISR(mask_);
  }
  else
  {
    taskEXIT_CRITICAL();
  }
}

SchedulerLock::SchedulerLock() noexcept
{
  vTaskSuspendAll();
}

SchedulerLock::~SchedulerLock() noexcept
{
  if(xTaskResumeAll() == pdTRUE)
  {
    os::ThisThread::yield();
  }
}

} /** namespace jel */

