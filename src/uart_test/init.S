
    .text
    .global _start

_start:
    ldr r13, =0x31000000
    b main
