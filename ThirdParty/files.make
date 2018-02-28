# Source includes for third party files.
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

#FreeRTOS files
ASOURCE += $(addprefix ThirdParty\amazon-freertos\lib\FreeRTOS\,\
		\
		)

CSOURCE += $(addprefix ThirdParty\amazon-freertos\lib\FreeRTOS\,\
   	event_groups.c \
   	list.c \
   	queue.c \
   	stream_buffer.c \
   	tasks.c \
   	timers.c \
    portable\MemMang\heap_4.c \
		)

CXXSOURCE += $(addprefix ThirdParty\amazon-freertos\lib\FreeRTOS\,\
		\
		)

#FreeRTOS files - Target specific porting files
TM4C123GH6PM_CSOURCE += $(addprefix ThirdParty\amazon-freertos\lib\FreeRTOS\portable\GCC\ARM_CM4F\,\
   	port.c \
		)

STM32F302RCT6_CSOURCE += $(addprefix ThirdParty\amazon-freertos\lib\FreeRTOS\portable\GCC\ARM_CM4F\,\
   	port.c \
		)
#STM HAL source files
STM32F302RCT6_HAL_CSOURCE += $(addprefix ThirdParty\STM-Cube-MX\jel_base\,\
   	Src\usart.c \
   	Src\main.c \
   	Src\gpio.c \
   	Src\tim.c \
   	Src\system_stm32f3xx.c \
   	Src\stm32f3xx_it.c \
   	Src\stm32f3xx_hal_msp.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_rcc_ex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_uart_ex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_uart.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_tim_ex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_rcc.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_cortex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_i2c_ex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_tim.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_i2c.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_pwr.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_flash.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_dma.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_pwr_ex.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_gpio.c \
   	Drivers\STM32F3xx_HAL_Driver\Src\stm32f3xx_hal_flash_ex.c \
		)
