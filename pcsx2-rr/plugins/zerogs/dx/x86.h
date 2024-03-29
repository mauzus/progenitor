/*  ZeroGS KOSMOS
 *	Copyright (C) 2005-2006 Gabest/zerofrog@gmail.com
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef ZEROGS_X86
#define ZEROGS_X86

#include <xmmintrin.h>
#include <emmintrin.h>
#include "GS.h"

extern "C" void FASTCALL(SwizzleBlock32_sse2(u8* dst, u8* src, int srcpitch, u32 WriteMask = 0xffffffff));
extern "C" void FASTCALL(SwizzleBlock16_sse2(u8* dst, u8* src, int srcpitch));
extern "C" void FASTCALL(SwizzleBlock8_sse2(u8* dst, u8* src, int srcpitch));
extern "C" void FASTCALL(SwizzleBlock4_sse2(u8* dst, u8* src, int srcpitch));
extern "C" void FASTCALL(SwizzleBlock32u_sse2(u8* dst, u8* src, int srcpitch, u32 WriteMask = 0xffffffff));
extern "C" void FASTCALL(SwizzleBlock16u_sse2(u8* dst, u8* src, int srcpitch));
extern "C" void FASTCALL(SwizzleBlock8u_sse2(u8* dst, u8* src, int srcpitch));
extern "C" void FASTCALL(SwizzleBlock4u_sse2(u8* dst, u8* src, int srcpitch));

// frame swizzling

// no AA
extern "C" void FASTCALL(FrameSwizzleBlock32_sse2(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(FrameSwizzleBlock16_sse2(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32Z_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16Z_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

// AA 2x
extern "C" void FASTCALL(FrameSwizzleBlock32A2_sse2(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(FrameSwizzleBlock16A2_sse2(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32A2_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32ZA2_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16A2_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16ZA2_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

// AA 4x
extern "C" void FASTCALL(FrameSwizzleBlock32A4_sse2(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(FrameSwizzleBlock16A4_sse2(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32A4_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock32ZA4_sse2(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16A4_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern "C" void FASTCALL(Frame16SwizzleBlock16ZA4_sse2(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

extern void FASTCALL(SwizzleBlock32_c(u8* dst, u8* src, int srcpitch, u32 WriteMask = 0xffffffff));
extern void FASTCALL(SwizzleBlock16_c(u8* dst, u8* src, int srcpitch));
extern void FASTCALL(SwizzleBlock8_c(u8* dst, u8* src, int srcpitch));
extern void FASTCALL(SwizzleBlock4_c(u8* dst, u8* src, int srcpitch));

// no AA
extern void FASTCALL(FrameSwizzleBlock32_c(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(FrameSwizzleBlock16_c(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32Z_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16Z_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

// AA 2x
extern void FASTCALL(FrameSwizzleBlock32A2_c(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(FrameSwizzleBlock16A2_c(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32A2_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32ZA2_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16A2_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16ZA2_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

// AA 4x
extern void FASTCALL(FrameSwizzleBlock32A4_c(u32* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(FrameSwizzleBlock16A4_c(u16* dst, u32* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32A4_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock32ZA4_c(u32* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16A4_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));
extern void FASTCALL(Frame16SwizzleBlock16ZA4_c(u16* dst, Vector_16F* src, int srcpitch, u32 WriteMask));

extern void FASTCALL(SwizzleColumn32_c(int y, u8* dst, u8* src, int srcpitch, u32 WriteMask = 0xffffffff));
extern void FASTCALL(SwizzleColumn16_c(int y, u8* dst, u8* src, int srcpitch));
extern void FASTCALL(SwizzleColumn8_c(int y, u8* dst, u8* src, int srcpitch));
extern void FASTCALL(SwizzleColumn4_c(int y, u8* dst, u8* src, int srcpitch));

extern "C" void FASTCALL(WriteCLUT_T16_I8_CSM1_sse2(u32* vm, u32* clut));
extern "C" void FASTCALL(WriteCLUT_T32_I8_CSM1_sse2(u32* vm, u32* clut));
extern "C" void FASTCALL(WriteCLUT_T16_I4_CSM1_sse2(u32* vm, u32* clut));
extern "C" void FASTCALL(WriteCLUT_T32_I4_CSM1_sse2(u32* vm, u32* clut));
extern void FASTCALL(WriteCLUT_T16_I8_CSM1_c(u32* vm, u32* clut));
extern void FASTCALL(WriteCLUT_T32_I8_CSM1_c(u32* vm, u32* clut));

extern void FASTCALL(WriteCLUT_T16_I4_CSM1_c(u32* vm, u32* clut));
extern void FASTCALL(WriteCLUT_T32_I4_CSM1_c(u32* vm, u32* clut));

#ifdef ZEROGS_SSE2

#define SwizzleBlock32 SwizzleBlock32_sse2
#define SwizzleBlock16 SwizzleBlock16_sse2
#define SwizzleBlock8 SwizzleBlock8_sse2
#define SwizzleBlock4 SwizzleBlock4_sse2
#define SwizzleBlock32u SwizzleBlock32u_sse2
#define SwizzleBlock16u SwizzleBlock16u_sse2
#define SwizzleBlock8u SwizzleBlock8u_sse2
#define SwizzleBlock4u SwizzleBlock4u_sse2

#define FrameSwizzleBlock32 FrameSwizzleBlock32_c
#define FrameSwizzleBlock16 FrameSwizzleBlock16_c
#define Frame16SwizzleBlock32 Frame16SwizzleBlock32_c
#define Frame16SwizzleBlock32Z Frame16SwizzleBlock32Z_c
#define Frame16SwizzleBlock16 Frame16SwizzleBlock16_c
#define Frame16SwizzleBlock16Z Frame16SwizzleBlock16Z_c

#define FrameSwizzleBlock32A2 FrameSwizzleBlock32A2_c
#define FrameSwizzleBlock16A2 FrameSwizzleBlock16A2_c
#define Frame16SwizzleBlock32A2 Frame16SwizzleBlock32A2_c
#define Frame16SwizzleBlock32ZA2 Frame16SwizzleBlock32ZA2_c
#define Frame16SwizzleBlock16A2 Frame16SwizzleBlock16A2_c
#define Frame16SwizzleBlock16ZA2 Frame16SwizzleBlock16ZA2_c

#define FrameSwizzleBlock32A4 FrameSwizzleBlock32A4_c
#define FrameSwizzleBlock16A4 FrameSwizzleBlock16A4_c
#define Frame16SwizzleBlock32A4 Frame16SwizzleBlock32A4_c
#define Frame16SwizzleBlock32ZA4 Frame16SwizzleBlock32ZA4_c
#define Frame16SwizzleBlock16A4 Frame16SwizzleBlock16A4_c
#define Frame16SwizzleBlock16ZA4 Frame16SwizzleBlock16ZA4_c

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_sse2
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_sse2
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_sse2
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_sse2

#else

#define SwizzleBlock32 SwizzleBlock32_c
#define SwizzleBlock16 SwizzleBlock16_c
#define SwizzleBlock8 SwizzleBlock8_c
#define SwizzleBlock4 SwizzleBlock4_c
#define SwizzleBlock32u SwizzleBlock32_c
#define SwizzleBlock16u SwizzleBlock16_c
#define SwizzleBlock8u SwizzleBlock8_c
#define SwizzleBlock4u SwizzleBlock4_c

#define FrameSwizzleBlock32 FrameSwizzleBlock32_c
#define FrameSwizzleBlock16 FrameSwizzleBlock16_c
#define Frame16SwizzleBlock32 Frame16SwizzleBlock32_c
#define Frame16SwizzleBlock32Z Frame16SwizzleBlock32Z_c
#define Frame16SwizzleBlock16 Frame16SwizzleBlock16_c
#define Frame16SwizzleBlock16Z Frame16SwizzleBlock16Z_c

#define FrameSwizzleBlock32A2 FrameSwizzleBlock32A2_c
#define FrameSwizzleBlock16A2 FrameSwizzleBlock16A2_c
#define Frame16SwizzleBlock32A2 Frame16SwizzleBlock32A2_c
#define Frame16SwizzleBlock32ZA2 Frame16SwizzleBlock32ZA2_c
#define Frame16SwizzleBlock16A2 Frame16SwizzleBlock16A2_c
#define Frame16SwizzleBlock16ZA2 Frame16SwizzleBlock16ZA2_c

#define FrameSwizzleBlock32A4 FrameSwizzleBlock32A4_c
#define FrameSwizzleBlock16A4 FrameSwizzleBlock16A4_c
#define Frame16SwizzleBlock32A4 Frame16SwizzleBlock32A4_c
#define Frame16SwizzleBlock32ZA4 Frame16SwizzleBlock32ZA4_c
#define Frame16SwizzleBlock16A4 Frame16SwizzleBlock16A4_c
#define Frame16SwizzleBlock16ZA4 Frame16SwizzleBlock16ZA4_c

#define WriteCLUT_T16_I8_CSM1 WriteCLUT_T16_I8_CSM1_c
#define WriteCLUT_T32_I8_CSM1 WriteCLUT_T32_I8_CSM1_c
#define WriteCLUT_T16_I4_CSM1 WriteCLUT_T16_I4_CSM1_c
#define WriteCLUT_T32_I4_CSM1 WriteCLUT_T32_I4_CSM1_c

#endif

#endif
