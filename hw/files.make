# Source includes for jel hardware abstraction layer files.
# 
#	Written By Jonathan Thomson
#
####################################################################################################
# MIT License
# 
# Copyright 2018, Jonathan Thomson 
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
# associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute,
# sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
# NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
# OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
####################################################################################################
#

ASOURCE += $(addprefix hw/,\
		\
		)

CSOURCE += $(addprefix hw/,\
 		\
		)

CXXSOURCE += $(addprefix hw/,\
		generic/startup.cpp \
		generic/uart.cpp \
		generic/wdt.cpp \
		generic/gpio.cpp \
		)

TM4C123GH6PM_ASOURCE += $(addprefix hw/targets/tm4c/,\
		\
		)

TM4C123GH6PM_CSOURCE += $(addprefix hw/targets/tm4c/,\
 		\
		)

RM57L843_CXXSOURCE += $(addprefix hw/targets/rm57/,\
		startup.cpp \
		irq.cpp \
		gpio.cpp \
		sysclock.cpp \
		uart.cpp \
		wdt.cpp \
		)

TM4C123GH6PM_CXXSOURCE += $(addprefix hw/targets/tm4c/,\
		startup.cpp \
		irq.cpp \
		gpio.cpp \
		sysclock.cpp \
		uart.cpp \
		wdt.cpp \
		)

TM4C1294NCPDT_CXXSOURCE += $(addprefix hw/targets/tm4c/,\
		startup.cpp \
		irq.cpp \
		gpio.cpp \
		sysclock.cpp \
		uart.cpp \
		wdt.cpp \
		)

STM32F302RCT6_CXXSOURCE += $(addprefix hw/targets/stm32f3/,\
		startup.cpp \
		irq.cpp \
		gpio.cpp \
		sysclock.cpp \
		uart.cpp \
		wdt.cpp \
		)
