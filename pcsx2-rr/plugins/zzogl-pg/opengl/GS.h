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

#ifndef __GS_H__
#define __GS_H__


#define USE_OLD_REGS

#include "Util.h"
#include "GifTransfer.h"

using namespace std;

extern float fFPS;

#define MEMORY_END 0x00400000

extern int g_LastCRC;
extern u8* g_pBasePS2Mem;

extern u8* g_pbyGSMemory;

class GSMemory
{
	public:
		void init();
		void destroy();
		u8* get();
		u8* get(u32 addr);
		u8* get_raw(u32 addr);
};

extern u8* g_pbyGSClut;		// the temporary clut buffer

class GSClut
{
	public:
		void init();
		void destroy();
		u8* get();
		u8* get(u32 addr);
		u8* get_raw(u32 addr);
};

struct Vector_16F
{
	u16 x, y, z, w;
};

// PS2 vertex

struct VertexGPU
{
	// gained from XYZ2, XYZ3, XYZF2, XYZF3,
	// X -- bits 0-15, Y-16-31. Z - 32-63 if no F used, 32-55 otherwise, F (fog) - 56-63
	// X, Y stored in 12d3 format,
	s16 x, y, f, resv0;		// note: xy is 12d3
	// Vertex color settings. RGB -- luminance of red/green/blue, A -- alpha. 1.0 == 0x80.
	// Goes grom RGBAQ register, bits 0-7, 8-15, 16-23 and 24-31 accordingly
	u32 rgba;
	u32 z;
	// Texture coordinates. S & T going from ST register (bits 0-31, and 32-63).
	// Q goes from RGBAQ register, bits 32-63
	float s, t, q;
};

// Almost same as previous, controlled by prim.fst flags

struct Vertex
{
	u16 x, y, f, resv0;		// note: xy is 12d3
	u32 rgba;
	u32 z;
	float s, t, q;
	// Texel coordinate of vertex. Used if prim.fst == 1
	// Bits 0-14 and 16-30 of UV
	u16 u, v;
};

extern GSconf conf;

// PSM values
// PSM types == Texture Storage Format
enum PSM_value
{
	PSMCT32		= 0,		// 00 0000
	PSMCT24		= 1,		// 00 0001
	PSMCT16		= 2,		// 00 0010
	PSMCT16S	= 10,		// 00 1010
	PSMT8		= 19,		// 01 0011
	PSMT4		= 20,		// 01 0100
	PSMT8H		= 27,		// 01 1011
	PSMT4HL		= 36,		// 10 0100
	PSMT4HH		= 44,		// 10 1100
	PSMT32Z		= 48,		// 11 0000
	PSMT24Z		= 49,		// 11 0001
	PSMT16Z		= 50,		// 11 0010
	PSMT16SZ	= 58,		// 11 1010
};

// Check target bit mode. PSMCT32 and 32Z return 0, 24 and 24Z - 1
// 16, 16S, 16Z, 16SZ -- 2, PSMT8 and 8H - 3, PSMT4, 4HL, 4HH -- 4.
// This code returns the same value on Z-textures, so texel storage mode is (BITMODE and !ISZTEX).
inline int PSMT_BITMODE(int psm) {return (psm & 0x7);}

inline int PSMT_BITS_NUM(int psm)
{
	// Treat these as 32 bit.
	if ((psm == PSMT8H) || (psm == PSMT4HL) || (psm == PSMT4HH)) 
	{
		return 4;
	}
	
	switch (PSMT_BITMODE(psm))
	{
		case 4: 
			return 0;
			
		case 3:
			return 1;
			
		case 2:
			return 2;
			
		default:
			return 4;
	}
}

// CLUT = Color look up table. Set proper color to table according CLUT table.
// Used for PSMT8, PSMT8H, PSMT4, PSMT4HH, PSMT4HL textures
inline bool PSMT_ISCLUT(int psm) { return (PSMT_BITMODE(psm) > 2);}

// PSMCT16, PSMCT16S, PSMT16Z, PSMT16SZ is 16-bit targets and usually there is
// two of them in each 32-bit word.
inline bool PSMT_IS16BIT(int psm) { return (PSMT_BITMODE(psm) == 2);}

// PSMT32Z, PSMT24Z, PSMT16Z, PSMT16SZ is Z-buffer textures
inline bool PSMT_ISZTEX(int psm) {return ((psm & 0x30) == 0x30);}

// PSMCT16, PSMCT16S, PSMT8, PSMT8H, PSMT16Z and PSMT16SZ use only half 16 bit per pixel.
inline bool PSMT_ISHALF(int psm) {return ((psm & 2) == 2);}

// PSMT8 and PSMT8H use IDTEX8 CLUT, PSMT4H, PSMT4HL, PSMT4HH -- IDTEX4.
// Don't use it on non clut entries, please!
inline bool PSMT_IS8CLUT(int psm) {return ((psm & 3) == 3);}

// PSM16Z and PSMT16SZ use -1 offset to z-buff. Need to check this thesis.
inline bool PSMT_IS16Z(int psm) {return ((psm & 0x32) == 0x32);}

// Check to see if it is 32 bits. According to code comments, anyways.
// I'll have to look closer at it, because it'd seem like it'd return true for 24 bits.
inline bool PSMT_IS32BIT(int psm) {return !!(psm <= 1);}

// When color format is RGB24 (PSMCT24) or RGBA16 (PSMCT16 & 16S) alpha value expanded, based on
// TEXA register and AEM status.
inline int PSMT_ALPHAEXP(int psm) {return (psm == PSMCT24 || psm == PSMCT16 || psm == PSMCT16S);}


// This function updates the 6th and 5th bit of psm
// 00 or 11 -> 00 ; 01 -> 10 ; 10 -> 01
inline int Switch_Top_Bytes (int X) {
	if ( ( X & 0x30 ) == 0 )
		return X;
	else
		return (X ^ 0x30);
}

// How many pixel stored in 1 word.
// PSMT8 has 4 pixels per 32bit, PSMT4 has 8. All 16-bit textures are 2 pixel per bit. And all others are 1 pixel in texture.
inline int PIXELS_PER_WORD(int psm) 
{
	if (psm == PSMT8)
		return 4;
	if (psm == PSMT4)
		return 8;
	if (PSMT_IS16BIT(psm))
		return 2;
	return 1;
}

// Some storage formats could share the same memory block (2 textures in 1 format). This include following combinations:
// PSMT24(24Z) with either 8H, 4HL, 4HH and PSMT4HL with PSMT4HH.
// We use slightly different versions of this function on comparison with GSDX, Storage format XOR 0x30 made Z-textures
// similar to normal ones and change higher bits on short (8 and 4 bits) textures.
inline bool PSMT_HAS_SHARED_BITS (int fpsm, int tpsm) {
	int SUM = Switch_Top_Bytes(fpsm)  + Switch_Top_Bytes(tpsm) ;
	return (SUM == 0x15 || SUM == 0x1D || SUM == 0x2C || SUM == 0x30);
}


//----------------------- Data from registers -----------------------

typedef union
{
	s64 SD;
	u64 UD;
	s32 SL[2];
	u32 UL[2];
	s16 SS[4];
	u16 US[4];
	s8  SC[8];
	u8  UC[8];
} reg64;

/* general purpose regs structs */

typedef struct
{
	int fbp;
	int fbw;
	int fbh;
	int psm;
	u32 fbm;
} frameInfo;

// Create frame structure from known data
inline frameInfo CreateFrame(int fbp, int fbw, int fbh, int psm, u32 fbm)
{
	frameInfo frame;
	frame.fbp = fbp;
	frame.fbw = fbw;
	frame.fbh = fbh;
	frame.psm = psm;
	frame.fbm = fbm;
	return frame;
}

typedef struct 
{
	u16 prim;

	union 
	{
		struct 
		{
			u16 iip : 1;
			u16 tme : 1;
			u16 fge : 1;
			u16 abe : 1;
			u16 aa1 : 1;
			u16 fst : 1;
			u16 ctxt : 1;
			u16 fix : 1;
			u16 resv : 8;
		};
		u16 _val;
	};
} primInfo;

extern primInfo *prim;

typedef union 
{
	struct 
	{
		u32 ate : 1;
		u32 atst : 3;
		u32 aref : 8;
		u32 afail : 2;
		u32 date : 1;
		u32 datm : 1;
		u32 zte : 1;
		u32 ztst : 2;
		u32 resv : 13;
	};
	u32 _val;
} pixTest;

typedef struct
{
	int bp;
	int bw;
	int psm;
} bufInfo;

typedef struct
{
	int tbp0;
	int tbw;
	int cbp;
	u16 tw, th;
	u8 psm;
	u8 tcc;
	u8 tfx;
	u8 cpsm;
	u8 csm;
	u8 csa;
	u8 cld;
} tex0Info;

union tex_0_info
{
	struct
	{
		u64 tbp0 : 14;
		u64 tbw : 6;
		u64 psm : 6;
		u64 tw : 4;
		u64 th : 4;
		u64 tcc : 1;
		u64 tfx : 2;
		u64 cbp : 14;
		u64 cpsm : 4;
		u64 csm : 1;
		u64 csa : 5;
		u64 cld : 3;
	};

	u64 _u64;
	u32 _u32[2];
	u16 _u16[4];
	u8 _u8[8];
	
	tex_0_info(u64 data) { _u64 = data; }
	tex_0_info(u32 data) { _u32[0] = data; _u32[1] = 0; }
	tex_0_info(u32 data0, u32 data1) { _u32[0] = data0; _u32[1] = data1; }

	u32 tbw_mult()
	{
		if (tbw == 0)
			return 64;
		else
			return ((u32)tbw << 6);
	}

	u32 psm_fix()
	{
		//	printf ("psm %d\n", psm);
		if (psm == 9) return 1;

		return psm;
	}

	u32 tw_exp()
	{
		if (tw > 10) return (1 << 10);

		return (1 << tw);
	}

	u32 th_exp()
	{
		if (th > 10) return (1 << 10);

		return (1 << th);
	}

	u32 cpsm_fix()
	{
		return cpsm & 0xe;
	}

	u32 csa_fix()
	{
		if (cpsm < 2)
			return (csa & 0xf);
		else
			return (csa & 0x1f);
	}
};

#define TEX_MODULATE 0
#define TEX_DECAL 1
#define TEX_HIGHLIGHT 2
#define TEX_HIGHLIGHT2 3

typedef struct
{
	int lcm;
	int mxl;
	int mmag;
	int mmin;
	int mtba;
	int l;
	int k;
} tex1Info;

typedef struct
{
	int wms;
	int wmt;
	int minu;
	int maxu;
	int minv;
	int maxv;
} clampInfo;

typedef struct
{
	int cbw;
	int cou;
	int cov;
} clutInfo;

typedef struct
{
	int tbp[3];
	int tbw[3];
} miptbpInfo;

typedef struct
{
	u16 aem;
	u8 ta[2];
	float fta[2];
} texaInfo;

typedef struct
{
	int sx;
	int sy;
	int dx;
	int dy;
#ifdef USE_OLD_REGS
	int dir;
#else
	int diry;
	int dirx;
#endif
} trxposInfo;

typedef struct 
{
	union 
	{
		struct 
		{
			u8 a : 2;
			u8 b : 2;
			u8 c : 2;
			u8 d : 2;
		};
		u8 abcd;
	};

	u8 fix : 8;
} alphaInfo;

typedef struct
{
	u16 zbp;		// u16 address / 64
	u8 psm;
	u8 zmsk;
} zbufInfo;

typedef struct
{
	int fba;
} fbaInfo;

typedef struct
{
	Vertex gsvertex[3];
	u32 rgba;
	float q;
	Vertex vertexregs;

	int primC;		// number of verts current storing
	int primIndex;	// current prim index
	int nTriFanVert;

	int prac;
	int dthe;
	int colclamp;
	int fogcol;
	int smask;
	int pabe;
	u64 buff[2];
	int buffsize;
	int cbp[2];		// internal cbp registers

	u32 CSRw;

	primInfo _prim[2];
	bufInfo srcbuf, srcbufnew;
	bufInfo dstbuf, dstbufnew;

	clutInfo clut;

	texaInfo texa;
	trxposInfo trxpos, trxposnew;

	int imageWtemp, imageHtemp;

	int imageTransfer;
	int imageWnew, imageHnew, imageX, imageY, imageEndX, imageEndY;

	pathInfo path[4];
	GIFRegDIMX dimx;
	GSMemory mem;
	GSClut clut_buffer;
	int primNext(int inc = 1)
	{
		return ((primIndex + inc) % ARRAY_SIZE(gsvertex));
	}
	
	void setRGBA(u32 r, u32 g, u32 b, u32 a)
	{
		rgba = (r & 0xff) |
			  ((g & 0xff) <<  8) |
			  ((b & 0xff) << 16) |
			  ((a & 0xff) << 24);
	}
	
	void add_vertex(u16 x, u16 y, u32 z, u16 f)
	{
		vertexregs.x = x;
		vertexregs.y = y;
		vertexregs.z = z;
		vertexregs.f = f;
		gsvertex[primIndex] = vertexregs;
		primIndex = primNext();
	}
	
	void add_vertex(u16 x, u16 y, u32 z)
	{
		vertexregs.x = x;
		vertexregs.y = y;
		vertexregs.z = z;
		gsvertex[primIndex] = vertexregs;
		primIndex = primNext();
	}
} GSinternal;

extern GSinternal gs;

static __forceinline u16 RGBA32to16(u32 c)
{
	return (u16)((((c) & 0x000000f8) >>  3) |
				 (((c) & 0x0000f800) >>  6) |
				 (((c) & 0x00f80000) >>  9) |
				 (((c) & 0x80000000) >> 16));
}

static __forceinline u32 RGBA16to32(u16 c)
{
	return	(((c) & 0x001f) <<  3) |
		   (((c) & 0x03e0) <<  6) |
		   (((c) & 0x7c00) <<  9) |
		   (((c) & 0x8000) ? 0xff000000 : 0);
}

// converts float16 [0,1] to BYTE [0,255] (assumes value is in range, otherwise will take lower 8bits)
// f is a u16
static __forceinline u16 Float16ToBYTE(u16 f)
{
	//assert( !(f & 0x8000) );
	if (f & 0x8000) return 0;

	u16 d = ((((f & 0x3ff) | 0x400) * 255) >> (10 - ((f >> 10) & 0x1f) + 15));

	return d > 255 ? 255 : d;
}

static __forceinline u16 Float16ToALPHA(u16 f)
{
	//assert( !(f & 0x8000) );
	if (f & 0x8000) return 0;

	// round up instead of down (crash and burn), too much and charlie breaks
	u16 d = (((((f & 0x3ff) | 0x400)) * 255) >> (10 - ((f >> 10) & 0x1f) + 15));

	d = (d) >> 1;

	return d > 255 ? 255 : d;
}

#ifndef COLOR_ARGB
#define COLOR_ARGB(a,r,g,b) \
	((u32)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

// assumes that positive in [1,2] (then extracts fraction by just looking at the specified bits)
#define Float16ToBYTE_2(f) ((u8)(*(u16*)&f>>2))
#define Float16To5BIT(f) (Float16ToBYTE(f)>>3)

#define Float16Alpha(f) (((*(u16*)&f&0x7c00)>=0x3900)?0x8000:0) // alpha is >= 1

// converts an array of 4 u16s to a u32 color
// f is a pointer to a u16
#define Float16ToARGB(f) COLOR_ARGB(Float16ToALPHA(f.w), Float16ToBYTE(f.x), Float16ToBYTE(f.y), Float16ToBYTE(f.z));

#define Float16ToARGB16(f) (Float16Alpha(f.w)|(Float16To5BIT(f.x)<<10)|(Float16To5BIT(f.y)<<5)|Float16To5BIT(f.z))

// used for Z values
#define Float16ToARGB_Z(f) COLOR_ARGB((u32)Float16ToBYTE_2(f.w), Float16ToBYTE_2(f.x), Float16ToBYTE_2(f.y), Float16ToBYTE_2(f.z))
#define Float16ToARGB16_Z(f) ((Float16ToBYTE_2(f.y)<<8)|Float16ToBYTE_2(f.z))


inline float Clamp(float fx, float fmin, float fmax)
{
	if (fx < fmin) return fmin;

	return fx > fmax ? fmax : fx;
}

// Get pixel storage format from tex0. Clutted textures store pixels in cpsm format.
inline int PIXEL_STORAGE_FORMAT(const tex0Info& tex) {
	if (PSMT_ISCLUT(tex.psm)) 
		return tex.cpsm;
 	else
		return tex.psm;
 }
 
// If pixel storage format not PSMCT24 ot PSMCT32, then it is 16-bit. 
// Z-textures have 0x30 upper bits, so we eliminate them by &&(~0x30)
inline bool PSMT_ISHALF_STORAGE(const tex0Info& tex0) { return ((PIXEL_STORAGE_FORMAT(tex0) & (~0x30)) > 1); }

//--------------------------- Inlines for bitwise ops
//--------------------------- textures
// Tex0Info (TEXD_x registers) bits, lower word
// The register is really 64-bit, but we use 2 32bit ones to represent it
// Obtain tbp0 -- Texture Buffer Base Pointer (Word Address/64) -- from data. Bits 0-13.
static __forceinline int ZZOglGet_tbp0_TexBits(u32 data)
{
	//return tex_0_info(data).tbp0;
	return (data) & 0x3fff;
}

// Obtain tbw -- Texture Buffer Width (Texels/64) -- from data, do not multiply to 64. Bits 14-19
// ( data & 0xfc000 ) >> 14
static __forceinline int ZZOglGet_tbw_TexBits(u32 data)
{
	//return tex_0_info(data).tbw;
	return (data >> 14) & 0x3f;
}

// Obtain tbw -- Texture Buffer Width (Texels) -- from data, do multiply to 64, never return 0.
static __forceinline int ZZOglGet_tbw_TexBitsMult(u32 data)
{
	//return text_0_info(data).tbw_mult();
	int result = ZZOglGet_tbw_TexBits(data);

	if (result == 0)
		return 64;
	else
		return (result << 6);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25.
// (data & 0x3f00000) >> 20
static __forceinline int ZZOglGet_psm_TexBits(u32 data)
{
	//return tex_0_info(data).psm;
	return	((data >> 20) & 0x3f);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25. Fix incorrect psm == 9
static __forceinline int ZZOglGet_psm_TexBitsFix(u32 data)
{
	//return tex_0_info(data).psm_fix();
	int result = ZZOglGet_psm_TexBits(data) ;
//	printf ("result %d\n", result);

	if (result == 9) result = 1;

	return result;
}

// Obtain tw -- Texture Width (Width = 2^TW) -- from data. Bits 26-29
// (data & 0x3c000000)>>26
static __forceinline u16 ZZOglGet_tw_TexBits(u32 data)
{
	//return tex_0_info(data).tw;
	return	((data >> 26) & 0xf);
}

// Obtain tw -- Texture Width (Width = TW) -- from data. Width could newer be more than 1024.
static __forceinline u16 ZZOglGet_tw_TexBitsExp(u32 data)
{
	//return tex_0_info(data).tw_exp();
	u16 result = ZZOglGet_tw_TexBits(data);

	if (result > 10) result = 10;

	return (1 << result);
}

// TH set at the border of upper and higher words.
// Obtain th -- Texture Height (Height = 2^TH) -- from data. Bits 30-31 lower, 0-1 higher
// (dataLO & 0xc0000000) >> 30 + (dataHI & 0x3) * 0x4
static __forceinline u16 ZZOglGet_th_TexBits(u32 dataLO, u32 dataHI)
{
	//return tex_0_info(dataLO, dataHI).th;
	return (((dataLO >> 30) & 0x3) | ((dataHI & 0x3) << 2));
}

// Obtain th --Texture Height (Height = 2^TH) -- from data. Height could newer be more than 1024.
static __forceinline u16 ZZOglGet_th_TexBitsExp(u32 dataLO, u32 dataHI)
{
	//return tex_0_info(dataLO, dataHI).th_exp();
	u16 result = ZZOglGet_th_TexBits(dataLO, dataHI);

	if (result > 10) result = 10;

	return	(1 << result);
}

// Tex0Info bits, higher word.
// Obtain tcc -- Texture Color Component 0=RGB, 1=RGBA + use Alpha from TEXA reg when not in PSM -- from data. Bit 3
// (data & 0x4)>>2
static __forceinline u8 ZZOglGet_tcc_TexBits(u32 data)
{
	//return tex_0_info(0, data).tcc;
	return ((data >>  2) & 0x1);
}

// Obtain tfx -- Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2) -- from data. Bit 4-5
// (data & 0x18)>>3
static __forceinline u8 ZZOglGet_tfx_TexBits(u32 data)
{
	//return tex_0_info(0, data).tfx;
	return ((data >>  3) & 0x3);
}

// Obtain cbp from data -- Clut Buffer Base Pointer (Address/256) -- Bits 5-18
// (data & 0x7ffe0)>>5
static __forceinline int ZZOglGet_cbp_TexBits(u32 data)
{
	//return tex_0_info(0, data).cbp;
	return ((data >>  5) & 0x3fff);
}

// Obtain cpsm from data -- Clut pixel Storage Format -- Bits 19-22. 22nd is at no use.
// (data & 0x700000)>>19
// 0000 - psmct32; 0010 - psmct16; 1010 - psmct16s.
static __forceinline u8 ZZOglGet_cpsm_TexBits(u32 data)
{
	//return (tex_0_info(0, data).cpsm & 0xe);
	return ((data >> 19) & 0xe);
}

// Obtain csm -- I don't know what is it -- from data. Bit 23
// (data & 0x800000)>>23
// csm is the clut storage mode. 0 for CSM1, 1 for CSM2.
static __forceinline u8 ZZOglGet_csm_TexBits(u32 data)
{
	//return tex_0_info(0, data).csm;
	return ((data >> 23) & 0x1);
}

// Obtain csa -- -- from data. Bits 24-28
// (data & 0x1f000000)>>24
static __forceinline u8 ZZOglGet_csa_TexBits(u32 data)
{
	//return tex_0_info(0, data).csa_fix();

	if ((data & 0x700000) == 0)  // it is cpsm < 2 check
		return ((data >> 24) & 0xf);
	else
		return ((data >> 24) & 0x1f);
}

// Obtain cld --   -- from data. Bits 29-31
// (data & 0xe0000000)>>29
static __forceinline u8 ZZOglGet_cld_TexBits(u32 data)
{
	//return tex_0_info(0, data).cld;
	return ((data >> 29) & 0x7);
}

//-------------------------- frames
// FrameInfo bits.
// Obtain fbp -- frame Buffer Base Pointer (Word Address/2048) -- from data. Bits 0-15
inline int ZZOglGet_fbp_FrameBits(u32 data)
{
	return ((data) & 0x1ff);
}

// So we got address / 64, henceby frame fbp and tex tbp have the same dimension -- "real address" is x64.
inline int ZZOglGet_fbp_FrameBitsMult(u32 data)
{
	return (ZZOglGet_fbp_FrameBits(data) << 5);
}

// Obtain fbw -- width (Texels/64) -- from data. Bits 16-23
inline int ZZOglGet_fbw_FrameBits(u32 data)
{
	return ((data >> 16) & 0x3f);
}

inline int ZZOglGet_fbw_FrameBitsMult(u32 data)
{
	return (ZZOglGet_fbw_FrameBits(data) << 6);
}


// Obtain psm -- Pixel Storage Format -- from data. Bits 24-29.
// (data & 0x3f000000) >> 24
inline int ZZOglGet_psm_FrameBits(u32 data)
{
	return	((data >> 24) & 0x3f);
}

// Function for calculating overal height from frame data.
inline int ZZOgl_fbh_Calc(int fbp, int fbw, int psm)
{
	int fbh = (1024 * 1024 - 64 * fbp) / fbw;
	fbh &= ~0x1f;

	if (PSMT_ISHALF(psm)) fbh *= 2;
	if (fbh > 1024) fbh = 1024;

	//ZZLog::Debug_Log("ZZOgl_fbh_Calc: 0x%x", fbh);
	return fbh;
}

inline int ZZOgl_fbh_Calc(frameInfo frame)
{
	return ZZOgl_fbh_Calc(frame.fbp, frame.fbw, frame.psm);
}

// Calculate fbh from data, It does not set in register
inline int ZZOglGet_fbh_FrameBitsCalc(u32 data)
{
	int fbh = 0;
	int fbp = ZZOglGet_fbp_FrameBits(data);
	int fbw = ZZOglGet_fbw_FrameBits(data);
	int psm = ZZOglGet_psm_FrameBits(data);

	if (fbw > 0) fbh = ZZOgl_fbh_Calc(fbp, fbw, psm) ;

	return fbh ;
}

// Obtain fbm -- frame mask -- from data. All higher word.
inline u32 ZZOglGet_fbm_FrameBits(u32 data)
{
	return (data);
}

// Obtain fbm -- frame mask -- from data. All higher word. Fixed from psm == PCMT24 (without alpha)
inline u32 ZZOglGet_fbm_FrameBitsFix(u32 dataLO, u32 dataHI)
{
	if (PSMT_BITMODE(ZZOglGet_psm_FrameBits(dataLO)) == 1)
		return (dataHI | 0xff000000);
	else
		return dataHI;
}

// obtain colormask RED
inline u32 ZZOglGet_fbmRed_FrameBits(u32 data)
{
	return (data & 0xff);
}

// obtain colormask Green
inline u32 ZZOglGet_fbmGreen_FrameBits(u32 data)
{
	return ((data >> 8) & 0xff);
}

// obtain colormask Blue
inline u32 ZZOglGet_fbmBlue_FrameBits(u32 data)
{
	return ((data >> 16) & 0xff);
}

// obtain colormask Alpha
inline u32 ZZOglGet_fbmAlpha_FrameBits(u32 data)
{
	return ((data >> 24) & 0xff);
}

// obtain colormask Alpha
inline u32 ZZOglGet_fbmHighByte(u32 data)
{
	return (!!(data & 0x80000000));
}

//-------------------------- tex0 comparison
// Check if old and new tex0 registers have only clut difference
inline bool ZZOglAllExceptClutIsSame(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[0] == newtex[0]) && ((oldtex[1] & 0x1f) == (newtex[1] & 0x1f)));
}

// Check if the CLUT registers are same, except CLD
inline bool ZZOglClutMinusCLDunchanged(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[1] & 0x1fffffe0) == (newtex[1] & 0x1fffffe0));
}

// Check if CLUT storage mode is not changed (CSA, CSM and CSPM)
inline bool ZZOglClutStorageUnchanged(const u32* oldtex, const u32* newtex)
{
	return ((oldtex[1] & 0x1ff10000) == (newtex[1] & 0x1ff10000));
}

// CSA and CPSM bitmask 0001 1111 0111 1000 ...
//                         60   56   52
#define CPSM_CSA_BITMASK 0x1f780000
#define CPSM_CSA_NOTMASK 0xe0870000

#endif
