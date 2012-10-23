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
#include "level.h"
#include "zone.h"

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
// G_ClientThink
//

#define MOVE_VELOCITY   2.85f
#define SWIM_VELOCITY   0.25f
#define JUMP_VELOCITY   11.612f

void G_ClientThink(actor_t *client, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float velocity;

    sy = (float)sin(client->yaw);
    cy = (float)cos(client->yaw);

    velocity = MOVE_VELOCITY;

    if(G_ActorOnWaterSurface(client))
    {
        vsy = (float)sin(client->pitch);
        vcy = (float)cos(client->pitch);

        velocity = SWIM_VELOCITY;

        if(!G_ActorInWaterArea(client))
        {
            float ang = client->pitch;

            Ang_Clamp(&ang);
            
            if(ang < (45 * M_RAD) &&
                ang > -(45 * M_RAD))
            {
                vsy = 0;
            }
            else
            {
                vsy *= MOVE_VELOCITY;
            }
        }
        else
        {
            vsy *= SWIM_VELOCITY;
        }
    }
    else
    {
        vsy = 0;
        vcy = 1;
    }

    if(cmd->buttons & BT_FORWARD)
    {
        client->velocity[0] += (velocity * sy) * vcy;
        client->velocity[1] -= vsy;
        client->velocity[2] += (velocity * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        client->velocity[0] -= (velocity * sy) * vcy;
        client->velocity[1] += vsy;
        client->velocity[2] -= (velocity * cy) * vcy;
    }

    sy = (float)sin(client->yaw + (90.0f * M_RAD));
    cy = (float)cos(client->yaw + (90.0f * M_RAD));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        client->velocity[0] += velocity * sy;
        client->velocity[2] += velocity * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        client->velocity[0] -= velocity * sy;
        client->velocity[2] -= velocity * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        if(G_ActorOnWaterSurface(client))
        {
            client->velocity[1] += SWIM_VELOCITY;
        }
        else if(G_ActorOnPlane(client))
        {
            client->velocity[1] = JUMP_VELOCITY;
        }
    }

    // TEMP
    G_ActorMovement(client);
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();
}

