// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
// Copyright(C) 2005 Simon Howard
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
//
// DESCRIPTION: Packets for sending across networks (based off of Chocolate Doom)
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "common.h"
#include "zone.h"
#include "kernel.h"

//
// Packet_Read8
//

kbool Packet_Read8(ENetPacket *packet, unsigned int *data)
{
    if(packet->pos + 1 > packet->dataLength)
    {
        return false;
    }

    *data = packet->data[packet->pos];
    packet->pos += 1;

    return true;
}

//
// Packet_Read16
//

kbool Packet_Read16(ENetPacket *packet, unsigned int *data)
{
    byte *p;

    if(packet->pos + 2 > packet->dataLength)
    {
        return false;
    }

    p = packet->data + packet->pos;

    *data = (p[0] << 8) | p[1];
    packet->pos += 2;

    return true;
}

//
// Packet_Read32
//

kbool Packet_Read32(ENetPacket *packet, unsigned int *data)
{
    byte *p;

    if(packet->pos + 4 > packet->dataLength)
    {
        return false;
    }

    p = packet->data + packet->pos;

    *data = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    packet->pos += 4;
    
    return true;
}

//
// Packet_ReadFloat
//

kbool Packet_ReadFloat(ENetPacket *packet, float *data)
{
    fint_t fi;

    if(!Packet_Read32(packet, &fi.i))
        return false;

    *data = fi.f;
    
    return true;
}

//
// Packet_ReadVector
//

kbool Packet_ReadVector(ENetPacket *packet, vec3_t *data)
{
    if(!Packet_ReadFloat(packet, &(*data)[0]))
        return false;

    if(!Packet_ReadFloat(packet, &(*data)[1]))
        return false;

    if(!Packet_ReadFloat(packet, &(*data)[2]))
        return false;
    
    return true;
}

//
// Packet_ReadString
//

char *Packet_ReadString(ENetPacket *packet)
{
    char *start;

    start = (char*)packet->data + packet->pos;

    // Search forward for a NUL character
    while(packet->pos < packet->dataLength && packet->data[packet->pos] != '\0')
    {
        packet->pos++;
    }

    if(packet->pos >= packet->dataLength)
    {
        // Reached the end of the packet
        return NULL;
    }

    // packet->data[packet->pos] == '\0': We have reached a terminating
    // NULL.  Skip past this NULL and continue reading immediately 
    // after it.
    packet->pos++;
    
    return start;
}

//
// Packet_Write8
//

void Packet_Write8(ENetPacket *packet, unsigned int i)
{
    if(packet->dataLength + 1 > packet->dataLength)
    {
        enet_packet_resize(packet, packet->dataLength + 1);
    }

    packet->data[packet->pos] = i;
    packet->pos += 1;
}

//
// Packet_Write16
//

void Packet_Write16(ENetPacket *packet, unsigned int i)
{
    byte *p;

    if(packet->dataLength + 2 > packet->dataLength)
    {
        enet_packet_resize(packet, packet->dataLength + 2);
    }

    p = packet->data + packet->pos;

    p[0] = (i >> 8) & 0xff;
    p[1] = i & 0xff;

    packet->pos += 2;
}

//
// Packet_Write32
//

void Packet_Write32(ENetPacket *packet, unsigned int i)
{
    byte *p;

    if(packet->dataLength + 4 > packet->dataLength)
    {
        enet_packet_resize(packet, packet->dataLength + 4);
    }

    p = packet->data + packet->pos;

    p[0] = (i >> 24) & 0xff;
    p[1] = (i >> 16) & 0xff;
    p[2] = (i >> 8) & 0xff;
    p[3] = i & 0xff;

    packet->pos += 4;
}

//
// Packet_WriteFloat
//

void Packet_WriteFloat(ENetPacket *packet, float i)
{
    fint_t fi;

    fi.f = i;
    Packet_Write32(packet, fi.i);
}

//
// Packet_WriteVector
//

void Packet_WriteVector(ENetPacket *packet, vec3_t vec)
{
    Packet_WriteFloat(packet, vec[0]);
    Packet_WriteFloat(packet, vec[1]);
    Packet_WriteFloat(packet, vec[2]);
}

//
// Packet_WriteString
//

void Packet_WriteString(ENetPacket *packet, char *string)
{
    byte *p;
    unsigned int len;
    char *string2;

    string2 = Z_Strdupa(string);
    len = strlen(string2) + 1;

    // Increase the packet size until large enough to hold the string
    if(packet->dataLength + len > packet->dataLength)
    {
        enet_packet_resize(packet, packet->dataLength + len);
    }

    p = packet->data + packet->pos;

    strcpy((char*)p, string2);
    packet->pos += len;
}

//
// Packet_Send
//

void Packet_Send(ENetPacket *packet, ENetPeer *peer)
{
    packet->pos = 0;
    enet_peer_send(peer, 0, packet);
}

//
// Packet_New
//

ENetPacket *Packet_New(void)
{
    ENetPacket *packet;

    packet = enet_packet_create(NULL, 0, ENET_PACKET_FLAG_RELIABLE);
    return packet;
}
