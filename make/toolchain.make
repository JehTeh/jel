# Toolchain definitions to be included in the top level jel makfile.
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
ifeq ($(OS), Windows_NT)
  PLATFORM_PATH_SEP=\\
else
  PLATFORM_PATH_SEP=/
endif


#Compiler and linker. jel only supports the ARM GCC bare metal toolchain.
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
#GCC is intentionally used for linking instead of ld.
LD = arm-none-eabi-gcc
#Other tools used for archiving.
AR = arm-none-eabi-gcc-ar
NM = arm-none-eabi-nm
READELF = arm-none-eabi-readelf
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
#Clang-tidy is integrated into the makefile to allow for linting on individual files as needed.
CLANGTIDY = clang-tidy
#JLINK related definitions. Only useful if you have a segger jlink probe with J-FLASH license.
JLINK_EXE = "C:\Program Files (x86)\SEGGER\JLink_V634a\JFlash.exe"
#C/C++ stdlib locations (in this case, newlib). Note: If updating to a new version of arm-none-eabi
#or using a custom compiled newlib, (or have arm-none-eabi installed in a non-standard directory) 
#ensure you update this path.
LIBCXX_INCLUDE_PATH_LINKER_HARDFLOAT = \
  "C:\Program Files (x86)\GNU Tools ARM Embedded\7 2017-q4-major\arm-none-eabi\lib\hard"
LIBCXX_INCLUDE_PATH_LINKER_SOFTFLOAT = \
  "C:\Program Files (x86)\GNU Tools ARM Embedded\7 2017-q4-major\arm-none-eabi\lib"
LIBCXX_INCLUDE_PATH_COMPILER = \
  "C:\Program Files (x86)\GNU Tools ARM Embedded\7 2017-q4-major\arm-none-eabi\include"
