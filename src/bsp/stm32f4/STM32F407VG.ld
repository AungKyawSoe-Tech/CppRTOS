/**
 * STM32F407VG Linker Script
 * 
 * Memory layout for STM32F407VGT6:
 * - 1 MB Flash (0x08000000 - 0x080FFFFF)
 * - 192 KB RAM (0x20000000 - 0x2002FFFF)
 * - CCM RAM 64 KB (0x10000000 - 0x1000FFFF) - fast memory for stacks
 */

/* Entry point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20030000;    /* End of RAM (192K) */

/* Stack sizes */
_Min_Heap_Size = 0x200;   /* Required for malloc */
_Min_Stack_Size = 0x400;  /* Required for main stack */

/* Memory regions */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 192K
  CCM (xrw)       : ORIGIN = 0x10000000, LENGTH = 64K
}

/* Sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)         /* ARM/Thumb interworking */
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* End of code section */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

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

  /* Used by startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* Start of data section */
    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;        /* End of data section */
  } >RAM AT> FLASH

  /* CCM section for critical stacks and data */
  .ccm (NOLOAD) :
  {
    . = ALIGN(4);
    _sccm = .;
    *(.ccm)
    *(.ccm*)
    . = ALIGN(4);
    _eccm = .;
  } >CCM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;         /* Start of BSS section */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* End of BSS section */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section for checking heap/stack overlap */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
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
