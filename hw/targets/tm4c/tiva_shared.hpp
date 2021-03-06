/** @file hw/targets/tm4c/tiva_shared.hpp
 *  @brief Definitions shared by all tiva C hardware implementations.
 *
 *  @detail
 *    Due to the similarities between all tiva series MCUs, most hardware driver functionality is
 *    shared and conditionally compiled using #define statements.
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

/** Tivaware Library Headers */
#define TIVAWARE
#ifdef HW_TARGET_TM4C123GH6PM
#define PART_TM4C123GH6PM
#define TARGET_IS_TM4C123_RB2
#endif
#ifdef HW_TARGET_TM4C1294NCPDT
#define PART_TM4C1294NCPDT
#endif
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

namespace jel
{
namespace hw
{
#ifdef HW_TARGET_TM4C123GH6PM
  constexpr uint32_t systemClockFrequency_Hz() { return 80'000'000; }
#endif
#ifdef HW_TARGET_TM4C1294NCPDT
  inline uint32_t systemClockFrequency_Hz() { return 120'000'000; }
#endif

} /** namespace hw */
} /** namespace jel */

