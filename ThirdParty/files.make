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
ASOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/,\
		\
		)

CSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/,\
   	event_groups.c \
   	list.c \
   	queue.c \
   	stream_buffer.c \
   	tasks.c \
   	timers.c \
    portable/MemMang/heap_4.c \
		)

CXXSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/,\
		\
		)

#FreeRTOS files - Target specific porting files
RM57L843_ASOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CRx_No_GIC/,\
    portASM.S \
		)

RM57L843_CSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CRx_No_GIC/,\
    port.c \
		)

TM4C123GH6PM_CSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM4F/,\
   	port.c \
		)

TM4C1294NCPDT_CSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM4F/,\
   	port.c \
		)

STM32F302RCT6_CSOURCE += $(addprefix ThirdParty/amazon-freertos/lib/FreeRTOS/portable/GCC/ARM_CM4F/,\
   	port.c \
		)
#TI RM57 HAL source files
RM57L843_HAL_ASOURCE += $(addprefix ThirdParty/halcogen/rm57/jel_hcg/source/,\
    HL_sys_core.s \
    HL_sys_intvecs.s \
    HL_sys_mpu.s \
    HL_sys_pmu.s \
		)

RM57L843_HAL_CSOURCE += $(addprefix ThirdParty/halcogen/rm57/jel_hcg/source/,\
    HL_adc.c \
    HL_can.c \
    HL_crc.c \
    HL_dcc.c \
    HL_ecap.c \
    HL_emac.c \
    HL_emif.c \
    HL_epc.c \
    HL_eqep.c \
    HL_errata.c \
    HL_esm.c \
    HL_etpwm.c \
    HL_gio.c \
    HL_het.c \
    HL_i2c.c \
    HL_lin.c \
    HL_mdio.c \
    HL_mibspi.c \
    HL_nmpu.c \
    HL_notification.c \
    HL_phy_dp83640.c \
    HL_phy_tlk111.c \
    HL_pinmux.c \
    HL_pom.c \
    HL_rti.c \
    HL_sci.c \
    HL_sys_dma.c \
    HL_sys_main.c \
    HL_sys_pcr.c \
    HL_sys_phantom.c \
    HL_sys_pmm.c \
    HL_sys_vim.c \
    HL_system.c \
		)
#HL_sys_startup.c  -  Do not link TI's startup code

#STM HAL source files
STM32F302RCT6_HAL_CSOURCE += $(addprefix ThirdParty/STM-Cube-MX/jel_base/,\
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c \
    Src/tim.c \
    Src/stm32f3xx_hal_timebase_TIM.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c \
    Src/stm32f3xx_it.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim_ex.c \
    Src/system_stm32f3xx.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c \
    Src/usart.c \
    Src/main.c \
    Src/gpio.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart_ex.c \
    Src/stm32f3xx_hal_msp.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc_ex.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim.c \
    Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c \
		)
