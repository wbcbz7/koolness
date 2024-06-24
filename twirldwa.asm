        section .text
        use32
        align   16

%if 1
; ----------------------------------
; void twirl_draw_tiled8x8_lace(uint8_t *buf, uint8_t* tex, uint16_t *tab, uint32_t ofs, uint32_t width);
; #pragma aux twirl_draw_tiled8x8_lace "*_" parm [edi] [esi] [edx] [ebx] [ecx]
global  twirl_draw_tiled8x8_laced_
twirl_draw_tiled8x8_laced_:
        pusha
        add         esi, ebx
        ; NB: limited for 320x200 and pitch = 384 bytes in 8x8 laced tiles at this moment

        xor         eax, eax
        xor         ebx, ebx

        mov         ebp, (100 / 4)
        xor         ecx, ecx
    .y_loop:
        push        ebp
        mov         ebp, (320 / 8)
    .x_loop:
        ; here goes unrolled code for the entire 8x8 tile
%assign sss 0
%assign yyy 0
%rep 4
        ; outer Y loop
%assign xxx 0
        ; inner X loop (fully unrolled/paired)
        mov         bx, [edx + 4 + (0*8) + (sss*8)]
        mov         cx, [edx + 6 + (0*8) + (sss*8)]

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        shl         eax, 16
    
        mov         bx, [edx + 0 + (0*8) + (sss*8)]
        mov         cx, [edx + 2 + (0*8) + (sss*8)]

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        mov         [edi + (0*4) + yyy], eax
        mov         bx, [edx + 4 + (1*8) + (sss*8)]

        shr         eax, 1
        mov         cx, [edx + 6 + (1*8) + (sss*8)]

        and         eax, 0x7F7F7F7F
        mov         [edi + (0*4) + yyy + 384], eax

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        shl         eax, 16
    
        mov         bx, [edx + 0 + (1*8) + (sss*8)]
        mov         cx, [edx + 2 + (1*8) + (sss*8)]

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        mov         [edi + (1*4) + yyy], eax
        shr         eax, 1
        and         eax, 0x7F7F7F7F
        mov         [edi + (1*4) + yyy + 384], eax

%assign sss sss + 2
%assign yyy yyy + (384*2)
%endrep
        add         edi, 8
        add         edx, (8*4*(8/4))

        dec         ebp
        jnz         .x_loop

        ; -----
        pop         ebp
        add         edi, 8*384 - 320      ; pitch fixup

        dec         ebp
        jnz         .y_loop

        popa
        ret
%else
; ----------------------------------
; void twirl_draw_tiled8x8_lace(uint8_t *buf, uint8_t* tex, uint16_t *tab, uint32_t ofs, uint32_t width);
; #pragma aux twirl_draw_tiled8x8_lace "*_" parm [edi] [esi] [edx] [ebx] [ecx]
global  twirl_draw_tiled8x8_laced_
twirl_draw_tiled8x8_laced_:
        pusha
        add         esi, ebx
        ; NB: limited for 320x200 and pitch = 384 bytes in 8x8 laced tiles at this moment

        xor         eax, eax
        xor         ebx, ebx

        mov         ebp, (100 / 4)
        xor         ecx, ecx
    .y_loop:
        push        ebp
        mov         ebp, (320 / 8)
    .x_loop:
        ; here goes unrolled code for the entire 8x8 tile
%assign sss 0
%assign yyy 0
%rep 4
        ; outer Y loop
%assign xxx 0
%rep (8/4)
        ; inner X loop
        mov         bx, [edx + 4 + (sss*8)]
        mov         cx, [edx + 6 + (sss*8)]

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        shl         eax, 16
    
        mov         bx, [edx + 0 + (sss*8)]
        mov         cx, [edx + 2 + (sss*8)]

        mov         al, [esi + ebx]
        mov         ah, [esi + ecx]

        mov         [edi + (xxx*4) + yyy], eax
        shr         eax, 1
        and         eax, 0x7F7F7F7F
        mov         [edi + (xxx*4) + yyy + 384], eax
%assign sss sss + 1
%assign xxx xxx + 1
%endrep

%assign yyy yyy + (384*2)
%endrep
        add         edi, 8
        add         edx, (8*4*(8/4))

        dec         ebp
        jnz         .x_loop

        ; -----
        pop         ebp
        add         edi, 8*384 - 320      ; pitch fixup

        dec         ebp
        jnz         .y_loop

        popa
        ret
%endif