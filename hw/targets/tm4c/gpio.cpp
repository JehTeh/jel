/** @file hw/targets/tm4c/gpio.cpp
 *  @brief Tiva C GPIO definitions
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
#include <cassert>
/** jel Library Headers */
#include "hw/targets/tm4c/tiva_shared.hpp"
#include "hw/api_gpio.hpp"
#include "hw/api_exceptions.hpp"
/** Tivaware Library Headers */
#include "driverlib/gpio.h"

__attribute__((weak)) jel::hw::gpio::Pin
  jel_HeartbeatIdlePin(jel::hw::gpio::PortName::gpioPort0, 0);

namespace jel
{
namespace hw
{
namespace gpio
{

constexpr PortName portNameToGpioPortPointer(PortName port)
{
  switch(port)
  {
    case PortName::gpioPortA: return static_cast<PortName>(GPIO_PORTA_BASE);
    case PortName::gpioPortB: return static_cast<PortName>(GPIO_PORTB_BASE);
    case PortName::gpioPortC: return static_cast<PortName>(GPIO_PORTC_BASE);
    case PortName::gpioPortD: return static_cast<PortName>(GPIO_PORTD_BASE);
    case PortName::gpioPortE: return static_cast<PortName>(GPIO_PORTE_BASE);
    default: return (PortName)(0);
  }
};

void GpioController::initializeGpio()
{
  auto init = [](uint32_t peripheral)
  {
    SysCtlPeripheralEnable(peripheral);
    while(!SysCtlPeripheralReady(peripheral));
  };
  init(SYSCTL_PERIPH_GPIOA);
  init(SYSCTL_PERIPH_GPIOB);
  init(SYSCTL_PERIPH_GPIOC);
  init(SYSCTL_PERIPH_GPIOD);
  init(SYSCTL_PERIPH_GPIOE);
}

Pin::Pin(PortName port, uint8_t pin) : port_(portNameToGpioPortPointer(port)), pin_(pin)
{
  if(static_cast<intptr_t>(port_) == 0)
  {
    throw Exception(ExceptionCode::driverFeatureNotSupported, 
      "This port is not available on this processor.");
  }
}

void Pin::set()
{
  GPIOPinWrite(static_cast<intptr_t>(port_), 1 << pin_, 0xFF);
}

void Pin::reset()
{
  GPIOPinWrite(static_cast<intptr_t>(port_), 1 << pin_, 0x00);
}

bool Pin::read() const
{
  return GPIOPinRead(static_cast<intptr_t>(port_), 1 << pin_) != 0 ? true : false;
}

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

