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
// DESCRIPTION: Packet manager that handles reading and writing of ENet packets
//
//-----------------------------------------------------------------------------

#include "enet/enet.h"
#include "common.h"
#include "zone.h"
#include "system.h"
#include "packet.h"

kexPacketManager packetManager;

//
// kexPacketManager::kexPacketManager
//

kexPacketManager::kexPacketManager(void) {
}

//
// kexPacketManager::~kexPacketManager
//

kexPacketManager::~kexPacketManager(void) {
}

//
// kexPacketManager::Read8
//

bool kexPacketManager::Read8(ENetPacket *packet, unsigned int *data) {
    if(packet->pos + 1 > packet->dataLength) {
        return false;
    }
    *data = packet->data[packet->pos];
    packet->pos += 1;
    return true;
}

//
// kexPacketManager::Read16
//

bool kexPacketManager::Read16(ENetPacket *packet, unsigned int *data) {
    if(packet->pos + 2 > packet->dataLength) {
        return false;
    }
    byte *p = packet->data + packet->pos;
    *data = (p[0] << 8) | p[1];
    packet->pos += 2;
    return true;
}

//
// kexPacketManager::Read32
//

bool kexPacketManager::Read32(ENetPacket *packet, unsigned int *data) {
    if(packet->pos + 4 > packet->dataLength) {
        return false;
    }
    byte *p = packet->data + packet->pos;
    *data = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    packet->pos += 4;
    return true;
}

//
// kexPacketManager::ReadFloat
//

bool kexPacketManager::ReadFloat(ENetPacket *packet, float *data) {
    fint_t fi;
    if(!Read32(packet, (unsigned int*)&fi.i))
        return false;
    *data = fi.f;
    return true;
}

//
// kexPacketManager::ReadVector
//

bool kexPacketManager::ReadVector(ENetPacket *packet, vec3_t *data) {
    if(!ReadFloat(packet, &(*data)[0]))
        return false;
    if(!ReadFloat(packet, &(*data)[1]))
        return false;
    if(!ReadFloat(packet, &(*data)[2]))
        return false;
    return true;
}

//
// kexPacketManager::ReadString
//

char *kexPacketManager::ReadString(ENetPacket *packet) {
    char *start = (char*)packet->data + packet->pos;
    // Search forward for a NUL character
    while(packet->pos < packet->dataLength && packet->data[packet->pos] != '\0') {
        packet->pos++;
    }
    if(packet->pos >= packet->dataLength) {
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
// kexPacketManager::Write8
//

void kexPacketManager::Write8(ENetPacket *packet, unsigned int i) {
    if(packet->dataLength + 1 > packet->dataLength) {
        enet_packet_resize(packet, packet->dataLength + 1);
    }
    packet->data[packet->pos] = i;
    packet->pos += 1;
}

//
// kexPacketManager::Write16
//

void kexPacketManager::Write16(ENetPacket *packet, unsigned int i) {
    if(packet->dataLength + 2 > packet->dataLength) {
        enet_packet_resize(packet, packet->dataLength + 2);
    }
    byte *p = packet->data + packet->pos;
    p[0] = (i >> 8) & 0xff;
    p[1] = i & 0xff;
    packet->pos += 2;
}

//
// kexPacketManager::Write32
//

void kexPacketManager::Write32(ENetPacket *packet, unsigned int i) {
    if(packet->dataLength + 4 > packet->dataLength) {
        enet_packet_resize(packet, packet->dataLength + 4);
    }
    byte *p = packet->data + packet->pos;
    p[0] = (i >> 24) & 0xff;
    p[1] = (i >> 16) & 0xff;
    p[2] = (i >> 8) & 0xff;
    p[3] = i & 0xff;
    packet->pos += 4;
}

//
// kexPacketManager::WriteFloat
//

void kexPacketManager::WriteFloat(ENetPacket *packet, float i) {
    fint_t fi;
    fi.f = i;
    Write32(packet, fi.i);
}

//
// kexPacketManager::WriteVector
//

void kexPacketManager::WriteVector(ENetPacket *packet, vec3_t vec) {
    WriteFloat(packet, vec[0]);
    WriteFloat(packet, vec[1]);
    WriteFloat(packet, vec[2]);
}

//
// kexPacketManager::WriteString
//

void kexPacketManager::WriteString(ENetPacket *packet, char *string) {
    char *string2 = Z_Strdupa(string);
    unsigned int len = strlen(string2) + 1;
    // Increase the packet size until large enough to hold the string
    if(packet->dataLength + len > packet->dataLength) {
        enet_packet_resize(packet, packet->dataLength + len);
    }
    byte *p = packet->data + packet->pos;
    strcpy((char*)p, string2);
    packet->pos += len;
}

//
// kexPacketManager::Send
//

void kexPacketManager::Send(ENetPacket *packet, ENetPeer *peer) {
    packet->pos = 0;
    enet_peer_send(peer, 0, packet);
}

//
// kexPacketManager::Create
//

ENetPacket *kexPacketManager::Create(void) {
    return enet_packet_create(NULL, 0, ENET_PACKET_FLAG_RELIABLE);
}
