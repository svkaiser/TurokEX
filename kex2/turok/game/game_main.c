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
}

//
// G_ClientReborn
//

void G_ClientReborn(gclient_t *client)
{
    memset(client, 0, sizeof(gclient_t));

    client->hasbackpack                 = false;
    client->maxammo[am_clip]            = 100;
    client->maxammo[am_shells]          = 20;
    client->maxammo[am_expshells]       = 10;
    client->maxammo[am_cell]            = 100;
    client->maxammo[am_tekarrows]       = 15;
    client->maxammo[am_arrows]          = 30;
    client->maxammo[am_mini]            = 125;
    client->maxammo[am_grenade]         = 20;
    client->maxammo[am_rocket]          = 6;
    client->maxammo[am_fusion]          = 2;
    client->maxammo[am_chrono]          = 3;
    client->weaponowned[wp_knife]       = true;
    client->weaponowned[wp_crossbow]    = true;
    client->activeweapon                = wp_knife;
    client->actor.health                = 100;
}

//
// G_ClientThink
//

void G_ClientThink(void)
{
    // TODO - TEMP
    G_WeaponThink(&weapons[wp_shotgun]);
}

//
// FCmd_NoClip
//

static void FCmd_NoClip(void)
{
    if(g_currentmap == NULL)
    {
        return;
    }

    // TODO: TEMP
    if(client.pmove.terraintype == TT_NOCLIP)
    {
        moveframe_t *frame = &client.moveframe;

        client.pmove.terraintype = TT_NORMAL;
        frame->plane = G_FindClosestPlane(frame->origin);
        client.pmove.plane = frame->plane - g_currentmap->planes;
    }
    else
    {
        client.pmove.terraintype = TT_NOCLIP;
    }
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();
    G_InitWeapons();

    // TODO: TEMP
    Cmd_AddCommand("noclip", FCmd_NoClip);
}

