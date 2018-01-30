# Target definitions for the Texas Instruments Tiva TM4C123GH6PM ARM M4F Processor.
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

#Target specific flags, including -DHW_TARGET_[XXX] flag.
TM4C123GH6PM_CUSTOM_FLAGS = -DHW_TARGET_TM4C123GH6PM

#Definition of target specific rules found in the ./make/rules.make file.
TARGET_NAME = tm4c123gh6pm_dbg
AFLAGS = $(AFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
CFLAGS = $(CFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
CXXFLAGS = $(CXXFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
LDFLAGS = $(LDFLAGS_BASE_DEBUG)
LINKER_SCRIPT =
ARCHIVER_SCRIPT = 

ALL_OBJECT_FILES =
TARGET_ASOURCE =
TARGET_CSOURCE =
TARGET_CXXSOURCE =

OUTPUT_BINARY_FILE =
OUTPUT_ELF_FILE =
OUTPUT_LIBRARY_FILE =
OUTPUT_DIRECTORY_BASE =
OUTPUT_DIRECTORY_OBJECTS =
OUTPUT_DIRECTORY_EXEC =
OUTPUT_DIRECTORY_INFO =

#Evaluation of template recipe rules found in the ./make/rules.make file.
