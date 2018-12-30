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
/** Third Party Headers */
#include "ThirdParty/misc/enum_flags.h"

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

/** enum PinNumber
 *  @brief Numerical pin identifiers.
 *  Used to identify the pins on an arbitrary hardware port. Note that not all targets will support
 *  all pins; it is the responsibility of the application to ensure pins used are valid for the
 *  target hardware port.
 *  @note PinNumbers are masked bitflags and support bitwise operations with other PinNumbers. This
 *  generally allows for multiple pins to be treated as a single combined pin where appropriate.
 * */
ENUM_FLAGS(PinNumber, uint32_t)
enum class PinNumber : uint32_t 
{
  pin0  = 0x00'00'00'01,
  pin1  = 0x00'00'00'02,
  pin2  = 0x00'00'00'04,
  pin3  = 0x00'00'00'08,
  pin4  = 0x00'00'00'10,
  pin5  = 0x00'00'00'20,
  pin6  = 0x00'00'00'40,
  pin7  = 0x00'00'00'80,
  pin8  = 0x00'00'01'00,
  pin9  = 0x00'00'02'00,
  pin10 = 0x00'00'04'00,
  pin11 = 0x00'00'08'00,
  pin12 = 0x00'00'10'00,
  pin13 = 0x00'00'20'00,
  pin14 = 0x00'00'40'00,
  pin15 = 0x00'00'80'00,
  none  = 0x00'00'00'00,
  all   = 0xFF'FF'FF'FF
};

/** class Pin
 *  @brief A Pin object defines a single I/O pin that can be read or controlled.
 *  Most hardware targets supported by the jel can use a bitflag style pin definition. This allows
 *  for multiple 'pins' to be treated as a single logical pin if so desired, simply by constructing
 *  a Pin object with multiple pins on the same port. For example,
 *  @code
 *    Pin singlePin(PortName::gpioPortA, PinNumber::pin0); //Single pin on port A.
 *    Pin multiPin(PortName::gpioPortA, PinNumber::pin0 | PinNumber::pin1); //Multiple pins on port
 *    //A. Overlap is allowed in Pin definitions.
 *  @endcode
 *  While this can allow for controlling mutliple pins on the same port, it does not provide the
 *  single/multiple pin masking and selection offered via the Port object, as all pins defined in
 *  the pin mask are either set/reset/toggled at once. Reading the state of the pins, where multiple
 *  pins are masked, is generally excepted to always return true if one or more pins in the mask is
 *  set.
 * */
class Pin
{
public:
  /** Construct a new pin object. This requires a valid hardware port name and a pin number on that
   * port.
   * @throws Exception::driverFeatureNotSupported if the port and/or pin number is invalid for the
   * target. */
  Pin(PortName port, PinNumber pin);
  /** Sets the pin to a logical '1' state. */
  void set();
  /** Clears the pin to a logical '0' state. */
  void reset();
  /** Returns the logical state of the pin. */
  bool read() const;
  /** Toggles the state of the pin. */
  inline void toggle() { if(*this){ reset(); } else { set(); }}
  /** Evaluates the state of the pin using the read() function. */
  inline bool operator==(bool state) const { return state == read() ? true : false; }
  /** Assigns the state of the pin using either the set() or reset() functions. */
  inline void operator=(bool newState) { if(newState) { set(); } else { reset(); }}
  /** Evaluates the state of the pin using the read() function. */
  inline operator bool() const { return read(); }
private:
  PortName port_;
  PinNumber pin_;
};

/** class Port
 *  @brief A Port object defines a single hardware I/O port interface. Multiple pins can be read or
 *  toggled on the port as desired.
 * */
class Port
{
public:
  /** Constructs a new port object.
   * @throws Exception::driverFeatureNotSupported if the portis invalid for the * target. */
  Port(PortName port);
  /** Writes to the port. Non-zeroed pins in the 'pins' argument are set to logical '1', zeroed pins
   * in the argument are cleared. */
  void write(PinNumber pins);
  /** Performs a read-modify-write (atomicity is target dependent) to the port. Operates in the same
   * manner as the single argument write() function, except for ensuring that any pins not defined
   * in the mask argument will not have their values modified. */
  void write(PinNumber pins, PinNumber mask);
  /** Reads the state of all the pins on the port. If a mask argument is specified, pins that are
   * not set in the mask argument will be read as zero irrespective of their actual state. */
  PinNumber read(PinNumber mask = PinNumber::all) const;
  /** Assignment operator that performs a write() operation. */
  inline void operator=(PinNumber pins) { write(pins); }
  /** Conversion operator that returns the value of a read() call with no masking. */
  inline operator PinNumber() const { return read(); }
private:
  PortName port_;
};

/** On hardware targets that support GPIO loopback, a pin or port can be wrapped in an IoLoopbackWrapper that enables
 * the hardware loopback mode on that pin or port. */
class IoLoopbackWrapper
{
public:
  IoLoopbackWrapper(Pin& pin, bool internalLoopbackOnly = true);
  IoLoopbackWrapper(Port& port, bool internalLoopbackOnly = true);
  ~IoLoopbackWrapper() noexcept;
};

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

