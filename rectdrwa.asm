        section .text
        use32
        align   16

; ----------------------------------
; void rect_fill_add(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_fill_add parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_fill_add_
rect_fill_add_:
        push        ebp
        mov         ebp, eax

.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        add         [edi], esi
        add         edi, 4

        dec         ecx
        jnz         .x_loop
        
        ; ---------------------
        pop         ecx
        add         edi, edx

        dec         ecx
        jnz         .y_loop

        pop         ebp
        ret

        ; ----------------------------------
; void rect_fill_avg(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_fill_avg parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_fill_avg_
rect_fill_avg_:
        push        ebp
        mov         ebp, eax

        and         esi, 0x7F7F7F7F

.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        mov         eax, [edi]
        add         eax, esi
        shr         eax, 1
        and         eax, 0x7F7F7F7F
        mov         [edi], eax
        add         edi, 4

        dec         ecx
        jnz         .x_loop
        
        ; ---------------------
        pop         ecx
        add         edi, edx

        dec         ecx
        jnz         .y_loop

        pop         ebp
        ret
        

; ----------------------------------
; void rect_fill_sub(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_fill_sub parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_fill_sub_
rect_fill_sub_:
        push        ebp
        mov         ebp, eax

.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        sub         [edi], esi
        add         edi, 4

        dec         ecx
        jnz         .x_loop
        
        ; ---------------------
        pop         ecx
        add         edi, edx
        
        dec         ecx
        jnz         .y_loop

        pop         ebp
        ret

; ----------------------------------
; src format is interleaved dword pixels/mask
; void rect_blit_mask(uint8_t *dst, uint32_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_blit_mask parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_blit_mask_
rect_blit_mask_:
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

; ----------------------------------
; void rect_blit_mov(uint8_t *dst, uint8_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_blit_mov parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_blit_mov_
rect_blit_mov_:
        push        ebp
        mov         ebp, eax
.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        rep         movsd

        ; -----------------
        pop         ecx
        
        add         edi, edx
        add         esi, ebp

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret

; ----------------------------------
; void rect_blit_add(uint8_t *dst, uint8_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_blit_add parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_blit_add_
rect_blit_add_:
        push        ebp
        mov         ebp, eax
.y_loop:
        push        ecx
        mov         ecx, ebx
.x_loop:
        mov         eax, [esi]
        add         esi, 4
        add         [edi], eax
        add         edi, 4
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

; ----------------------------------
; saturated sub for 0..127 pixel range
; void rect_blit_subs(uint8_t *dst, uint32_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_blit_subs parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_blit_subs_
rect_blit_subs_:
        push        ebp
        mov         ebp, eax
.y_loop:
        push        ecx
        push        ebx
        mov         ecx, ebx
.x_loop:
        ;       a |= 0x80808080;
        ;       a -= b;
        ;       b = a;
        ;       b &= 0x80808080;
        ;       b >>= 7;
        ;       b += 0x7F7F7F7F;
        ;       b ^= 0x7F7F7F7F;
        ;       b &= 0x7F7F7F7F;
        ;       a &= b;
        mov         eax, [edi]
        or          eax, 0x80808080
        sub         eax, [esi]
        mov         ebx, eax
        and         eax, 0x80808080
        shr         eax, 7
        add         eax, 0x7F7F7F7F
        xor         eax, 0x7F7F7F7F
        and         eax, 0x7F7F7F7F
        and         eax, ebx
        mov         [edi], eax

        add         edi, 4
        add         esi, 4
        dec         ecx
        jnz         .x_loop

        ; -----------------
        pop         ebx
        pop         ecx
        
        add         edi, edx
        add         esi, ebp

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret
