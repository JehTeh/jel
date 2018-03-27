# Target definitions for the Texas Instruments Tiva TM4C1294NCPDT ARM M4F Processor.
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
TM4C1294NCPDT_CPU_FLAGS = $(CPU_TARGET_CORTEX_M4F_FLAGS)
TM4C1294NCPDT_CUSTOM_FLAGS = -I $(RTOS_INCLUDE_PATH_ARM_M4F) -I "./ThirdParty/tivaware" -DHW_TARGET_TM4C1294NCPDT
TM4C1294NCPDT_LIBRARY_INCLUDES = "./ThirdParty/tivaware/driverlib/gcc/libdriver.a"

#Definition of target specific rules found in the ./make/rules.make file.
#DEBUG BUILD
TARGET_NAME = tm4c1294ncpdt_dbg
$(eval GLOBAL_TARGET_LIST += $(TARGET_NAME))
AFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(AFLAGS_BASE_DEBUG) $(TM4C1294NCPDT_CUSTOM_FLAGS)
CFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(CFLAGS_BASE_DEBUG) $(TM4C1294NCPDT_CUSTOM_FLAGS)
CXXFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(CXXFLAGS_BASE_DEBUG) $(TM4C1294NCPDT_CUSTOM_FLAGS)
LDFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(LDFLAGS_BASE_DEBUG) $(TM4C1294NCPDT_LIBRARY_INCLUDES)
LINKER_SCRIPT = ./ld/targets/tm4c1294ncpdt.ld
ARCHIVER_SCRIPT = ./ar/tm4c1294ncpdt_dbg.ar

OUTPUT_DIRECTORY_BASE = build_$(TARGET_NAME)
OUTPUT_DIRECTORY_OBJECTS = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_OBJECTS)
OUTPUT_DIRECTORY_EXEC = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_EXECUTABLES)
OUTPUT_DIRECTORY_INFO = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_INFO)
OUTPUT_BINARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_HEX)
OUTPUT_ELF_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_ELF)
OUTPUT_LIBRARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_LIB)

ALL_OBJECT_FILES =
TARGET_ASOURCE = $(ASOURCE) $(TM4C1294NCPDT_ASOURCE)
TARGET_CSOURCE = $(CSOURCE) $(TM4C1294NCPDT_CSOURCE)
TARGET_CXXSOURCE = $(CXXSOURCE) $(TM4C1294NCPDT_CXXSOURCE)
ALL_OBJECT_FILES += $(strip $(patsubst %.s, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_ASOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.c, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CSOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.cpp, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CXXSOURCE)))
CUSTOM_BINARY_DEPS = 

#Evaluation of template recipe rules found in the ./make/rules.make file. 
$(eval $(TEMPLRECIPE_INFO))
$(eval $(TEMPLRECIPE_SEGGER_JFLASH))
$(eval $(TEMPLRECIPE_TARGET_LIBRARY))
$(eval $(TEMPLRECIPE_BINARY))
$(eval $(TEMPLRECIPE_LIB_ARCHIVE_FILE))
$(eval $(TEMPLRECIPE_AFILE))
$(eval $(TEMPLRECIPE_CFILE))
$(eval $(TEMPLRECIPE_CXXFILE))
$(eval $(TEMPLRECIPE_CLEAN))
$(eval $(TEMPLRECIPE_GDB))

#RELEASE BUILD
TARGET_NAME = tm4c1294ncpdt_rel
$(eval GLOBAL_TARGET_LIST += $(TARGET_NAME))
AFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(AFLAGS_BASE) $(TM4C1294NCPDT_CUSTOM_FLAGS)
CFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(CFLAGS_BASE) $(TM4C1294NCPDT_CUSTOM_FLAGS)
CXXFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(CXXFLAGS_BASE) $(TM4C1294NCPDT_CUSTOM_FLAGS)
LDFLAGS = $(TM4C1294NCPDT_CPU_FLAGS) $(LDFLAGS_BASE) $(TM4C1294NCPDT_LIBRARY_INCLUDES)
LINKER_SCRIPT = ./ld/targets/tm4c1294ncpdt.ld
ARCHIVER_SCRIPT = ./ar/tm4c1294ncpdt_dbg.ar

OUTPUT_DIRECTORY_BASE = build_$(TARGET_NAME)
OUTPUT_DIRECTORY_OBJECTS = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_OBJECTS)
OUTPUT_DIRECTORY_EXEC = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_EXECUTABLES)
OUTPUT_DIRECTORY_INFO = $(OUTPUT_DIRECTORY_BASE)/$(OUTPUT_DIRECTORY_NAME_INFO)
OUTPUT_BINARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_HEX)
OUTPUT_ELF_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_ELF)
OUTPUT_LIBRARY_FILE = $(OUTPUT_DIRECTORY_EXEC)/$(TARGET_NAME)$(FILEEXTENSION_LIB)

ALL_OBJECT_FILES =
TARGET_ASOURCE = $(ASOURCE) $(TM4C1294NCPDT_ASOURCE)
TARGET_CSOURCE = $(CSOURCE) $(TM4C1294NCPDT_CSOURCE)
TARGET_CXXSOURCE = $(CXXSOURCE) $(TM4C1294NCPDT_CXXSOURCE)
ALL_OBJECT_FILES += $(strip $(patsubst %.s, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_ASOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.c, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CSOURCE)))
ALL_OBJECT_FILES += $(strip $(patsubst %.cpp, $(OUTPUT_DIRECTORY_OBJECTS)\\%.o, $(TARGET_CXXSOURCE)))
CUSTOM_BINARY_DEPS = 

#Evaluation of template recipe rules found in the ./make/rules.make file. 
$(eval $(TEMPLRECIPE_INFO))
$(eval $(TEMPLRECIPE_SEGGER_JFLASH))
$(eval $(TEMPLRECIPE_TARGET_LIBRARY))
$(eval $(TEMPLRECIPE_BINARY))
$(eval $(TEMPLRECIPE_LIB_ARCHIVE_FILE))
$(eval $(TEMPLRECIPE_AFILE))
$(eval $(TEMPLRECIPE_CFILE))
$(eval $(TEMPLRECIPE_CXXFILE))
$(eval $(TEMPLRECIPE_CLEAN))
$(eval $(TEMPLRECIPE_GDB))
