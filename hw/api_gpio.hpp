/** @file hw/api_gpio.hpp
 *  @brief General purpose I/O API.
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

#pragma once

/** C/C++ Standard Library Headers */
#include <cstdint>
/** jel Library Headers */

namespace jel
{
namespace hw
{
namespace gpio
{

class GpioController
{
public:
  /** Enables the GPIO pins and default multiplexing options. */
  static void initializeGpio();
};

/** enum PortName
 *  @brief Generic port names for use with multiple microcontrollers.
 *  @note Not all ports are supported by all targets. It is up to the application which ports to use
 *  @note GPIO ports can be referred to either numerically or alphabetically. For the purposes of
 *  the GPIO driver these are equivalent. 
 * */
enum class PortName : intptr_t
{
  gpioPortA, gpioPortB, gpioPortC, gpioPortD, gpioPortE, gpioPortF,
  gpioPortG, gpioPortH, gpioPortI, gpioPortJ, gpioPortK, gpioPortL,
  gpioPortM, gpioPortN, gpioPortO, gpioPortP, gpioPortQ, gpioPortR,
  gpioPortS, gpioPortT, gpioPortU, gpioPortV, gpioPortW,
  gpioPort0 = gpioPortA, gpioPort1 = gpioPortB, gpioPort2 = gpioPortC,
  gpioPort3 = gpioPortD, gpioPort4 = gpioPortE, gpioPort5 = gpioPortF,
  gpioPort6 = gpioPortG, gpioPort7 = gpioPortH, gpioPort8 = gpioPortI,
  gpioPort9 = gpioPortJ, gpioPort11 = gpioPortK, gpioPort12 = gpioPortL,
  gpioPort13 = gpioPortM, gpioPort14 = gpioPortN, gpioPort15 = gpioPortO,
  gpioPort16 = gpioPortP, gpioPort17 = gpioPortQ, gpioPort18 = gpioPortR,
  gpioPort19 = gpioPortS, gpioPort20 = gpioPortT, gpioPort21 = gpioPortU,
  gpioPort22 = gpioPortV, gpioPort23 = gpioPortW, 
  nullPort
};

class Pin
{
public:
  Pin(PortName port, uint8_t pin);
  void set();
  void reset();
  bool read() const;
  bool operator==(bool state) const;
private:
  PortName port_;
  uint8_t pin_;
};

class Port
{
public:
  void write(uint32_t mask, uint32_t value);
  uint32_t read(uint32_t mask) const;
private:
};

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

