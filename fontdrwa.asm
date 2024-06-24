        use32
        section code code align=32

; edi - dst, esi - src, ebx - w, ecx - h, edx - dst_fixup, eax - src_fixup
global draw_glyph_masked_a_
draw_glyph_masked_a_:
        push        ebp
        mov         ebp, eax
.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        mov         eax, [edi]
        and         eax, [esi + 4]
        or          eax, [esi + 0]
        mov         [edi], eax
        add         edi, 4
        add         esi, 8
        dec         ecx
        jnz         .x_loop

        ; -----------------
        pop         ecx
        
        add         edi, edx
        add         esi, ebp

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret

; edi - dst, esi - src, ebx - w, ecx - h, edx - colormask, [esp+4] - dst_fixup, [esp+8] - src_fixup
global draw_glyph_masked_color_a_
draw_glyph_masked_color_a_:
        push        ebp
.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        mov         eax, [edi]
        mov         ebp, [esi + 0]

        and         eax, [esi + 4]
        and         ebp, edx
        
        or          eax, ebp
        add         esi, 8

        mov         [edi], eax
        add         edi, 4
        
        dec         ecx
        jnz         .x_loop

        ; -----------------
        pop         ecx
        
        add         edi, [esp+8]
        add         esi, [esp+12]

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret

; edi - dst, esi - src, ebx - w, ecx - h, edx - colormask, [esp+4] - dst_fixup, [esp+8] - src_fixup
global draw_glyph_add_color_a_
draw_glyph_add_color_a_:
        push        ebp
.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        mov         eax, [edi]
        mov         ebp, [esi + 0]

        and         ebp, edx
        
        add         eax, ebp
        add         esi, 8

        mov         [edi], eax
        add         edi, 4
        
        dec         ecx
        jnz         .x_loop

        ; -----------------
        pop         ecx
        
        add         edi, [esp+8]
        add         esi, [esp+12]

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret


