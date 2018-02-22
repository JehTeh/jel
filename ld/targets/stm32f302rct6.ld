/******************************************************************************
 *
 * STM32F302RCT6 Linker file for the jel.
 * Written by Jonathan Thomson
 *
 *****************************************************************************/
MEMORY
{
  /** The STM32F302RCT6 has 256kB of flash. */
  FLASH         (rx)  : ORIGIN = 0x08000000, LENGTH = 256K  
  /** 40kB RAM. */
  RAM           (rwx) : ORIGIN = 0x20000000, LENGTH = 40K
}

SECTIONS
{
  /** Program code, exception handler tables, etc. all stored in flash. Note that the vector table
   * must come first. */
  .text :
  {
    KEEP(*(.vectorTable))
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH
  /** Constant data stored in flash. */
 .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH
  /** Exception unwind tables stored in flash. */
  .ARM.extab   : 
  { 
    *(.ARM.extab* .gnu.linkonce.armextab.*) 
  } >FLASH
  .ARM : 
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH
  /** C++ static constructors and destructors, stored in flash. */
  .ctors :
  {
    __CTOR_LIST__ = .;
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    __CTOR_END__ = .;
  } >FLASH 
  .dtors :
  {
    __DTOR_LIST__ = .;
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    __DTOR_END__ = .;
  } >FLASH 
  /** libc C++ pre-initialization and initialization function lists, included static constructor
   * lists. Stored in flash. */
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH
  /** Symbol representing the start of the data initialization info. Copied on startup from flash
   * to the .data segment in RAM. */
  _sidata = .; 
  /** Data section. All non-constant data is stored in ram, but initialized from flash by copy. */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;         /* Data start position in RAM.*/
    *(.data)            /* .data sections */
    *(.data*)           /* .data* sections */
    . = ALIGN(4);     
    _edata = .;         /* Data end position in RAM.*/
  } >RAM AT> FLASH

  /** Uninitialized data section. Primarily for system startup stack, locating it here allows
   * copying in .data and zeroing .bss without clobbering stack on bootup. */
  .noinit :
  {
    . = ALIGN(4);
    _snoinit = .;       /* noinit start position.  */
    *(.noinit)
    *(.noinit*)
    . = ALIGN(4);
    _enoinit = .;       /* noinit end position.  */
  } >RAM
  /** Zero initialized data, located in RAM. */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM
  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
