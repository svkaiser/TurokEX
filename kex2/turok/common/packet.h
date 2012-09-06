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

kbool Packet_Read8(ENetPacket *packet, unsigned int *data);
kbool Packet_Read16(ENetPacket *packet, unsigned int *data);
kbool Packet_Read32(ENetPacket *packet, unsigned int *data);
char *Packet_ReadString(ENetPacket *packet);
void Packet_Write8(ENetPacket *packet, unsigned int i);
void Packet_Write16(ENetPacket *packet, unsigned int i);
void Packet_Write32(ENetPacket *packet, unsigned int i);
void Packet_WriteString(ENetPacket *packet, char *string);
void Packet_Send(ENetPacket *packet, ENetPeer *peer);
ENetPacket *Packet_New(void);

#endif