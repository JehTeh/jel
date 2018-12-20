# JT's Embedded Libraries (JEL)

General C++ libraries designed for use in real-time embedded systems.

## Detailed Description

The JEL has been designed for use as a baseline library/framework for rapidly developing embedded applications.
It can meet be used while maintaining hard real-time guarantees and integrates various 'nice to have' features to
speed development and testing. It should be possible to start working on a serious real-time application using the JEL
with only an off-the-shelf development board and (optionally, but recommended) a J-Link programmer.

It has been created by me (Jonathan Thomson) as a personal project that addresses my needs and requirements for a
general 'base' project on embedded systems within the targeted resource range. I am also using it to experiment with and
further my understanding of API and library design using modern C++.

The JEL is built primarily around FreeRTOS, but being programmed in C++, provides an extensive set of wrappers
supporting RAII design patterns and modern C++ functionality (such as move-semantics). The following functionality is
currently offered by the JEL:

  * A unified precision time-keeping scheme based around std::chrono and used extensively throughout the library.
  * Extensive, modern C++ wrappers and extensions around FreeRTOS primitives. This includes:
      * All locking primitives feature full RAII functionality and automatic interrupt context detection
      * Templated Queue interface for queues that support both POD style objects and smart objects.
      * Thread/task functionality wrapper allowing more comprehensive tracking and statistics information about resource
        usage.
      * FreeRTOS headers are not required to be directly included in any application code that uses the JEL.
  * An integrated logging library that provides both general purpose logging functions and extremely fast logging 
    functionality purpose build for burst usage in ISRs.
  * A generic serial I/O library allowing for easily porting the JEL and writing custom I/O drivers for any desired
    interface.
  * A comprehensive and easily extendable Command-Line-Interface that allows for rapid implementation of custom
    commands. 
  * Basic exception support. In future JEL builds, exceptions will be optional (once C++17 header \<charconv\> is fully
    supported).
  * Fully supports builds at debug and release levels (-O0, -O1 and -O2. *-O3 support is not included at this time*).
  * RTOS and newlib initialization is performed in a controlled manner before C++ constructors are called. This allows
    for 'application level' static C++ objects that make use of RTOS functionality.
  * Various useful object types, including some specialized allocator schemes, scope guard templates, etc.
  * Support for cross compilation from Windows or from the Windows Subsystem for Linux (WSL). This allows for the build
    to play nicely with tools written for linux, such as the library I use to generate the compile_commands.json file 
    (https://github.com/nickdiego/compiledb), among others.

## Roadmap
  
  * [Currently Ongoing] TI RM57L843 basic jel support.
  * fatfs - Used to provide filesystem functionality. This is hidden behind the standard library calls, so
    fatfs does not need to be interacted with directly, only via portable calls to \<stdio.h\> / \<fstream\> functions.
  * Better exception support - Currently exceptions are required and add a significant amount of 'bloat' to the binary
    for the unwind tables. Work is being done to make these entirely optional (while maintaining CLI module
    functionality). Additionally, for builds making use of exceptions, work is being done to allow for finer-grained
    control of the GCC allocation scheme (i.e. use a seperate heap or static region for exceptions).
  * Integration of a C++ lock-free queue scheme relying on processor atomics for faster operation than the current
    freeRTOS based queues.
  * Additional CppuTest integration to validate all JEL functionality on-target.
  * Additional documentation, specifically use-case examples for each type.
  * [COMPLETE] Support for cross compilation from both linux and windows (i.e. cross-platform makefile). Tested soley
    under WSL up to this point, hardcoded arm-none-eabi library paths may need to be changed to run under a native linux
    os.
  * [COMPLETE] CppUTest - Some targets, specifically those with larger memory capacities, support builds
    that integrate CppUTest into the JEL CLI. This allows for on-target unit testing with relative ease via a popular
    framework. 

## Supported Targets
Currently, the following targets have 'out of the box' support:

    * ST Micros STM32F3 MCUs, tested on the STM32F302RCT6.
    * TI's Tiva series, tested on the TM4C123GH6PM and TM4C1294NCPDT.

Generally speaking, porting to other micros in these families should be a fairly trivial task. 

Additionally, I am currently looking into supporting the following MCUs, as I am familiar with them:

    * TI's RM57L843.
    * NXP's LPC4357.

### Notes on target requirements
'Minimum' MCU requirements:

    * ARM M4 CPU (FPU preferred but not required).
    * [Pending] ARM R5F CPU.
    * 128kB Flash/ROM (Minimal JEL build currently requires ~80kB ROM).
    * 32kB RAM.

The JEL is not designed to run on 'minimal footprint' MCUs. If a project requires a simple MCU to, for example, read a
sensor and relay the data to a PC, the JEL is unlikely to be suitable. It is targeted more towards systems that use a
single MCU for various aggregate duties, some of which may have real-time constraints.

Smaller targets (specifically the TM4C123) do not have sufficient memory for features such as CppuTest and may not
ever be able to support such features. 

## Hardware Wrappers
The JEL provides only a minimal set of hardware wrappers. This is because there is a significant breadth of differences
in various peripheral implementations, and at its core the JEL does not need to implement a wide array of peripherals.
However, certain peripherals are required for the JEL to function, including:

    * GPIO interface for heartbeat functionality.
    * Device specific hardware startup routines.
    * Device specific interrupt controller interfacing.
    * High precision system clock interface.
    * UART interface.
    * WDT controller, specifically for system reset functionality.

Ports of the JEL must all implement the above driver functionality as per the jel/hw/api\_xxx.hpp files. Some generic
routines are included (generally weakly linked) that will go most of the way to getting a working JEL build compiling.
In addition, a generic UART driver is implemented that wraps some very common basic hardware calls (see
jel/hw/uart.hpp::BasicUart\_Base -> protected virtual functions) such as R/W buffer and enable/disable ISR that can be
very rapidly ported on most systems, but may not be highly performant.

## Tooling
The JEL can be compiled locally with the arm-none-eabi-gcc toolchain. mingw32-make is used to build the project. All
targets can be built by executing 'ming32-make' in the JEL root directory.

To build individual targets, execute 'mingw-32 make targets'. This will print a list of targets that can be built as
desired. Additionally, 'mingw32-make info\_[target name]' can be used to quickly dump the .elf data for a given target.

Note that there are certain dependencies which are not included by default in the JEL project. See ThirdParty/tivaware 
for details. Tiva targets can not be built without these files present.

Prebuilt binaries are included for -O0 and -O2 levels (dbg and rel respectively). These can be linked against in an end
user application as desired with no need to run make.

At this time only mingw32 and arm-none-eabi-gcc, on windows, have been tested.

## License

All JEL-specific code is licensed under the MIT License. Third party libraries are selected for use only if they include
permissive licenses. 

