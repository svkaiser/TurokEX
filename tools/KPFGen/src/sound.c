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

#include "types.h"
#include "common.h"
#include "pak.h"

typedef struct
{
    int offset;
    int size;
} sndstruct_t;

void SND_StoreSounds(void)
{
    int numsounds;
    sndstruct_t *snds;
    byte *buffer;
    byte **sndnames;
    int i;

    Com_GetCartFile(
        "PC Turok Sound File (.11k) \0*.11k\0All Files (*.*)\0*.*\0",
        "Locate TUROKSND.11k");

    if(cartfile == NULL)
        return;

    PK_AddFolder("sounds/");

    numsounds = Com_GetCartOffset(cartfile, 0, 0);

    if(numsounds < 1)
    {
        Com_CloseCartFile();
        return;
    }

    snds = (sndstruct_t*)(cartfile + 8);
    buffer = ((cartfile + 8) + sizeof(sndstruct_t) * numsounds + 4);

    sndnames = (byte**)Com_Alloc(sizeof(byte*) * numsounds);

    for(i = 0; i < numsounds; i++)
    {
        sndnames[i] = buffer;
        buffer += strlen(sndnames[i]) + 1;
    }

    for(i = 0; i < numsounds; i++)
    {
        char name[256];

        sprintf(name, "sounds/%s", sndnames[i]);
        PK_AddFile(name, buffer + snds[i].offset, snds[i].size, false);
    }

    Com_CloseCartFile();
}
