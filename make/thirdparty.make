# Third party flags and variables to be included in top level jel makefile.
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

#Include paths for RTOS functionality. The freeRTOS core files are always built, but depending on
#the target CPU or freeRTOS+TCP requirements, a variation in porting files may be needed.
RTOS_INCLUDE_PATH_BASE = ".\ThirdParty\amazon-freertos\lib\include"
RTOS_INCLUDE_PATH_ARM_R5 = ".\ThirdParty\amazon-freertos\lib\FreeRTOS\portable\GCC\ARM_CR5"
RTOS_INCLUDE_PATH_ARM_M4F = ".\ThirdParty\amazon-freertos\lib\FreeRTOS\portable\GCC\ARM_CM4F"
#Cpputest include directory. Note: cpputest is not built by the master project makefile. It must be
#manually built to a cpputest\lib\[TARGET_CPU].a file.
CPPUTEST_INCLUDE_PATH_BASE = ".\ThirdParty\cpputest\include"
