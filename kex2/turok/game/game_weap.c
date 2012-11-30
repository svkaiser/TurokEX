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
// DESCRIPTION: Main weapon system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "level.h"
#include "game.h"

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
// G_SetupWeapon
//

void G_InitWeapons(void)
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

        Mdl_SetAnimState(&weapon->animstate, weapon->idle,
            weapon->speed, ANF_LOOP);
        Vec_Set3(weapon->origin,
            -weaponinfo[i].x * 341.334f,
            -weaponinfo[i].y * 341.334f,
             weaponinfo[i].z * 341.334f - 275.456f);
    }
}

//
// G_WeaponThink
//

#define WEAPONTURN_MAX      0.08f
#define WEAPONTURN_EPSILON  0.001f

void G_WeaponThink(weapon_t *weapon)
{
    float d;

    if(g_currentmap == NULL)
    {
        return;
    }

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

    if(weapon->animstate.flags & ANF_STOPPED)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->idle, weapon->speed, 8, ANF_LOOP);
    }

    if(client.cmd.buttons & BT_ATTACK)
    {
        Mdl_BlendAnimStates(&weapon->animstate,
            weapon->fire, weapon->speed, 4, ANF_NOINTERRUPT);
    }
    else
    {
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

    Mdl_UpdateAnimState(&weapon->animstate);
}

