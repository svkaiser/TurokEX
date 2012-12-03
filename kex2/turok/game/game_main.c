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
    client->actor->health               = 100;
}

//
// G_SetupPlayer
//

void G_SetupPlayer(actor_t *actor)
{
    unsigned int i;

    // setup local client
    Vec_Copy3(client.moveframe.origin, actor->origin);

    client.pmove.origin[0].f    = actor->origin[0];
    client.pmove.origin[1].f    = actor->origin[1];
    client.pmove.origin[2].f    = actor->origin[2];
    client.pmove.angles[0].f    = actor->yaw;
    client.pmove.angles[1].f    = actor->pitch;
    client.moveframe.yaw        = actor->yaw;
    client.moveframe.pitch      = actor->pitch;
    client.pmove.centerheight.f = actor->object.centerheight;
    client.pmove.viewheight.f   = actor->object.viewheight;
    client.pmove.radius.f       = actor->object.width;
    client.pmove.height.f       = actor->object.height;
    client.pmove.plane          = actor->object.plane_id;

    // setup svclients
    for(i = 0; i < server.maxclients; i++)
    {
        if(svclients[i].state == SVC_STATE_ACTIVE)
        {
            pmove_t *pmove;
            svclient_t *svcl;
            actor_t *p;
            actor_t *prev;
            actor_t *next;

            svcl = &svclients[i];
            svcl->gclient.actor = G_SpawnActor();
            p = svcl->gclient.actor;
            prev = p->prev;
            next = p->next;

            memcpy(p, actor, sizeof(actor_t));
            p->prev = prev;
            p->next = next;

            svcl->state = SVC_STATE_INGAME;
            pmove = &svcl->pmove;

            pmove->origin[0].f      = p->origin[0];
            pmove->origin[1].f      = p->origin[1];
            pmove->origin[2].f      = p->origin[2];
            pmove->angles[0].f      = p->yaw;
            pmove->angles[1].f      = p->pitch;
            pmove->centerheight.f   = p->object.centerheight;
            pmove->viewheight.f     = p->object.viewheight;
            pmove->radius.f         = p->object.width;
            pmove->height.f         = p->object.height;
            pmove->plane            = p->object.plane_id;
        }
    }
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
// G_NoClip
//

void G_NoClip(svclient_t *svcl)
{
    if(svcl->state != SVC_STATE_INGAME || g_currentmap == NULL ||
        svcl->gclient.actor == NULL)
        return;

    if(svcl->pmove.terraintype == TT_NOCLIP)
    {
        plane_t *plane;

        svcl->pmove.terraintype = TT_NORMAL;
        plane = G_FindClosestPlane(svcl->gclient.actor->origin);
        svcl->pmove.plane = plane - g_currentmap->planes;
    }
    else
    {
        svcl->pmove.terraintype = TT_NOCLIP;
    }
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();
    G_InitWeapons();
}

