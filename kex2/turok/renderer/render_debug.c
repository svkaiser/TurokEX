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
// DESCRIPTION: Debug Renderer Utilities
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "gl.h"
#include "render.h"
#include "mathlib.h"
#include "client.h"
#include "level.h"

kbool R_FrustrumTestBox(bbox_t box);
kbool R_FrustrumTestPlane(plane_t *plane);

//
// R_DrawCollision
//

void R_DrawCollision(void)
{
    unsigned int i;

    if(g_currentmap == NULL)
    {
        return;
    }

    dglEnable(GL_ALPHA_TEST);
    dglDisable(GL_FOG);

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    for(i = 0; i < g_currentmap->numplanes; i++)
    {
        plane_t *p = &g_currentmap->planes[i];

        if(!R_FrustrumTestPlane(p))
        {
            continue;
        }

        dglBegin(GL_TRIANGLES);

        if(p->flags & CLF_UNKNOWN65536)
            dglColor4ub(0, 0, 0, 192);
        else if(p->flags & CLF_ENDLESSPIT)
            dglColor4ub(255, 255, 0, 80);
        else if(p->flags & CLF_SLOPETEST)
            dglColor4ub(255, 128, 0, 80);
        else if(p->flags & CLF_DAMAGE_LAVA)
            dglColor4ub(255, 0, 0, 80);
        else if(p->flags & CLF_UNKNOWN4096)
            dglColor4ub(224, 208, 128, 80);
        else if(p->flags & CLF_UNKNOWN2048)
            dglColor4ub(0, 64, 96, 80);
        else if(p->flags & CLF_UNKNOWN1024)
            dglColor4ub(64, 0, 96, 80);
        else if(p->flags & CLF_HIDDEN)
            dglColor4ub(0, 255, 0, 80);
        else if(p->flags & CLF_ENTERCRAWL)
            dglColor4ub(64, 128, 64, 80);
        else if(p->flags & CLF_CRAWL)
            dglColor4ub(0, 64, 64, 80);
        else if(p->flags & CLF_FRONTNOCLIP)
            dglColor4ub(255, 255, 255, 80);
        else if(p->flags & CLF_CHECKHEIGHT)
            dglColor4ub(0, 0, 255, 80);
        else if(p->flags & CLF_ONESIDED)
            dglColor4ub(0, 255, 255, 80);
        else if(p->flags & CLF_CLIMB)
            dglColor4ub(255, 0, 255, 80);
        else if(p->flags & CLF_TOGGLEON)
            dglColor4ub(255, 96, 255, 80);
        else if(p->flags & CLF_TOGGLEOFF)
            dglColor4ub(32, 8, 4, 80);
        else if(p->flags & CLF_WATER)
            dglColor4ub(128, 96, 255, 80);
        else
            dglColor4ub(255, 128, 128, 80);

        dglVertex3fv(p->points[2]);
        dglVertex3fv(p->points[1]);
        dglVertex3fv(p->points[0]);
        dglEnd();

        if(p->flags & CLF_CHECKHEIGHT)
        {
            dglBegin(GL_TRIANGLES);
            dglColor4ub(128, 128, 128, 192);
            dglVertex3f(p->points[0][0], p->height[0], p->points[0][2]);
            dglVertex3f(p->points[1][0], p->height[1], p->points[1][2]);
            dglVertex3f(p->points[2][0], p->height[2], p->points[2][2]);
            dglEnd();
        }

        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        dglBegin(GL_TRIANGLES);
        dglColor4ub(255, 255, 255, 255);
        dglVertex3fv(p->points[2]);
        dglVertex3fv(p->points[1]);
        dglVertex3fv(p->points[0]);
        dglEnd();

        if(p->flags & CLF_CHECKHEIGHT)
        {
            dglBegin(GL_TRIANGLES);
            dglVertex3f(p->points[0][0], p->height[0], p->points[0][2]);
            dglVertex3f(p->points[1][0], p->height[1], p->points[1][2]);
            dglVertex3f(p->points[2][0], p->height[2], p->points[2][2]);
            dglEnd();
        }

        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);

    dglDisable(GL_ALPHA_TEST);
}

//
// R_DrawBoundingBox
//

void R_DrawBoundingBox(bbox_t bbox, byte r, byte g, byte b)
{
    if(g_currentmap == NULL)
    {
        return;
    }

    if(!R_FrustrumTestBox(bbox))
    {
        return;
    }

    dglEnable(GL_DEPTH_TEST);
    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    dglColor4ub(r, g, b, 255);

    dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.min[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.min[0], bbox.max[1], bbox.max[2]);
    dglEnd();

    dglBegin(GL_POLYGON);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.min[2]);
    dglVertex3d(bbox.max[0], bbox.max[1], bbox.max[2]);
    dglVertex3d(bbox.max[0], bbox.min[1], bbox.max[2]);
    dglEnd();
    
    dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);

    dglDisable(GL_DEPTH_TEST);
}

