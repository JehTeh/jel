/** @file os/api_time.hpp
 *  @brief System timekeeping components
 *
 *  @detail
 *    All jel systems make use of std::chrono based time primitives for timestamps and deltaT
 *    measurements. Furthermore, a common SteadyClock singleton clock class is provided that allows
 *    generation of timestamps from a minimum microsecond precision source. The SteadyClock
 *    implementation is based on the std::chrono::steady_clock class and is intended to comply as
 *    fully as possible within the bounds of the underlying microcontroller hardware.
 *
 *  @todo
 *    -Add RTC interface for systems that include an RTC. Ideally designed such that if no RTC
 *    hardware is available, a simplistic approximation can be built using the SteadyClock instead.
 *    Furthermore, efforts will be made to ensure the SteadyClock and RTC can be related in a
 *    meaningful manner.
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
#include <chrono>

/** jel Library Headers */
#include "os/api_common.hpp"
#include "hw/api_sysclock.hpp"

namespace jel
{

/** @class SteadyClock
 *  @brief Provides a constantly increasing microsecond or greater resolution clock.
 *
 *  The SteadyClock static class is built around the std::chrono library duration and time_point
 *  types. It provides a continuously upwards counting clock source with a guaranteed resolution of
 *  at least 1 microsecond. As per the C++ standard, is_steady is always true for this clock source
 *  (i.e. it will never be adjusted to match network time or similar, it instead reflects the time
 *  since the last system boot).
 *  See the Timestamp class for an example of the SteadyClock usage.
 * */
struct SteadyClock
{
public:
  static constexpr uint64_t SteadyClockFreq_Hz = 1'000'000;
  typedef std::chrono::duration<int64_t, std::ratio<1, SteadyClockFreq_Hz>> duration;
  typedef duration::rep rep; //Note: rep is a signed 64b type on arm-none-eabi-gcc.
  typedef duration::period period;
  typedef std::chrono::time_point<SteadyClock> time_point;
  static const bool is_steady = true;
  static time_point now() noexcept 
  { return time_point{duration{hw::sysclock::SystemSteadyClockSource::readClock()}};}
  static constexpr time_point zero() noexcept { return time_point{duration{0}}; }
private:
};

/** @class Duration
 *  @brief Wraps the std::chrono::duration type built into SteadyClock to provide additional
 *  functionality.
 *
 *  The Duration class is designed to extend the functionality of std::chrono::duration. It provides
 *  an interface based solely around the systems SteadyClock while still being compatible with all
 *  std::chrono features. Some useful elements of the Duration class include:
 *    -Built in, rounding conversion to seconds/milliseconds/microseconds.
 *    -Simplified construction of duration spans from integer types.
 *  Example of usage:
 *  @code
 *  systemIo.scan(buffr, buffer_len, Duration::milliseconds{250}); //Pend on input for up to 250ms.
 *  //...
 *  auto d = Duration::microseconds{1234567)};
 *  log->print("Dur. length is %lu seconds, or %lu milliseconds", d.toSeconds(), d.toMilliseconds());
 *  @endcode
 *
 * */
class Duration : public SteadyClock::duration
{
public:
  /** Default construct a duration with a span of 0 SteadyClock::rep. */
  inline constexpr Duration() : duration{SteadyClock::duration{0}} {}
  /** Copy constructor. */
  inline constexpr Duration(const Duration& d) : duration{SteadyClock::duration{d}} {}
  /** Move constructor. */
  inline constexpr Duration(Duration&& d) : duration{SteadyClock::duration{std::move(d)}} {}
  /** Copy assignment. */
  inline constexpr Duration& operator=(const Duration& d) 
    { this->duration::operator=(d); return *this; }
  /** Copy construct from a std::chrono::duration. */
  inline constexpr Duration(const SteadyClock::duration& d) : duration{d} { }
  /** Move construct from another std::chrono::duration. */
  inline constexpr Duration(const SteadyClock::duration&& d) : duration{std::move(d)} { }
  /** Returns the current span of the duration, in microseconds. Rounding is performed if the system
   * SteadyClock is running at sub-microsecond precision. */
  inline constexpr int64_t toMicroseconds() const
    {
      constexpr int64_t d = period().den; constexpr int64_t ratio = 1'000'000;
      return (count() + ((d / ratio) / 2)) / (d / ratio);
    }
  /** Returns the current span of the duration, in milliseconds. Rounding is performed. */
  inline constexpr int64_t toMilliseconds() const
    {
      constexpr int64_t d = period().den; constexpr int64_t ratio = 1'000;
      return (count() + ((d / ratio) / 2)) / (d / ratio);
    }
  /** Returns the current span of the duration, in integer seconds. Rounding is performed. */
  inline constexpr int64_t toSeconds() const
    {
      constexpr int64_t d = period().den; constexpr int64_t ratio = 1;
      return (count() + ((d / ratio) / 2)) / (d / ratio);
    }
  /** Create a duration that is t microseconds in span. */
  static constexpr Duration microseconds(const int64_t t) 
    { return Duration{std::chrono::microseconds{t}}; }
  /** Create a duration that is t milliseconds in span. */
  static constexpr Duration milliseconds(const int64_t t)
    { return Duration{std::chrono::milliseconds{t}}; }
  /** Create a duration that is t seconds in span. */
  static constexpr Duration seconds(const int64_t t)
    { return Duration{std::chrono::seconds{t}}; }
};

/** @class Timestamp
 *  @brief Extends std::chrono::time_point functionality based on the system SteadyClock.
 *
 *  Timestamps can be used for storing points in time, which is useful when deltaT calculations need
 *  to be performed or for identifying when during system operation a specific event took place.
 *  Timestamps are essentially composed of a duration and a reference point, for the Timestamp class
 *  this reference point is the '0' time of the SteadyClock (last system boot).
 *  Example usage:
 *  @code
 *  Timestamp start = SteadyClock::now(); //Snapshot current time.
 *  runBenchmarkCode(); //Perform some action.
 *  Timestamp end = SteadyClock::now(); //Snapshot completion time.
 *  Duration deltaT = end - start; //We now know how long the 'runBenchmarkCode()' operation took.
 *  @endcode
 *
 * */
class Timestamp : public SteadyClock::time_point
{
public:
  /** Default construct a timestamp with a value of 0. */
  inline constexpr Timestamp() : time_point{SteadyClock::zero()} { }
  /** Copy construct from another timestamp. */
  inline constexpr Timestamp(const Timestamp& ts) : time_point{ts} { }
  /** Move construct from another timestamp. */
  inline constexpr Timestamp(Timestamp&& ts) : time_point{std::move(ts)} { }
  /** Copy construct from a time_point. */
  inline constexpr Timestamp(const time_point& tp) : time_point{tp} { }
  /** Move construct from a time_point. */
  inline constexpr Timestamp(time_point&& tp) : time_point{std::move(tp)} { }
  /** Copy construct from a duration. */
  inline constexpr Timestamp(const SteadyClock::duration& d) : time_point{d} { }
  /** Move construct from a duration. */
  inline constexpr Timestamp(SteadyClock::duration&& d) : time_point{std::move(d)} { }
  /** Move and copy assignment from Timestamps and time_points */
  inline Timestamp& operator=(const Timestamp& rhs) 
    { time_point::operator= (rhs); return *this; }
  inline Timestamp& operator=(Timestamp&& rhs) 
    { time_point::operator= (rhs); return *this; }
  inline Timestamp& operator=(const time_point& rhs) 
    { time_point::operator= (rhs); return *this; }
  inline Timestamp& operator=(const time_point&& rhs) 
    { time_point::operator= (rhs); return *this; }
  inline constexpr Duration toDuration() const { return Duration{this->time_since_epoch()}; }
};

}
