/** @file os/internal/config.cpp
 *  @brief Definition of default jel runtime configurations for various targets.
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
#include "os/internal/indef.hpp"
#include "os/api_config.hpp"

namespace jel
{
namespace config
{

static constexpr auto defaultBaud = hw::uart::Baudrate::bps1Mbit;

#ifdef HW_TARGET_TM4C123GH6PM
const JelRuntimeConfiguration jelRuntimeConfiguration_tm4c123gh6pm =
{
  "tm4c123gh6pm_default", SerialPortType::uart0,
  {
    hw::uart::UartInstance::uart0, defaultBaud, hw::uart::Parity::none,
    hw::uart::StopBits::one, hw::uart::WordLength::eight, hw::uart::BlockingMode::isr, 
    hw::uart::BlockingMode::isr
  },
};

extern const JelRuntimeConfiguration __attribute__((weak)) jelRuntimeConfiguration = 
  jelRuntimeConfiguration_tm4c123gh6pm;
#elif defined(HW_TARGET_STM32F302RCT6)
const JelRuntimeConfiguration jelRuntimeConfiguration_stm32f302rct6=
{
  "stm32f302rct6_default", SerialPortType::uart0,
  {
    hw::uart::UartInstance::uart1, hw::uart::Baudrate::bps115200, hw::uart::Parity::none,
    hw::uart::StopBits::one, hw::uart::WordLength::eight, hw::uart::BlockingMode::isr, 
    hw::uart::BlockingMode::polling
  },
};

extern const JelRuntimeConfiguration __attribute__((weak)) jelRuntimeConfiguration = 
  jelRuntimeConfiguration_stm32f302rct6;
#else
const JelRuntimeConfiguration jelRuntimeConfiguration_nocfg =
{
  "nocfg_default", SerialPortType::uart0,
  {
    hw::uart::UartInstance::uart0, defaultBaud, hw::uart::Parity::none,
    hw::uart::StopBits::one, hw::uart::WordLength::eight, hw::uart::BlockingMode::isr, 
    hw::uart::BlockingMode::isr
  },
};

extern const JelRuntimeConfiguration __attribute__((weak)) jelRuntimeConfiguration =
  jelRuntimeConfiguration_nocfg;
#endif

} /** namespace config */
} /** namespace jel */

