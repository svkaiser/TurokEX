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

weapon_t weapons[NUMWEAPONS];

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
// G_SetupWeapon
//

static void G_SetupWeapon(weapon_t *weapon, const char *model,
                          float x, float y, float z)
{
    memset(weapon, 0, sizeof(weapon_t));

    weapon->speed = 4;
    weapon->model = Mdl_Load(model);
    Mdl_SetAnimState(&weapon->animstate, weapon->model, "anim00", false, 0);
    Vec_Set3(weapon->origin,
        -x * 341.334f,
        -y * 341.334f,
         z * 341.334f - 275.456f);
}

//
// G_WeaponThink
//

#define WEAPONTURN_MAX      0.08f
#define WEAPONTURN_EPSILON  0.001f

void G_WeaponThink(actor_t *owner, weapon_t *weapon)
{
    float d;

    weapon->yaw = (weapon->yaw - (client.cmd.mouse[0].f * 0.1f)) * 0.9f;
    if(weapon->yaw >  WEAPONTURN_MAX) weapon->yaw =  WEAPONTURN_MAX;
    if(weapon->yaw < -WEAPONTURN_MAX) weapon->yaw = -WEAPONTURN_MAX;
    if(weapon->yaw <  WEAPONTURN_EPSILON &&
        weapon->yaw > -WEAPONTURN_EPSILON)
    {
        weapon->yaw = 0;
    }

    weapon->pitch = (weapon->pitch - (client.cmd.mouse[1].f * 0.1f)) * 0.9f;
    if(weapon->pitch >  WEAPONTURN_MAX) weapon->pitch =  WEAPONTURN_MAX;
    if(weapon->pitch < -WEAPONTURN_MAX) weapon->pitch = -WEAPONTURN_MAX;
    if(weapon->pitch <  WEAPONTURN_EPSILON &&
        weapon->pitch > -WEAPONTURN_EPSILON)
    {
        weapon->pitch = 0;
    }

    if((d = Vec_Unit2(owner->velocity)) >= 0.1f)
    {
        Mdl_SetAnimState(&weapon->animstate, weapon->model, "anim02", true,
            (float)client.tics + weapon->speed);
    }
    else
    {
        Mdl_SetAnimState(&weapon->animstate, weapon->model, "anim00", true,
            (float)client.tics + weapon->speed);
    }

    Mdl_UpdateAnimState(&weapon->animstate, weapon->speed,
        (float)client.tics + weapon->speed);

    Com_Printf("%f\n", weapon->yaw * M_DEG);
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
// G_ClientThink
//

void G_ClientThink(actor_t *client)
{
    // TODO - TEMP
    if(g_currentmap != NULL)
    {
        G_WeaponThink(client, &weapons[wp_pulse]);
    }
}

//
// FCmd_NoClip
//

static void FCmd_NoClip(void)
{
    actor_t *actor;

    // TODO: TEMP
    actor = &client.localactor;
    if(actor->terriantype == TT_NOCLIP)
    {
        actor->terriantype = TT_NORMAL;
        actor->plane = G_FindClosestPlane(actor->origin);
    }
    else
    {
        actor->terriantype = TT_NOCLIP;
    }
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();

    // TODO - set these up as definitions
    G_SetupWeapon(&weapons[wp_knife],
        "models/mdl653/mdl653.kmesh", 0.5f, 0.45f, 0.78f);
    G_SetupWeapon(&weapons[wp_crossbow],
        "models/mdl644/mdl644.kmesh", 0.39f, 0.44f, 0.77f);
    G_SetupWeapon(&weapons[wp_pistol],
        "models/mdl663/mdl663.kmesh", 0.47f, 0.54f, 0.76f);
    G_SetupWeapon(&weapons[wp_shotgun],
        "models/mdl669/mdl669.kmesh", 0.5f, 0.5f, 0.78f);
    G_SetupWeapon(&weapons[wp_autoshotgun],
        "models/mdl642/mdl642.kmesh", 0.5f, 0.52f, 0.8f);
    G_SetupWeapon(&weapons[wp_rifle],
        "models/mdl665/mdl665.kmesh", 0.5f, 0.6f, 0.75f);
    G_SetupWeapon(&weapons[wp_pulse],
        "models/mdl655/mdl655.kmesh", 0.5f, 0.5f, 0.78f);
    G_SetupWeapon(&weapons[wp_grenade],
        "models/mdl650/mdl650.kmesh", 0.5f, 0.45f, 0.78f);
    G_SetupWeapon(&weapons[wp_missile],
        "models/mdl666/mdl666.kmesh", 0.5f, 0.45f, 0.78f);
    G_SetupWeapon(&weapons[wp_accelerator],
        "models/mdl668/mdl668.kmesh", 0.5f, 0.45f, 0.78f);
    G_SetupWeapon(&weapons[wp_bfg],
        "models/mdl662/mdl662.kmesh", 0.5f, -0.7f, 0.68f);
    G_SetupWeapon(&weapons[wp_chrono],
        "models/mdl645/mdl645.kmesh", 0.6f, 0.17f, 0.85f);

    // TODO: TEMP
    Cmd_AddCommand("noclip", FCmd_NoClip);
}

