// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#ifndef __KPF_DECODERS__
#define __KPF_DECODERS__

extern const float flt_48EC8C;
extern int off_49CF1C[0x1000];

void DC_BuildAnimTable(int **a1, byte *a2, int a3);
void DC_DecodeData(byte *a1, byte *a2, int a3);
int DC_LookupDataIndex(int *a1, int a2, int a3);
signed int DC_LookupParticleFX(byte *a1, int a2, int id, int *a4, int *a5);

#endif