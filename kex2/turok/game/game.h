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

#ifndef __GAME_H__
#define __GAME_H__

#include "client.h"
#include "actor.h"

#define ONPLANE_EPSILON     0.512f
#define VELOCITY_EPSILON    0.0001f
#define SLOPE_THRESHOLD     25.0f

enum
{
    wp_knife    = 0,
    wp_crossbow,
    wp_pistol,
    wp_shotgun,
    wp_autoshotgun,
    wp_rifle,
    wp_pulse,
    wp_minigun,
    wp_grenade,
    wp_aliengun,
    wp_missile,
    wp_accelerator,
    wp_bfg,
    wp_chrono,
    NUMWEAPONS
};

enum
{
    am_clip     = 0,
    am_shells,
    am_expshells,
    am_cell,
    am_tekarrows,
    am_arrows,
    am_mini,
    am_grenade,
    am_rocket,
    am_fusion,
    am_chrono,
    NUMAMMO
};

typedef enum
{
    WS_DEACTIVATED,
    WS_READY,
    WS_SWAPOUT,
    WS_SWAPIN,
    WS_FIRING,
    WS_HOLDSTER
} wpnstate_t;

typedef struct
{
    vec3_t      origin;
    float       yaw;
    float       pitch;
    kmodel_t    *model;
    animstate_t animstate;
    float       speed;
    anim_t      *idle;
    anim_t      *walk;
    anim_t      *running;
    anim_t      *fire;
    anim_t      *swap_in;
    anim_t      *swap_out;
    wpnstate_t  state;
} weapon_t;

extern weapon_t weapons[NUMWEAPONS];

void G_WeaponThink(void);
void G_InitWeapons(void);

//
// gclient - game-side client controller
// handled mostly on server side and contains
// persistent player data
//
struct gclient_s
{
    kbool       weaponowned[NUMWEAPONS];
    int         activeweapon;
    int         ammo[NUMAMMO];
    int         maxammo[NUMAMMO];
    kbool       hasbackpack;
    kbool       hasarmor;
    int         armorpoints;
    actor_t     actor;
};

enum
{
    WL_INVALID  = 0,
    WL_OVER     = 1,
    WL_BETWEEN  = 2,
    WL_UNDER    = 3
};

void G_CheckObjectStep(vec3_t origin, vec3_t velocity, plane_t *plane);
int G_CheckWaterLevel(vec3_t origin, float centeroffs, plane_t *plane);
void G_ApplyFriction(vec3_t velocity, float friction, kbool effectY);
void G_ClipMovement(vec3_t origin, vec3_t velocity, plane_t **plane,
                    float width, float offset, float yaw, trace_t *t);

void G_Shutdown(void);
void G_Ticker(void);
void G_NoClip(svclient_t *svcl);
void G_SetupPlayer(actor_t *actor);
void G_ClientThink(void);
void G_Init(void);

#endif

