/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "Common.h"
#include "GS.h"
#include "Gif.h"
#include "Vif_Dma.h"
#include "Vif.h"
#include <xmmintrin.h>

// --------------------------------------------------------------------------------------
//  GIFpath -- the GIFtag Parser
// --------------------------------------------------------------------------------------

enum GIF_FLG
{
	GIF_FLG_PACKED	= 0,
	GIF_FLG_REGLIST	= 1,
	GIF_FLG_IMAGE	= 2,
	GIF_FLG_IMAGE2	= 3
};

enum GIF_REG
{
	GIF_REG_PRIM	= 0x00,
	GIF_REG_RGBA	= 0x01,
	GIF_REG_STQ		= 0x02,
	GIF_REG_UV		= 0x03,
	GIF_REG_XYZF2	= 0x04,
	GIF_REG_XYZ2	= 0x05,
	GIF_REG_TEX0_1	= 0x06,
	GIF_REG_TEX0_2	= 0x07,
	GIF_REG_CLAMP_1	= 0x08,
	GIF_REG_CLAMP_2	= 0x09,
	GIF_REG_FOG		= 0x0a,
	GIF_REG_XYZF3	= 0x0c,
	GIF_REG_XYZ3	= 0x0d,
	GIF_REG_A_D		= 0x0e,
	GIF_REG_NOP		= 0x0f,
};

// GIFTAG
// Members of this structure are in CAPS to help visually denote that they are representative
// of actual hw register states of the GIF, unlike the internal tracking vars in GIFPath, which
// are modified during the GIFtag unpacking process.
struct GIFTAG
{
	u16 NLOOP	: 15;
	u16 EOP		: 1;

	// Note that contents of the Dummy bits on real hardware is likely used to maintain state
	// information regarding tag processing (namely nllop and curreg info, so to resume partial
	// transfers later).
	u16 _dummy0	: 16;
	u32 _dummy1	: 14;

	u32 PRE		: 1;
	u32 PRIM	: 11;
	u32 FLG		: 2;
	u32 NREG	: 4;
	u32 REGS[2];

	GIFTAG() {}
};

// --------------------------------------------------------------------------------------
//  GIFPath -- PS2 GIFtag info (one for each path).
// --------------------------------------------------------------------------------------
// fixme: The real PS2 has a single internal PATH and 3 logical sources, not 3 entirely
// separate paths.  But for that to work properly we need also interlocked path sources.
// That is, when the GIF selects a source, it sticks to that source until an EOP.  Currently
// this is not emulated!

struct GIFPath
{
	const GIFTAG tag;	// A copy of the "original" tag -- modification allowed only by SetTag(), so let's make it const.
	u8 regs[16];		// positioned after tag ensures 16-bit aligned (in case we SSE optimize later)

	u32 nloop;			// local copy nloop counts toward zero, and leaves the tag copy unmodified.
	u32 curreg;			// reg we left of on (for traversing through loops)
	u32 numregs;		// number of regs (when NREG is 0, numregs is 16)
	u32 DetectE;

	GIFPath();

	void Reset();
	void PrepPackedRegs();
	bool StepReg();
	u8 GetReg();
	bool IsActive() const;

	template< bool Aligned >
	void SetTag(const void* mem);

	template< GIF_PATH pathidx, bool Aligned >
	int CopyTag(const u128* pMem, u32 size);

	int ParseTagQuick(GIF_PATH pathidx, const u8* pMem, u32 size);
};

typedef void (__fastcall *GIFRegHandler)(const u32* data);

struct GifPathStruct
{
	const GIFRegHandler	Handlers[0x100-0x60];		// handlers for 0x60->0x100
	GIFPath				path[3];

	__fi GIFPath& operator[]( int idx ) { return path[idx]; }
};


// --------------------------------------------------------------------------------------
//  SIGNAL / FINISH / LABEL
// --------------------------------------------------------------------------------------

bool SIGNAL_IMR_Pending = false;
u32 SIGNAL_Data_Pending[2];


// SIGNAL : This register is a double-throw.  If the SIGNAL bit in CSR is clear, set the CSR
//   and raise a gsIrq.  If CSR is already *set*, then do not raise a gsIrq, and ignore all
//   subsequent drawing operations and writes to general purpose registers to the GS. (note:
//   I'm pretty sure this includes direct GS and GSreg accesses, as well as those coming
//   through the GIFpath -- but that behavior isn't confirmed yet).  Privileged writes are
//   still active.
//
//   Ignorance continues until the SIGNAL bit in CSR is manually cleared by the EE.  And here's
//   the tricky part: the interrupt from the second SIGNAL is still pending, and should be
//   raised once the EE has reset the *IMR* mask for SIGNAL -- meaning setting the bit to 1
//   (disabled/masked) and then back to 0 (enabled/unmasked).  Until the *IMR* is cleared, the
//   SIGNAL is still in the second throw stage, and will freeze the GS upon being written.
//
static void __fastcall RegHandlerSIGNAL(const u32* data)
{
	// HACK:
	// Soul Calibur 3 seems to be doing SIGNALs on PATH2 and PATH3 simultaneously, and isn't
	// too happy with the results (dies on bootup).  It properly clears the SIGNAL interrupt
	// but seems to get stuck on a VBLANK OVERLAP loop.  Fixing SIGNAL so that it properly
	// stalls the GIF might fix it.  Investigating the game's internals more deeply may also
	// be revealing. --air

	if (CSRreg.SIGNAL)
	{
		// Time to ignore all subsequent drawing operations. (which is not yet supported)
		if (!SIGNAL_IMR_Pending)
		{
			//DevCon.WriteLn( Color_StrongOrange, "GS SIGNAL double throw encountered!" );
			SIGNAL_IMR_Pending	= true;
			SIGNAL_Data_Pending[0]	= data[0];
			SIGNAL_Data_Pending[1]	= data[1];
			
			// [TODO] (SIGNAL) : Disable GIFpath DMAs here!
			//   All PATHs and DMAs should be disabled until the CSR is written and the
			//   SIGNAL bit cleared.
		}
	}
	else
	{
		GIF_LOG("GS SIGNAL data=%x_%x IMR=%x CSRr=%x",data[0], data[1], GSIMR, GSCSRr);
		GSSIGLBLID.SIGID = (GSSIGLBLID.SIGID&~data[1])|(data[0]&data[1]);

		if (!(GSIMR&0x100))
			gsIrq();

		CSRreg.SIGNAL = true;
	}
}

// FINISH : Enables end-of-draw signaling.  When FINISH is written it tells the GIF to
//   raise a gsIrq and set the FINISH bit of CSR when the *current drawing operation* is
//   finished.  Translation: Only after all three logical GIFpaths are in EOP status.
//
//   This feature can be used for both reversing the GS transfer mode (downloading post-
//   processing effects to the EE), and more importantly for *DMA synch* between the
//   three logical GIFpaths.
//
static void __fastcall RegHandlerFINISH(const u32* data)
{
	GIF_LOG("GIFpath FINISH data=%x_%x CSRr=%x", data[0], data[1], GSCSRr);

	// The FINISH bit is set here, and then it will be cleared when all three
	// logical GIFpaths finish their packets (EOPs) At that time (found below
	// in the GIFpath_Parser), IMR is tested and a gsIrq() raised if needed.

	CSRreg.FINISH = true;
}

static void __fastcall RegHandlerLABEL(const u32* data)
{
	GIF_LOG( "GIFpath LABEL" );
	GSSIGLBLID.LBLID = (GSSIGLBLID.LBLID&~data[1])|(data[0]&data[1]);
}

static void __fastcall RegHandlerUNMAPPED(const u32* data)
{
	const int regidx = ((u8*)data)[8];

	// Known "unknowns":
	//  It's possible that anything above 0x63 should just be silently ignored, but in the
	//  offhand chance not, I'm documenting known cases of unknown register use here.
	//
	//  0x7F -->
	//   the bios likes to write to 0x7f using an EOP giftag with NLOOP set to 4.
	//   Not sure what it's trying to accomplish exactly.  Ignoring seems to work fine,
	//   and is probably the intended behavior (it's likely meant to be a NOP).
	//
	//  0xEE -->
	//   .hack Infection [PAL confirmed, NTSC unknown] uses 0xee when you zoom the camera.
	//   The use hasn't been researched yet so parameters are unknown.  Everything seems
	//   to work fine as usual -- The 0xEE address in common programming terms is typically
	//   left over uninitialized data, and this might be a case of that, which is to be
	//   silently ignored.
	//
	//  Guitar Hero 3+ : Massive spamming when using superVU (along with several VIF errors)
	//  Using microVU avoids the GIFtag errors, so probably just one of sVU's hacks conflicting
	//  with one of VIF's hacks, and causing corrupted packet data.

	if( regidx != 0x7f && regidx != 0xee )
		DbgCon.Warning( "Ignoring Unmapped GIFtag Register, Index = %02x", regidx );
}

#define INSERT_UNMAPPED_4	RegHandlerUNMAPPED, RegHandlerUNMAPPED, RegHandlerUNMAPPED, RegHandlerUNMAPPED,
#define INSERT_UNMAPPED_16	INSERT_UNMAPPED_4 INSERT_UNMAPPED_4 INSERT_UNMAPPED_4 INSERT_UNMAPPED_4
#define INSERT_UNMAPPED_64	INSERT_UNMAPPED_16 INSERT_UNMAPPED_16 INSERT_UNMAPPED_16 INSERT_UNMAPPED_16

static __aligned16 GifPathStruct s_gifPath =
{
	RegHandlerSIGNAL, RegHandlerFINISH, RegHandlerLABEL, RegHandlerUNMAPPED,

	// Rest are mapped to Unmapped
	INSERT_UNMAPPED_4  INSERT_UNMAPPED_4  INSERT_UNMAPPED_4
	INSERT_UNMAPPED_64 INSERT_UNMAPPED_64 INSERT_UNMAPPED_16
};

// --------------------------------------------------------------------------------------
//  GIFPath Method Implementations
// --------------------------------------------------------------------------------------

GIFPath::GIFPath() : tag()
{
	Reset();
}

__fi void GIFPath::Reset()
{
	memzero(*this);
	const_cast<GIFTAG&>(tag).EOP = 1;
}

__fi bool GIFPath::StepReg()
{
	if (++curreg >= numregs) {
		curreg = 0;
		if (--nloop == 0) {
			return false;
		}
	}
	return true;
}

__fi u8 GIFPath::GetReg() { return regs[curreg]; }

// Unpack the registers - registers are stored as a sequence of 4 bit values in the
// upper 64 bits of the GIFTAG.  That sucks for us when handling partialized GIF packets
// coming in from paths 2 and 3, so we unpack them into an 8 bit array here.
//
__fi void GIFPath::PrepPackedRegs()
{
	// Only unpack registers if we're starting a new pack.  Otherwise the unpacked
	// array should have already been initialized by a previous partial transfer.

	if (curreg != 0) return;
	DetectE = 0;
	u32 tempreg = tag.REGS[0];
	numregs		= ((tag.NREG-1)&0xf) + 1;

	for (u32 i = 0; i < numregs; i++) {
		if (i == 8) tempreg = tag.REGS[1];
		regs[i] = tempreg & 0xf;
		if(regs[i] == 0xe) DetectE++;
		tempreg >>= 4;
	}
}


template< bool Aligned >
__fi void GIFPath::SetTag(const void* mem)
{
	_mm_store_ps( (float*)&tag, Aligned ? _mm_load_ps((const float*)mem) : _mm_loadu_ps((const float*)mem) );

	nloop	= tag.NLOOP;
	curreg	= 0;
}

__fi bool GIFPath::IsActive() const
{
	return (nloop != 0) || !tag.EOP;
}

void SaveStateBase::gifPathFreeze()
{
	FreezeTag( "GIFpath" );
	Freeze( s_gifPath.path );
}


static __fi void gsHandler(const u8* pMem)
{
	const int reg = pMem[8];

	if (reg == 0x50)
	{
		vif1.BITBLTBUF._u64 = *(u64*)pMem;
	}
	else if (reg == 0x52)
	{
		vif1.TRXREG._u64 = *(u64*)pMem;
	}
	else if (reg == 0x53)
	{
		// local -> host
		if ((pMem[0] & 3) == 1)
		{
			//Onimusha does TRXREG without BLTDIVIDE first, so we "assume" 32bit for this equation, probably isnt important.
			// ^ WTF, seriously? This is really important (pseudonym)
			u8 bpp = 32;

			switch(vif1.BITBLTBUF.SPSM & 7)
			{
			case 0:
				bpp = 32;
				break;
			case 1:
				bpp = 24;
				break;
			case 2:
				bpp = 16;
				break;
			case 3:
				bpp = 8;
				break;
			// 4 is 4 bit but this is forbidden
			default:
				Console.Error("Illegal format for GS upload: SPSM=0%02o", vif1.BITBLTBUF.SPSM);
			}

			VIF_LOG("GS Download %dx%d SPSM=%x bpp=%d", vif1.TRXREG.RRW, vif1.TRXREG.RRH, vif1.BITBLTBUF.SPSM, bpp);

			// qwords, rounded down; any extra bits are lost
			// games must take care to ensure transfer rectangles are exact multiples of a qword
			vif1.GSLastDownloadSize = vif1.TRXREG.RRW * vif1.TRXREG.RRH * bpp >> 7;
			//DevCon.Warning("GS download in progress");
			gifRegs.stat.OPH = true;
		}
	}
	if (reg >= 0x60)
	{
		// Question: What happens if an app writes to uncharted register space on real PS2
		// hardware (handler 0x63 and higher)?  Probably a silent ignorance, but not tested
		// so just guessing... --air

		s_gifPath.Handlers[reg-0x60]((const u32*)pMem);
	}
}

#define incTag(y) do {				\
	pMem += (y*16);						\
	size -= (y);						\
} while(false)

#define aMin(x, y) std::min(x, y)

// Parameters:
//   size - max size of incoming data stream, in qwc (simd128).  If the path is PATH1, and the
//     path does not terminate (EOP) within the specified size, it is assumed that the path must
//     loop around to the start of VU memory and continue processing.
__fi int GIFPath::ParseTagQuick(GIF_PATH pathidx, const u8* pMem, u32 size)
{
	u32	startSize =  size;						// Start Size

	while (size > 0) {
		if (!nloop) {

			SetTag<false>(pMem);
			incTag(1);
		}
		else
		{
			switch(tag.FLG) {
				case GIF_FLG_PACKED:
				{
					GIF_LOG("Packed Mode");
					numregs	= ((tag.NREG-1)&0xf) + 1;

					// Note: curreg is *usually* zero here, but can be non-zero if a previous fragment was
					// handled via this optimized copy code below.

					const u32 listlen = (nloop * numregs) - curreg;	// the total length of this packed register list (in QWC)
					u32 len;

					if(size < listlen)
					{
						len = size;

						// We need to calculate both the number of full iterations of regs copied (nloops),
						// and any remaining registers not copied by this fragment.  A div/mod pair should
						// hopefully be optimized by the compiler into a single x86 div. :)

						const int nloops_copied		= len / numregs;
						const int regs_not_copied	= len % numregs;

						// Make sure to add regs_not_copied to curreg, to handle cases of multiple partial fragments.
						// (example: 3 fragments each of only 2 regs, then curreg should be 0, 2, and then 4 after
						//  each call to GIFPath_Parse; with no change to NLOOP).  Because of this we also need to
						//  check for cases where curreg wraps past an nloop.

						nloop -= nloops_copied;
						curreg += regs_not_copied;
						if(curreg >= numregs)
						{
							--nloop;
							curreg -= numregs;
						}
					}
					else 
					{
						len = listlen;
						curreg = 0;
						nloop = 0;
					}
					incTag(len);
				}
				break;
				case GIF_FLG_REGLIST:
				{
					GIF_LOG("Reglist Mode EOP %x", tag.EOP);

					// In reglist mode, the GIF packs 2 registers into each QWC.  The nloop however
					// can be an odd number, in which case the upper half of the final QWC is ignored (skipped).

					numregs	= ((tag.NREG-1)&0xf) + 1;
					const u32 total_reglen = (nloop * numregs) - curreg;	// total 'expected length' of this packed register list (in registers)
					const u32 total_listlen = (total_reglen+1) / 2;			// total 'expected length' of the register list, in QWC!  (+1 so to round it up)

					u32 len;

					if(size < total_listlen)
					{
						//Console.Warning("GIF path %d Fragmented REGLIST!  Please report if you experience problems", pathidx + 1);

						len = size;
						const u32 reglen = len * 2;

						const int nloops_copied		= reglen / numregs;
						const int regs_not_copied	= reglen % numregs;

						//DevCon.Warning("Hit it path %d", pathidx + 1);
						curreg += regs_not_copied;
						nloop -= nloops_copied;

						if(curreg >= numregs)
						{
							--nloop;
							curreg -= numregs;
						}
					}
					else 
					{
						len = total_listlen;
						curreg = 0;
						nloop = 0;
					}

					incTag(len);
					//if(curreg != 0 || (len % numregs) > 0) DevCon.Warning("Oops c %x n %x m %x r %x", curreg, nloop, (len % numregs), numregs);
				}
				break;
				case GIF_FLG_IMAGE:
				case GIF_FLG_IMAGE2:
				{
					GIF_LOG("IMAGE Mode");
					int len = aMin(size, nloop);
					incTag(len);
					nloop -= len;
				}
				break;
			}
		}
		if(pathidx == GIF_PATH_1)
		{
			if(size == 0 && (!tag.EOP || nloop > 0))
			{
				if(startSize < 0x400)
				{
					size = 0x400 - startSize;
					startSize = 0x400;
					pMem -= 0x4000;
				}
				else
				{
					// Note: The BIOS does an XGKICK on the VU1 and lets it DMA to the GS without an EOP
					// (seemingly to loop forever), only to write an EOP later on.  No other game is known to
					// do anything of the sort.
					// So lets just cap the DMA at 16k, and force it to "look" like it's terminated for now.
					// (note: truly accurate emulation would mean having the VU1's XGKICK break execution,
					//  split time to EE and other processors, and then resume the kick's DMA later.  
					//  ... yea, not happening for a while. ;) -- air

					Console.Warning("GIFTAG error, size exceeded VU memory size %x", startSize);
					nloop	= 0;
					const_cast<GIFTAG&>(tag).EOP = 1;
				}
			}
		}
		if (tag.EOP && !nloop) break;
	}

	size = (startSize - size);


	return size;
}

__ri void MemCopy_WrappedDest( const u128* src, u128* destBase, uint& destStart, uint destSize, uint len )
{
	uint endpos = destStart + len;
	if( endpos < destSize )
	{
		memcpy_qwc(&destBase[destStart], src, len );
		destStart += len;
	}
	else
	{
		uint firstcopylen = destSize - destStart;
		memcpy_qwc(&destBase[destStart], src, firstcopylen );

		destStart = endpos % destSize;
		memcpy_qwc(destBase, src+firstcopylen, destStart );
	}
}

__ri void MemCopy_WrappedSrc( const u128* srcBase, uint& srcStart, uint srcSize, u128* dest, uint len )
{
	uint endpos = srcStart + len;
	if( endpos < srcSize )
	{
		memcpy_qwc(dest, &srcBase[srcStart], len );
		srcStart += len;
	}
	else
	{
		uint firstcopylen = srcSize - srcStart;
		memcpy_qwc(dest, &srcBase[srcStart], firstcopylen );

		srcStart = endpos % srcSize;
		memcpy_qwc(dest+firstcopylen, srcBase, srcStart );
	}
}

#define copyTag() do {						\
	_mm_store_ps( (float*)&RingBuffer.m_Ring[ringpos], Aligned ? _mm_load_ps((float*)pMem128) : _mm_loadu_ps((float*)pMem128)); \
	++pMem128; --size;						\
	ringpos = (ringpos+1)&RingBufferMask;	\
} while(false)

// Parameters:
//   size - max size of incoming data stream, in qwc (simd128).  If the path is PATH1, and the
//     path does not terminate (EOP) within the specified size, it is assumed that the path must
//     loop around to the start of VU memory and continue processing.
template< GIF_PATH pathidx, bool Aligned > 
__fi int GIFPath::CopyTag(const u128* pMem128, u32 size)
{
	uint& ringpos = GetMTGS().m_packet_writepos;
	const uint original_ringpos = ringpos;

	u32	startSize =  size;						// Start Size

	while (size > 0) {
		if (!nloop) {

			SetTag<Aligned>((u8*)pMem128);
			copyTag();
			
			if(nloop > 0)
			{
				switch(pathidx)
				{
					case GIF_PATH_1:
						if(tag.FLG & 2)GSTransferStatus.PTH1 = IMAGE_MODE;
						else GSTransferStatus.PTH1 = TRANSFER_MODE;
						break;
					case GIF_PATH_2:
						if(tag.FLG & 2)GSTransferStatus.PTH2 = IMAGE_MODE;
						else GSTransferStatus.PTH2 = TRANSFER_MODE;
						break;
					case GIF_PATH_3:
						if(vif1Regs.mskpath3 == 1 && GSTransferStatus.PTH3 == STOPPED_MODE) 
						{
							GSTransferStatus.PTH3 = IDLE_MODE;
							
						}
						else
						{
							if(tag.FLG & 2)	GSTransferStatus.PTH3 = IMAGE_MODE;
							else GSTransferStatus.PTH3 = TRANSFER_MODE;
						}
						break;
				}
				
			}	
			if(GSTransferStatus.PTH3 < PENDINGSTOP_MODE || pathidx != 2)
			{
				gifRegs.stat.OPH = true;
				gifRegs.stat.APATH = pathidx + 1;	
			}

			if(pathidx == GIF_PATH_3) 
			{
				break;
			}
		}
		else
		{
			switch(pathidx)
			{
				case GIF_PATH_1:
					if(tag.FLG & 2)GSTransferStatus.PTH1 = IMAGE_MODE;
					else GSTransferStatus.PTH1 = TRANSFER_MODE;
					break;
				case GIF_PATH_2:
					if(tag.FLG & 2)GSTransferStatus.PTH2 = IMAGE_MODE;
					else GSTransferStatus.PTH2 = TRANSFER_MODE;
					break;
				case GIF_PATH_3:
					if(tag.FLG & 2)	GSTransferStatus.PTH3 = IMAGE_MODE;
					else GSTransferStatus.PTH3 = TRANSFER_MODE;

					break;
			}
			gifRegs.stat.APATH = pathidx + 1;
			gifRegs.stat.OPH = true;
	
			switch(tag.FLG) {
				case GIF_FLG_PACKED:
					GIF_LOG("Packed Mode EOP %x", tag.EOP);
					PrepPackedRegs();

					if(DetectE > 0)
					{
						do {
							if (GetReg() == 0xe) {
								gsHandler((u8*)pMem128);
							}
							copyTag();
						} while(StepReg() && size > 0 && SIGNAL_IMR_Pending == false);
					}
					else
					{
						//DevCon.WriteLn(Color_Orange, "No E detected on Path%d: nloop=%x, numregs=%x, curreg=%x, size=%x", pathidx + 1, nloop, numregs, curreg, size);

						// Note: curreg is *usually* zero here, but can be non-zero if a previous fragment was
						// handled via this optimized copy code below.

						const u32 listlen = (nloop * numregs) - curreg;	// the total length of this packed register list (in QWC)
						u32 len;

						if(size < listlen)
						{
							len = size;

							// We need to calculate both the number of full iterations of regs copied (nloops),
							// and any remaining registers not copied by this fragment.  A div/mod pair should
							// hopefully be optimized by the compiler into a single x86 div. :)

							const int nloops_copied		= len / numregs;
							const int regs_not_copied	= len % numregs;

							// Make sure to add regs_not_copied to curreg, to handle cases of multiple partial fragments.
							// (example: 3 fragments each of only 2 regs, then curreg should be 0, 2, and then 4 after
							//  each call to GIFPath_Parse; with no change to NLOOP).  Because of this we also need to
							//  check for cases where curreg wraps past an nloop.

							nloop -= nloops_copied;
							curreg += regs_not_copied;
							if(curreg >= numregs)
							{
								--nloop;
								curreg -= numregs;
							}
						}
						else 
						{
							len = listlen;
							curreg = 0;
							nloop = 0;
						}

						MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );
						pMem128 += len;
						size -= len;
					}
				break;
				case GIF_FLG_REGLIST:
				{
					GIF_LOG("Reglist Mode EOP %x", tag.EOP);

					// In reglist mode, the GIF packs 2 registers into each QWC.  The nloop however
					// can be an odd number, in which case the upper half of the final QWC is ignored (skipped).

					numregs	= ((tag.NREG-1)&0xf) + 1;
					const u32 total_reglen = (nloop * numregs) - curreg;	// total 'expected length' of this packed register list (in registers)
					const u32 total_listlen = (total_reglen+1) / 2;			// total 'expected length' of the register list, in QWC!  (+1 so to round it up)

					u32 len;

					if(size < total_listlen)
					{
						//Console.Warning("GIF path %d Fragmented REGLIST!  Please report if you experience problems", pathidx + 1);

						len = size;
						const u32 reglen = len * 2;

						const int nloops_copied		= reglen / numregs;
						const int regs_not_copied	= reglen % numregs;

						//DevCon.Warning("Hit it path %d", pathidx + 1);
						curreg += regs_not_copied;
						nloop -= nloops_copied;

						if(curreg >= numregs)
						{
							--nloop;
							curreg -= numregs;
						}
					}
					else 
					{
						len = total_listlen;
						curreg = 0;
						nloop = 0;
					}

					MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );
					pMem128 += len;
					size -= len;
				}
				break;
				case GIF_FLG_IMAGE:
				case GIF_FLG_IMAGE2:
				{
					GIF_LOG("IMAGE Mode EOP %x", tag.EOP);
					int len = aMin(size, nloop);

					MemCopy_WrappedDest( pMem128, RingBuffer.m_Ring, ringpos, RingBufferSize, len );

					pMem128 += len;
					size -= len;
					nloop -= len;
				}
				break;
			}
		}

		if(pathidx == GIF_PATH_1)
		{
			if(size == 0 && (!tag.EOP || nloop > 0))
			{
				if(startSize < 0x3ff)
				{
					size = 0x3ff - startSize;
					startSize = 0x3ff;
					pMem128 -= 0x400;
				}
				else
				{
					// Note: The BIOS does an XGKICK on the VU1 and lets it DMA to the GS without an EOP
					// (seemingly to loop forever), only to write an EOP later on.  No other game is known to
					// do anything of the sort.
					// So lets just cap the DMA at 16k, and force it to "look" like it's terminated for now.
					// (note: truly accurate emulation would mean having the VU1's XGKICK break execution,
					//  split time to EE and other processors, and then resume the kick's DMA later.  
					//  ... yea, not happening for a while. ;) -- air

					Console.Warning("GIFTAG error, size exceeded VU memory size %x", startSize);
					nloop	= 0;
					const_cast<GIFTAG&>(tag).EOP = 1;

					// Don't send the packet to the GS -- its incomplete and might cause the GS plugin
					// to get confused and die. >_<
					
					ringpos = original_ringpos;
				}
			}
		}

		if (tag.EOP && !nloop)
		{
			if (CSRreg.FINISH)
			{
				// IMPORTANT: only signal FINISH if ALL THREE paths are stopped (nloop is zero and EOP is set)
				// FINISH is *not* a per-path register, and it seems to pretty clearly indicate that all active
				// drawing *and* image transfer actions must be finished before the IRQ raises.

				if(gifRegs.stat.P1Q || gifRegs.stat.P2Q || gifRegs.stat.P3Q) 					
				{
					//GH3 and possibly others have path data queued waiting for another path to finish! we need to check they are done too
					//DevCon.Warning("Early FINISH signal! P1 %x P2 %x P3 %x", gifRegs.stat.P1Q, gifRegs.stat.P2Q, gifRegs.stat.P3Q);
				}
				else if (!(GSIMR&0x200) && !s_gifPath.path[0].IsActive() && !s_gifPath.path[1].IsActive() && !s_gifPath.path[2].IsActive())
				{
					gsIrq();
				}
			}
			
			// [TODO] : DMAC Arbitration rights should select the next queued GIF transfer here.
			
			break;
		}
		if(SIGNAL_IMR_Pending == true)
		{
			//DevCon.Warning("Path %x", pathidx + 1);
			break;
		}
	}

	size = (startSize - size);

	if (tag.EOP && nloop == 0) {
	
		/*if(gifRegs.stat.DIR == 0)gifRegs.stat.OPH = false;
		gifRegs.stat.APATH = GIF_APATH_IDLE;*/
		switch(pathidx)
		{
			case GIF_PATH_1:
				GSTransferStatus.PTH1 = STOPPED_MODE;
				break;
			case GIF_PATH_2:
				GSTransferStatus.PTH2 = STOPPED_MODE;
				break;
			case GIF_PATH_3:
				//For huge chunks we may have delay problems, so we need to stall it till the interrupt, else we get desync (Lemmings)
				if(size > 8) GSTransferStatus.PTH3 = PENDINGSTOP_MODE;
				else  GSTransferStatus.PTH3 = STOPPED_MODE;
				if (gifch.chcr.STR) { //Make sure we are really doing a DMA and not using FIFO
					//GIF_LOG("Path3 end EOP %x NLOOP %x Status %x", tag.EOP, nloop, GSTransferStatus.PTH3);
					gifch.madr += size * 16;
					gifch.qwc  -= size;
				}
				break;
		}
	}
	else if(pathidx == 2)
	{
		//if(nloop <= 16 && GSTransferStatus.PTH3 == IMAGE_MODE)GSTransferStatus.PTH3 = PENDINGIMAGE_MODE;
		if (gifch.chcr.STR) { //Make sure we are really doing a DMA and not using FIFO
			//GIF_LOG("Path3 end EOP %x NLOOP %x Status %x", tag.EOP, nloop, GSTransferStatus.PTH3);
			gifch.madr += size * 16;
			gifch.qwc  -= size;
		}
	}

	return size;
}

// Parameters:
//   size - max size of incoming data stream, in qwc (simd128).  If the path is PATH1, and the
//     path does not terminate (EOP) within the specified size, it is assumed that the path must
//     loop around to the start of VU memory and continue processing.
__fi int GIFPath_CopyTag(GIF_PATH pathidx, const u128* pMem, u32 size)
{
	switch( pathidx )
	{
		case GIF_PATH_1:
			pxAssertMsg(!s_gifPath[GIF_PATH_2].IsActive(), "GIFpath conflict: Attempted to start PATH1 while PATH2 is already active.");
			pxAssertMsg(!s_gifPath[GIF_PATH_3].IsActive() || (GSTransferStatus.PTH3 == IMAGE_MODE), "GIFpath conflict: Attempted to start PATH1 while PATH3 is already active.");
			return s_gifPath[GIF_PATH_1].CopyTag<GIF_PATH_1,true>(pMem, size);
		case GIF_PATH_2:
			pxAssertMsg(!s_gifPath[GIF_PATH_1].IsActive(), "GIFpath conflict: Attempted to start PATH2 while PATH1 is already active.");
			pxAssertMsg(!s_gifPath[GIF_PATH_3].IsActive() || (GSTransferStatus.PTH3 == IMAGE_MODE), "GIFpath conflict: Attempted to start PATH2 while PATH3 is already active.");
			return s_gifPath[GIF_PATH_2].CopyTag<GIF_PATH_2,false>(pMem, size);
		case GIF_PATH_3:
			pxAssertMsg(!s_gifPath[GIF_PATH_1].IsActive(), "GIFpath conflict: Attempted to start PATH3 while PATH1 is already active.");
			pxAssertMsg(!s_gifPath[GIF_PATH_2].IsActive(), "GIFpath conflict: Attempted to start PATH3 while PATH2 is already active.");
			return s_gifPath[GIF_PATH_3].CopyTag<GIF_PATH_3,true>(pMem, size);

		jNO_DEFAULT;
	}
	
	return 0;		// unreachable
}

// Quick version for queuing PATH1 data.
// This version calculates the real length of the packet data only.  It does not process
// IRQs or DMA status updates.
__fi int GIFPath_ParseTagQuick(GIF_PATH pathidx, const u8* pMem, u32 size)
{
	int retSize = s_gifPath[pathidx].ParseTagQuick(pathidx, pMem, size);
	return retSize;
}

// Clears all GIFpath data to zero.
void GIFPath_Reset()
{
	for(uint i=0; i<3; ++i )
		s_gifPath.path[i].Reset();
}

// This is a hackfix tool provided for "canceling" the contents of the GIFpath when
// invalid GIFdma states are encountered (typically needed for PATH3 only).
__fi void GIFPath_Clear( GIF_PATH pathidx )
{
	memzero(s_gifPath.path[pathidx]);
	s_gifPath.path[pathidx].Reset();

	GSTransferStatus._u32 &= ~(0xf << (pathidx * 4));
	GSTransferStatus._u32 |= (0x5 << (pathidx * 4));
	if( GSgifSoftReset == NULL ) return;
	GetMTGS().SendSimplePacket( GS_RINGTYPE_SOFTRESET, (1<<pathidx), 0, 0 );
}
