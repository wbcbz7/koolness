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
        add         edi, 4
        and         eax, [esi + 4]
        or          eax, [esi + 0]
        add         esi, 8
        mov         [edi - 4], eax
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
        add         edi, 4
        add         eax, 0x7F7F7F7F
        add         esi, 4
        xor         eax, 0x7F7F7F7F
        and         eax, 0x7F7F7F7F
        and         eax, ebx
        dec         ecx
        mov         [edi - 4], eax
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

; ----------------------------------
; saturated sub for 0..127 pixel range, process 2 dwords in parallel
; void rect_blit_subs_2(uint8_t *dst, uint32_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
; #pragma aux rect_blit_subs_2 parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]
global  rect_blit_subs_2_
rect_blit_subs_2_:
        push        ebp
        mov         ebp, eax
.y_loop:
        push        ebp
        push        edx
        push        ecx
        push        ebx
        mov         ebp, ebx
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
        mov         ecx, [edi + 4]

        or          eax, 0x80808080
        or          ecx, 0x80808080

        sub         eax, [esi]
        sub         ecx, [esi + 4]
        
        mov         ebx, eax
        mov         edx, ecx

        and         eax, 0x80808080
        and         ecx, 0x80808080
        
        shr         eax, 7
        add         edi, 8

        shr         ecx, 7
        add         esi, 8

        add         eax, 0x7F7F7F7F
        add         ecx, 0x7F7F7F7F
        
        xor         eax, 0x7F7F7F7F
        xor         ecx, 0x7F7F7F7F

        and         eax, 0x7F7F7F7F
        and         ecx, 0x7F7F7F7F

        and         eax, ebx
        and         ecx, edx

        mov         [edi - 8], eax
        mov         [edi - 4], ecx

        dec         ebp
        jnz         .x_loop

        ; -----------------
        pop         ebx
        pop         ecx
        pop         edx
        pop         ebp
        
        add         edi, edx
        add         esi, ebp

        dec         ecx
        jnz         .y_loop

        ; -----------------
        
        pop         ebp
        ret
