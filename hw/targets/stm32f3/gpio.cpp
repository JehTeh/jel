/** @file hw/targets/stm32f3/gpio.cpp
 *  @brief STM32 GPIO definitions
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
#include "hw/api_gpio.hpp"
#include "hw/api_exceptions.hpp"
/** STM HAL Headers */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister" 
#include "gpio.h"
#pragma GCC diagnostic pop

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
    case PortName::gpioPortA: return static_cast<PortName>(reinterpret_cast<intptr_t>(GPIOA));
    case PortName::gpioPortB: return static_cast<PortName>(reinterpret_cast<intptr_t>(GPIOB));
    case PortName::gpioPortC: return static_cast<PortName>(reinterpret_cast<intptr_t>(GPIOC));
    case PortName::gpioPortD: return static_cast<PortName>(reinterpret_cast<intptr_t>(GPIOD));
    case PortName::gpioPortF: return static_cast<PortName>(reinterpret_cast<intptr_t>(GPIOF));
    default: return (PortName)(0);
  }
};

void GpioController::initializeGpio()
{
  MX_GPIO_Init();
}

Pin::Pin(const PortName port, const uint8_t pin) : port_(portNameToGpioPortPointer(port)), pin_(pin)
{
  if(static_cast<intptr_t>(port_) == 0)
  {
    throw Exception(ExceptionCode::driverFeatureNotSupported, 
      "This port is not available on this processor.");
  }
}

void Pin::set()
{
  HAL_GPIO_WritePin(reinterpret_cast<GPIO_TypeDef*>(static_cast<intptr_t>(port_)),
    1 << pin_, GPIO_PIN_SET);
}

void Pin::reset()
{
  HAL_GPIO_WritePin(reinterpret_cast<GPIO_TypeDef*>(static_cast<intptr_t>(port_)),
    1 << pin_, GPIO_PIN_RESET);
}

bool Pin::read() const
{
  return HAL_GPIO_ReadPin(reinterpret_cast<GPIO_TypeDef*>(static_cast<intptr_t>(port_)),
    1 << pin_) 
    != 0 ? true : false;
}

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

