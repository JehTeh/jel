/** @file hw/api_pwm.hpp
 *  @brief PWM API
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
#include "jel/os/api_time.hpp"

namespace jel
{
namespace hw
{
namespace pwm
{

/** enum PwmChannelName
 *  @brief Generic PWM channel names for use with multiple microcontrollers.
 *  @note Not all channels are supported by all targets. It is up to the application which channels to use.
 * */
enum class PwmInstance
{
  pwmInstance0, pwmInstance1, pwmInstance2, pwmInstance3,
  pwmInstance4, pwmInstance5, pwmInstance6, pwmInstance7,
  pwmInstance8, pwmInstance9, pwmInstance10, pwmInstance11,
  pwmInstance12, pwmInstance13, pwmInstance14, pwmInstance15,
  pwmInstance16, pwmInstance17, pwmInstance18, pwmInstance19, 
  pwmInstance20, pwmInstance21, pwmInstance22, pwmInstance23,
};

/** class BasicPwmChannel_Base
 * @brief A PwmChannel handles specific PWM operations associated with a single PWM output channel in hardware.
 * This is a simplified base class interface, and may or may not be implemented by the underlying hardware platform.
 * Typically, MCU ports will extend the BasicPwmChannel interface as needed to provide additional functionality
 * supported by the platform, such as deadzones, linked PWMs, interrupts, and so on.
 * @note As the JEL does not currently make use of the BasicPWMChannel_Base API, only a simplified floating point
 * interface is supported as a template for functionality. For targets that do not feature hardware float support it may
 * be desirable to extend the PWM interface with fixed integer support using the BasicPwmChannel_IntegerInterface class.
 * */
class BasicPwmChannel_Base
{
public:
  BasicPwmChannel_Base(PwmInstance instance) : inst_(instance) {  }
  /** Sets the PWM 'active' duty cycle, in terms of percent full duty cycle. Values are clamped within the [0.0f -
   * 100.0f] range, a value above 100.0f will result in a duty cycle of 100.0f, a value below zero will result in a duty
   * cycle
   * of zero. */
  virtual void set_percent(float onDuty_percent) = 0;
  /** Returns the current 'active' duty cycle, in terms of percent full duty cycle. [0.0f - 100.0f] */
  virtual float get_percent() const = 0;
  /** Returns the duration of the currently set duty cycle. 
   * @note: This may not match exactly the desired duty cycle due to hardware limitations. */
  virtual Duration getPeriod() const = 0;
  /** Sets the PWM channel period as closely as possible to the desiredPeriod. The error between the desired and actual
   * period that has been set is returned. */
  virtual Duration setPeriod(const Duration& desiredPeriod) const = 0;
  /** Returns the maximum achievable accuracy of the PWM, in terms of the resolution of a single step size, given the
   * currently configured period. */
  virtual Duration getMinimumStepSize() const = 0;
protected:
  PwmInstance inst_;
};

/** BasicPwmChannel_IntegerInterface
 * @brief An optional, purely virtual extension to the BasicPwmChannel_Base class that implements an integer interface.
 * */
class BasicPwmChannel_IntegerInterface
{
public:
  /** Sets the current PWM 'active' duty cycle, in terms of fractional full-scale increments. The maximum possible
   * onDuty is given by getIntegerFullscale(). */
  virtual void set_integer(uint32_t onDuty_fractionOfFullscale) = 0;
  /** Returns the current PWM 'active' duty cycle, in terms of fractional full-scale increments. The maximum possible
   * duty returned is given by getIntegerFullscale(). */
  virtual uint32_t get_integer() const = 0;
  /** Returns the maximum possible duty cycle available in terms of fractional full-scale increments, based on the
   * current PwmChannel configuration. */
  virtual uint32_t getIntegerFullscale() const = 0;
};


} /** namespace gpio */
} /** namespace hw */
} /** namespace jel */

