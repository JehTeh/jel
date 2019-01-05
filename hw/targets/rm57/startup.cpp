/** @file hw/targets/rm57/startup.cpp
 *  @brief Hardware specific startup routines for the RM57 MCU.
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
#include "hw/api_startup.hpp"
/** TI driver layer headers */
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sys_vim.h"
#include "HL_sys_core.h"
#include "HL_esm.h"
#include "HL_sys_mpu.h"

extern "C"
{
extern void _resetVector(void) __attribute__((noreturn));

void _jelEntry(void) __attribute__((noreturn));
}

void _jelEntry(void) 
{
  /** TODO */
  _resetVector();
}

namespace jel
{
namespace hw
{
namespace startup
{
namespace reset
{
  extern ResetSourceType lastResetSource;
}
  void defaultInitializeClocks();
  void enableFpu();

  void customDispatcher() noexcept
  {
    _memInit_();
    _coreInitRegisters_();
    _coreInitStackPointer_();
    switch(getResetSource())
    {
      case POWERON_RESET:
      case DEBUG_RESET:
      case EXT_RESET:
        _coreEnableEventBusExport_();
        if ((esmREG->SR1[2]) != 0U)
        {
          esmGroup3Notification(esmREG,esmREG->SR1[2]);               
        }
        systemInit();
        _coreEnableIrqVicOffset_();
	      vimInit();
        esmInit();
        break;
      case CPU0_RESET:
      case SW_RESET:
        _coreEnableEventBusExport_();
        if ((esmREG->SR1[2]) != 0U)
        {
          esmGroup3Notification(esmREG,esmREG->SR1[2]);               
        }
        systemInit();
        _coreEnableIrqVicOffset_();
	      vimInit();
        esmInit();
        break;
      case OSC_FAILURE_RESET:
      case WATCHDOG_RESET:
      case WATCHDOG2_RESET:
      default:
        break;
    }
    _mpuInit_();
    _cacheEnable_();
    switch(getResetSource())
    {
      case POWERON_RESET: reset::lastResetSource = reset::ResetSourceType::powerOnReset; break;
      case DEBUG_RESET: reset::lastResetSource = reset::ResetSourceType::debugReset; break;
      case EXT_RESET: reset::lastResetSource = reset::ResetSourceType::externalReset; break;
      case CPU0_RESET: reset::lastResetSource = reset::ResetSourceType::cpu0Reset; break;
      case SW_RESET: reset::lastResetSource = reset::ResetSourceType::softwareReset; break;
      case OSC_FAILURE_RESET: reset::lastResetSource = reset::ResetSourceType::oscillatorFailureReset; break;
      case WATCHDOG_RESET: reset::lastResetSource = reset::ResetSourceType::watchdogReset; break;
      case WATCHDOG2_RESET: reset::lastResetSource = reset::ResetSourceType::watchdog2Reset; break;
      default: reset::lastResetSource = reset::ResetSourceType::unknown; break;
    }
  }

  void defaultInitializeClocks()
  {
    //For the RM57, clocks are initialized as part of the systemInit(); call made in the customDispatcher.
  }

  void enableFpu()
  {
    //For the RM57, the FPU is initialized as part of the systemInit(); call made in the customDispatcher.
  }

  void enableMpu()
  {
    //For the RM57, the MPU is initialized as part of the customDispatcher function.
  }

namespace reset
{

  ResetSourceType getResetSource() noexcept
  {
    return lastResetSource;
  }

  const char* resetSourceToString(const ResetSourceType source) noexcept
  {
    switch(source)
    {
      case ResetSourceType::powerOnReset: return "Power On";
      case ResetSourceType::debugReset: return "Debug";
      case ResetSourceType::externalReset: return "External";
      case ResetSourceType::cpu0Reset: return "CPU0";
      case ResetSourceType::softwareReset: return "Software";
      case ResetSourceType::oscillatorFailureReset: return "Oscillator Failure";
      case ResetSourceType::watchdogReset: return "Watchdog";
      case ResetSourceType::watchdog2Reset: return "Secondary Watchdog";
      case ResetSourceType::unknown: return "Unknown";
      default: return "Invalid Reset Source";
    }
  }

}
}
}
}
