# Common make definitions used by the jel. Intended to be included in the top level jel makefile.
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
############ COMPILER ############
#Generic compiler warning flags, used for all builds
COMPILER_FLAGS_WARNING = -Wall -Wextra -Wno-unused-parameter -fmax-errors=25
#Generic compiler include flags, used for all builds
COMPILER_FLAGS_INCLUDE_GENERIC = -I . -I $(RTOS_INCLUDE_PATH_BASE) -I $(RTOS_PRIVATE_INCLUDE_PATH_BASE) -I $(RTOS_CONFIGURATION_INCLUDE_PATH_BASE)
#Special compiler flags used for debug builds (Cpputest and regular debug builds).
COMPILER_FLAGS_DEBUGBUILD = -gdwarf-2 -g -ffunction-sections -fdata-sections -Wl,--gc-sections
#Special compiler flags for optimized builds.
COMPILER_FLAGS_RELEASEBUILD = -ffunction-sections -fdata-sections -Wl,--gc-sections
#Compiler optimization levels.
COMPILER_FLAGS_DEBUGBUILD_LEVEL = -O0
COMPILER_FLAGS_RELEASEBUILD_LEVEL = -O2
#Final, common compiler flags
FINAL_COMPILER_COMMON_FLAGS_DEBUGBUILD = $(COMPILER_FLAGS_DEBUGBUILD_LEVEL) $(COMPILER_FLAGS_WARNING) $(COMPILER_FLAGS_DEBUGBUILD) $(COMPILER_FLAGS_INCLUDE_GENERIC)
FINAL_COMPILER_COMMON_FLAGS_RELEASEBUILD = $(COMPILER_FLAGS_RELEASEBUILD_LEVEL) $(COMPILER_FLAGS_WARNING) $(COMPILER_FLAGS_RELEASEBUILD) $(COMPILER_FLAGS_INCLUDE_GENERIC)
#C++ standard
COMPILER_FLAGS_CPP_STANDARD = -std=c++14
#Compiler flag enabling Link-Time-Optimization. Can be disabled by simply commenting out the flag.
#This may need to be done as some arm-none-eabi versions have issues with LTO.
#COMPILER_FLAGS_LTO = -flto -fuse-linker-plugin -Wl,-flto,-fuse-linker-plugin
############ LINKER ############
#Standard libraries to include with the linker
#Only libstdc++ and libc are needed for building the jel.
LINKER_FLAGS_STANDARD_LIBRARIES = -lstdc++ -lc
#Common linker flags used for all targets
LINKER_FLAGS_COMMON = -static -nostartfiles $(COMPILER_FLAGS_LTO) $(LINKER_FLAGS_STANDARD_LIBRARIES)
#Linker includes. 
LINKER_FLAGS_INCLUDE_GENERIC_HF = -I . -I $(LIBCXX_INCLUDE_PATH_LINKER_HARDFLOAT)
LINKER_FLAGS_INCLUDE_GENERIC_SF = -I . -I $(LIBCXX_INCLUDE_PATH_LINKER_SOFTFLOAT)
#Functions that are wrapped by the linker. This is used to override newlib memory allocation
#completely.
LINKER_FLAGS_WRAP_FUNCTIONS = \
  -Xlinker --wrap=malloc -Xlinker --wrap=_malloc_r -Xlinker --wrap=realloc -Xlinker \
  --wrap=_realloc_r -Xlinker --wrap=calloc -Xlinker --wrap=_calloc_r -Xlinker \
  --wrap=free -Xlinker --wrap=_free_r -Xlinker --wrap=pvPortMalloc -Xlinker --wrap=vPortFree
############# FLAGS ############
#Common flags 
AFLAGS_BASE = -c $(FINAL_COMPILER_COMMON_FLAGS_RELEASEBUILD)
CFLAGS_BASE = -c $(FINAL_COMPILER_COMMON_FLAGS_RELEASEBUILD) $(COMPILER_FLAGS_LTO)
CXXFLAGS_BASE = -c $(FINAL_COMPILER_COMMON_FLAGS_RELEASEBUILD) $(COMPILER_FLAGS_CPP_STANDARD) $(COMPILER_FLAGS_LTO)
LDFLAGS_BASE = $(LINKER_FLAGS_COMMON) $(FINAL_COMPILER_COMMON_FLAGS_RELEASEBUILD) $(LINKER_FLAGS_WRAP_FUNCTIONS)
AFLAGS_BASE_DEBUG = -c $(FINAL_COMPILER_COMMON_FLAGS_DEBUGBUILD)
CFLAGS_BASE_DEBUG = -c $(FINAL_COMPILER_COMMON_FLAGS_DEBUGBUILD) 
CXXFLAGS_BASE_DEBUG = -c $(FINAL_COMPILER_COMMON_FLAGS_DEBUGBUILD) $(COMPILER_FLAGS_CPP_STANDARD) 
LDFLAGS_BASE_DEBUG = $(LINKER_FLAGS_COMMON) $(FINAL_COMPILER_COMMON_FLAGS_DEBUGBUILD) $(LINKER_FLAGS_WRAP_FUNCTIONS)

