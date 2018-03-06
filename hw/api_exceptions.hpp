/** @file hw/api_exceptions.hpp
 *  @brief Exception definitions used by hardware peripherals.
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
#include "os/api_exceptions.hpp"

namespace jel
{
namespace hw
{

enum class ExceptionCode : uint32_t
{
  driverInstantiationFailed,
  driverFeatureNotSupported,
  driverInstanceNotAvailable,
  receiveOverrun,
};

class Exception : public Exception_Base<RESERVED_HW_MODULE_ID, ExceptionCode>
{
public:
  template<typename ... A>
  Exception(const ExceptionCode& code, const char* msg, A&& ... printfArgs) :
    Exception_Base<RESERVED_HW_MODULE_ID, ExceptionCode>(code, msg, std::forward<A>(printfArgs) ...)
    {}
};

} /** namespace hw */
} /** namespace jel */

