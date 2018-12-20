# JT's Embedded Libraries (JEL)

General C++ framework library designed for use in hard real-time embedded systems.

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

    * TI's RM57L843 (WIP).
    * NXP's LPC4357.

### Notes on target requirements
'Minimum' MCU requirements:

    * ARM M4 CPU (FPU preferred but not required) OR
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
    * WDT controller, exclusively for system reset functionality.

Ports of the JEL must all implement the above driver functionality as per the jel/hw/api\_xxx.hpp files. Some generic
routines are included (generally weakly linked) that will go most of the way to getting a working JEL build compiling.
In addition, a generic UART driver is implemented that wraps some very common basic hardware calls (see
jel/hw/uart.hpp::BasicUart\_Base -> protected virtual functions) such as R/W buffer and enable/disable ISR that can be
very rapidly ported on most systems, but may not be highly performant.

## FreeRTOS wrappers and Extensions
A set of FreeRTOS C++ wrapper classes are provided by the JEL. While the intention is to eventually fully encapsulate
all FreeRTOS functionality, currently only the following primitives are available:
  
    * Non-grouped locking/synchronization primitives (i.e. semaphores, mutexes, etc. but no event groups).
    * Queues.
    * Task creation and control primitives.
    * Memory allocator objects.
    * Various utility functions.

This leaves out (at the time of this writing):

    * Event groups.
    * Queue Sets.
    * Software Timers.
    * Stream and Message buffers.
    * Co-routine support.
    
Generally speaking, the effort involved to implement those remaining features is not significant, beyond a few days of
work. If/when I require one of those items in some of my dependent projects, they will be implemented, otherwise there
are no firm implementation plans at this time. 

### JEL OS layer 
The 'OS' layer provides the interface to the underlying FreeRTOS implementation, in addition to various extensions. 

#### Startup/Initialization
All static objects are initialized by the JEL on bootup, before calling the application and after properly setting up
system clocks, libc/c++, and the RTOS. This means that, generally speaking, everything happening at the application
layer in the JEL can make full use of things like RTOS primitives in static initializers safely.

When bringing up an application, it is recommended that startup work is done via constructors for statically allocated
classes. This includes the creation of any system threads and master synchronization mechanisms. Note that because
static constructor execution order is not easily determined, no external calls should be made within static
constructors. Proper application startup synchronization can still easily be achieved, however. An example is shown
below:

```
=== boot_module.cpp ===

...

class MainController
{
  jel::Thread* thread_ptr_;
  static void main_thread(void* unused)
  {
    jel::log().pDbg("MainController main_thread started.");
    //Spawn other threads and synchronization primitives here ...
    jel::log().pDbg("MainController main_thread system initialization completed.");
    while(true)
    {
      //Perform main_thread appropriate functions...
    }
  }
public:
  MainController() 
  {
    jel::log().pDbg("MainController spawning main thread...");
    new jel::Thread(&main_thread, this, "main", 2048, jel::Thread::Priority::normal);
  }
}

//This will get constructed on startup, after the OS has started.
static MainController mc;

...

```

Note that while semaphores and mutexes can be statically constructed, it is generally unsafe to rely on this method for
system startup synchronization across multiple threads, as there is no guarantee what order the construction will be
performed.
#### System Interface Functionality
Core system functionality is exposed in the os/api\_system.hpp header. This includes the 'System' class, which allows
for determination of the current CPU exception and ISR state.

Additionally, CriticalSection and SchedulerLock RAII objects are provided. These objects, once created within a scope,
ensure everything in that scope either occurs in a critical section or while the scheduler is locked out. For example,

```

void foo()
{
  //... do some things that do not require a critical section.
  {
    jel::CriticalSection cs();
    //...Everything done within this scope occurs within a critical section. No need to worry about manually exiting.
  }
  //Once the scope is exited, so is the critical section.
}

```

#### Semaphores and Mutexes
All FreeRTOS locking primitives are exposed through the classes in the os/api\_locks.hpp header file. A standard, cat
all base class 'Lock' object is provided, that can take the form of either a semaphore, counting semaphore, mutex or
recursive mutex. This 'Lock' object provides generic lock, unlock, and comparison operations that are shared across all
child classes. It is generally advised against directly creating locks using the 'Lock' object, the child classes are
the preferred method for instantiating locking primitives.

'Lock' specializations are provided, that allow for easy and quick use of locking objects. These specializations are, as
expected, for semaphore, counting semaphore, mutex and recursive mutex classes.

An RAII 'LockGuard' class is also provided. This behaves similar to the std::lock\_guard class and is the preferred
method for acquiring locks in a safe manner.

Note that all locking functionality relies on the System::inIsr() method to automatically select between the FreeRTOS
xSemaphoreTake and xSemaphoreTakeFromISR functions. This does add overhead to these function calls, which may be
unacceptable in an ISR context. The lock APIs can be trivially extended to account for this case with an explicit
function call if required.

#### Queues
Queue objects are based around the 'Queue' template class in os/api\_queues.hpp. This template class provides two
primary modes of functionality, that of a typical FreeRTOS plain-old-data (POD) style copy queue and a 'smart' queue
that wraps the POD style queue functionality and allows for objects that implement move constructors (such as smart
pointers, like std::unique\_ptr).

Generally speaking, detection of the underlying object type is automatic and handled by the compiler, which will
instantiate the correct queue template (either POD or smart). It is important to understand that smart queue objects
have some additional overhead in both stack usage and execution time, because a 'ghost' copy and default object must be
created in the background to accommodate the move operation. In general, this is unlikely to be an issue for small
objects, such as smart pointers, but can introduce problems with larger primitives and should be carefully evaluated in
such cases. Furthermore, the existing implementation does not support objects whose move constructors rely on 'address
of self' information (i.e. They cannot reference the address of a member variable) because of the 'under the hood'
memcpy performed by FreeRTOS. These limitations will likely be addressed at a future date with a third specialized queue
class.

#### Allocators
Memory allocation functionality is exposed in os/api\_allocators.hpp. This functionality includes a standard
AllocatorStatisticsInterface class in addition to the system allocator, a templated pool allocator and a raw
block allocator.

All raw memory allocators in the JEL implement an AllocatorInterface function. This includes the SystemAllocator
singleton, which is the same allocator used by calls to new, delete, malloc, etc.

The  AllocatorStatisticsInterface class provides functionality used to determine the total memory consumption and
overall new/delete counts passing through a given allocator. This functionality can then be easily exposed via the
system CLI commands for development purposes, such as ensuring no allocations are occurring after system startup.

An ObjectPool template class is also provided. This is not a raw memory allocator - it provides an RAII based container
for a 'pool' of objects that can be passed around the system as needed, and return to the pool when no longer in use.
This is handy for certain types of objects like strings, where generally only a few modules need a string at a given
time and can share from a common pool.

A BlockAllocator is also provided. The BlockAllocator is a simple implementation of a fixed-size raw memory allocator,
which has the advantage of being much faster than a dual linked list implementation (which is used by FreeRTOS/the
system allocator) for both allocations and deallocations. It must be carefully sized to the allocation size, however, to
avoid significant wasted memory.

#### Threads
A full Task/Thread wrapper is provided by the JEL in the os/api\_threads.hpp header file. Two major components are
included, a 'ThisThread' static class used for controlling an active thread and a 'Thread' class used for creating new
threads. Note that the terms 'thread' and 'task' are used interchangeably throughout the JEL, but all refer to a FreeRTOS
based task.

The ThisThread class provides basic functions that a thread can call to sleep, yield or self destruct. These functions
are called directly from a running thread, for example:

```

void MyThread(void* unused)
{
  while(true)
  {
    //..Doing some work...
    jel::ThisThread::sleepfor(jel::Duration::milliseconds(50)) //Sleep for 50 milliseconds
  }
}

```

The Thread class provides an interface to create a new RTOS thread. This includes some additional parameters, such as
behavior on uncaught exceptions and statistics tracking controls. By default, new threads are automatically registered
with the RTOS but also maintain a separate statistics mechanism that allows for finer grained information than that
available by default in FreeRTOS.

#### Timekeeping
Timekeeping in the JEL is all based around the functionality defined in the os/api\_time.hpp header. Throughout the JEL,
all time is kept in Duration and Timestamp objects (based around the standard chrono library) that allow for accurate
comparison and significantly reduced risk over an untyped 'int milliseconds' or 'TickType\_t time' variables.

The system wide SteadyClock is also exposed in this header. It provides access to the underlying timebase used
throughout the JEL, and on all supported targets features at least a resolution of a microsecond (note - accuracy is not
specified or guaranteed). Furthermore, the SteadyClock is guaranteed to be monotonic - i.e. successive reads from the
clock will always be either equal to or greater than the previous read (assuming they are performed in the same power
cycle). This makes the SteadyClock very useful for rough timekeeping and event tracking, but a custom precision clock
implementation may be more suitable for real-world and longer term timekeeping. Note that the underlying 64 bit wide
integer implementation is valid for at least 240 years in span, assuming nanosecond resolution.

Duration types are used throughout the system to specify an arbitrary span of time. These spans of time can be converted
to and from various meaningful real-world units as required. For printing, a family of .to[Micro/Milli/]seconds
functions are provided that automatically round the stored duration to the appropriate unit. Note that many functions,
such as lock calls, allow a duration as an input. While the duration can be of an arbitrary precision, the underlying
functionality implemented may only support up to a limited precision and as such any finer-grained detail is unused. For
example, a .lock(Duration::microseconds(5)) call will be rounded to either 0 or 1 RTOS ticks, depending on the JEL
configuration. Note that all Duration objects are 64 bit wide integer types by default.

Timestamp types are available to specify explicit points in time, such as when an event occurred. They are, by default,
based around the SteadyClock, but are compatible with std::chrono's time\_point functions. Because they inherit from
std::chrono::time\_point, all associated functionality such as comparisons operators is available.

#### Basic I/O
The JEL system I/O functionality is exposed in os/api\_io.hpp. This includes some basic generic synchronous and
asynchronous serial I/O base classes (typically implemented by a UART, SPI or similar in the HW layer) that provide
generic I/O abstractions to JEL functionality requiring it, such as the CLI.

In addition, an AnsiFormatter class is provided that implements common ANSI terminal control character definitions and
terminal control functionality, simplifying things like colored output over a TTY.

Finally, a PrettyPrinter class is provided that formats ASCII output to the terminal in a suitable manner for human
readability. The PrettyPrinter is configurable to allow for different line lengths, indent levels and whether or not to
remove special ANSI formatting characters. Note that at this time additional testing is needed to validate the
PrettyPrinter class in various use cases.

#### Filesystem I/O (Requires fatfs, currently in development)
All fatfs calls are wrapped by the standard newlib filesystem stubs. This means that outside of media specific control
functions, filesystem related calls should all go through the C or C++ standard libraries. Generally speaking, it is
recommended that including <iostream> directly be avoided in favor of the standard C library due to the significant
flash footprint of the C++ standard I/O library, but either option is suitable.

#### Logging
Specalized thread-aware traditional and real-time friendly high-speed logging primitives are provided by the JEL in the
os/api\_log.hpp header file. Logging in the JEL is generally done via the async-safe system logging channel, exposed by
the jel::log() function. The system logging channel is a full Logger class instance configured to capture and display
origin thread, timestamp and the message type/level automatically as a prefix when logging functions are called. 

The Logger class supports printf-style formatting for all standard print functions (no 'f' prefix). These can be called,
for example, as jel::log().printDebug("Message %s", my\_string); to queue an asynchronous print operation to the system
console. Thread name, timestamp and a [DBG] preface are automatically included at message print time.

For real-time situations, a buffered plain ascii only fast print family of functions are available. These are indicated
by the 'f' prefix, such as fprintDebug, and are more limited in that they cannot perform printf style operations and may
only print strings from constant memory or strings that have been pre-allocated somewhere in the system. However, on
even the minimal JEL targets such as the Tiva TM4C123, the logging function call takes around a single microsecond to
process and is even interrupt safe, making it useful for debugging certain types of real-time events.

A StreamLoggerHelper class is implemented to allow an iostreams-like interface to the JEL logger for common types. A
custom StreamLogger was built to avoid the significant overhead of the standard \<iostreams\> library. 

#### Exceptions

Documentation Upcoming

#### Command Line Interface (CLI)

Documentation Upcoming

#### Common Utilities
Some common utility classes are provided in the os/api\_common.hpp header. These include basic iterator wrappers and a
generic RAII scope-guard template class.

## Building and Tooling
The JEL can be compiled locally with the arm-none-eabi-gcc toolchain. mingw32-make is used to build the project. All
targets can be built by executing 'ming32-make' in the JEL root directory.

To build individual targets, execute 'mingw-32 make targets'. This will print a list of targets that can be built as
desired. Additionally, 'mingw32-make info\_[target name]' can be used to quickly dump the .elf data for a given target.

Note that there are certain dependencies which are not included by default in the JEL project. See ThirdParty/tivaware 
for details. Tiva targets can not be built without these files present.

Prebuilt binaries are included for -O0 and -O2 levels (dbg and rel respectively). These can be linked against in an end
user application as desired with no need to run make.

At this time only mingw32 and arm-none-eabi-gcc, on windows and the WSL, have been tested.

Note that for builds under a full linux environment, paths in the make/toolchain.make file need to be updated to point
at the appropriate resources. Likewise, on windows systems that install the arm-none-eabi toolchain to a non-standard
location, these paths will need to be updated to reflect that.

### Building the JEL for external applications and stand-alone usage
The JEL is capable of 'stand-alone' usage. This is of minimal use in developing an application, however it does allow
for easy testing of the JEL framework. By default, calls to 'make' or 'make \<target\_[dbg/rel]\>' undergo two stages:
The first compiles the JEL and required third party files into a '\<target\_[dbg/rel]\>/bin/\<target\_[dbg/rel]\>.a'
archive/library file that can be linked to externally from a third party application. These files are generally included
by default in release builds of the JEL, and are typically what is used when creating a third party application.

The second stage generates an ELF file (suitable for use with GDB in the case of \_dbg targets) and a HEX file that can
be flashed directly to the target via either manufacturer specific tools or a Segger j-link probe. This second stage is
optional and simply produces a stand-alone JEL build for that target. Stand-alone builds are useful for debugging JEL
ports, as they feature the CLI and, if supported by the target, CppuTest functionality.

If there is no desire to fully build the JEL to a stand-alone target then a static library can be built using make
'build\_\<target\_[dbg/rel]\>/bin/\<target\_[dbg/rel]\>.a'.

### Additional JEL tooling functionality
Some additional 'make' functions are included with the JEL. These are generally per target, and are called using the
form 'make [command]\_\<target\_[dbg/rel]\>'. These commands include:

    * clean\_: Erases a specific targets build output files.
    * gdb\_: Starts a command line GDB session using the given builds ELF file.
    * info\_: Prints information about the binary output size and section composition.
    * flash\_: If a licensed J-LINK probe supporting Segger J-Flash is available and connected, and a .jflash file is
      available under ./jlink/jflash/targets, then this command will automatically upload the binary to the connected
      target.

Additionally, some general commands are available. These take the form 'make [command]':

    * all: Builds all available JEL targets. (default)
    * clean: Erases all targets build outputs.
    * info: Prints information about all target binaries.
    * targets: Prints a list of supported JEL targets.

## License

All JEL-specific code is licensed under the MIT License. Third party libraries are selected for use only if they include
permissive licenses. 

