ENTRY(Reset_Handler)
/* STM32F407VG Memory Layout */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{

    .isr_vector :
    {
        . = ALIGN(512);
        KEEP(*(.isr_vector))
    } > FLASH

    /* Code and constants go in Flash */
    .text :
    {
        *(.text*)                /* All code */
        *(.rodata*)              /* Read-only data (const) */
        _etext = .;              /* Symbol marking end of Flash content */
    } > FLASH

    /* Initialized global variables */
    /* Stored in Flash, copied to RAM on boot */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;              /* RAM start of .data */
        *(.data*)
        . = ALIGN(4);
        _edata = .;              /* RAM end of .data */
    } > RAM AT > FLASH           /* LMA in Flash, VMA in RAM */

    /* Start of .data in Flash (where startup.c copies FROM) */
    _sidata = LOADADDR(.data);

    /* Uninitialized globals — zeroed by startup code */
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Stack at top of RAM */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}