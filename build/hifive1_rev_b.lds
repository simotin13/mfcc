ENTRY(_main);
SECTIONS
{
        . = 0x20010000;
        .text : { *(.text) }
}
