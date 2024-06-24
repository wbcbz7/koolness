        use32
        section code code align=32

extern _linedraw_aa_tab;
extern _linedraw_pitchtab;

; --------dx-------
; -----------------
global linedraw_mov_dx_a_
linedraw_mov_dx_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         [edi + eax], cl ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        add         edi, edx        ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_avg_dx_a_
linedraw_avg_dx_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         ch, [edi + eax]
        add         ch, cl
        rcr         ch, 1 
        mov         [edi + eax], ch ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        add         edi, edx        ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_add_dx_a_
linedraw_add_dx_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        add         [edi + eax], cl ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y

        add         edi, edx        ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_sub_dx_a_
linedraw_sub_dx_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        sub         [edi + eax], cl ; *(dst + (y >> 16)) -= cl | also agi on P5
        mov         eax, ebx        ; eax = y

        add         edi, edx        ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_adds_dx_a_
linedraw_adds_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + color
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          ch, dl          ; ch = src &= mask 

        mov         [edi + eax], ch ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

global linedraw_adds_128_dx_a_
linedraw_adds_128_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y

        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + color
        add         ch, ch
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          ch, dl          ; ch = src &= mask 
        shr         ch, 1

        mov         [edi + eax], ch ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length
        jz          .done

        ; regular non-saturated add loop
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        add         [edi + eax], cl ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y

        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop    
.done:         


        pop         edx
        pop         ebp
        ret

global linedraw_subs_dx_a_
linedraw_subs_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx
        mov         ecx, eax        ; ecx =  color
        neg         ecx             ; ecx = -color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + (-color)
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        and         ch, dl          ; ch = src &= mask 

        mov         [edi + eax], ch ; *(dst + (y >> 16)) -= cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

global linedraw_add_aa_dx_a_
linedraw_add_aa_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5

        ; right pixel
        mov         dh, [edi + eax + 1] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        mov         [edi + eax + 1], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

; saturated add
global linedraw_adds_aa_dx_a_
linedraw_adds_aa_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5

        ; right pixel
        mov         dh, [edi + eax + 1] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax + 1], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

; saturated add
global linedraw_adds_128_aa_dx_a_
linedraw_adds_128_aa_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y

        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        add         dh, dh
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; dh = src &= mask 
        shr         dh, 1
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5

        ; right pixel
        mov         dh, [edi + eax + 1] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        add         dh, dh
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; dh = src &= mask 
        shr         dh, 1
        mov         [edi + eax + 1], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jz          .done

        ; non-saturated add loop
.loop:     
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5

        ; right pixel
        mov         dh, [edi + eax + 1] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        mov         [edi + eax + 1], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop
.done:

        pop         edx
        pop         ebp
        ret

; saturated sub
global linedraw_subs_aa_dx_a_
linedraw_subs_aa_dx_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        and         dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5

        ; right pixel
        mov         dh, [edi + eax + 1] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        and         dh, dl          ; ch = src &= mask 
        mov         [edi + eax + 1], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        add         edi, [esp]      ; edx = dst += pitch
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

; --------dy-------
; -----------------
; the hardest part since we need to use pitchtab here - well actually not that hardest but nevermind :)
global linedraw_mov_dy_a_
linedraw_mov_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        
        mov         [edi + eax], cl ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_avg_dy_a_
linedraw_avg_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        
        mov         ch, [edi + eax]
        add         ch, cl
        rcr         ch, 1 
        mov         [edi + eax], ch ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_add_dy_a_
linedraw_add_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        
        add         [edi + eax], cl ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_sub_dy_a_
linedraw_sub_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        
        sub         [edi + eax], cl ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_adds_dy_a_
linedraw_adds_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + color
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          ch, dl          ; ch = src &= mask 

        mov         [edi + eax], ch ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

global linedraw_adds_128_dy_a_
linedraw_adds_128_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        mov         eax, ebx        ; eax = y

        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + color
        add         ch, ch
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          ch, dl          ; ch = src &= mask 
        shr         ch, 1

        mov         [edi + eax], ch ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        inc         edi
        dec         ebp             ; ebp = --length
        jz          .done     

        ; non-saturated add loop
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        
        add         [edi + eax], cl ; *(dst + (y >> 16)) = cl | also agi on P5
        mov         eax, ebx

        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop

.done:
        pop         ebp
        ret

global linedraw_subs_dy_a_
linedraw_subs_dy_a_:
        push        ebp
        mov         ebp, ecx
        mov         ecx, eax        ; ecx - color
        neg         ecx             ; ecx = -color
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        add         ebx, esi        ; ebx = y += dydx

        mov         eax, [_linedraw_pitchtab + 4*eax]
        mov         ch, [edi + eax] ; ch = src = *(dst + (y >> 16))

        add         ch, cl          ; ch = src + color
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        and         ch, dl          ; ch = src &= mask 

        mov         [edi + eax], ch ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         ebp
        ret

; saturated add
global linedraw_adds_aa_dy_a_
linedraw_adds_aa_dy_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps
        mov         eax, [_linedraw_pitchtab + 4*eax]

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        add         edi, [esp]      ; edi = dst += pitch

        ; right pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        sub         edi, [esp]      ; edi = dst -= pitch
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

; saturated add
global linedraw_adds_128_aa_dy_a_
linedraw_adds_128_aa_dy_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y

        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps
        mov         eax, [_linedraw_pitchtab + 4*eax]

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        add         dh, dh
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; dh = src &= mask 
        shr         dh, 1
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        add         edi, [esp]      ; edi = dst += pitch

        ; right pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        add         dh, dh
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; dh = src &= mask 
        shr         dh, 1
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        sub         edi, [esp]      ; edi = dst -= pitch
        inc         edi
        dec         ebp             ; ebp = --length

        jz          .done

        ; non-saturated loop
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps
        mov         eax, [_linedraw_pitchtab + 4*eax]

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        add         edi, [esp]      ; edi = dst += pitch

        ; right pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        sub         edi, [esp]      ; edi = dst -= pitch
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop
.done:     
        pop         edx
        pop         ebp
        ret

; saturated sub
global linedraw_subs_aa_dy_a_
linedraw_subs_aa_dy_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps
        mov         eax, [_linedraw_pitchtab + 4*eax]

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        add         edi, [esp]      ; edi = dst += pitch

        ; right pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        sbb         dl, dl          ; dl = overflow mask (0xFF if overflow)
        or          dh, dl          ; ch = src &= mask 
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        sub         edi, [esp]      ; edi = dst -= pitch
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret

; saturated add
global linedraw_add_aa_dy_a_
linedraw_add_aa_dy_a_:
        push        ebp
        push        edx
        mov         ebp, ecx        ; ecx = free
        xor         ecx, ecx
        mov         eax, ebx        ; eax = y
.loop:
        shr         eax, 16         ; eax = (y >> 16)
        mov         cl, bh          ; ecx = (y >> 8) & 255 - aa_table offset
        shr         cl, 4           ; 16 steps
        mov         eax, [_linedraw_pitchtab + 4*eax]

        ; left pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         ebx, esi        ; ebx = y += dydx
        add         dh, [_linedraw_aa_tab + ecx + 0]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        add         edi, [esp]      ; edi = dst += pitch

        ; right pixel
        mov         dh, [edi + eax] ; ch = src = *(dst + (y >> 16))
        add         dh, [_linedraw_aa_tab + ecx + 16]
        mov         [edi + eax], dh ; *(dst + (y >> 16)) += cl | also agi on P5
        mov         eax, ebx        ; eax = y
        
        sub         edi, [esp]      ; edi = dst -= pitch
        inc         edi
        dec         ebp             ; ebp = --length

        jnz         .loop     

        pop         edx
        pop         ebp
        ret
