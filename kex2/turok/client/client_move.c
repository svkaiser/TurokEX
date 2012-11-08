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
// DESCRIPTION: Client Movement
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "mathlib.h"
#include "level.h"
#include "game.h"

#define MOVE_VELOCITY   2.85f
#define SWIM_VELOCITY   0.05f
#define JUMP_VELOCITY   11.612f
#define NOCLIPMOVE      (MOVE_VELOCITY * 6)

//
// CL_CheckJump
//

static kbool CL_CheckJump(actor_t *actor)
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
    else if(!(actor->flags & AF_CLIENTJUMP))
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
// CL_ClientWalk
//

static void CL_ClientWalk(actor_t *actor, ticcmd_t *cmd)
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
        if(CL_CheckJump(actor) && !cmd->heldtime[1])
        {
            actor->flags |= AF_CLIENTJUMP;
            actor->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// CL_ClientSwim
//

static void CL_ClientSwim(actor_t *actor, ticcmd_t *cmd)
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
// CL_ClientPaddle
//

static void CL_ClientPaddle(actor_t *actor, ticcmd_t *cmd)
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
        if(CL_CheckJump(actor) && !cmd->heldtime[1])
        {
            actor->flags |= AF_CLIENTJUMP;
            actor->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// CL_ClientNoClipMove
//

static void CL_ClientNoClipMove(actor_t *actor, ticcmd_t *cmd)
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
// CL_Move
//

void CL_Move(client_t *client)
{
    actor_t *actor;
    ticcmd_t *cmd;

    actor = &client->localactor;
    cmd = &client->cmd;

    switch(actor->terriantype)
    {
    case TT_WATER_SHALLOW:
        CL_ClientWalk(actor, cmd);
        break;

    case TT_WATER_SURFACE:
        CL_ClientPaddle(actor, cmd);
        break;

    case TT_WATER_UNDER:
        CL_ClientSwim(actor, cmd);
        break;

    case TT_LAVA:
        CL_ClientWalk(actor, cmd);
        break;

    case TT_NOCLIP:
        CL_ClientNoClipMove(actor, cmd);
        break;

    default:
        CL_ClientWalk(actor, cmd);
        break;
    }

    // TODO - TEMP
    G_ActorMovement(actor);
}

