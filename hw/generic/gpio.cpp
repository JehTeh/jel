/** @file hw/generic/gpio.cpp
 *  @brief Generic GPIO definitions. These do not perform any action and are strictly stubs.
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
#include "hw/api_gpio.hpp"

jel::hw::gpio::Pin
  __attribute__((weak)) jel_HeartbeatIdlePin(jel::hw::gpio::PortName::nullPort, 0);

namespace jel
{
namespace hw
{
namespace gpio
{

void __attribute__((weak)) GpioController::initializeGpio()
{
  /** Does nothing */
}

__attribute__((weak)) Pin::Pin(PortName port, uint8_t pin) : port_(port), pin_(pin)
{

}

void __attribute__((weak)) Pin::set()
{

}

void __attribute__((weak)) Pin::reset()
{

}

bool __attribute__((weak)) Pin::read() const
{
  return false;
}

bool __attribute__((weak)) Pin::operator==(bool state) const
{
  return read() == state ? true : false;
}

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */


