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
//
// DESCRIPTION: Base Network System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "network.h"

//
// kexNetwork::Destroy
//

void kexNetwork::Destroy(void) {
    if(host)
        enet_host_destroy(host);
}

//
// kexNetwork::Shutdown
//

void kexNetwork::Shutdown(void) {
    Destroy();
    enet_deinitialize();
}

//
// kexNetwork::CheckMessages
//

void kexNetwork::CheckMessages(void) {
    while(enet_host_service(host, &netEvent, 0) > 0)
    {
        switch(netEvent.type) {
        case ENET_EVENT_TYPE_CONNECT:
            OnConnect();
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            OnDisconnect();
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            OnRecieve();
            break;
        }
    }
}

//
// kexNetwork::OnRecieve
//

void kexNetwork::OnRecieve(void) {
    ProcessPackets(netEvent.packet);
}
