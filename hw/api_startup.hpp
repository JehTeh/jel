/** @file hw/api_startup.hpp
 *  @brief Abstracted system initialization routines.
 *
 *  @detail
 *    The startup routines that need to be implemented on various targets are defined here. They
 *    primarily relate to the custom crt0 _start function implementation (whether assembly or
 *    otherwise) that ensure the MCU stack is setup appropriately. Clock/CPU initialization
 *    functions may also be provided although are not required, default stubs are implemented that
 *    rely on the default MCU configuration after reset.
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

/** jel Library Headers */

namespace jel
{
namespace hw
{
namespace startup
{
  void defaultInitializeClocks();
  void enableFpu();
  void enableMpu();
} /** namespace startup */
} /** namespace hw */
} /** namespace jel */

