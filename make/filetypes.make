# Filetype and directory configuration information. Intended to be included in top-level jel
# makefile.
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

############ FILETYPES ############
#Intel HEX file. For use with JLINK JFLASH.
FILEEXTENSION_HEX = .hex
#Static library extension.
FILEEXTENSION_LIB = .a
#ELF file extension.
FILEEXTENSION_ELF = .elf
#Linker map file extension. 
FILEEXTENSION_MAP = .map
#Raw ASM output extension.
FILEEXTENSION_DUMP = .dump
#Text file output extension.
FILEEXTENSION_TXT = .txt
############ DIRECTORIES ############
#Information files such as linker map and size info is output here.
OUTPUT_DIRECTORY_NAME_INFO = inf
#Compiled object files are stored here.
OUTPUT_DIRECTORY_NAME_OBJECTS = obj
#Executable code output, such as binaries, is placed here. Currently library files (*.a) are also
#stored here.
OUTPUT_DIRECTORY_NAME_EXECUTABLES = bin
