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
TM4C123GH6PM_CPU_FLAGS = $(CPU_TARGET_CORTEX_M4F_FLAGS)
TM4C123GH6PM_CUSTOM_FLAGS = -I $(RTOS_INCLUDE_PATH_ARM_M4F) -DHW_TARGET_TM4C123GH6PM 
TM4C123GH6PM_LIBRARY_INCLUDES = 

#Definition of target specific rules found in the ./make/rules.make file.
TARGET_NAME = tm4c123gh6pm_dbg
AFLAGS = $(TM4C123GH6PM_CPU_FLAGS) $(AFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
CFLAGS = $(TM4C123GH6PM_CPU_FLAGS) $(CFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
CXXFLAGS = $(TM4C123GH6PM_CPU_FLAGS) $(CXXFLAGS_BASE_DEBUG) $(TM4C123GH6PM_CUSTOM_FLAGS)
LDFLAGS = $(TM4C123GH6PM_CPU_FLAGS) $(LDFLAGS_BASE_DEBUG)
LINKER_SCRIPT = ./ld/targets/tm4c123gh6pm.ld
ARCHIVER_SCRIPT = 

OUTPUT_DIRECTORY_BASE = build_$(TARGET_NAME)
OUTPUT_DIRECTORY_OBJECTS = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_OBJECTS)
OUTPUT_DIRECTORY_EXEC = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_EXECUTABLES)
OUTPUT_DIRECTORY_INFO = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_INFO)
OUTPUT_BINARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_HEX)
OUTPUT_ELF_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_ELF)
OUTPUT_LIBRARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_LIB)

ALL_OBJECT_FILES =
TARGET_ASOURCE = $(ASOURCE) $(TM4C123GH6PM_ASOURCE)
TARGET_CSOURCE = $(CSOURCE) $(TM4C123GH6PM_CSOURCE)
TARGET_CXXSOURCE = $(CXXSOURCE) $(TM4C123GH6PM_CXXSOURCE)
ALL_OBJECT_FILES += $(strip $(patsubst %.s, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_ASOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.c, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CSOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.cpp, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CXXSOURCE)))

#Evaluation of template recipe rules found in the ./make/rules.make file. 
$(eval $(TEMPLRECIPE_INFO))
$(eval $(TEMPLRECIPE_SEGGER_JFLASH))
$(eval $(TEMPLRECIPE_TARGET_LIBRARY))
$(eval $(TEMPLRECIPE_BINARY))
$(eval $(TEMPLRECIPE_LIB_ARCHIVE_FILE))
$(eval $(TEMPLRECIPE_AFILE))
$(eval $(TEMPLRECIPE_CFILE))
$(eval $(TEMPLRECIPE_CXXFILE))
