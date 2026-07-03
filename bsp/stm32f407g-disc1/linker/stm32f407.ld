/* STM32F407VG Memory Layout */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* Code and constants go in Flash */
    .text :
    {
        KEEP(*(.vectors))        /* Vector table must be first */
        *(.text*)                /* All code */
        *(.rodata*)              /* Read-only data (const) */
        _etext = .;              /* Symbol marking end of Flash content */
    } > FLASH

    /* Initialized global variables */
    /* Stored in Flash, copied to RAM on boot */
    .data :
    {
        _sdata = .;              /* RAM start of .data */
        *(.data*)
        _edata = .;              /* RAM end of .data */
    } > RAM AT > FLASH           /* LMA in Flash, VMA in RAM */

    /* Start of .data in Flash (where startup.c copies FROM) */
    _sidata = LOADADDR(.data);

    /* Uninitialized globals — zeroed by startup code */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    /* Stack at top of RAM */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}