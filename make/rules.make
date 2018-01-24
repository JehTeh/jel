# Generic build rule templates
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
############ TEMPLATE RECIPE VARIABLES ############
TARGET_NAME =
AFLAGS =
CFLAGS =
CXXFLAGS =
LDFLAGS =
LINKER_SCRIPT =
ARCHIVER_SCRIPT = 

ALL_OBJECT_FILES =
TARGET_ASOURCE =
TARGET_CSOURCE =
TARGET_CXXSOURCE =

############ TEMPLATE RECIPES ############
# The template recipes are instantiated by each individual target as it is added to the build
# process. Each template recipe requires that a target define the template recipe variables
# appropriately before instantiating the recipe with the $(eval [TEMPLRECIPE_XXX]) function.
#
# GENERICRULE_INFO
# Expanded to a recipe that prints information about the build output, including binary size and
# individual section composition.
#
define TEMPLRECIPE_INFO
info_$(TOP_NAME) : $(BINOUTFILE)
	@echo "Info: \r\n"
	@arm-none-eabi-size -d $(ELFOUTFILE)
	@echo ================================================================================
	@arm-none-eabi-readelf -h -l -S $(ELFOUTFILE)
endef

# TEMPLRECIPE_SEGGER_JFLASH
# Expands into a recipe that can be used to rapidly flash flash a binary to a target. Requires seggers
# jlink toolchain to be available and configured.
#
define TEMPLRECIPE_SEGGER_JFLASH
flash_$(TOP_NAME): $(BINOUTFILE) jlink\$(TARGET_NAME)_flash.jflash force
		@echo "Flashing binary..."
		@$$(JLINK_EXE) -openprj./jlink/$(TARGET_NAME)_flash.jflash -open./$(BINOUTFILE) -connect -erasechip -program -startapp -disconnect -exit
		@echo "Binary flashed successfully."
endef

# TEMPLRECIPE_TARGET_LIBRARY
# Expands into a recipe that builds all necessary files for a target and outputs a linkable library
# file supporting that target. Can be used to quickly build/rebuild an entire target with just the 
# 'make [TARGET_NAME]' command.
#
define TEMPLRECIPE_TARGET_LIBRARY
$(TOP_NAME) : $(BINOUTFILE)
		@echo "$(BINOUTFILE) built successfully."
		@echo "Generating CSYS libfile..."
		@$$(AR) -M < "$(AR_SCRIPTFILE)" 
		@echo "CSYS libfile generated sucessfully."
endef

# TEMPLRECIPE_BINARY
# Expands to a recipe that produces a flashable binary file of only the core jel library and
# hardware drivers. Useful for testing jel changes in isolation or evaluating the library
# stand-alone on a target.
#
define TEMPLRECIPE_BINARY
$(BINOUTFILE) : $(LIBOUTFILE) 
		@if not exist $(BUILDOUT_DIRECTORY_INF) mkdir $(BUILDOUT_DIRECTORY_INF)
		@echo "Linking into .ELF..."
		@$$(LD) -o $(ELFOUTFILE) $(ALLOBJECTS) -L$(BUILDOUT_DIRECTORY_BIN) -T $(LD_SCRIPTFILE) $(LDFLAGS)
		@echo "ELF generated successfully. Stripping down to .hex..."
		@$$(OBJCOPY) $(OBJCOPYFLAGS) --strip-debug --remove-section .bss \
			-O ihex $(ELFOUTFILE) $(BINOUTFILE)
		@echo ".hex file generated successfully."
endef

# TEMPLRECIPE_LIB_ARCHIVE_FILE
# Expands to a recipe that outputs a linkable jel.a static library for the target. The jel can then
# be added to an external application as desired.
#
define TEMPLRECIPE_LIB_ARCHIVE_FILE
$(LIBOUTFILE) : $(ALLOBJECTS)
		@echo "Archiving files to library..."
		@if not exist $(BUILDOUT_DIRECTORY_TOP) mkdir $(BUILDOUT_DIRECTORY_TOP)
		@if not exist $(BUILDOUT_DIRECTORY_BIN) mkdir $(BUILDOUT_DIRECTORY_BIN)
		@$$(AR) -r --target=elf32-littlearm $$@ $(ALLOBJECTS) 
		@echo "Archiving complete."
endef

# TEMPLRECIPE_AFILE
# Recipe template used for assembling assembly files.
#
define TEMPLRECIPE_AFILE
$(BUILDOUT_DIRECTORY_OBJ)\\%.o : %.s
		@echo "Assembling file:    $$<"
		@if not exist $(BUILDOUT_DIRECTORY_TOP) mkdir $(BUILDOUT_DIRECTORY_TOP)
		@if not exist $(BUILDOUT_DIRECTORY_OBJ) mkdir $(BUILDOUT_DIRECTORY_OBJ)
		@if not exist "$$(dir $$@)" mkdir $$(dir $$@)
		@$$(CC) $(AFLAGS) -o $$@ $$<
endef

# TEMPLRECIPE_CFILE
# Recipe template used for compiling C files.
#
define TEMPLRECIPE_CFILE
$(BUILDOUT_DIRECTORY_OBJ)\\%.o : %.c
		@echo "Compiling C file:   $$<"
		@if not exist $(BUILDOUT_DIRECTORY_TOP) mkdir $(BUILDOUT_DIRECTORY_TOP)
		@if not exist $(BUILDOUT_DIRECTORY_OBJ) mkdir $(BUILDOUT_DIRECTORY_OBJ)
		@if not exist "$$(dir $$@)" mkdir $$(dir $$@)
		@$$(CC) $(CFLAGS) -o $$@ $$<
endef

# TEMPLRECIPE_CXXFILE
# Recipe template used for compiling c++ files.
#
define TEMPLRECIPE_CXXFILE
$(BUILDOUT_DIRECTORY_OBJ)\\%.o : %.cpp
		@echo "Compiling C++ file: $$<"
		@if not exist $(BUILDOUT_DIRECTORY_TOP) mkdir $(BUILDOUT_DIRECTORY_TOP)
		@if not exist $(BUILDOUT_DIRECTORY_OBJ) mkdir $(BUILDOUT_DIRECTORY_OBJ)
		@if not exist "$$(dir $$@)" mkdir $$(dir $$@)
		@$$(CXX) $(CXXFLAGS) -o $$@ $$<
endef
