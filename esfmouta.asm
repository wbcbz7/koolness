        section .text
        use32
        align   16


extern _esfm_base
extern _mainprops
extern _ptc_force_vblank

global esfm_out_
esfm_out_:
        mov     edx, [_esfm_base]
        add     edx, 2
        cmp     byte [_mainprops], 0
        jnz     _fast

        out     dx, al
        in      al, 0xE1
        in      al, 0xE1
        mov     al, ah
        inc     edx
        out     dx, al
        in      al, 0xE1
        in      al, 0xE1
        mov     al, cl
        sub     edx, 2
        out     dx, al
        in      al, 0xE1
        mov     edx, 0x3DA
        in      al, dx
        and     al, 8
        or      [_ptc_force_vblank], al
        ret

        _fast:
        out     dx, al
        mov     al, ah
        inc     edx
        out     dx, al
        mov     al, cl
        sub     edx, 2
        out     dx, al
        mov     edx, 0x3DA
        in      al, dx
        and     al, 8
        or      [_ptc_force_vblank], al
        ret
