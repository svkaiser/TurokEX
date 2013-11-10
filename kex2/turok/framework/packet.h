// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _PACKET_H_
#define _PACKET_H_

#include "enet/enet.h"

class kexPacketManager {
public:
                kexPacketManager(void);
                ~kexPacketManager(void);

    bool        Read8(ENetPacket *packet, unsigned int *data);
    bool        Read16(ENetPacket *packet, unsigned int *data);
    bool        Read32(ENetPacket *packet, unsigned int *data);
    bool        ReadFloat(ENetPacket *packet, float *data);
    char        *ReadString(ENetPacket *packet);
    void        Write8(ENetPacket *packet, unsigned int i);
    void        Write16(ENetPacket *packet, unsigned int i);
    void        Write32(ENetPacket *packet, unsigned int i);
    void        WriteFloat(ENetPacket *packet, float i);
    void        WriteString(ENetPacket *packet, char *string);
    void        Send(ENetPacket *packet, ENetPeer *peer);
    ENetPacket  *Create(void);
};

extern kexPacketManager packetManager;

#endif