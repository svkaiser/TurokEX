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
// DESCRIPTION: Prediction Movement
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "pred.h"
#include "client.h"
#include "mathlib.h"
#include "level.h"
#include "game.h"

#define MOVE_VELOCITY   2.85f
#define SWIM_VELOCITY   0.05f
#define JUMP_VELOCITY   11.612f
#define NOCLIPMOVE      (MOVE_VELOCITY * 6)

typedef struct
{
    actor_t     actor;
    float       roll;
    float       deltatime;
    pmflags_t   flags;
} localpmove_t;

static localpmove_t lpmove;

//
// Pred_CheckJump
//

static kbool Pred_CheckJump(actor_t *actor)
{
    if(actor->plane == NULL)
    {
        return false;
    }

    if(actor->terriantype == TT_WATER_SURFACE)
    {
        if(actor->origin[1] >
            (Map_GetArea(actor->plane)->waterplane -
            actor->object.centerheight))
        {
            return true;
        }
    }
    else if(Plane_IsAWall(actor->plane) &&
        actor->origin[1] - Plane_GetDistance(actor->plane, actor->origin) <= 8)
    {
        return false;
    }
    else if(!(lpmove.flags & PMF_JUMP))
    {
        if(actor->velocity[1] < 0 && actor->velocity[1] > -16)
        {
            return true;
        }
    }

    if((actor->origin[1] + actor->velocity[1]) -
        Plane_GetDistance(actor->plane, actor->origin) < ONPLANE_EPSILON)
    {
        return true;
    }

    return false;
}

//
// Pred_Walk
//

static void Pred_Walk(actor_t *actor, ticcmd_t *cmd)
{
    float sy;
    float cy;

    sy = (float)sin(actor->yaw);
    cy = (float)cos(actor->yaw);

    if(cmd->buttons & BT_FORWARD)
    {
        actor->velocity[0] += MOVE_VELOCITY * sy;
        actor->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        actor->velocity[0] -= MOVE_VELOCITY * sy;
        actor->velocity[2] -= MOVE_VELOCITY * cy;
    }

    sy = (float)sin(actor->yaw + DEG2RAD(90));
    cy = (float)cos(actor->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        actor->velocity[0] += MOVE_VELOCITY * sy;
        actor->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        actor->velocity[0] -= MOVE_VELOCITY * sy;
        actor->velocity[2] -= MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        if(Pred_CheckJump(actor) && !cmd->heldtime[1])
        {
            lpmove.flags |= PMF_JUMP;
            actor->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// Pred_Swim
//

static void Pred_Swim(actor_t *actor, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float vel;

    if(cmd->heldtime[0] == 0 &&
        Vec_Unit3(actor->velocity) < 3)
    {
        vel = SWIM_VELOCITY * 60;
    }
    else
    {
        vel = SWIM_VELOCITY;
    }

    sy = (float)sin(actor->yaw);
    cy = (float)cos(actor->yaw);
    vsy = (float)sin(actor->pitch);
    vcy = (float)cos(actor->pitch);

    if(cmd->buttons & BT_FORWARD)
    {
        actor->velocity[0] += (vel * sy) * vcy;
        actor->velocity[1] -= (vel * vsy);
        actor->velocity[2] += (vel * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        actor->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        actor->velocity[1] += (SWIM_VELOCITY * vsy);
        actor->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(actor->yaw + DEG2RAD(90));
    cy = (float)cos(actor->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        actor->velocity[0] += SWIM_VELOCITY * sy;
        actor->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        actor->velocity[0] -= SWIM_VELOCITY * sy;
        actor->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        actor->velocity[1] += SWIM_VELOCITY;
    }
}

//
// Pred_Paddle
//

static void Pred_Paddle(actor_t *actor, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float ang;
    float vel;

    sy = (float)sin(actor->yaw);
    cy = (float)cos(actor->yaw);
    vsy = (float)sin(actor->pitch);
    vcy = (float)cos(actor->pitch);

    ang = actor->pitch;

    Ang_Clamp(&ang);
            
    if(ang < DEG2RAD(45))
    {
        vsy = 0;
    }
    else
    {
        vsy *= MOVE_VELOCITY;
    }

    if(cmd->heldtime[0] == 0 &&
        Vec_Unit2(actor->velocity) < 2)
    {
        vel = SWIM_VELOCITY * 80;
    }
    else
    {
        vel = SWIM_VELOCITY;
    }

    if(cmd->buttons & BT_FORWARD)
    {
        actor->velocity[0] += (vel * sy) * vcy;
        actor->velocity[1] -= vsy;
        actor->velocity[2] += (vel * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        actor->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        actor->velocity[1] += vsy;
        actor->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(actor->yaw + DEG2RAD(90));
    cy = (float)cos(actor->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        actor->velocity[0] += SWIM_VELOCITY * sy;
        actor->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        actor->velocity[0] -= SWIM_VELOCITY * sy;
        actor->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        if(Pred_CheckJump(actor) && !cmd->heldtime[1])
        {
            lpmove.flags |= PMF_JUMP;
            actor->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// Pred_NoClipMove
//

static void Pred_NoClipMove(actor_t *actor, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;

    sy = (float)sin(actor->yaw);
    cy = (float)cos(actor->yaw);
    vsy = (float)sin(actor->pitch);
    vcy = (float)cos(actor->pitch);

    x1 = y1 = z1 = x2 = y2 = z2 = 0;

    if(cmd->buttons & BT_FORWARD)
    {
        x1 = (NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * -vsy;
        z1 = (NOCLIPMOVE * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        x1 = -(NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * vsy;
        z1 = -(NOCLIPMOVE * cy) * vcy;
    }

    sy = (float)sin(actor->yaw + DEG2RAD(90));
    cy = (float)cos(actor->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        x2 = NOCLIPMOVE * sy;
        z2 = NOCLIPMOVE * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        x2 = -NOCLIPMOVE * sy;
        z2 = -NOCLIPMOVE * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        y2 = NOCLIPMOVE;
    }

    actor->velocity[0] = x1 + x2;
    actor->velocity[1] = y1 + y2;
    actor->velocity[2] = z1 + z2;
}

//
// Pred_Move
//

void Pred_Move(pred_t *pred)
{
    actor_t *actor;
    object_t *obj;
    ticcmd_t *cmd;

    memset(&lpmove, 0, sizeof(localpmove_t));

    actor = &lpmove.actor;
    obj = &actor->object;
    cmd = &pred->cmd;

    Vec_Set3(actor->origin,
        pred->pmove.origin[0].f,
        pred->pmove.origin[1].f,
        pred->pmove.origin[2].f);

    Vec_Set3(actor->velocity,
        pred->pmove.velocity[0].f,
        pred->pmove.velocity[1].f,
        pred->pmove.velocity[2].f);

    actor->yaw          = pred->pmove.angles[0].f;
    actor->pitch        = pred->pmove.angles[1].f;
    lpmove.roll         = pred->pmove.angles[2].f;
    lpmove.deltatime    = pred->cmd.msec.f;
    lpmove.flags        = pred->pmove.flags;
    obj->width          = pred->pmove.radius.f;
    obj->height         = pred->pmove.height.f;
    obj->centerheight   = pred->pmove.centerheight.f;
    obj->viewheight     = pred->pmove.viewheight.f;
    actor->terriantype  = pred->pmove.terraintype;
    actor->plane        = pred->pmove.plane != -1 ?
        &g_currentmap->planes[pred->pmove.plane] : NULL;

    switch(actor->terriantype)
    {
    case TT_WATER_SHALLOW:
        Pred_Walk(actor, cmd);
        break;

    case TT_WATER_SURFACE:
        Pred_Paddle(actor, cmd);
        break;

    case TT_WATER_UNDER:
        Pred_Swim(actor, cmd);
        break;

    case TT_LAVA:
        Pred_Walk(actor, cmd);
        break;

    case TT_NOCLIP:
        Pred_NoClipMove(actor, cmd);
        break;

    default:
        Pred_Walk(actor, cmd);
        break;
    }

    G_ActorMovement(actor);

    if(actor->flags & AF_ONGROUND)
    {
        lpmove.flags &= ~PMF_JUMP;
        lpmove.flags |= PMF_ONGROUND;
    }

    if(actor->flags & AF_SUBMERGED)
    {
        lpmove.flags |= PMF_SUBMERGED;
    }
    else
    {
        lpmove.flags &= ~PMF_SUBMERGED;
    }

    pred->pmove.angles[0].f     = actor->yaw;
    pred->pmove.angles[1].f     = actor->pitch;
    pred->pmove.origin[0].f     = actor->origin[0];
    pred->pmove.origin[1].f     = actor->origin[1];
    pred->pmove.origin[2].f     = actor->origin[2];
    pred->pmove.velocity[0].f   = actor->velocity[0];
    pred->pmove.velocity[1].f   = actor->velocity[1];
    pred->pmove.velocity[2].f   = actor->velocity[2];
    pred->pmove.flags           = lpmove.flags;
    pred->pmove.terraintype     = actor->terriantype;
    pred->pmove.plane           = (actor->plane - g_currentmap->planes);
}

//
// Pred_TryMovement
//

void Pred_TryMovement(void)
{
    pred_t pred;
    moveframe_t *frame;

    if(client.state != CL_STATE_READY)
    {
        return;
    }

    if(g_currentmap == NULL)
    {
        return;
    }

    memset(&pred, 0, sizeof(pred_t));
    pred.pmove = client.pmove;
    pred.cmd = client.cmd;

    Pred_Move(&pred);

    frame = &client.moveframe;

    client.pmove        = pred.pmove;
    frame->origin[0]    = client.pmove.origin[0].f;
    frame->origin[1]    = client.pmove.origin[1].f;
    frame->origin[2]    = client.pmove.origin[2].f;
    frame->velocity[0]  = client.pmove.velocity[0].f;
    frame->velocity[1]  = client.pmove.velocity[1].f;
    frame->velocity[2]  = client.pmove.velocity[2].f;
    frame->yaw          = client.pmove.angles[0].f;
    frame->pitch        = client.pmove.angles[1].f;
    frame->plane        = &g_currentmap->planes[client.pmove.plane];
}

