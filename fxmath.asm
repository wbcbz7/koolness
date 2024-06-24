
    use32
    
    segment NASM_DATA data ALIGN=32

global __fadd_magic_32_0
__fadd_magic_32_0   dd      0x59C00000      ; int = (int)(float)

global __fadd_magic_24_8
__fadd_magic_24_8   dd      0x55C00000      ; int = (int)(float * 256.0)

global __fadd_magic_16_16
__fadd_magic_16_16  dd      0x51C00000      ; int = (int)(float * 65536.0)

global __fadd_magic_8_24
__fadd_magic_8_24   dd      0x4DC00000      ; int = (int)(float * 65536.0 * 256.0)

global __fadd_temp
__fadd_temp         dq      0               ; for fadd tricked float->int

