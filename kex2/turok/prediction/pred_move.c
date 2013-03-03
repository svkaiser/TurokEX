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
// DESCRIPTION: Client/Server Prediction Movement
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "pred.h"
#include "client.h"
#include "server.h"
#include "mathlib.h"
#include "level.h"
#include "game.h"
#include "js.h"

#define DELTAMOVE(x)    ((x) * move->deltatime)

#define MOVE_VELOCITY       2.85f
#define SWIM_VELOCITY       0.05f
#define JUMP_VELOCITY       11.612f
#define CLIMB_VELOCITY      6.125f
#define NOCLIPMOVE          17.1f

#define FRICTION_GROUND     0.5f
#define FRICTION_LAVA       0.205f
#define FRICTION_WATERMASS  0.019f
#define FRICTION_CLIMB      0.935f

#define GRAVITY_NORMAL      0.62f
#define GRAVITY_WATER       0.005f
#define GRAVITY_FLOAT       0.45f

#define WATERHEIGHT         15.36f
#define SHALLOWHEIGHT       51.2f

move_t movecontroller;

typedef void (*movefunction_t)(move_t*);

//
// Pred_CheckJump
//

static kbool Pred_CheckJump(move_t *move)
{
    if(move->plane == NULL)
    {
        return false;
    }

    // check if we can jump out of the water
    if(move->movetype == MT_WATER_SURFACE)
    {
        if(move->origin[1] >
            (Map_GetArea(move->plane)->waterplane -
            move->center_y))
        {
            return true;
        }
    }
    // cannot jump while standing on something steep
    else if(Plane_IsAWall(move->plane) &&
        move->origin[1] - Plane_GetDistance(move->plane, move->origin) <= 8)
    {
        return false;
    }
    else if(!(move->flags & PMF_JUMP))
    {
        if(move->velocity[1] < 0 && move->velocity[1] > -16)
        {
            // can do a delay jump while falling
            return true;
        }
    }

    // check if standing on ground
    if((move->origin[1] + move->velocity[1]) -
        Plane_GetDistance(move->plane, move->origin) < ONPLANE_EPSILON)
    {
        return true;
    }

    return false;
}

//
// Pred_UpdateMoveType
//

static void Pred_UpdateMoveType(move_t *move)
{
    float dist;
    plane_t *plane;

    move->flags &= ~PMF_SUBMERGED;

    if(move->movetype == MT_NOCLIP)
    {
        // ignore while in noclip mode
        return;
    }

    plane = move->plane;

    if(!plane)
    {
        move->movetype = MT_NORMAL;
        return;
    }

    // still climbing on surface?
    if(move->movetype == MT_CLIMB && plane->flags & CLF_CLIMB)
        return;

    dist = Plane_GetDistance(plane, move->origin);

    if(plane->flags & CLF_WATER)
    {
        float waterheight = Map_GetArea(plane)->waterplane;

        // shallow water
        if(dist - move->origin[1] <= WATERHEIGHT &&
            waterheight - dist <= SHALLOWHEIGHT)
        {
            move->movetype = MT_WATER_SHALLOW;
            return;
        }

        switch(G_CheckWaterLevel(move->origin, move->center_y, plane))
        {
        case WL_BETWEEN:
            if(move->movetype == MT_WATER_UNDER)
                move->velocity[1] = 0;

            move->movetype = MT_WATER_SURFACE;
            return;

        case WL_UNDER:
            move->movetype = MT_WATER_UNDER;
            move->flags |= PMF_SUBMERGED;
            return;

        default:
            break;
        }
    }
    
    // lava
    if(plane->flags & CLF_DAMAGE_LAVA && dist < ONPLANE_EPSILON)
    {
        move->movetype = MT_LAVA;
        return;
    }

    // normal ground
    move->movetype = MT_NORMAL;
}

//
// Pred_UpdatePosition
//

static void Pred_UpdatePosition(move_t *move)
{
    vec3_t position;
    float dist;
    float friction;
    trace_t trace;
    kbool yfriction;

    if(move->plane == NULL)
    {
        if(!(move->plane = Map_FindClosestPlane(move->origin)))
            return;
    }

    // slide against planes and clip velocity
    G_ClipMovement(move->origin, move->velocity, &move->plane,
        move->width, move->center_y, move->yaw, &trace);

    // handle special case for interactive planes
    if(trace.hitpl && trace.type == TRT_INTERACT)
    {
        if(trace.hitpl->flags & CLF_CLIMB)
        {
            // force a deadstop and begin climbing
            move->movetype = MT_CLIMB;
            Vec_Add(move->origin, move->origin, move->velocity);
            Vec_Set3(move->velocity, 0, 0, 0);
            return;
        }
    }

    Vec_Add(position, move->origin, move->velocity);
    friction = FRICTION_GROUND;

    move->flags &= ~PMF_ONGROUND;

    // hit surface and update position/velocity
    if(move->plane)
    {
        plane_t *pl = move->plane;

        if(pl->flags & CLF_CHECKHEIGHT)
        {
            dist = Plane_GetHeight(pl, position) -
                (move->center_y + move->view_y);

            if(position[1] > dist)
            {
                // hit ceiling
                position[1] = dist;
                move->velocity[1] = 0;
            }
        }

        // get floor distance
        dist = position[1] - Plane_GetDistance(pl, position);

        if(dist < ONPLANE_EPSILON)
        {
            position[1] = position[1] - dist;

            // continue sliding if on a slope
            if(!Plane_IsAWall(pl) && move->movetype != MT_WATER_UNDER)
            {
                // surface was hit, kill vertical velocity
                move->velocity[1] = 0;
            }

            move->flags |= PMF_ONGROUND;
        }

        Pred_UpdateMoveType(move);

        yfriction = false;

        //
        // update gravity
        //
        switch(move->movetype)
        {
        case MT_WATER_SHALLOW:
            // normal gravity
            move->velocity[1] -= GRAVITY_NORMAL;
            break;

        case MT_WATER_SURFACE:
            friction = FRICTION_WATERMASS;
            break;

        case MT_WATER_UNDER:
            friction = FRICTION_WATERMASS;
            yfriction = true;
            if(move->velocity[1] > 0.1f)
            {
                area_t *area = Map_GetArea(move->plane);

                // swimming back up to the surface?
                if(position[1] - 2.048f + move->center_y >=
                    area->waterplane -
                    (move->center_y * 0.5f) - 20.48f)
                {
                    vec3_t lerp;

                    // lerp to the surface
                    Vec_Set3(lerp, position[0], area->waterplane + 2.048f, position[2]);
                    Vec_Lerp3(position, 0.05f, position, lerp);

                    if(move->flags & PMF_SUBMERGED)
                        move->flags &= ~PMF_SUBMERGED;
                }
            }
            break;

        case MT_LAVA:
            // slow movement in lava
            friction = FRICTION_LAVA;
            break;

        case MT_NOCLIP:
            move->velocity[1] = move->velocity[1] * friction;

            if(move->velocity[1] < VELOCITY_EPSILON &&
                move->velocity[1] > -VELOCITY_EPSILON)
            {
                move->velocity[1] = 0;
            }
            break;

        case MT_CLIMB:
            {
                vec3_t dir;
                vec3_t snap;

                Vec_Scale(dir, move->plane->normal,
                    Vec_Dot(position, move->plane->normal) -
                    Vec_Dot(move->plane->points[0], move->plane->normal));

                // pull position towards surface
                Vec_Sub(snap, position, dir);
                Vec_Lerp3(position, 0.05f, position, snap);

                move->velocity[0] = -dir[0];
                move->velocity[1] *= FRICTION_CLIMB;
                move->velocity[2] = -dir[2];
            }
            break;

        default:
            // normal gravity
            if(Plane_IsAWall(move->plane) && dist <= 10.24f)
            {
                vec3_t push;

                Vec_Scale(push, move->plane->normal, 10.24f);
                Vec_Sub(move->velocity, move->velocity, push);
            }
            else if(dist >= ONPLANE_EPSILON)
            {
                move->velocity[1] -= GRAVITY_NORMAL;
            }
            break;
        }
    }

    G_ApplyFriction(move->velocity, friction, yfriction);

    // update move to new position
    Vec_Copy3(move->origin, position);
    G_CheckObjectStep(move->origin, move->velocity, move->plane);
}

//
// Pred_ProcessMove
//

void Pred_ProcessMove(move_t *move, float friction, float gravity)
{
    vec3_t position;
    float dist;
    trace_t trace;

    if(move->plane == NULL)
    {
        if(!(move->plane = Map_FindClosestPlane(move->origin)))
            return;
    }

    // slide against planes and clip velocity
    G_ClipMovement(move->origin, move->velocity, &move->plane,
        move->width, move->center_y, move->yaw, &trace);

    Vec_Add(position, move->origin, move->velocity);

    if(move->plane)
    {
        plane_t *pl = move->plane;

        if(pl->flags & CLF_CHECKHEIGHT)
        {
            dist = Plane_GetHeight(pl, position) -
                (move->center_y + move->view_y);

            if(position[1] > dist)
            {
                // hit ceiling
                position[1] = dist;
                move->velocity[1] = 0;
            }
        }

        // get floor distance
        dist = position[1] - Plane_GetDistance(pl, position);

        if(dist < ONPLANE_EPSILON && !(move->plane->flags & CLF_CLIMB))
        {
            position[1] = position[1] - dist;

            if(!Plane_IsAWall(pl))
            {
                // surface was hit, kill vertical velocity
                move->velocity[1] = 0;
            }
        }
    }

    // update gravity and apply friction
    move->velocity[1] -= gravity;

    // update move to new position
    Vec_Copy3(move->origin, position);
}
//
// Pred_SetDirection
//

void Pred_SetDirection(move_t *move, float yaw, float pitch, float roll)
{
    float sy, cy, sp, cp, sr, cr;

    sy = (float)sin(yaw);
    cy = (float)cos(yaw);
    sp = (float)sin(pitch);
    cp = (float)cos(pitch);
    sr = (float)sin(roll);
    cr = (float)cos(roll);

    move->forward[0] = sy * cp;
    move->forward[1] = -sp;
    move->forward[2] = cy * cp;

    move->right[0] = sr * sp * sy + cr * cy;
    move->right[1] = sr * cp;
    move->right[2] = sr * sp * cy + cr * -sy;

    move->up[0] = cr * sp * sy + -sr * cy;
    move->up[1] = cr * cp;
    move->up[2] = cr * sp * cy + -sr * -sy;
}

//
// Pred_Walk
//

static void Pred_Walk(move_t *move)
{
    float fwd = 0;
    float rgt = 0;
    vec3_t forward;
    vec3_t right;

    Pred_SetDirection(move, move->yaw, 0, 0);

    if(move->cmd->buttons[1])   fwd =  MOVE_VELOCITY;
    if(move->cmd->buttons[2])   fwd = -MOVE_VELOCITY;
    if(move->cmd->buttons[5])   rgt =  MOVE_VELOCITY;
    if(move->cmd->buttons[6])   rgt = -MOVE_VELOCITY;

    Vec_Scale(forward, move->forward, fwd);
    Vec_Scale(right, move->right, rgt);
    Vec_Add(move->velocity, move->velocity, forward);
    Vec_Add(move->velocity, move->velocity, right);

    if(move->cmd->buttons[8])
    {
        if(Pred_CheckJump(move) && !move->cmd->heldtime[8])
        {
            move->flags |= PMF_JUMP;
            move->velocity[1] = JUMP_VELOCITY;
        }
    }

    Pred_UpdatePosition(move);

    if(move->flags & PMF_ONGROUND)
        move->flags &= ~PMF_JUMP;
}

//
// Pred_GetWaterSinkHeight
//

static float Pred_GetWaterSinkHeight(move_t *move)
{
    float dist;

    if(move->plane == NULL)
        return 0;

    dist = Plane_GetDistance(move->plane, move->origin);

    if(move->origin[1] - dist <= 0.512f)
        return 0;

    return dist;
}

//
// Pred_Swim
//

static void Pred_Swim(move_t *move)
{
    float ang;
    vec3_t forward;
    vec3_t right;

    ang = move->pitch;

    Ang_Clamp(&ang);
            
    if(move->movetype == MT_WATER_SURFACE && ang < DEG2RAD(45))
        ang = 0;

    Pred_SetDirection(move, move->yaw, ang, 0);

    // lerp forward and right movement speeds
    move->accel[2] = -move->accel[2] * 0.015f + move->accel[2];
    move->accel[0] = -move->accel[0] * 0.015f + move->accel[0];

    // set upward speed to last known velocity if on the surface, otherwise
    // lerp the speed
    if(move->movetype == MT_WATER_SURFACE)
        move->accel[1] = move->velocity[1];
    else
        move->accel[1] = -move->accel[1] * 0.125f + move->accel[1];

    if(move->cmd->buttons[1])
    {
        if(move->cmd->heldtime[1] == 0 &&
            Vec_Unit3(move->velocity) < 3)
        {
            // handle extra thrust
            move->accel[2] = SWIM_VELOCITY * 160;
        }
        else
            move->accel[2] += SWIM_VELOCITY;
    }

    if(move->cmd->buttons[2]) move->accel[2] -= SWIM_VELOCITY;
    if(move->cmd->buttons[5]) move->accel[0] += SWIM_VELOCITY;
    if(move->cmd->buttons[6]) move->accel[0] -= SWIM_VELOCITY;

    if(move->cmd->buttons[8])
    {
        if(move->movetype == MT_WATER_SURFACE)
        {
            // allow jumping while on the surface
            if(Pred_CheckJump(move) && !move->cmd->heldtime[8])
            {
                move->flags |= PMF_JUMP;
                move->accel[1] = JUMP_VELOCITY;
            }
        }
        else
            move->accel[1] += 0.360448f;
    }

    Vec_Scale(forward, move->forward, move->accel[2]);
    Vec_Scale(right, move->right, move->accel[0]);
    Vec_Add(move->velocity, forward, right);

    // apply extra vertical velocity from jump commands
    move->velocity[1] += move->accel[1];

    Pred_UpdatePosition(move);

    if( (move->accel[2] <= 0.5f && move->accel[2] >= -0.5f) &&
        (move->accel[0] <= 0.5f && move->accel[0] >= -0.5f) &&
        (move->accel[1]  <= 0.5f) &&
        move->movetype == MT_WATER_UNDER)
    {
        // sink
        move->accel[1] -= 0.05f;
    }
}

//
// Pred_ClimbMove
//

static void Pred_ClimbMove(move_t *move)
{
    if(move->plane)
    {
        float diff;
        
        diff = Ang_Diff(move->yaw + M_PI, Plane_GetYaw(move->plane));
        Ang_Clamp(&diff);

        // field of view is limited so pull yaw towards the wall
        move->yaw = -diff * 0.084f + move->yaw;
    }

    if(move->cmd->buttons[1])
    {
       if(Vec_Unit3(move->velocity) < 0.5f)
       {
           // climb up and hug the wall
           move->velocity[0] -= (move->plane->normal[0]);
           move->velocity[1] = CLIMB_VELOCITY;
           move->velocity[2] -= (move->plane->normal[2]);
       }
    }

    if(move->cmd->buttons[8])
    {
        if(!move->cmd->heldtime[8])
        {
            move->flags |= PMF_JUMP;

            // jump away from wall
            move->velocity[0] = (move->plane->normal[0]) * 32;
            move->velocity[2] = (move->plane->normal[2]) * 32;
        }
    }

    Pred_UpdatePosition(move);
}

//
// Pred_NoClipMove
//

static void Pred_NoClipMove(move_t *move)
{
    float fwd = 0;
    float rgt = 0;
    vec3_t forward;
    vec3_t right;

    move->velocity[0] = move->velocity[1] = move->velocity[2] = 0;

    Pred_SetDirection(move, move->yaw, move->pitch, 0);

    if(move->cmd->buttons[1])   fwd =  NOCLIPMOVE;
    if(move->cmd->buttons[2])   fwd = -NOCLIPMOVE;
    if(move->cmd->buttons[5])   rgt =  NOCLIPMOVE;
    if(move->cmd->buttons[6])   rgt = -NOCLIPMOVE;

    Vec_Scale(forward, move->forward, fwd);
    Vec_Scale(right, move->right, rgt);
    Vec_Add(move->velocity, move->velocity, forward);
    Vec_Add(move->velocity, move->velocity, right);

    if(move->cmd->buttons[8])
        move->velocity[1] = NOCLIPMOVE;

    Vec_Add(move->origin, move->origin, move->velocity);
}

//
// Pred_Move
//

static const movefunction_t movefuncs[NUMMOVETYPES] =
{
    Pred_Walk,
    Pred_Walk,
    Pred_Swim,
    Pred_Swim,
    Pred_Walk,
    Pred_Walk,
    Pred_NoClipMove,
    Pred_ClimbMove
};

static void Pred_Move(pred_t *pred, kbool local)
{
    move_t *move = &movecontroller;

    memset(move, 0, sizeof(move_t));

    Vec_Copy3(move->origin, pred->pmove.origin);
    Vec_Copy3(move->velocity, pred->pmove.velocity);
    Vec_Copy3(move->accel, pred->pmove.accel);

    move->local     = local;
    move->cmd       = &pred->cmd;
    move->movetime  = pred->pmove.movetime;
    move->yaw       = pred->pmove.angles[0];
    move->pitch     = pred->pmove.angles[1];
    move->roll      = pred->pmove.angles[2];
    move->deltatime = pred->cmd.msec.f;
    move->flags     = pred->pmove.flags;
    move->width     = pred->pmove.radius;
    move->height    = pred->pmove.height;
    move->center_y  = pred->pmove.centerheight;
    move->view_y    = pred->pmove.viewheight;
    move->movetype  = pred->pmove.movetype;
    move->plane     = pred->pmove.plane != -1 ?
        &g_currentmap->planes[pred->pmove.plane] : NULL;

    //J_RunSimulator("userMovement");

    movefuncs[move->movetype](move);

    Vec_Copy3(pred->pmove.origin, move->origin);
    Vec_Copy3(pred->pmove.velocity, move->velocity);
    Vec_Copy3(pred->pmove.accel, move->accel);

    pred->pmove.movetime    = move->movetime;
    pred->pmove.angles[0]   = move->yaw;
    pred->pmove.angles[1]   = move->pitch;
    pred->pmove.angles[2]   = move->roll;
    pred->pmove.flags       = move->flags;
    pred->pmove.movetype    = move->movetype;
    pred->pmove.plane       = (move->plane - g_currentmap->planes);
}

//
// Pred_ClientMovement
//

void Pred_ClientMovement(void)
{
    pred_t pred;
    int current;

    if(client.state != CL_STATE_READY)
        return;

    if(g_currentmap == NULL)
        return;

    memset(&pred, 0, sizeof(pred_t));
    pred.pmove = client.pmove;
    pred.cmd = client.cmd;

    Pred_Move(&pred, true);

    current = (client.ns.outgoing-1) & (NETBACKUPS-1);

    client.pmove = pred.pmove;
    client.oldmoves[current] = client.pmove;
    client.latency[current] = client.time;
}

//
// Pred_ServerMovement
//

void Pred_ServerMovement(void)
{
    /*unsigned int i;

    if(g_currentmap == NULL)
    {
        return;
    }

    for(i = 0; i < server.maxclients; i++)
    {
        pred_t pred;
        svclient_t *svcl;
        actor_t *actor;
        object_t *obj;

        svcl = &svclients[i];

        if(svcl->state != SVC_STATE_INGAME)
            continue;

        svcl->pmove.angles[0] = svcl->cmd.angle[0].f;
        svcl->pmove.angles[1] = svcl->cmd.angle[1].f;

        memset(&pred, 0, sizeof(pred_t));
        pred.pmove = svcl->pmove;
        pred.cmd = svcl->cmd;

        Pred_Move(&pred, false);

        actor = &svcl->gclient.actor;

        svcl->pmove         = pred.pmove;

        Vec_Copy3(actor->origin, svcl->pmove.origin);
        Vec_Copy3(actor->velocity, svcl->pmove.velocity);

        actor->yaw          = svcl->pmove.angles[0];
        actor->pitch        = svcl->pmove.angles[1];
        actor->plane        = &g_currentmap->planes[svcl->pmove.plane];

        obj = &actor->object;

        Vec_SetQuaternion(obj->rotation, actor->yaw, 0, 1, 0);
        Mtx_ApplyRotation(obj->rotation, obj->matrix);

        Mtx_Scale(obj->matrix,
            obj->scale[0],
            obj->scale[1],
            obj->scale[2]);

        Mtx_AddTranslation(obj->matrix,
            actor->origin[0],
            actor->origin[1],
            actor->origin[2]);
    }*/
}

