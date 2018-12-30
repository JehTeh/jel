/** @file hw/targets/rm57/gpio.cpp
 *  @brief RM57 GPIO definitions
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
/** TI Halcogen Library Headers */
#include "HL_gio.h"

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
    case PortName::gpioPortA: return static_cast<PortName>(reinterpret_cast<intptr_t>(const_cast<gioPort*>(gioPORTA)));
    case PortName::gpioPortB: return static_cast<PortName>(reinterpret_cast<intptr_t>(const_cast<gioPort*>(gioPORTB)));
    default: return (PortName)(0);
  }
};

void GpioController::initializeGpio()
{
  gioInit();
}

Pin::Pin(PortName port, PinNumber pin) : port_(portNameToGpioPortPointer(port)), pin_(pin)
{
  if(static_cast<intptr_t>(port_) == 0)
  {
    throw Exception(ExceptionCode::driverFeatureNotSupported, 
      "This port is not available on this processor.");
  }
}

void Pin::set()
{
  const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DSET = static_cast<uint32_t>(pin_);
}

void Pin::reset()
{
  const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DCLR = static_cast<uint32_t>(pin_);
}

bool Pin::read() const
{
  return (const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DIN & 
      static_cast<uint32_t>(pin_)) != 0 ? true : false;
}

Port::Port(const PortName port) : port_(portNameToGpioPortPointer(port))
{
  if(static_cast<intptr_t>(port_) == 0)
  {
    throw Exception(ExceptionCode::driverFeatureNotSupported, 
      "This port is not available on this processor.");
  }
}

void Port::write(const PinNumber pins)
{
  const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DSET = static_cast<uint32_t>(pins);
}

void Port::write(const PinNumber pins, const PinNumber mask)
{
  const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DSET = static_cast<uint32_t>(pins) &
    static_cast<uint32_t>(mask);
}

PinNumber Port::read(const PinNumber mask) const
{
  return static_cast<PinNumber>(const_cast<gioPORT_t*>(reinterpret_cast<gioPort*>(static_cast<intptr_t>(port_)))->DIN & 
      static_cast<uint32_t>(mask));
}

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

