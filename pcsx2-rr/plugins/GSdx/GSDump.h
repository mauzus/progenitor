/*
 *	Copyright (C) 2007-2009 Gabest
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

#pragma once

#include "GS.h"
#include "GSVertexSW.h"

/*

Dump file format:
- [crc/4] [state size/4] [state data/size] [PMODE/0x2000] [id/1] [data/?] .. [id/1] [data/?]

Transfer data (id == 0)
- [0/1] [path index/1] [size/4] [data/size]

VSync data (id == 1)
- [1/1] [field/1]

ReadFIFO2 data (id == 2)
- [2/1] [size/?]

Regs data (id == 3)
- [PMODE/0x2000]

*/

class GSDump
{
	FILE* m_gs;
	FILE* m_obj;
	int m_frames;
	int m_objects;
	int m_vertices;

public:
	GSDump();
	virtual ~GSDump();

	void Open(const string& fn, uint32 crc, const GSFreezeData& fd, const GSPrivRegSet* regs);
	void Close();
	void ReadFIFO(uint32 size);
	void Transfer(int index, const uint8* mem, size_t size);
	void VSync(int field, bool last, const GSPrivRegSet* regs);
	void Object(GSVertexSW* vertices, int count, GS_PRIM_CLASS primclass);
	operator bool() {return m_gs != NULL;}
};
