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

#pragma once
//#define mVUlogProg // Dumps MicroPrograms to \logs\*.html

class AsciiFile;
using namespace std;
using namespace x86Emitter;

#include <deque>

#include "VU.h"
#include "GS.h"
#include "Gif.h"
#include "iR5900.h"
#include "R5900OpcodeTables.h"
#include "x86emitter/x86emitter.h"
#include "SamplProf.h"
#include "microVU_Misc.h"
#include "microVU_IR.h"

struct microBlockLink {
	microBlock		block;
	microBlockLink*	next;
};

class microBlockManager {
private:
	microBlockLink* blockList;
	microBlockLink* blockEnd;
	int listI;

public:
	microBlockManager() {
		listI = 0;
		blockEnd = blockList = NULL;
	}
	~microBlockManager() { reset(); }
	void reset() {
		microBlockLink* linkI = blockList;
		while( linkI != NULL )
		{
			microBlockLink* freeI = linkI;
			linkI = linkI->next;
			_aligned_free(freeI);
		}
		listI = 0;
		blockEnd = blockList = NULL;
	};
	microBlock* add(microBlock* pBlock) {
		microBlock* thisBlock = search(&pBlock->pState);
		if (!thisBlock) {
			listI++;
			microBlockLink* newBlock = (microBlockLink*)_aligned_malloc(sizeof(microBlockLink), 16);
			newBlock->next = NULL;

			if (blockEnd) {
				blockEnd->next	= newBlock;
				blockEnd		= newBlock;
			}
			else {
				blockEnd = blockList = newBlock;
			}

			memcpy_const(&newBlock->block, pBlock, sizeof(microBlock));
			thisBlock =  &newBlock->block;
		}
		return thisBlock;
	}
	__ri microBlock* search(microRegInfo* pState) {
		if (pState->needExactMatch) { // Needs Detailed Search (Exact Match of Pipeline State)
			for (microBlockLink* linkI = blockList; linkI != NULL; linkI = linkI->next) {
				if (mVUquickSearch((void*)pState, (void*)&linkI->block.pState, sizeof(microRegInfo)))
					return &linkI->block;
			}
		}
		else { // Can do Simple Search (Only Matches the Important Pipeline Stuff)
			for (microBlockLink* linkI = blockList; linkI != NULL; linkI = linkI->next) {
				if (doConstProp && (linkI->block.pState.vi15 != pState->vi15)) continue;
				if (linkI->block.pState.quick32[0] != pState->quick32[0]) continue;
				if (linkI->block.pState.quick32[1] != pState->quick32[1]) continue;
				return &linkI->block;
			}
		}
		return NULL;
	}
	void printInfo(int pc) {
		if (listI < 7) return;
		microBlockLink* linkI = blockList;
		for (int i = 0; i <= listI; i++) {
			u32 viCRC = 0, vfCRC = 0, crc = 0, z = sizeof(microRegInfo)/4;
			for (u32 j = 0; j < 4;  j++) viCRC -= ((u32*)linkI->block.pState.VI)[j];
			for (u32 j = 0; j < 32; j++) vfCRC -= linkI->block.pState.VF[j].reg;
			for (u32 j = 0; j < z;  j++) crc   -= ((u32*)&linkI->block.pState)[j];
			DevCon.WriteLn(Color_Green, "[%04x][Block #%d][crc=%08x][q=%02d][p=%02d][xgkick=%d][vi15=%08x][viBackup=%02d]"
			"[flags=%02x][exactMatch=%x][blockType=%d][viCRC=%08x][vfCRC=%08x]", pc, i, crc, linkI->block.pState.q, 
			linkI->block.pState.p, linkI->block.pState.xgkick, linkI->block.pState.vi15, linkI->block.pState.viBackUp, 
			linkI->block.pState.flags, linkI->block.pState.needExactMatch, linkI->block.pState.blockType, viCRC, vfCRC);
			linkI = linkI->next;
		}
	}
};

struct microRange {
	s32 start; // Start PC (The opcode the block starts at)
	s32 end;   // End PC   (The opcode the block ends with)
};

#define mProgSize (0x4000/4)
struct microProgram {
	u32				   data [mProgSize];   // Holds a copy of the VU microProgram
	microBlockManager* block[mProgSize/2]; // Array of Block Managers
	deque<microRange>* ranges;			   // The ranges of the microProgram that have already been recompiled
	u32 startPC; // Start PC of this program
	int idx;	 // Program index
};

typedef deque<microProgram*> microProgramList;

struct microProgramQuick {
	microBlockManager*    block; // Quick reference to valid microBlockManager for current startPC
	microProgram*		  prog;	 // The microProgram who is the owner of 'block'
};

struct microProgManager {
	microIR<mProgSize>	IRinfo;				// IR information
	microProgramList*	prog [mProgSize/2];	// List of microPrograms indexed by startPC values
	microProgramQuick	quick[mProgSize/2];	// Quick reference to valid microPrograms for current execution
	microProgram*		cur;				// Pointer to currently running MicroProgram
	int					total;				// Total Number of valid MicroPrograms
	int					isSame;				// Current cached microProgram is Exact Same program as mVU->regs().Micro (-1 = unknown, 0 = No, 1 = Yes)
	int					cleared;			// Micro Program is Indeterminate so must be searched for (and if no matches are found then recompile a new one)
	u32					curFrame;			// Frame Counter
	u8*					x86ptr;				// Pointer to program's recompilation code
	u8*					x86start;			// Start of program's rec-cache
	u8*					x86end;				// Limit of program's rec-cache
	microRegInfo		lpState;			// Pipeline state from where program left off (useful for continuing execution)
};

#define mVUdispCacheSize (0x1000) // Dispatcher Cache Size
#define mVUcacheSize     ((index) ? (_1mb *  17) : (_1mb *  7)) // Initial Size (Excluding Safe-Zone)
#define mVUcacheMaxSize  ((mVU->index) ? (_1mb * 100) : (_1mb * 50)) // Max Size allowed to grow to
#define mVUcacheGrowBy	 ((mVU->index) ? (_1mb *  15) : (_1mb * 10)) // Grows by this amount
#define mVUcacheSafeZone ((index) ? (_1mb *   3) : (_1mb *  3)) // Safe-Zone for last program

struct microVU {

	__aligned16 u32 macFlag[4];		 // 4 instances of mac  flag (used in execution)
	__aligned16 u32 clipFlag[4];	 // 4 instances of clip flag (used in execution)
	__aligned16 u32 xmmCTemp[4];	 // Backup used in mVUclamp2()
	__aligned16 u32 xmmBackup[8][4]; // Backup for xmm0~xmm7

	u32 index;			// VU Index (VU0 or VU1)
	u32 cop2;			// VU is in COP2 mode?  (No/Yes)
	u32 vuMemSize;		// VU Main  Memory Size (in bytes)
	u32 microMemSize;	// VU Micro Memory Size (in bytes)
	u32 progSize;		// VU Micro Memory Size (in u32's)
	u32 progMemMask;	// VU Micro Memory Size (in u32's)
	u32 cacheSize;		// VU Cache Size

	microProgManager prog;		// Micro Program Data
	ScopedPtr<microRegAlloc>	regAlloc;	// Reg Alloc Class
	ScopedPtr<AsciiFile>		logFile;	// Log File Pointer

	u8*		cache;		 // Dynarec Cache Start (where we will start writing the recompiled code to)
	u8*		dispCache;	 // Dispatchers Cache (where startFunct and exitFunct are written to)
	u8*		startFunct;	 // Ptr Function to the Start code for recompiled programs
	u8*		exitFunct;	 // Ptr Function to the Exit  code for recompiled programs
	u32		code;		 // Contains the current Instruction
	u32		divFlag;	 // 1 instance of I/D flags
	u32		VIbackup;	 // Holds a backup of a VI reg if modified before a branch
	u32		VIxgkick;	 // Holds a backup of a VI reg used for xgkick-delays
	u32		branch;		 // Holds branch compare result (IBxx) OR Holds address to Jump to (JALR/JR)
	u32		badBranch;	 // For Branches in Branch Delay Slots, holds Address the first Branch went to + 8
	u32		evilBranch;	 // For Branches in Branch Delay Slots, holds Address to Jump to
	u32		p;			 // Holds current P instance index
	u32		q;			 // Holds current Q instance index
	u32		totalCycles; // Total Cycles that mVU is expected to run for
	u32		cycles;		 // Cycles Counter

	VURegs& regs() const { return ::vuRegs[index]; }

	VIFregisters& getVifRegs() const	{ return regs().GetVifRegs(); }
	__fi REG_VI& getVI(uint reg) const	{ return regs().VI[reg]; }
	__fi VECTOR& getVF(uint reg) const	{ return regs().VF[reg]; }


	__fi s16 Imm5() const	{ return ((code & 0x400) ? 0xfff0 : 0) | ((code >> 6) & 0xf); }
	__fi s32 Imm11() const	{ return (code & 0x400) ? (0xfffffc00 | (code & 0x3ff)) : (code & 0x3ff); }
	__fi u32 Imm12() const	{ return (((code >> 21) & 0x1) << 11) | (code & 0x7ff); }
	__fi u32 Imm15() const	{ return ((code >> 10) & 0x7800) | (code & 0x7ff); }
	__fi u32 Imm24() const	{ return code & 0xffffff; }

	// Fetches the PC and instruction opcode relative to the current PC.  Used to rewind and
	// fast-forward the IR state while calculating VU pipeline conditions (branches, writebacks, etc)
	__fi void advancePC( int x )
	{
		prog.IRinfo.curPC += x;
		prog.IRinfo.curPC &= progMemMask;
		code = ((u32*)regs().Micro)[prog.IRinfo.curPC];
	}
	
	__ri uint getBranchAddr() const
	{
		pxAssumeDev((prog.IRinfo.curPC & 1) == 0, "microVU recompiler: Upper instructions cannot have valid branch addresses.");
		return (((prog.IRinfo.curPC + 2)  + (Imm11() * 2)) & progMemMask) * 4;
	}

	__ri uint getBranchAddrN() const
	{
		pxAssumeDev((prog.IRinfo.curPC & 1) == 0, "microVU recompiler: Upper instructions cannot have valid branch addresses.");
		return (((prog.IRinfo.curPC + 4)  + (Imm11() * 2)) & progMemMask) * 4;
	}

	void init(uint vuIndex);
	void reset();
	void close();
};

// microVU rec structs
extern __aligned16 microVU microVU0;
extern __aligned16 microVU microVU1;

// Debug Helper
int mVUdebugNow = 0;

// Main Functions
static void  mVUclear(mV, u32, u32);
static void  mVUresizeCache(mV, u32);
static void* mVUblockFetch(microVU* mVU, u32 startPC, uptr pState);
_mVUt extern void* __fastcall mVUcompileJIT(u32 startPC, uptr pState);

// Prototypes for Linux
extern void  __fastcall mVUcleanUpVU0();
extern void  __fastcall mVUcleanUpVU1();
mVUop(mVUopU);
mVUop(mVUopL);

// Private Functions
_mVUt extern void  mVUcacheProg (microProgram&  prog);
_mVUt extern void  mVUdeleteProg(microProgram*& prog);
_mVUt extern void* mVUsearchProg(u32 startPC, uptr pState);
_mVUt extern microProgram* mVUfindLeastUsedProg();
extern void* __fastcall mVUexecuteVU0(u32 startPC, u32 cycles);
extern void* __fastcall mVUexecuteVU1(u32 startPC, u32 cycles);

// recCall Function Pointer
typedef void (__fastcall *mVUrecCall)(u32, u32);

