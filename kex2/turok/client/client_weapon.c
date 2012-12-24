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
// DESCRIPTION: Local weapon animations / movement
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "level.h"
#include "game.h"
#include "packet.h"

weapon_t weapons[NUMWEAPONS];

typedef struct
{
    char *model;
    char *idle;
    char *walk;
    char *run;
    char *attack;
    char *swapin;
    char *swapout;
    float x;
    float y;
    float z;
} weaponinfo_t;

// TODO - set these up as definitions
static const weaponinfo_t weaponinfo[NUMWEAPONS] = 
{
    // knife
    {
        "models/mdl653/mdl653.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim06", "anim07",
        0.5f, 0.45f, 0.78f
    },

    // bow
    {
        "models/mdl644/mdl644.kmesh",
        "anim00", "anim01", "anim02",
        "anim04", "anim05", "anim06",
        0.39f, 0.44f, 0.77f
    },

    // pistol
    {
        "models/mdl663/mdl663.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.47f, 0.54f, 0.76f
    },

    // shotgun
    {
        "models/mdl669/mdl669.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.5f, 0.78f
    },

    // autoshotgun
    {
        "models/mdl642/mdl642.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.52f, 0.8f
    },

    // rifle
    {
        "models/mdl665/mdl665.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.6f, 0.75f
    },

    // pulse rifle
    {
        "models/mdl655/mdl655.kmesh",
        "anim00", "anim01", "anim02",
        "anim05", "anim03", "anim04",
        0.5f, 0.5f, 0.78f
    },

    // minigun
    {
        "models/mdl661/mdl661.kmesh",
        "anim00", "anim01", "anim02",
        "anim05", "anim03", "anim04",
        0.48f, 0.48f, 0.79f
    },

    // grenade launcher
    {
        "models/mdl650/mdl650.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.45f, 0.78f
    },

    // alien gun
    {
        "models/mdl652/mdl652.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.45f, 0.78f
    },

    // missile launcher
    {
        "models/mdl666/mdl666.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, 0.45f, 0.78f
    },

    // particle accelerator
    {
        "models/mdl668/mdl668.kmesh",
        "anim00", "anim01", "anim02",
        "anim04", "anim05", "anim06",
        0.5f, 0.45f, 0.78f
    },

    // fusion cannon
    {
        "models/mdl662/mdl662.kmesh",
        "anim00", "anim01", "anim02",
        "anim03", "anim04", "anim05",
        0.5f, -0.7f, 0.68f
    },

    // chrono
    {
        "models/mdl645/mdl645.kmesh",
        "anim00", "anim01", "anim02",
        "anim04", "anim06", "anim07",
        0.6f, 0.17f, 0.85f
    }
};

//
// CL_ChangeWeapon
//

void CL_ChangeWeapon(ENetPacket *packet)
{
    weapon_t *w;
    int weapon;

    Packet_Read8(packet, &weapon);

    if(weapon < 0 || weapon >= NUMWEAPONS)
        return;

    w = &weapons[client.gt.weapon];
    client.gt.pendingweapon = weapon;

    if(w->animstate.track.anim == w->swap_out)
        return;

    Mdl_SetAnimState(&w->animstate, w->idle,
            w->speed, ANF_LOOP);

    Mdl_BlendAnimStates(&w->animstate,
        w->swap_out, w->speed, 4, ANF_NOINTERRUPT);

    w->state = WS_SWAPOUT;
}

//
// CL_CheckHoldster
//

static kbool CL_CheckHoldster(weapon_t *weapon)
{
    if(client.pmove.movetype == MT_CLIMB && weapon->state != WS_HOLDSTER)
    {
        Mdl_SetAnimState(&weapon->animstate, weapon->idle,
            weapon->speed, ANF_LOOP);

        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->swap_out, weapon->speed, 4, ANF_NOINTERRUPT);

        weapon->state = WS_HOLDSTER;

        return true;
    }

    return false;
}

//
// CL_CheckWeaponChange
//

static kbool CL_CheckWeaponChange(void)
{
    if(client.cmd.buttons[11] && !client.cmd.heldtime[11])
    {
        ENetPacket *packet;

        if(packet = Packet_New())
        {
            Packet_Write8(packet, cp_changeweapon);
            Packet_Write8(packet, true);
            Packet_Write8(packet, true);
            Packet_Send(packet, client.peer);
            return true;
        }
    }

    if(client.cmd.buttons[12] && !client.cmd.heldtime[12])
    {
        ENetPacket *packet;

        if(packet = Packet_New())
        {
            Packet_Write8(packet, cp_changeweapon);
            Packet_Write8(packet, true);
            Packet_Write8(packet, false);
            Packet_Send(packet, client.peer);
            return true;
        }
    }

    return false;
}

//
// CL_WeaponStateReady
//

static void CL_WeaponStateReady(weapon_t *weapon)
{
    float d;

    if(CL_CheckHoldster(weapon))
        return;

    if(client.cmd.buttons[0])
    {
        Mdl_SetAnimState(&weapon->animstate, weapon->idle,
            weapon->speed, ANF_LOOP);

        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->fire, weapon->speed, 4, ANF_NOINTERRUPT);

        weapon->state = WS_FIRING;
        return;
    }

    if(CL_CheckWeaponChange())
        return;

    d = Vec_Unit2(client.moveframe.velocity);

    if(d >= 1.35f)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->running, weapon->speed, 8, ANF_LOOP);
    }
    else if(d >= 0.1f)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->walk, weapon->speed, 8, ANF_LOOP);
    }
    else
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->idle, weapon->speed, 8, ANF_LOOP);
    }
}

//
// CL_WeaponStateFire
//

static void CL_WeaponStateFire(weapon_t *weapon)
{
    if(CL_CheckHoldster(weapon))
    {
        weapon->animstate.flags &= ~ANF_LOOP;
        return;
    }

    if(!(client.cmd.buttons[0]))
        weapon->animstate.flags &= ~ANF_LOOP;

    if(weapon->animstate.flags & ANF_STOPPED)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->idle, weapon->speed, 8, ANF_LOOP);

        weapon->state = WS_READY;
    }
}

//
// CL_WeaponStateHoldster
//

static void CL_WeaponStateHoldster(weapon_t *weapon)
{
    if(client.pmove.movetype != MT_CLIMB)
    {
        Mdl_SetAnimState(&weapon->animstate, weapon->swap_in,
            weapon->speed, ANF_NOINTERRUPT);

        weapon->state = WS_SWAPIN;
    }
}

//
// CL_WeaponStateSwapIn
//

static void CL_WeaponStateSwapIn(weapon_t *weapon)
{
    if(CL_CheckHoldster(weapon))
        return;

    if(CL_CheckWeaponChange())
        return;

    if(weapon->animstate.flags & ANF_STOPPED)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->idle, weapon->speed, 8, ANF_LOOP);

        weapon->state = WS_READY;
    }
}

//
// CL_WeaponStateSwapOut
//

static void CL_WeaponStateSwapOut(weapon_t *weapon)
{
    if(CL_CheckHoldster(weapon))
        return;

    if(CL_CheckWeaponChange())
        return;

    if(weapon->animstate.flags & ANF_STOPPED)
    {
        weapon_t *w;

        client.gt.weapon = client.gt.pendingweapon;
        w = &weapons[client.gt.weapon];

        Mdl_SetAnimState(&w->animstate, w->swap_in,
            w->speed, ANF_NOINTERRUPT);

        w->state = WS_SWAPIN;
    }
}

//
// CL_WeaponThink
//

#define WEAPONTURN_MAX      0.08f
#define WEAPONTURN_EPSILON  0.001f

void CL_WeaponThink(void)
{
    weapon_t *weapon;

    if(g_currentmap == NULL)
    {
        return;
    }

    weapon = &weapons[client.gt.weapon];

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

    switch(weapon->state)
    {
    case WS_READY:
        CL_WeaponStateReady(weapon);
        break;
    case WS_SWAPIN:
        CL_WeaponStateSwapIn(weapon);
        break;
    case WS_SWAPOUT:
        CL_WeaponStateSwapOut(weapon);
        break;
    case WS_FIRING:
        CL_WeaponStateFire(weapon);
        break;
    case WS_HOLDSTER:
        CL_WeaponStateHoldster(weapon);
        break;
    default:
        break;
    }

    if(weapon->state != WS_DEACTIVATED)
        Mdl_UpdateAnimState(&weapon->animstate);
}

//
// FCmd_ChangeWeapon
//

static void FCmd_ChangeWeapon(void)
{
    weapon_t *w;
    int weapon;

    if(Cmd_GetArgc() < 2)
        return;

    weapon = atoi(Cmd_GetArgv(1));

    if(weapon < 0 || weapon >= NUMWEAPONS)
        return;

    w = &weapons[client.gt.weapon];
    client.gt.pendingweapon = weapon;

    if(w->animstate.track.anim == w->swap_out)
        return;

    Mdl_SetAnimState(&w->animstate, w->idle,
            w->speed, ANF_LOOP);

    Mdl_BlendAnimStates(&w->animstate,
        w->swap_out, w->speed, 4, ANF_NOINTERRUPT);

    w->state = WS_SWAPOUT;
}

//
// CL_SetupWeapon
//

void CL_InitWeapons(void)
{
    weapon_t *weapon;
    int i;

    for(i = 0; i < NUMWEAPONS; i++)
    {
        weapon = &weapons[i];

        memset(weapon, 0, sizeof(weapon_t));

        weapon->speed       = 4;
        weapon->model       = Mdl_Load(weaponinfo[i].model);
        weapon->idle        = Mdl_GetAnim(weapon->model, weaponinfo[i].idle);
        weapon->walk        = Mdl_GetAnim(weapon->model, weaponinfo[i].walk);
        weapon->running     = Mdl_GetAnim(weapon->model, weaponinfo[i].run);
        weapon->fire        = Mdl_GetAnim(weapon->model, weaponinfo[i].attack);
        weapon->swap_in     = Mdl_GetAnim(weapon->model, weaponinfo[i].swapin);
        weapon->swap_out    = Mdl_GetAnim(weapon->model, weaponinfo[i].swapout);
        weapon->state       = WS_READY;

        Mdl_SetAnimState(&weapon->animstate, weapon->idle,
            weapon->speed, ANF_LOOP);
        Vec_Set3(weapon->origin,
            -weaponinfo[i].x * 341.334f,
            -weaponinfo[i].y * 341.334f,
             weaponinfo[i].z * 341.334f - 275.456f);
    }

    Cmd_AddCommand("setweapon", FCmd_ChangeWeapon);
}

