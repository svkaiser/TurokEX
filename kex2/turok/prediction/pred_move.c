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

#define DELTAMOVE(x)    ((x) * move->deltatime)

#define MOVE_VELOCITY       2.85f
#define SWIM_VELOCITY       0.05f
#define JUMP_VELOCITY       11.612f
#define CLIMB_VELOCITY      6.125f
#define NOCLIPMOVE          17.1f

#define FRICTION_GROUND     0.5f
#define FRICTION_LAVA       0.205f
#define FRICTION_WATERMASS  0.025f
#define FRICTION_WTRIMPACT  0.095f
#define FRICTION_CLIMB      0.935f

#define GRAVITY_NORMAL      0.62f
#define GRAVITY_WATER       0.005f
#define GRAVITY_FLOAT       0.45f

#define WATERHEIGHT         15.36f
#define SHALLOWHEIGHT       51.2f

typedef struct
{
    vec3_t      origin;
    vec3_t      velocity;
    float       width;
    float       height;
    float       center_y;
    float       view_y;
    float       yaw;
    float       pitch;
    float       roll;
    float       deltatime;
    int         movetype;
    plane_t     *plane;
    pmflags_t   flags;
    ticcmd_t    *cmd;
} move_t;

static move_t move;

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
            vec3_t lerp;

            // lerp player back to the surface
            Vec_Set3(lerp, position[0], position[1] - dist, position[2]);
            Vec_Lerp3(position, 0.125f, position, lerp);

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
                    {
                        move->flags &= ~PMF_SUBMERGED;
                    }
                }
            }
            else if(Vec_Unit2(move->velocity) < FRICTION_WATERMASS)
            {
                // sink
                move->velocity[1] -= GRAVITY_WATER;
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
// Pred_Walk
//

static void Pred_Walk(move_t *move)
{
    float sy;
    float cy;

    sy = (float)sin(move->yaw);
    cy = (float)cos(move->yaw);

    if(move->cmd->buttons & BT_FORWARD)
    {
        move->velocity[0] += MOVE_VELOCITY * sy;
        move->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_BACKWARD)
    {
        move->velocity[0] -= MOVE_VELOCITY * sy;
        move->velocity[2] -= MOVE_VELOCITY * cy;
    }

    sy = (float)sin(move->yaw + DEG2RAD(90));
    cy = (float)cos(move->yaw + DEG2RAD(90));

    if(move->cmd->buttons & BT_STRAFELEFT)
    {
        move->velocity[0] += MOVE_VELOCITY * sy;
        move->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_STRAFERIGHT)
    {
        move->velocity[0] -= MOVE_VELOCITY * sy;
        move->velocity[2] -= MOVE_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_JUMP)
    {
        if(Pred_CheckJump(move) && !move->cmd->heldtime[1])
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
// Pred_Swim
//

static void Pred_Swim(move_t *move)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float vel;

    sy = (float)sin(move->yaw);
    cy = (float)cos(move->yaw);
    vsy = (float)sin(move->pitch);
    vcy = (float)cos(move->pitch);

    if(move->cmd->buttons & BT_FORWARD)
    {
        if(move->cmd->heldtime[0] == 0 &&
            Vec_Unit3(move->velocity) < 3)
        {
            vel = SWIM_VELOCITY * 60;
        }
        else
            vel = SWIM_VELOCITY;

        move->velocity[0] += (vel * sy) * vcy;
        move->velocity[1] -= (vel * vsy);
        move->velocity[2] += (vel * cy) * vcy;
    }

    if(move->cmd->buttons & BT_BACKWARD)
    {
        move->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        move->velocity[1] += (SWIM_VELOCITY * vsy);
        move->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(move->yaw + DEG2RAD(90));
    cy = (float)cos(move->yaw + DEG2RAD(90));

    if(move->cmd->buttons & BT_STRAFELEFT)
    {
        move->velocity[0] += SWIM_VELOCITY * sy;
        move->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_STRAFERIGHT)
    {
        move->velocity[0] -= SWIM_VELOCITY * sy;
        move->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_JUMP)
    {
        move->velocity[1] += SWIM_VELOCITY;
    }

    Pred_UpdatePosition(move);
}

//
// Pred_Paddle
//

static void Pred_Paddle(move_t *move)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float ang;
    float vel;

    sy = (float)sin(move->yaw);
    cy = (float)cos(move->yaw);
    vsy = (float)sin(move->pitch);
    vcy = (float)cos(move->pitch);

    ang = move->pitch;

    Ang_Clamp(&ang);
            
    if(ang < DEG2RAD(45))
    {
        vsy = 0;
    }
    else
    {
        vsy *= MOVE_VELOCITY;
    }

    if(move->cmd->heldtime[0] == 0 &&
        Vec_Unit2(move->velocity) < 2)
    {
        vel = SWIM_VELOCITY * 80;
    }
    else
    {
        vel = SWIM_VELOCITY;
    }

    if(move->cmd->buttons & BT_FORWARD)
    {
        move->velocity[0] += (vel * sy) * vcy;
        move->velocity[1] -= vsy;
        move->velocity[2] += (vel * cy) * vcy;
    }

    if(move->cmd->buttons & BT_BACKWARD)
    {
        move->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        move->velocity[1] += vsy;
        move->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(move->yaw + DEG2RAD(90));
    cy = (float)cos(move->yaw + DEG2RAD(90));

    if(move->cmd->buttons & BT_STRAFELEFT)
    {
        move->velocity[0] += SWIM_VELOCITY * sy;
        move->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_STRAFERIGHT)
    {
        move->velocity[0] -= SWIM_VELOCITY * sy;
        move->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(move->cmd->buttons & BT_JUMP)
    {
        if(Pred_CheckJump(move) && !move->cmd->heldtime[1])
        {
            move->flags |= PMF_JUMP;
            move->velocity[1] = JUMP_VELOCITY;
        }
    }

    Pred_UpdatePosition(move);
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

    if(move->cmd->buttons & BT_FORWARD)
    {
       if(Vec_Unit3(move->velocity) < 0.5f)
       {
           // climb up and hug the wall
           move->velocity[0] -= (move->plane->normal[0]);
           move->velocity[1] = CLIMB_VELOCITY;
           move->velocity[2] -= (move->plane->normal[2]);
       }
    }

    if(move->cmd->buttons & BT_JUMP)
    {
        if(!move->cmd->heldtime[1])
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

    sy = (float)sin(move->yaw);
    cy = (float)cos(move->yaw);
    vsy = (float)sin(move->pitch);
    vcy = (float)cos(move->pitch);

    x1 = y1 = z1 = x2 = y2 = z2 = 0;

    if(move->cmd->buttons & BT_FORWARD)
    {
        x1 = (NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * -vsy;
        z1 = (NOCLIPMOVE * cy) * vcy;
    }

    if(move->cmd->buttons & BT_BACKWARD)
    {
        x1 = -(NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * vsy;
        z1 = -(NOCLIPMOVE * cy) * vcy;
    }

    sy = (float)sin(move->yaw + DEG2RAD(90));
    cy = (float)cos(move->yaw + DEG2RAD(90));

    if(move->cmd->buttons & BT_STRAFELEFT)
    {
        x2 = NOCLIPMOVE * sy;
        z2 = NOCLIPMOVE * cy;
    }

    if(move->cmd->buttons & BT_STRAFERIGHT)
    {
        x2 = -NOCLIPMOVE * sy;
        z2 = -NOCLIPMOVE * cy;
    }

    if(move->cmd->buttons & BT_JUMP)
    {
        y2 = NOCLIPMOVE;
    }

    move->velocity[0] = x1 + x2;
    move->velocity[1] = y1 + y2;
    move->velocity[2] = z1 + z2;

    Vec_Add(move->origin, move->origin, move->velocity);
}

//
// Pred_Move
//

static const movefunction_t movefuncs[NUMMOVETYPES] =
{
    Pred_Walk,
    Pred_Walk,
    Pred_Paddle,
    Pred_Swim,
    Pred_Walk,
    Pred_Walk,
    Pred_NoClipMove,
    Pred_ClimbMove
};

void Pred_Move(pred_t *pred)
{
    memset(&move, 0, sizeof(move_t));

    Vec_Copy3(move.origin, pred->pmove.origin);
    Vec_Copy3(move.velocity, pred->pmove.velocity);

    move.cmd        = &pred->cmd;
    move.yaw        = pred->pmove.angles[0];
    move.pitch      = pred->pmove.angles[1];
    move.roll       = pred->pmove.angles[2];
    move.deltatime  = pred->cmd.msec.f;
    move.flags      = pred->pmove.flags;
    move.width      = pred->pmove.radius;
    move.height     = pred->pmove.height;
    move.center_y   = pred->pmove.centerheight;
    move.view_y     = pred->pmove.viewheight;
    move.movetype   = pred->pmove.movetype;
    move.plane      = pred->pmove.plane != -1 ?
        &g_currentmap->planes[pred->pmove.plane] : NULL;

    movefuncs[move.movetype](&move);

    Vec_Copy3(pred->pmove.origin, move.origin);
    Vec_Copy3(pred->pmove.velocity, move.velocity);

    pred->pmove.angles[0]   = move.yaw;
    pred->pmove.angles[1]   = move.pitch;
    pred->pmove.flags       = move.flags;
    pred->pmove.movetype    = move.movetype;
    pred->pmove.plane       = (move.plane - g_currentmap->planes);
}

//
// Pred_ClientMovement
//

void Pred_ClientMovement(void)
{
    pred_t pred;

    if(client.state != CL_STATE_READY)
        return;

    if(g_currentmap == NULL)
        return;

    memset(&pred, 0, sizeof(pred_t));
    pred.pmove = client.pmove;
    pred.cmd = client.cmd;

    Pred_Move(&pred);

    client.pmove = pred.pmove;
}

//
// Pred_ServerMovement
//

void Pred_ServerMovement(void)
{
    unsigned int i;

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

        Pred_Move(&pred);

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
    }
}

