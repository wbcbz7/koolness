        use32
        section code code align=32

struc edge_buffer_t
    .p      resd    1
    .length resd    1
    .u      resd    1
    .v      resd    1
    .dudx   resd    1
    .dvdx   resd    1
    .l      resd    1
    .dldx   resd    1
endstruc

; esi - edgebuf, eax - color, ecx - length of edgebuf

global edge_draw_flat_mov_a_
edge_draw_flat_mov_a_:
        push        ebp
        mov         ebp, ecx
.edge_loop:
        mov         edi, [esi + edge_buffer_t.p]
        mov         ecx, [esi + edge_buffer_t.length]
        ; fill loop
%if 1
        rep         stosb
%else
        mov         edx, ecx
        and         edx, 3
        shr         ecx, 2
        
        // copy by dwords
        rep         stosd

        // copy remainder
        mov         ecx, edx
        rep         stosb
%endif
        add         esi, edge_buffer_t_size
        dec         ebp
        jnz         .edge_loop
        ; --------------
        pop         ebp
        ret

%if 0
global edge_draw_flat_avg_a_
edge_draw_flat_avg_a_:
        push        ebp
        mov         ebp, ecx
        shr         eax, 1
        and         eax, 0x7F7F7F7F  
.edge_loop:
        mov         edi, [esi + edge_buffer_t.p]
        mov         ecx, [esi + edge_buffer_t.length]
        test        ecx, ecx
        jz          .skip
.line_loop:
        mov         ah, [edi]
        shr         ah, 1
        add         ah, al
        mov         [edi], ah
        add         edi, 1
        dec         ecx
        jnz         .line_loop 

        ; --------------
.skip:
        add         esi, edge_buffer_t_size
        dec         ebp
        jnz         .edge_loop
        ; --------------
        pop         ebp
        ret
%endif


; -------------
; broken, fix
%if 1
global edge_draw_flat_avg_a_
edge_draw_flat_avg_a_:
        push        ebp
        mov         ebp, ecx
        ; precalc color for avg
        shr         eax, 1
        and         eax, 0x7F7F7F7F

.edge_loop:
        mov         edi, [esi + edge_buffer_t.p]
        mov         ecx, [esi + edge_buffer_t.length]
        test        ecx, ecx
        jz          .skip
.line_loop:
        mov         edx, ecx
        shr         ecx, 2
        jz          .skip_dword

.dword_loop:
        mov         ebx, [edi]
        and         ebx, 0xFEFEFEFE
        shr         ebx, 1
        add         ebx, eax
        mov         [edi], ebx
        add         edi, 4
        dec         ecx
        jnz         .dword_loop 

.skip_dword:
        and         edx, 3
        jz          .skip
.1px:
        mov         bl, [edi]
        shr         bl, 1
        add         bl, al
        mov         [edi], bl
        inc         edi
        dec         edx
        jnz         .1px

        ; --------------
.skip:
        add         esi, edge_buffer_t_size
        dec         ebp
        jnz         .edge_loop
        ; --------------
        pop         ebp
        ret
%endif
