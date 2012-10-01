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

#include "type.h"
#include "client.h"
#include "actor.h"

plane_t *G_FindClosestPlane(vec3_t coord);
void G_ClampVelocity(vec3_t velocity);
void G_TryMoveActor(actor_t *actor);

void G_Shutdown(void);
void G_Ticker(void);
void G_ClientThink(actor_t *client, ticcmd_t *cmd);
void G_Init(void);

#endif

