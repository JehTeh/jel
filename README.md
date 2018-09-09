# JT's Embedded Libraries (JEL)

General C++ libraries designed for use in real-time embedded systems.

## Detailed Description

The JEL has been designed for use as a baseline library/framework for rapidly developing useful embedded applications
that can meet hard real-time guarantees while integrating various 'nice to have' features to speed development and
testing. It should be possible to start working on a serious real-time application using the JEL with only an
off-the-shelf development board and (optionally, but recommended) a J-Link programmer.

The JEL is built primarily around FreeRTOS, but being programmed in C++, provides an extensive set of wrappers
supporting RAII design patterns and modern C++ functionality like move-semantics. In addition to FreeRTOS, the following
third parties libraries feature some level of integration into the JEL:

  * [Under test on M4 Targets] CppUTest - Some targets, specifically those with larger memory capacities, support builds
    that integrate CppUTest into the JEL CLI. This allows for on-target unit testing with relative ease via a popular
    framework. 
  * [Upcoming] fatfs - Used to provide filesystem functionality. This is hidden behind the standard library calls, so
    fatfs does not need to be interacted with directly, only via portable calls to \<stdio.h\> / \<fstream\> functions. 

In addition to the above integration, the JEL provides the following functionality:

  * A unified precision time-keeping scheme based around std::chrono and used extensively throughout the library.
  * Extensive, modern C++ wrappers and extensions around FreeRTOS primitives. This includes:
      * All locking primitives feature full RAII functionality and automatic interrupt context detection
      * Templated Queue interface for queues that support both POD style objects and smart objects.
      * Thread/task functionality wrapper allowing more comprehensive tracking and statistics information about resource
        usage.
  * A specialized logging library that provides both general purpose logging functions and extremely fast logging 
    functionality purpose build for burst usage in ISRs.
  * A generic serial I/O library allowing for easily porting the JEL and writing custom I/O drivers for any desired
    interface.
  * A comprehensive and easily extendable Command-Line-Interface that allows for rapid implementation of custom
    commands. 
  * Basic exception support. In future JEL builds, exceptions will be optional (once C++17 header \<charconv\> is fully
    supported).
  * Fully supports builds at debug and release levels (-O0, -O1 and -O2).

#ONGOING 
  * Setup and install
  * Basic usage examples
  * Demo project using JEL

## License

All JEL-specific code is licensed under the MIT License. Third party libraries are selected for use only if they include
permissive licenses. 

