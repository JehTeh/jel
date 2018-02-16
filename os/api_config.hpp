/** @file os/api_config.hpp
 *  @brief jel Configuration components.
 *
 *  @detail
 *    Multiple jel features support custom configuration. These parameters are accessible via this
 *    API. Note that there are two types of parameters - those configurable at the application level
 *    and those that require a rebuild of the jel. Generally speaking, for each supported target
 *    processor, the jel defines an appropriate set of defaults for parameters that would require a
 *    rebuild. If these are not suitable, simply change the required parameters for your required
 *    target and rebuild the jel for that target. These parameters generally include things like
 *    statically allocated buffer sizes for the jel string pool.
 *    Numerous other parameters, however, can be configured by the application at link time. This
 *    includes features like which UART/USB Serial instance to use as the standard system I/O
 *    peripheral. These features should be set by the application by defining a configuration
 *    structure in one of their application source files and linking to the jel, where the weakly
 *    linked default configuration will be overwritten.
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
#include "hw/api_uart.hpp"

namespace jel
{
namespace config
{
#ifdef HW_TARGET_TM4C123GH6PM
/** Determines the total number of strings in the jel shared string pool. The string pool is used
 *  by the CLI and logger. 
 *  
 *  Generally the logger requires at least one String object for printing. The CLI typically
 *  requires at least 3, in addition to an extra string for each string paramater that may be
 *  passed into a command (strings are re-used between commands, so only enough strings are needed
 *  for the command that accepts the most string inputs). Note that each level of CLI history
 *  requires one additional string.
 *
 *  A minimum of 8 strings is strongly recommended, with a CLI history of 1. With a larger history
 *  depth, 16-24 strings is preferable.
 * */
constexpr size_t stringPoolStringCount = 24;
/** Determines the size of each string in the jel shared string pool.
 *  
 *  The total memory used by the string pool is ~100B + (stringPoolStringCount *
 *  stringPoolStringSize) without the optimizeStringMemory flag enabled. With the flag enabled, this
 *  is the maximum possible memory consumed by the string pool. 
 *  A minimum string size of 128B is recommended, although most CLI and other system functionality
 *  will generally work with 64B strings. Note that this value also determines the effective
 *  maximum command length that can be handled by the CLI.
 * */
constexpr size_t stringPoolStringSize = 256;
/** If true, all string pool memory will be allocated up-front during jel initialization. If false,
 * no string memory will be allocated up front but will be allocated as needed. Furthermore, strings
 * actively in use will be continuously trimmed to keep their memory consumption as low as possible.
 * This has the advantage of significantly reducing memory consumption but will require numerous
 * allocations at run-time, which depending on the allocator in use may or may not be ideal.
 * */
constexpr bool optimizeStringMemory = true;
/** Determines the maximum number of history items the CLI will keep in memory. */
constexpr size_t cliHistoryDepth = 8;
/** Determines the maximum number of separate arguments that a command can accept at once. Each
 * argument is defined according to the definitions in api_cli.hpp. 
 * This has a relatively low memory impact (<20B/arg) and will fully support all built in jel
 * functionality with a minimum of 8 arguments. 
 * */
constexpr size_t cliMaximumArguments = 12;
constexpr size_t cliMaximumStringLength = 128;
#else
constexpr size_t stringPoolStringCount = 24;
constexpr size_t stringPoolStringSize = 256;
constexpr bool optimizeStringMemory = true;
constexpr size_t cliHistoryDepth = 8;
constexpr size_t cliMaximumArguments = 12;
#endif

static_assert(stringPoolStringCount > (cliHistoryDepth + 4),
  "There are insufficient strings for the given CLI history depth.");

/** @enum SerialPortType
 *  @brief The type of serial port to instantiate for System I/O
 *
 *  The jel supports various serial port hardware configurations for the standard I/O channel. At
 *  this time, these include:
 *    -uart0: The default debug UART port on the microcontroller. Requires a client with a terminal
 *    emulator such as PuTTY or bash and a bidirectional serial port, either directly or over a USB
 *    to serial converter.
 *    -usbcdc0: The MCU USB port will be used to turn the MCU directly into a USB serial device, at
 *    which point it can be communicated with directly from the host system. The USB serial drivers
 *    take longer to start up than a typical UART driver, and are also not always available on all
 *    microcontrollers.
 *    -usbcomposite0: The MCU USB port will be used, as in usbcdc0, but as a member of a composite
 *    device. This allows later instantation of another USB device sharing the bus, such as a mass
 *    storage driver.
 *  */
enum class SerialPortType
{
  uart0, 
  usbcdc0, /**< Support is pending for TIVA targets. */
  usbcomposite0 /**< Support is pending for TIVA targets. */ 
};

struct JelRuntimeConfiguration
{
  /** Human readable name for this configuration. */
  const char* name;
  /** The type of serial port to instantiate on startup. */
  SerialPortType stdioPortType;
  /** The serial line configuration parameters to use for the system standard I/O channel. */
  const hw::uart::BasicUart::Config stdioUartConfiguration;
};

/** 
 *  If the application requires a custom configuration, it must define this symbol (under the
 *  jel::config:: namespace) somewhere. The linker will automatically override the default, weakly
 *  linked jel configuration with the application configuration.
 * */
extern const JelRuntimeConfiguration jelRuntimeConfiguration;

} /** namespace config */
} /** namespace jel */
