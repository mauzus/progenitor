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

#include "MemoryTypes.h"

// Specialized function pointers for each read type
typedef  mem8_t __fastcall vtlbMemR8FP(u32 addr);
typedef  mem16_t __fastcall vtlbMemR16FP(u32 addr);
typedef  mem32_t __fastcall vtlbMemR32FP(u32 addr);
typedef  void __fastcall vtlbMemR64FP(u32 addr,mem64_t* data);
typedef  void __fastcall vtlbMemR128FP(u32 addr,mem128_t* data);

// Specialized function pointers for each write type
typedef  void __fastcall vtlbMemW8FP(u32 addr,mem8_t data);
typedef  void __fastcall vtlbMemW16FP(u32 addr,mem16_t data);
typedef  void __fastcall vtlbMemW32FP(u32 addr,mem32_t data);
typedef  void __fastcall vtlbMemW64FP(u32 addr,const mem64_t* data);
typedef  void __fastcall vtlbMemW128FP(u32 addr,const mem128_t* data);

typedef u32 vtlbHandler;

extern void vtlb_Core_Alloc();
extern void vtlb_Core_Shutdown();
extern void vtlb_Init();
extern void vtlb_Reset();
extern void vtlb_Term();
extern u8* vtlb_malloc( uint size, uint align );
extern void vtlb_free( void* pmem, uint size );


extern vtlbHandler vtlb_NewHandler();

extern vtlbHandler vtlb_RegisterHandler(
	vtlbMemR8FP* r8,vtlbMemR16FP* r16,vtlbMemR32FP* r32,vtlbMemR64FP* r64,vtlbMemR128FP* r128,
	vtlbMemW8FP* w8,vtlbMemW16FP* w16,vtlbMemW32FP* w32,vtlbMemW64FP* w64,vtlbMemW128FP* w128
);

extern void vtlb_ReassignHandler( vtlbHandler rv,
	vtlbMemR8FP* r8,vtlbMemR16FP* r16,vtlbMemR32FP* r32,vtlbMemR64FP* r64,vtlbMemR128FP* r128,
	vtlbMemW8FP* w8,vtlbMemW16FP* w16,vtlbMemW32FP* w32,vtlbMemW64FP* w64,vtlbMemW128FP* w128
);


extern void vtlb_MapHandler(vtlbHandler handler,u32 start,u32 size);
extern void vtlb_MapBlock(void* base,u32 start,u32 size,u32 blocksize=0);
extern void* vtlb_GetPhyPtr(u32 paddr);
//extern void vtlb_Mirror(u32 new_region,u32 start,u32 size); // -> not working yet :(

//virtual mappings
extern void vtlb_VMap(u32 vaddr,u32 paddr,u32 sz);
extern void vtlb_VMapBuffer(u32 vaddr,void* buffer,u32 sz);
extern void vtlb_VMapUnmap(u32 vaddr,u32 sz);

//Memory functions

template< typename DataType >
extern DataType __fastcall vtlb_memRead(u32 mem);
extern void __fastcall vtlb_memRead64(u32 mem, mem64_t *out);
extern void __fastcall vtlb_memRead128(u32 mem, mem128_t *out);

template< typename DataType >
extern void __fastcall vtlb_memWrite(u32 mem, DataType value);
extern void __fastcall vtlb_memWrite64(u32 mem, const mem64_t* value);
extern void __fastcall vtlb_memWrite128(u32 mem, const mem128_t* value);

extern void vtlb_DynGenWrite(u32 sz);
extern void vtlb_DynGenRead32(u32 bits, bool sign);
extern void vtlb_DynGenRead64(u32 sz);

extern void vtlb_DynGenWrite_Const( u32 bits, u32 addr_const );
extern void vtlb_DynGenRead64_Const( u32 bits, u32 addr_const );
extern void vtlb_DynGenRead32_Const( u32 bits, bool sign, u32 addr_const );

namespace vtlb_private
{
	// Allocate enough memory for both EE and IOP memory space (IOP is roughly 2.5mb,
	// so we alloc 4mb for now -- a little more than is needed).
	static const uint VTLB_ALLOC_SIZE = sizeof(*eeMem) + (_1mb*4);

	static const uint VTLB_PAGE_BITS = 12;
	static const uint VTLB_PAGE_MASK = 4095;
	static const uint VTLB_PAGE_SIZE = 4096;

	static const uint VTLB_PMAP_ITEMS = 0x20000000 / VTLB_PAGE_SIZE;
	static const uint VTLB_PMAP_SZ = 0x20000000;
	static const uint VTLB_VMAP_ITEMS = 0x100000000ULL / VTLB_PAGE_SIZE;

	struct MapData
	{
		u8* alloc_base;			//base of the memory array
		int alloc_current;		//current base

		s32 pmap[VTLB_PMAP_ITEMS];	//512KB
		s32 vmap[VTLB_VMAP_ITEMS];   //4MB

		// first indexer -- 8/16/32/64/128 bit tables [values 0-4]
		// second indexer -- read/write  [0 or 1]
		// third indexer -- 128 possible handlers!
		void* RWFT[5][2][128];
	};

	extern __aligned(64) MapData vtlbdata;
}
