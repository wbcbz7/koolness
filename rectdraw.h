#pragma once
#include <stdint.h>

extern "C" {

void rect_fill_add(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_fill_add parm caller [edi] [esi] [ebx] [ecx] [edx] [eax] modify [eax ebx ecx edx esi edi]

void rect_fill_avg(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_fill_avg parm caller [edi] [esi] [ebx] [ecx] [edx] [eax] modify [eax ebx ecx edx esi edi]

void rect_fill_sub(uint8_t *dst, uint32_t data, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_fill_sub parm caller [edi] [esi] [ebx] [ecx] [edx] [eax] modify [eax ebx ecx edx esi edi]

void rect_blit_mov(uint8_t *dst, uint8_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_blit_mov parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]

void rect_blit_mask(uint8_t *dst, uint32_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_blit_mask parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]

void rect_blit_subs(uint8_t *dst, uint8_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_blit_subs parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]

void rect_blit_add(uint8_t *dst, uint8_t* src, uint32_t width, uint32_t height, uint32_t dst_fixup, uint32_t src_fixup);
#pragma aux rect_blit_add parm caller [edi] [esi] [ebx] [ecx] [edx] [eax]

}