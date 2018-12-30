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
#include "os/api_common.hpp"
#include "hw/api_exceptions.hpp"

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

__attribute__((weak)) Pin::Pin(PortName port, PinNumber pin) : port_(port), pin_(pin)
{
  /** Does nothing */
}

void __attribute__((weak)) Pin::set()
{
  /** Does nothing */
}

void __attribute__((weak)) Pin::reset()
{
  /** Does nothing */
}

bool __attribute__((weak)) Pin::read() const
{
  return false;
}

__attribute__((weak)) Port::Port(const PortName port) : port_(port)
{
  /** Does nothing */
}

void __attribute__((weak)) Port::write(const PinNumber)
{
  /** Does nothing */
}

void __attribute__((weak)) Port::write(const PinNumber, const PinNumber)
{
  /** Does nothing */
}

PinNumber __attribute__((weak)) Port::read(const PinNumber) const
{
  return PinNumber::none;
}

#ifdef TARGET_SUPPORTS_CPPUTEST
/** The GPIO_Pin test group just evaluates some simple instantations of the Pin class - Depending on the hardware design
 * these default tests could interfere with the board and it may be better to avoid running them. On targets with
 * hardware loopback support, it may be preferable to enable that explicitly before running any tests. */
TEST_GROUP(JEL_TestGroup_HW_GPIO_Pin)
{
  /** Port to run PIN tests on */
  static constexpr PortName gioTestPort = PortName::gpioPort0;
  /** Maximum pin value to test */
  static constexpr PinNumber maxPin = PinNumber::pin15;
  /** Minimum pin value to test */
  static constexpr PinNumber minPin = PinNumber::pin0;
  /** Other pin value to test */
  static constexpr PinNumber testPin0 = PinNumber::pin1;
  /** Other pin value to test */
  static constexpr PinNumber testPin1 = PinNumber::pin5;
  void setup()
  {
    
  }
  void teardown()
  {

  }
  template<PinNumber pinNum> void TestSetPin(void)
  {
    try
    {
      Pin pin(gioTestPort, pinNum);
      #ifdef HW_TARGET_RM57L843
      IoLoopbackWrapper lpbk(pin);
      #endif
      pin.set();
      CHECK(pin.read());
    }
    catch(const Exception& e)
    {
      if(e.error == ExceptionCode::driverFeatureNotSupported)
      {
        FAIL(e.what());
      }
      else
      {
        throw(e);
      }
    }
  }
  template<PinNumber pinNum> void TestClearPin(void)
  {
    try
    {
      Pin pin(gioTestPort, pinNum);
      #ifdef HW_TARGET_RM57L843
      IoLoopbackWrapper lpbk(pin);
      #endif
      pin.reset();
      CHECK(!pin.read());
    }
    catch(const Exception& e)
    {
      if(e.error == ExceptionCode::driverFeatureNotSupported)
      {
        FAIL(e.what());
      }
      else
      {
        throw(e);
      }
    }
  }
  template<PinNumber pinNum> void TestTogglePin(void)
  {
    try
    {
      Pin pin(gioTestPort, pinNum);
      #ifdef HW_TARGET_RM57L843
      IoLoopbackWrapper lpbk(pin);
      #endif
      auto first_read = pin.read();
      pin.toggle();
      CHECK(pin.read() != first_read);
    }
    catch(const Exception& e)
    {
      if(e.error == ExceptionCode::driverFeatureNotSupported)
      {
        FAIL(e.what());
      }
      else
      {
        throw(e);
      }
    }
  }
}; 

TEST(JEL_TestGroup_HW_GPIO_Pin, SetMaxTestPin) { TestSetPin<maxPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, SetMinTestPin) { TestSetPin<minPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, SetTestPin0) { TestSetPin<testPin0>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, SetTestPin1) { TestSetPin<testPin1>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ClearMaxTestPin) { TestClearPin<maxPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ClearMinTestPin) { TestClearPin<minPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ClearTestPin0) { TestClearPin<testPin0>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ClearTestPin1) { TestClearPin<testPin1>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ToggleMaxTestPin) { TestTogglePin<maxPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ToggleMinTestPin) { TestTogglePin<minPin>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ToggleTestPin0) { TestTogglePin<testPin0>(); }
TEST(JEL_TestGroup_HW_GPIO_Pin, ToggleTestPin1) { TestTogglePin<testPin1>(); }
#endif

} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */


