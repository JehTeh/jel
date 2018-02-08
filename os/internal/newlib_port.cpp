/** @file os/internal/newlib_port.cpp
 *  @brief All libc/c++ porting functions are supplied here.
 *
 *  @detail
 *    To provide significant additonal functionality, newlib support is built into the jel. The jel
 *    leverages various portions of the C/C++ standard libraries and also provides the low level
 *    porting functionality needed to run the C/C++ STL on various embedded targets.
 *
 *    The following functionality is supported:
 *      -Newlib printf/scanf, both via stdout and stdin. These are redirected to the same interface
 *      the jel uses for the CLI and logging.
 *      -Malloc/New overrides. All allocations are performed via the System Allocator by default,
 *      with the option to specify a custom allocator that supports the jel api_allocator interface.
 *      -Filesystem functionality. On targets with hardware support, such as a microSD card, the
 *      fatfs filesystem is exposed via the standard fopen()/fclose() functions and their C++
 *      equivalents. 
 *      -Other required newlib porting functions (for compilation with -lnosys). 
 *
 *    The jel is built to make minimal use of newlib functions that have a large memory impact. This
 *    means that printf floats are not used, nor is the std::string or std::cout library components
 *    and associated functionality. This functionality may be used by the target if desired, in
 *    which case the linker will include the needed functions in the final binary.
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


