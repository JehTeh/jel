/** @file os/api_system.hpp
 *  @brief Interface to various system functions such as current CPU context.
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

class System
{
public:
  /** Returns true if called from within a CPU exception state. This includes IRQ and FIQs, in
   * addition to various abort states on ARM. */
  static bool cpuExceptionActive() noexcept;
  /** Returns true if called from within an IRQ execution state. */
  static bool inIsr() noexcept;
};

/** @class CriticalSection
 *  @brief An RAII object that halts all scheduler execution and interrupts while it exists.
 *  
 *  A CriticalSection can be created as needed to protect data from asynchronous access in other
 *  threads and interrupts. This should be used extremely sparingly and for limited scope, as it
 *  may result in interrupt or task execution deadlines being missed. Where protection from
 *  interrupts is not needed (for example, an interrupt would never interact with the data being
 *  protected) then use of a SchedulerLock is preferred. 
 * */
class CriticalSection
{
public:
  CriticalSection() noexcept;
  ~CriticalSection() noexcept;
  CriticalSection(const CriticalSection&) = delete;
  CriticalSection(CriticalSection&&) = delete;
  CriticalSection& operator=(const CriticalSection&) = delete;
  CriticalSection& operator=(CriticalSection&&) = delete;
protected:
  uint32_t mask_;
};

/** @class SchedulerLock
 *  @brief An RAII object that halts only scheduler operation while it exists.
 *
 *  A SchedulerLock is similar to a CriticalSection in that it prevents any thread rescheduling from
 *  occurring (including scheduling the execution of a higher priority thread). It does not, 
 *  however, stop interrupts from occurring and as such is preferred over a CriticalSection where
 *  possible.
 * */
class SchedulerLock
{
public:
  SchedulerLock() noexcept;
  ~SchedulerLock() noexcept;
  SchedulerLock(const SchedulerLock&) = delete;
  SchedulerLock(SchedulerLock&&) = delete;
  SchedulerLock& operator=(const SchedulerLock&) = delete;
  SchedulerLock& operator=(SchedulerLock&&) = delete;
};

}


