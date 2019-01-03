/** @file hw/targets/rm57/vectorTable_rm57l843.cpp
 *  @brief The default flash vector table for the RM57L843 MCU.
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

/** C/C++ Standard Library Headers */
#include <cstdint>
/** jel Library Headers */

extern "C"
{
extern void _jelEntry(void) __attribute__((noreturn));
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
}

namespace jel
{
namespace hw
{
namespace irq 
{

extern void phantomIsr();
extern void faultIsr();

using IsrPtr = void(*)(void);

static IsrPtr __attribute__((section (".vectorTable"), used)) vectorTable[] =
{
  /** TODO: proper VIM integration, freeRTOS porting (9.0 -> 10.0 R5 build) */
    &phantomIsr,
    &faultIsr,                                      /* Channel 0  esmHighInterrupt */
    &phantomIsr,                                    /* Channel 1   */
    &xPortSysTickHandler,                           /* Channel 2  rtiCompare0Interrupt */
    &phantomIsr,                                    /* Channel 3   */
    &phantomIsr,                                    /* Channel 4   */
    &phantomIsr,                                    /* Channel 5   */
    &phantomIsr,                                    /* Channel 6   */
    &phantomIsr,                                    /* Channel 7   */
    &phantomIsr,                                    /* Channel 8   */
    &phantomIsr,                                    /* Channel 9   */
    &phantomIsr,                                    /* Channel 10  */
    &phantomIsr,                                    /* Channel 11  */
    &phantomIsr,                                    /* Channel 12  */
    &phantomIsr,                                    /* Channel 13  */
    &phantomIsr,                                    /* Channel 14  */
    &phantomIsr,                                    /* Channel 15  */
    &phantomIsr,                                    /* Channel 16  */
    &phantomIsr,                                    /* Channel 17  */
    &phantomIsr,                                    /* Channel 18  */
    &phantomIsr,                                    /* Channel 19  */
    &phantomIsr,                                    /* Channel 20  */
    &phantomIsr,                                    /* Channel 21  */
    &phantomIsr,                                    /* Channel 22  */
    &phantomIsr,                                    /* Channel 23  */
    &phantomIsr,                                    /* Channel 24  */
    &phantomIsr,                                    /* Channel 25  */
    &phantomIsr,                                    /* Channel 26  */
    &phantomIsr,                                    /* Channel 27  */
    &phantomIsr,                                    /* Channel 28  */
    &phantomIsr,                                    /* Channel 29  */
    &phantomIsr,                                    /* Channel 30  */
    &phantomIsr,                                    /* Channel 31  */
    &phantomIsr,                                    /* Channel 32  */
    &phantomIsr,                                    /* Channel 33  */
    &phantomIsr,                                    /* Channel 34  */
    &phantomIsr,                                    /* Channel 35  */
    &phantomIsr,                                    /* Channel 36  */
    &phantomIsr,                                    /* Channel 37  */
    &phantomIsr,                                    /* Channel 38  */
    &phantomIsr,                                    /* Channel 39  */
    &phantomIsr,                                    /* Channel 40  */
    &phantomIsr,                                    /* Channel 41  */
    &phantomIsr,                                    /* Channel 42  */
    &phantomIsr,                                    /* Channel 43  */
    &phantomIsr,                                    /* Channel 44  */
    &phantomIsr,                                    /* Channel 45  */
    &phantomIsr,                                    /* Channel 46  */
    &phantomIsr,                                    /* Channel 47  */
    &phantomIsr,                                    /* Channel 48  */
    &phantomIsr,                                    /* Channel 49  */
    &phantomIsr,                                    /* Channel 50  */
    &phantomIsr,                                    /* Channel 51  */
    &phantomIsr,                                    /* Channel 52  */
    &phantomIsr,                                    /* Channel 53  */
    &phantomIsr,                                    /* Channel 54  */
    &phantomIsr,                                    /* Channel 55  */
    &phantomIsr,                                    /* Channel 56  */
    &phantomIsr,                                    /* Channel 57  */
    &phantomIsr,                                    /* Channel 58  */
    &phantomIsr,                                    /* Channel 59  */
    &phantomIsr,                                    /* Channel 60  */
    &phantomIsr,                                    /* Channel 61  */
    &phantomIsr,                                    /* Channel 62  */
    &phantomIsr,                                    /* Channel 63  */
    &phantomIsr,                                    /* Channel 64  */
    &phantomIsr,                                    /* Channel 65  */
    &phantomIsr,                                    /* Channel 66  */
    &phantomIsr,                                    /* Channel 67  */
    &phantomIsr,                                    /* Channel 68  */
    &phantomIsr,                                    /* Channel 69  */
    &phantomIsr,                                    /* Channel 70  */
    &phantomIsr,                                    /* Channel 71  */
    &phantomIsr,                                    /* Channel 72  */
    &phantomIsr,                                    /* Channel 73  */
    &phantomIsr,                                    /* Channel 74  */
    &phantomIsr,                                    /* Channel 75  */
    &phantomIsr,                                    /* Channel 76  */
    &phantomIsr,                                    /* Channel 77  */
    &phantomIsr,                                    /* Channel 78  */
    &phantomIsr,                                    /* Channel 79  */
    &phantomIsr,                                    /* Channel 80  */
    &phantomIsr,                                    /* Channel 81  */
    &phantomIsr,                                    /* Channel 82  */
    &phantomIsr,                                    /* Channel 83  */
    &phantomIsr,                                    /* Channel 84  */
    &phantomIsr,                                    /* Channel 85  */
    &phantomIsr,                                    /* Channel 86  */
    &phantomIsr,                                    /* Channel 87  */
    &phantomIsr,                                    /* Channel 88  */
    &phantomIsr,                                    /* Channel 89  */
    &phantomIsr,                                    /* Channel 90  */
    &phantomIsr,                                    /* Channel 91  */
    &phantomIsr,                                    /* Channel 92  */
    &phantomIsr,                                    /* Channel 93  */
    &phantomIsr,                                    /* Channel 94  */
    &phantomIsr,                                    /* Channel 95  */
    &phantomIsr,                                    /* Channel 96  */
    &phantomIsr,                                    /* Channel 97  */
    &phantomIsr,                                    /* Channel 98  */
    &phantomIsr,                                    /* Channel 99  */
    &phantomIsr,                                    /* Channel 100 */
    &phantomIsr,                                    /* Channel 101 */
    &phantomIsr,                                    /* Channel 102 */
    &phantomIsr,                                    /* Channel 103 */
    &phantomIsr,                                    /* Channel 104 */
    &phantomIsr,                                    /* Channel 105 */
    &phantomIsr,                                    /* Channel 106 */
    &phantomIsr,                                    /* Channel 107 */
    &phantomIsr,                                    /* Channel 108 */
    &phantomIsr,                                    /* Channel 109 */
    &phantomIsr,                                    /* Channel 110 */
    &phantomIsr,                                    /* Channel 111 */
    &phantomIsr,                                    /* Channel 112 */
    &phantomIsr,                                    /* Channel 113 */
    &phantomIsr,                                    /* Channel 114 */
    &phantomIsr,                                    /* Channel 115 */
    &phantomIsr,                                    /* Channel 116 */
    &phantomIsr,                                    /* Channel 117 */
    &phantomIsr,                                    /* Channel 118 */
    &phantomIsr,                                    /* Channel 119 */
    &phantomIsr,                                    /* Channel 120 */
    &phantomIsr,                                    /* Channel 121 */
    &phantomIsr,                                    /* Channel 122 */
    &phantomIsr,                                    /* Channel 123 */
    &phantomIsr,                                    /* Channel 124 */
    &phantomIsr,                                    /* Channel 125 */
    &phantomIsr,                                    /* Channel 126 */
};

}
}
}

