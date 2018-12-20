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

#
############ TEMPLATE RECIPE VARIABLES ############
#These recipe variables must be defined by each target. Object files should include the appropriate
#path to allow for organized out-of-directory builds. Targets must then $(eval) each of the template
#recipes to force expansion with the variable definitions.
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

OUTPUT_BINARY_FILE =
OUTPUT_ELF_FILE =
OUTPUT_LIBRARY_FILE =
OUTPUT_DIRECTORY_BASE =
OUTPUT_DIRECTORY_OBJECTS =
OUTPUT_DIRECTORY_EXEC =
OUTPUT_DIRECTORY_INFO =

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
info_$(TARGET_NAME) : $(OUTPUT_BINARY_FILE)
	@echo "Info: \r\n"
	@arm-none-eabi-size -d $(OUTPUT_ELF_FILE)
	@echo ================================================================================
	@arm-none-eabi-readelf -h -l -S $(OUTPUT_ELF_FILE)
endef

# TEMPLRECIPE_SEGGER_JFLASH
# Expands into a recipe that can be used to rapidly flash flash a binary to a target. Requires seggers
# jlink toolchain to be available and configured.
#
define TEMPLRECIPE_SEGGER_JFLASH
flash_$(TARGET_NAME): $(OUTPUT_BINARY_FILE) jlink\jflash\targets\$(TARGET_NAME).jflash force
		@echo "Flashing binary..."
		$$(JLINK_EXE) -openprj./jlink/jflash/targets/$(TARGET_NAME).jflash -open./$(OUTPUT_BINARY_FILE) -connect -erasechip -program -startapp -disconnect -exit
		@echo "Binary flashed successfully."
endef

# TEMPLRECIPE_TARGET_LIBRARY
# Expands into a recipe that builds all necessary files for a target and outputs a linkable library
# file supporting that target. Can be used to quickly build/rebuild an entire target with just the 
# 'make [TARGET_NAME]' command.
#
define TEMPLRECIPE_TARGET_LIBRARY
$(TARGET_NAME) : $(OUTPUT_BINARY_FILE)
		@echo "$(OUTPUT_BINARY_FILE) built successfully."
		@echo "Archiving into static library..."
		@$$(AR) -M < "$(ARCHIVER_SCRIPT)" 
		@echo "Library created successfully."
endef

# TEMPLRECIPE_TARGET_LIBRARY_ONLY
# Expands into a recipe that builds *only* the library files for a target, and does not require any
# linking.
define TEMPLRECIPE_TARGET_LIBRARY_ONLY
$(TARGET_NAME) : $(OUTPUT_LIBRARY_FILE)
		@echo "$(OUTPUT_LIBRARY_FILE) built successfully."
endef

# TEMPLRECIPE_BINARY
# Expands to a recipe that produces a flashable binary file of only the core jel library and
# hardware drivers. Useful for testing jel changes in isolation or evaluating the library
# stand-alone on a target.
#
define TEMPLRECIPE_BINARY
$(OUTPUT_BINARY_FILE) : $(OUTPUT_LIBRARY_FILE) $(CUSTOM_BINARY_DEPS)
ifeq ($(OS), Windows_NT)
		@if not exist "$(OUTPUT_DIRECTORY_INFO)" mkdir "$(OUTPUT_DIRECTORY_INFO)"
else
		@mkdir -p "$(OUTPUT_DIRECTORY_INFO)"
endif
		@echo "Linking elf..."
		@$$(LD) -o $(OUTPUT_ELF_FILE) -L$(OUTPUT_DIRECTORY_EXEC) $(ALL_OBJECT_FILES) -T $(LINKER_SCRIPT) $(LDFLAGS) -Wl,-Map,$(OUTPUT_DIRECTORY_INFO)\$(TARGET_NAME)_mapfile$(FILEEXTENSION_MAP)
		@echo "elf linked. Creating flashable binary from elf..."
		@$$(OBJCOPY) $(OBJCOPYFLAGS) --strip-debug --remove-section .bss \
			-O ihex $(OUTPUT_ELF_FILE) $(OUTPUT_BINARY_FILE)
		@echo "binary file generated successfully."
endef

# TEMPLRECIPE_LIB_ARCHIVE_FILE
# Expands to a recipe that outputs a linkable jel.a static library for the target. The jel can then
# be added to an external application as desired.
#
define TEMPLRECIPE_LIB_ARCHIVE_FILE
$(OUTPUT_LIBRARY_FILE) : $(ALL_OBJECT_FILES)
		@echo "Archiving files to library..."
ifeq ($(OS), Windows_NT)
		@if not exist "$(OUTPUT_DIRECTORY_BASE)" mkdir "$(OUTPUT_DIRECTORY_BASE)"
		@if not exist "$(OUTPUT_DIRECTORY_EXEC)" mkdir "$(OUTPUT_DIRECTORY_EXEC)"
else
		@mkdir -p "$(OUTPUT_DIRECTORY_BASE)"
		@mkdir -p "$(OUTPUT_DIRECTORY_EXEC)"
endif
		@$$(AR) -r --target=elf32-littlearm $$@ $(ALL_OBJECT_FILES) 
		@echo "Archiving complete."
endef

# TEMPLRECIPE_AFILE
# Recipe template used for assembling assembly files.
#
define TEMPLRECIPE_AFILE
$(OUTPUT_DIRECTORY_OBJECTS)$(PLATFORM_PATH_SEP)%.o : %.s
		@echo "Assembling:      $$<"
ifeq ($(OS), Windows_NT)
		@if not exist "$(OUTPUT_DIRECTORY_BASE)" mkdir "$(OUTPUT_DIRECTORY_BASE)"
		@if not exist "$(OUTPUT_DIRECTORY_OBJECTS)" mkdir "$(OUTPUT_DIRECTORY_OBJECTS)"
		@if not exist "$$(dir $$@)" mkdir "$$(dir $$@)"
else
		@mkdir -p "$(OUTPUT_DIRECTORY_BASE)"
		@mkdir -p "$(OUTPUT_DIRECTORY_OBJECTS)"
		@mkdir -p "$$(dir $$@)"
endif
		@$$(CC) $(AFLAGS) -o $$@ $$<
endef

# TEMPLRECIPE_CFILE
# Recipe template used for compiling C files.
#
define TEMPLRECIPE_CFILE
$(OUTPUT_DIRECTORY_OBJECTS)$(PLATFORM_PATH_SEP)%.o : %.c
		@echo "Compiling (C):   $$<"
ifeq ($(OS), Windows_NT)
		@if not exist "$(OUTPUT_DIRECTORY_BASE)" mkdir "$(OUTPUT_DIRECTORY_BASE)"
		@if not exist "$(OUTPUT_DIRECTORY_OBJECTS)" mkdir "$(OUTPUT_DIRECTORY_OBJECTS)"
		@if not exist "$$(dir $$@)" mkdir "$$(dir $$@)"
else
		@mkdir -p "$(OUTPUT_DIRECTORY_BASE)"
		@mkdir -p "$(OUTPUT_DIRECTORY_OBJECTS)"
		@mkdir -p "$$(dir $$@)"
endif
		@$$(CC) $(CFLAGS) -o $$@ $$<
endef

# TEMPLRECIPE_CXXFILE
# Recipe template used for compiling c++ files.
#
define TEMPLRECIPE_CXXFILE
$(OUTPUT_DIRECTORY_OBJECTS)$(PLATFORM_PATH_SEP)%.o : %.cpp
		@echo "Compiling (C++): $$<"
ifeq ($(OS), Windows_NT)
		@if not exist "$(OUTPUT_DIRECTORY_BASE)" mkdir "$(OUTPUT_DIRECTORY_BASE)"
		@if not exist "$(OUTPUT_DIRECTORY_OBJECTS)" mkdir "$(OUTPUT_DIRECTORY_OBJECTS)"
		@if not exist "$$(dir $$@)" mkdir "$$(dir $$@)"
else
		@mkdir -p "$(OUTPUT_DIRECTORY_BASE)"
		@mkdir -p "$(OUTPUT_DIRECTORY_OBJECTS)"
		@mkdir -p "$$(dir $$@)"
endif
		@$$(CXX) $(CXXFLAGS) -o $$@ $$<
endef

# TEMPLRECIPE_CLEAN
# Recipe template used for cleaning the target build outputs
#
define TEMPLRECIPE_CLEAN
clean_$(TARGET_NAME) : 
ifeq ($(OS), Windows_NT)
		@del /S /Q $(OUTPUT_DIRECTORY_BASE)
else
		@rm -f -R $(OUTPUT_DIRECTORY_BASE)
endif
endef

define TEMPLRECIPE_GDB
gdb_$(TARGET_NAME): $(OUTPUT_ELF_FILE)
		@echo "Starting GDB remote..."
		arm-none-eabi-gdb -q -iex "set auto-load safe-path /" $(OUTPUT_ELF_FILE)
endef
