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
// DESCRIPTION: Main game code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "server.h"
#include "kernel.h"
#include "mathlib.h"
#include "level.h"
#include "zone.h"
#include "game.h"
#include "packet.h"
#include "js.h"

//
// G_Shutdown
//

void G_Shutdown(void)
{
    Z_FreeTags(PU_LEVEL, PU_LEVEL);
    Z_FreeTags(PU_ACTOR, PU_ACTOR);
}

//
// G_Ticker
//

void G_Ticker(void)
{
    if(g_currentmap != NULL)
    {
        g_currentmap->tics++;
        g_currentmap->time = (float)g_currentmap->tics * 0.1f;
    }

    Actor_ComponentFunc("onTick");
}

//
// G_SwitchWeapon
//

void G_SwitchWeapon(ENetEvent *sev, ENetPacket *packet)
{
    svclient_t *svcl;
    gclient_t *gc;
    kbool cycle;
    int wpn_id;

    svcl = &svclients[SV_GetPlayerID(sev->peer)];
    gc = &svcl->gclient;
    wpn_id = gc->activeweapon;

    Packet_Read8(packet, &cycle);

    if(cycle == true)
    {
        kbool cycle_next;

        Packet_Read8(packet, &cycle_next);

        if(cycle_next == true)
        {
            int weapon = gc->activeweapon + 1;

            if(weapon >= NUMWEAPONS)
                weapon = 0;

            while(weapon != gc->activeweapon)
            {
                if(gc->weaponowned[weapon])
                {
                    gc->activeweapon = weapon;
                    break;
                }

                if(++weapon >= NUMWEAPONS)
                    weapon = 0;
            }
        }
        else
        {
            int weapon = gc->activeweapon - 1;

            if(weapon < 0)
                weapon = (NUMWEAPONS - 1);

            while(weapon != gc->activeweapon)
            {
                if(gc->weaponowned[weapon])
                {
                    gc->activeweapon = weapon;
                    break;
                }

                if(--weapon < 0)
                    weapon = (NUMWEAPONS - 1);
            }
        }
    }
    else
    {
    }

    if(wpn_id != gc->activeweapon)
        SV_SendWeaponInfo(svcl);
}

//
// G_ClientThink
//

void G_ClientThink(void)
{
    Actor_ComponentFunc("onLocalTick");
}

//
// G_NoClip
//

void G_NoClip(svclient_t *svcl)
{
    /*if(svcl->state != SVC_STATE_INGAME || g_currentmap == NULL)
        return;

    if(svcl->pmove.movetype == MT_NOCLIP)
    {
        plane_t *plane;

        svcl->pmove.movetype = MT_NORMAL;
        plane = Map_FindClosestPlane(svcl->gclient.actor.origin);
        svcl->pmove.plane = plane - g_currentmap->planes;
        svcl->gclient.actor.plane = plane;
    }
    else
    {
        svcl->pmove.movetype = MT_NOCLIP;
    }*/
}

//
// G_GiveAll
//

void G_GiveAll(svclient_t *svcl)
{
    int i;

    if(svcl->state != SVC_STATE_INGAME || g_currentmap == NULL)
        return;

    for(i = 0; i < NUMWEAPONS; i++)
        svcl->gclient.weaponowned[i] = true;
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();
    CL_InitWeapons();
}

