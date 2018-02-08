# Top level makefile for the jel - JT's Embedded Libraries.
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
# To reduce clutter, this makefile is broken into multiple components. Most of these components can 
# be found inside the ./make subdirectory.

#Include toolchain definitions, such as compilers, linkers, standard lib locations, etc.
include ./make/toolchain.make
#Include RTOS/Cpputest/fatfs etc related definitions common across all targets.
include ./make/thirdparty.make
#Include common make definitions, such as compiler warning flags and directory name templates.
include ./make/common.make
#Include CPU target flags
include ./make/processors.make
#Include filetype definitions
include ./make/filetypes.make
#Include template recipe rules. These templates are expanded by each individual target.
include ./make/rules.make
############################
#   Source File Includes   #
############################
#All source files should be included from their respective directories sub .make files here.
#This includes any target specific source definitions, which will be used in the included targets
#section below.

include ./os/files.make
include ./ThirdParty/files.make

.PHONY: clean force targets

############################
#       Target Rules       #
############################
GLOBAL_TARGET_LIST =

default : all

include ./make/targets/tm4c123gh6pm.make

all : $(GLOBAL_TARGET_LIST)
clean : $(addprefix clean_, $(GLOBAL_TARGET_LIST))
info : $(addprefix info_, $(GLOBAL_TARGET_LIST))

force :
