/*  ZZ Open GL graphics plugin
 *  Copyright (c)2009-2010 zeydlitz@gmail.com, arcum42@gmail.com
 *  Based on Zerofrog's ZeroGS KOSMOS (c)2005-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "GS.h"
#include "Mem.h"
#include "x86.h"

#if defined(ZEROGS_SSE2)
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

// swizzling
#define _FrameSwizzleBlock(type, transfer, transfer16, incsrc) \
/* FrameSwizzleBlock32 */ \
void __fastcall FrameSwizzleBlock32##type##c(u32* dst, u32* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable32[0][0]; \
	\
	if( WriteMask == 0xffffffff ) { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				dst[d[j]] = (transfer); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				dst[d[j]] = ((transfer)&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
} \
\
void __fastcall FrameSwizzleBlock24##type##c(u32* dst, u32* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable32[0][0]; \
	\
	if( WriteMask == 0xffffffff ) { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				dst[d[j]] = (transfer); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				dst[d[j]] = ((transfer)&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
} \
\
/* FrameSwizzleBlock16 */ \
void __fastcall FrameSwizzleBlock16##type##c(u16* dst, u32* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable16[0][0]; \
	\
	if( WriteMask == 0xffff ) {  \
		for(int i = 0; i < 8; ++i, d += 16) { \
			for(int j = 0; j < 16; ++j) { \
				u32 temp = (transfer); \
				dst[d[j]] = RGBA32to16(temp); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 16) { \
			for(int j = 0; j < 16; ++j) { \
			u32 temp = (transfer); \
				u32 dsrc = RGBA32to16(temp); \
				dst[d[j]] = (dsrc&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
} \
\
/* Frame16SwizzleBlock32 */ \
void __fastcall Frame16SwizzleBlock32##type##c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable32[0][0]; \
\
	if( WriteMask == 0xffffffff ) {  \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				dst[d[j]] = Float16ToARGB(dsrc16); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				u32 dsrc = Float16ToARGB(dsrc16); \
				dst[d[j]] = (dsrc&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
 } \
\
/* Frame16SwizzleBlock32Z */ \
void __fastcall Frame16SwizzleBlock32Z##type##c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable32[0][0]; \
	if( WriteMask == 0xffffffff ) { /* breaks KH text if not checked */ \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				dst[d[j]] = Float16ToARGB_Z(dsrc16); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 8) { \
			for(int j = 0; j < 8; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				u32 dsrc = Float16ToARGB_Z(dsrc16); \
				dst[d[j]] = (dsrc&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
 } \
 \
 /* Frame16SwizzleBlock16 */ \
void __fastcall Frame16SwizzleBlock16##type##c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable16[0][0]; \
	\
	if( (WriteMask&0xfff8f8f8) == 0xfff8f8f8) {  \
		for(int i = 0; i < 8; ++i, d += 16) { \
			for(int j = 0; j < 16; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				dst[d[j]] = Float16ToARGB16(dsrc16); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
	else { \
		for(int i = 0; i < 8; ++i, d += 16) { \
			for(int j = 0; j < 16; ++j) { \
				Vector_16F dsrc16 = (transfer16); \
				u32 dsrc = Float16ToARGB16(dsrc16); \
				dst[d[j]] = (dsrc&WriteMask)|(dst[d[j]]&~WriteMask); \
			} \
			src += srcpitch << incsrc; \
		} \
	} \
 } \
 \
 /* Frame16SwizzleBlock16Z */ \
void __fastcall Frame16SwizzleBlock16Z##type##c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask) \
{ \
	u32* d = &g_columnTable16[0][0]; \
	\
	for(int i = 0; i < 8; ++i, d += 16) { \
		for(int j = 0; j < 16; ++j) { \
			Vector_16F dsrc16 = (transfer16); \
			dst[d[j]] = Float16ToARGB16_Z(dsrc16); \
		} \
		src += srcpitch << incsrc; \
	} \
} \
 
_FrameSwizzleBlock(_, src[j], src[j], 0);
_FrameSwizzleBlock(A2_, (src[2*j] + src[2*j+1]) >> 1, src[2*j], 0);
_FrameSwizzleBlock(A4_, (src[2*j] + src[2*j+1] + src[2*j+srcpitch] + src[2*j+srcpitch+1]) >> 2, src[2*j], 1);

#ifdef ZEROGS_SSE2

//void __fastcall WriteCLUT_T16_I8_CSM1_sse2(u32* vm, u32* clut)
//{
//  __asm {
//	  mov eax, vm
//	  mov ecx, clut
//	  mov edx, 8
//  }
//
//Extract32x2:
//  __asm {
//	  movdqa xmm0, qword ptr [eax]
//	  movdqa xmm1, qword ptr [eax+16]
//	  movdqa xmm2, qword ptr [eax+32]
//	  movdqa xmm3, qword ptr [eax+48]
//
//	  // rearrange
//	  pshuflw xmm0, xmm0, 0xd8
//	  pshufhw xmm0, xmm0, 0xd8
//	  pshuflw xmm1, xmm1, 0xd8
//	  pshufhw xmm1, xmm1, 0xd8
//	  pshuflw xmm2, xmm2, 0xd8
//	  pshufhw xmm2, xmm2, 0xd8
//	  pshuflw xmm3, xmm3, 0xd8
//	  pshufhw xmm3, xmm3, 0xd8
//
//	  movdqa xmm4, xmm0
//	  movdqa xmm6, xmm2
//
//	  shufps xmm0, xmm1, 0x88
//	  shufps xmm2, xmm3, 0x88
//
//	  shufps xmm4, xmm1, 0xdd
//	  shufps xmm6, xmm3, 0xdd
//
//	  pshufd xmm0, xmm0, 0xd8
//	  pshufd xmm2, xmm2, 0xd8
//	  pshufd xmm4, xmm4, 0xd8
//	  pshufd xmm6, xmm6, 0xd8
//
//	  // left column
//	  movhlps xmm1, xmm0
//	  movlhps xmm0, xmm2
//	  //movdqa xmm7, [ecx]
//
//	  movdqa [ecx], xmm0
//	  shufps xmm1, xmm2, 0xe4
//	  movdqa [ecx+16], xmm1
//
//	  // right column
//	  movhlps xmm3, xmm4
//	  movlhps xmm4, xmm6
//	  movdqa [ecx+32], xmm4
//	  shufps xmm3, xmm6, 0xe4
//	  movdqa [ecx+48], xmm3
//
//	  add eax, 16*4
//	  add ecx, 16*8
//	  sub edx, 1
//	  cmp edx, 0
//	  jne Extract32x2
//  }
//}

extern "C" void __fastcall WriteCLUT_T32_I8_CSM1_sse2(u32* vm, u32* clut)
{
	__m128i* src = (__m128i*)vm;
	__m128i* dst = (__m128i*)clut;

	for (int j = 0; j < 64; j += 32, src += 32, dst += 32)
	{
		for (int i = 0; i < 16; i += 4)
		{
			__m128i r0 = _mm_load_si128(&src[i+0]);
			__m128i r1 = _mm_load_si128(&src[i+1]);
			__m128i r2 = _mm_load_si128(&src[i+2]);
			__m128i r3 = _mm_load_si128(&src[i+3]);

			_mm_store_si128(&dst[i*2+0], _mm_unpacklo_epi64(r0, r1));
			_mm_store_si128(&dst[i*2+1], _mm_unpacklo_epi64(r2, r3));
			_mm_store_si128(&dst[i*2+2], _mm_unpackhi_epi64(r0, r1));
			_mm_store_si128(&dst[i*2+3], _mm_unpackhi_epi64(r2, r3));

			__m128i r4 = _mm_load_si128(&src[i+0+16]);
			__m128i r5 = _mm_load_si128(&src[i+1+16]);
			__m128i r6 = _mm_load_si128(&src[i+2+16]);
			__m128i r7 = _mm_load_si128(&src[i+3+16]);

			_mm_store_si128(&dst[i*2+4], _mm_unpacklo_epi64(r4, r5));
			_mm_store_si128(&dst[i*2+5], _mm_unpacklo_epi64(r6, r7));
			_mm_store_si128(&dst[i*2+6], _mm_unpackhi_epi64(r4, r5));
			_mm_store_si128(&dst[i*2+7], _mm_unpackhi_epi64(r6, r7));
		}
	}
}

extern "C" void __fastcall WriteCLUT_T32_I4_CSM1_sse2(u32* vm, u32* clut)
{
	__m128i* src = (__m128i*)vm;
	__m128i* dst = (__m128i*)clut;

	__m128i r0 = _mm_load_si128(&src[0]);
	__m128i r1 = _mm_load_si128(&src[1]);
	__m128i r2 = _mm_load_si128(&src[2]);
	__m128i r3 = _mm_load_si128(&src[3]);

	_mm_store_si128(&dst[0], _mm_unpacklo_epi64(r0, r1));
	_mm_store_si128(&dst[1], _mm_unpacklo_epi64(r2, r3));
	_mm_store_si128(&dst[2], _mm_unpackhi_epi64(r0, r1));
	_mm_store_si128(&dst[3], _mm_unpackhi_epi64(r2, r3));
}


static const __aligned16 int s_clut16mask2[4] = { 0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff };
static const __aligned16 int s_clut16mask[8] = { 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
										   0x0000ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff
										   };

extern "C" void __fastcall WriteCLUT_T16_I4_CSM1_sse2(u32* vm, u32* clut)
{
#if defined(_MSC_VER)
	__asm
	{
		mov eax, vm
		mov ecx, clut
		movdqa xmm0, qword ptr [eax]
		movdqa xmm1, qword ptr [eax+16]
		movdqa xmm2, qword ptr [eax+32]
		movdqa xmm3, qword ptr [eax+48]

		// rearrange
		pshuflw xmm0, xmm0, 0x88
		pshufhw xmm0, xmm0, 0x88
		pshuflw xmm1, xmm1, 0x88
		pshufhw xmm1, xmm1, 0x88
		pshuflw xmm2, xmm2, 0x88
		pshufhw xmm2, xmm2, 0x88
		pshuflw xmm3, xmm3, 0x88
		pshufhw xmm3, xmm3, 0x88

		shufps xmm0, xmm1, 0x88
		shufps xmm2, xmm3, 0x88

		pshufd xmm0, xmm0, 0xd8
		pshufd xmm2, xmm2, 0xd8

		pxor xmm6, xmm6

		test ecx, 15
		jnz WriteUnaligned

		movdqa xmm7, s_clut16mask // saves upper 16 bytes

		// have to save interlaced with the old data
		movdqa xmm4, [ecx]
		movdqa xmm5, [ecx+32]
		movhlps xmm1, xmm0
		movlhps xmm0, xmm2 // lower 8 colors

		pand xmm4, xmm7
		pand xmm5, xmm7

		shufps xmm1, xmm2, 0xe4 // upper 8 colors
		movdqa xmm2, xmm0
		movdqa xmm3, xmm1

		punpcklwd xmm0, xmm6
		punpcklwd xmm1, xmm6
		por xmm0, xmm4
		por xmm1, xmm5

		punpckhwd xmm2, xmm6
		punpckhwd xmm3, xmm6

		movdqa [ecx], xmm0
		movdqa [ecx+32], xmm1

		movdqa xmm5, xmm7
		pand xmm7, [ecx+16]
		pand xmm5, [ecx+48]

		por xmm2, xmm7
		por xmm3, xmm5

		movdqa [ecx+16], xmm2
		movdqa [ecx+48], xmm3
		jmp End

WriteUnaligned:
		// ecx is offset by 2
		sub ecx, 2

		movdqa xmm7, s_clut16mask2 // saves lower 16 bytes

		// have to save interlaced with the old data
		movdqa xmm4, [ecx]
		movdqa xmm5, [ecx+32]
		movhlps xmm1, xmm0
		movlhps xmm0, xmm2 // lower 8 colors

		pand xmm4, xmm7
		pand xmm5, xmm7

		shufps xmm1, xmm2, 0xe4 // upper 8 colors
		movdqa xmm2, xmm0
		movdqa xmm3, xmm1

		punpcklwd xmm0, xmm6
		punpcklwd xmm1, xmm6
		pslld xmm0, 16
		pslld xmm1, 16
		por xmm0, xmm4
		por xmm1, xmm5

		punpckhwd xmm2, xmm6
		punpckhwd xmm3, xmm6
		pslld xmm2, 16
		pslld xmm3, 16

		movdqa [ecx], xmm0
		movdqa [ecx+32], xmm1

		movdqa xmm5, xmm7
		pand xmm7, [ecx+16]
		pand xmm5, [ecx+48]

		por xmm2, xmm7
		por xmm3, xmm5

		movdqa [ecx+16], xmm2
		movdqa [ecx+48], xmm3

End:
	}
#else
	__asm__ __volatile__(".intel_syntax noprefix\n"
	"movdqa xmm0, xmmword ptr [%[vm]]\n"
	"movdqa xmm1, xmmword ptr [%[vm]+16]\n"
	"movdqa xmm2, xmmword ptr [%[vm]+32]\n"
	"movdqa xmm3, xmmword ptr [%[vm]+48]\n"

	// rearrange
	"pshuflw xmm0, xmm0, 0x88\n"
	"pshufhw xmm0, xmm0, 0x88\n"
	"pshuflw xmm1, xmm1, 0x88\n"
	"pshufhw xmm1, xmm1, 0x88\n"
	"pshuflw xmm2, xmm2, 0x88\n"
	"pshufhw xmm2, xmm2, 0x88\n"
	"pshuflw xmm3, xmm3, 0x88\n"
	"pshufhw xmm3, xmm3, 0x88\n"

	"shufps xmm0, xmm1, 0x88\n"
	"shufps xmm2, xmm3, 0x88\n"

	"pshufd xmm0, xmm0, 0xd8\n"
	"pshufd xmm2, xmm2, 0xd8\n"

	"pxor xmm6, xmm6\n"

	"test %[clut], 15\n"
	"jnz WriteUnaligned\n"

	"movdqa xmm7, %[s_clut16mask]\n" // saves upper 16 bits

	// have to save interlaced with the old data
	"movdqa xmm4, [%[clut]]\n"
	"movdqa xmm5, [%[clut]+32]\n"
	"movhlps xmm1, xmm0\n"
	"movlhps xmm0, xmm2\n"// lower 8 colors

	"pand xmm4, xmm7\n"
	"pand xmm5, xmm7\n"

	"shufps xmm1, xmm2, 0xe4\n" // upper 8 colors
	"movdqa xmm2, xmm0\n"
	"movdqa xmm3, xmm1\n"

	"punpcklwd xmm0, xmm6\n"
	"punpcklwd xmm1, xmm6\n"
	"por xmm0, xmm4\n"
	"por xmm1, xmm5\n"

	"punpckhwd xmm2, xmm6\n"
	"punpckhwd xmm3, xmm6\n"

	"movdqa [%[clut]], xmm0\n"
	"movdqa [%[clut]+32], xmm1\n"

	"movdqa xmm5, xmm7\n"
	"pand xmm7, [%[clut]+16]\n"
	"pand xmm5, [%[clut]+48]\n"

	"por xmm2, xmm7\n"
	"por xmm3, xmm5\n"

	"movdqa [%[clut]+16], xmm2\n"
	"movdqa [%[clut]+48], xmm3\n"
	"jmp WriteCLUT_T16_I4_CSM1_End\n"

	"WriteUnaligned:\n"
	// %[clut] is offset by 2
	"sub %[clut], 2\n"

	"movdqa xmm7, %[s_clut16mask2]\n" // saves lower 16 bits

	// have to save interlaced with the old data
	"movdqa xmm4, [%[clut]]\n"
	"movdqa xmm5, [%[clut]+32]\n"
	"movhlps xmm1, xmm0\n"
	"movlhps xmm0, xmm2\n" // lower 8 colors

	"pand xmm4, xmm7\n"
	"pand xmm5, xmm7\n"

	"shufps xmm1, xmm2, 0xe4\n" // upper 8 colors
	"movdqa xmm2, xmm0\n"
	"movdqa xmm3, xmm1\n"

	"punpcklwd xmm0, xmm6\n"
	"punpcklwd xmm1, xmm6\n"
	"pslld xmm0, 16\n"
	"pslld xmm1, 16\n"
	"por xmm0, xmm4\n"
	"por xmm1, xmm5\n"

	"punpckhwd xmm2, xmm6\n"
	"punpckhwd xmm3, xmm6\n"
	"pslld xmm2, 16\n"
	"pslld xmm3, 16\n"

	"movdqa [%[clut]], xmm0\n"
	"movdqa [%[clut]+32], xmm1\n"

	"movdqa xmm5, xmm7\n"
	"pand xmm7, [%[clut]+16]\n"
	"pand xmm5, [%[clut]+48]\n"

	"por xmm2, xmm7\n"
	"por xmm3, xmm5\n"

	"movdqa [%[clut]+16], xmm2\n"
	"movdqa [%[clut]+48], xmm3\n"
	"WriteCLUT_T16_I4_CSM1_End:\n"
	"\n"
	".att_syntax\n" 
    :
    : [vm] "r" (vm), [clut] "r" (clut), [s_clut16mask] "m" (*s_clut16mask), [s_clut16mask2] "m" (*s_clut16mask2)
    : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
       );
#endif // _MSC_VER
}

#endif // ZEROGS_SSE2

void __fastcall WriteCLUT_T16_I8_CSM1_c(u32* _vm, u32* _clut)
{
	const static u32 map[] =
	{
		0, 2, 8, 10, 16, 18, 24, 26,
		4, 6, 12, 14, 20, 22, 28, 30,
		1, 3, 9, 11, 17, 19, 25, 27,
		5, 7, 13, 15, 21, 23, 29, 31
	};

	u16* vm = (u16*)_vm;
	u16* clut = (u16*)_clut;

	int left = ((u32)(uptr)clut & 2) ? 512 : 512 - (((u32)(uptr)clut) & 0x3ff) / 2;

	for (int j = 0; j < 8; j++, vm += 32, clut += 64, left -= 32)
	{
		if (left == 32)
		{
			assert(left == 32);

			for (int i = 0; i < 16; i++)
				clut[2*i] = vm[map[i]];

			clut = (u16*)((uptr)clut & ~0x3ff) + 1;

			for (int i = 16; i < 32; i++)
				clut[2*i] = vm[map[i]];
		}
		else
		{
			if (left == 0)
			{
				clut = (u16*)((uptr)clut & ~0x3ff) + 1;
				left = -1;
			}

			for (int i = 0; i < 32; i++)
				clut[2*i] = vm[map[i]];
		}
	}
}

void __fastcall WriteCLUT_T32_I8_CSM1_c(u32* vm, u32* clut)
{
	u64* src = (u64*)vm;
	u64* dst = (u64*)clut;

	for (int j = 0; j < 2; j++, src += 32)
	{
		for (int i = 0; i < 4; i++, dst += 16, src += 8)
		{
			dst[0] = src[0];
			dst[1] = src[2];
			dst[2] = src[4];
			dst[3] = src[6];
			dst[4] = src[1];
			dst[5] = src[3];
			dst[6] = src[5];
			dst[7] = src[7];

			dst[8] = src[32];
			dst[9] = src[32+2];
			dst[10] = src[32+4];
			dst[11] = src[32+6];
			dst[12] = src[32+1];
			dst[13] = src[32+3];
			dst[14] = src[32+5];
			dst[15] = src[32+7];
		}
	}
}

void __fastcall WriteCLUT_T16_I4_CSM1_c(u32* _vm, u32* _clut)
{
	u16* dst = (u16*)_clut;
	u16* src = (u16*)_vm;

	dst[0] = src[0];
	dst[2] = src[2];
	dst[4] = src[8];
	dst[6] = src[10];
	dst[8] = src[16];
	dst[10] = src[18];
	dst[12] = src[24];
	dst[14] = src[26];
	dst[16] = src[4];
	dst[18] = src[6];
	dst[20] = src[12];
	dst[22] = src[14];
	dst[24] = src[20];
	dst[26] = src[22];
	dst[28] = src[28];
	dst[30] = src[30];
}

void __fastcall WriteCLUT_T32_I4_CSM1_c(u32* vm, u32* clut)
{
	u64* src = (u64*)vm;
	u64* dst = (u64*)clut;

	dst[0] = src[0];
	dst[1] = src[2];
	dst[2] = src[4];
	dst[3] = src[6];
	dst[4] = src[1];
	dst[5] = src[3];
	dst[6] = src[5];
	dst[7] = src[7];
}

void SSE2_UnswizzleZ16Target(u16* dst, u16* src, int iters)
{

#if defined(_MSC_VER)
	__asm
	{
		mov edx, iters
		pxor xmm7, xmm7
		mov eax, dst
		mov ecx, src

Z16Loop:
		// unpack 64 bytes at a time
		movdqa xmm0, [ecx]
		movdqa xmm2, [ecx+16]
		movdqa xmm4, [ecx+32]
		movdqa xmm6, [ecx+48]

		movdqa xmm1, xmm0
		movdqa xmm3, xmm2
		movdqa xmm5, xmm4

		punpcklwd xmm0, xmm7
		punpckhwd xmm1, xmm7
		punpcklwd xmm2, xmm7
		punpckhwd xmm3, xmm7

		// start saving
		movdqa [eax], xmm0
		movdqa [eax+16], xmm1

		punpcklwd xmm4, xmm7
		punpckhwd xmm5, xmm7

		movdqa [eax+32], xmm2
		movdqa [eax+48], xmm3

		movdqa xmm0, xmm6
		punpcklwd xmm6, xmm7

		movdqa [eax+64], xmm4
		movdqa [eax+80], xmm5

		punpckhwd xmm0, xmm7

		movdqa [eax+96], xmm6
		movdqa [eax+112], xmm0

		add ecx, 64
		add eax, 128
		sub edx, 1
		jne Z16Loop
	}
#else // _MSC_VER

	__asm__ __volatile__(".intel_syntax\n"
	"pxor %%xmm7, %%xmm7\n"

	"Z16Loop:\n"
	// unpack 64 bytes at a time
	"movdqa %%xmm0, [%[src]]\n"
	"movdqa %%xmm2, [%[src]+16]\n"
	"movdqa %%xmm4, [%[src]+32]\n"
	"movdqa %%xmm6, [%[src]+48]\n"

	"movdqa %%xmm1, %%xmm0\n"
	"movdqa %%xmm3, %%xmm2\n"
	"movdqa %%xmm5, %%xmm4\n"

	"punpcklwd %%xmm0, %%xmm7\n"
	"punpckhwd %%xmm1, %%xmm7\n"
	"punpcklwd %%xmm2, %%xmm7\n"
	"punpckhwd %%xmm3, %%xmm7\n"

	// start saving
	"movdqa [%[dst]], %%xmm0\n"
	"movdqa [%[dst]+16], %%xmm1\n"

	"punpcklwd %%xmm4, %%xmm7\n"
	"punpckhwd %%xmm5, %%xmm7\n"

	"movdqa [%[dst]+32], %%xmm2\n"
	"movdqa [%[dst]+48], %%xmm3\n"

	"movdqa %%xmm0, %%xmm6\n"
	"punpcklwd %%xmm6, %%xmm7\n"

	"movdqa [%[dst]+64], %%xmm4\n"
	"movdqa [%[dst]+80], %%xmm5\n"

	"punpckhwd %%xmm0, %%xmm7\n"

	"movdqa [%[dst]+96], %%xmm6\n"
	"movdqa [%[dst]+112], %%xmm0\n"

	"add %[src], 64\n"
	"add %[dst], 128\n"
	"sub %[iters], 1\n"
	"jne Z16Loop\n"

".att_syntax\n" 
    : "=&r"(src), "=&r"(dst), "=&r"(iters)
    : [src] "0"(src), [dst] "1"(dst), [iters] "2"(iters)
    : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
       );
#endif // _MSC_VER
}

