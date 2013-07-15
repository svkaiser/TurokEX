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

kbool R_FrustumTestBox(bbox_t box);
kbool R_FrustumTestPlane(plane_t *plane);

//
// R_DrawCollision
//

void R_DrawCollision(void)
{
    unsigned int i;

    if(!gLevel.loaded)
        return;

    dglEnable(GL_ALPHA_TEST);
    dglDisable(GL_FOG);

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    for(i = 0; i < gLevel.numplanes; i++)
    {
        plane_t *p = &gLevel.planes[i];

        if(!R_FrustumTestPlane(p))
            continue;

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
        else if(p->flags & CLF_TOGGLE)
            dglColor4ub(255, 96, 255, 80);
        else if(p->flags & CLF_BLOCK)
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
// R_DrawPlaneNormals
//

void R_DrawPlaneNormals(void)
{
    unsigned int i;

    if(!gLevel.loaded)
        return;

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);

    for(i = 0; i < gLevel.numplanes; i++)
    {
        float x, y, z;

        plane_t *p = &gLevel.planes[i];

        if(!R_FrustumTestPlane(p))
            continue;

        x = (p->points[0][0] + p->points[1][0] + p->points[2][0]) / 3;
        y = (p->points[0][1] + p->points[1][1] + p->points[2][1]) / 3;
        z = (p->points[0][2] + p->points[1][2] + p->points[2][2]) / 3;

        dglBegin(GL_LINES);
        dglColor4ub(0, 32, 255, 255);
        dglVertex3f(x, y, z);
        dglColor4ub(0, 255, 0, 255);
        dglVertex3f(
            x + (16 * p->normal[0]),
            y + (16 * p->normal[1]),
            z + (16 * p->normal[2]));
        dglEnd();

        if(p->flags & CLF_CHECKHEIGHT)
        {
            y = (p->height[0] + p->height[1] + p->height[2]) / 3;

            dglBegin(GL_LINES);
            dglColor4ub(255, 128, 128, 255);
            dglVertex3f(x, y, z);
            dglColor4ub(255, 255, 0, 255);
            dglVertex3f(
                x + (16 * p->ceilingNormal[0]),
                y + (16 * p->ceilingNormal[1]),
                z + (16 * p->ceilingNormal[2]));
            dglEnd();
        }
    }

    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);
}

//
// R_DrawRadius
//

void R_DrawRadius(float x, float y, float z, float radius, float height,
                  byte r, byte g, byte b)
{
    float an;
    int i;

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    an = DEG2RAD(360 / 32);

    dglBegin(GL_LINES);
    dglColor4ub(r, g, b, 255);

    for(i = 0; i < 32; i++)
    {
        float s1 = (float)sin(an * i);
        float c1 = (float)cos(an * i);
        float s2 = (float)sin(an * ((i+1)%31));
        float c2 = (float)cos(an * ((i+1)%31));

        dglVertex3f(x + (radius * s1), y, z + (radius * c1));
        dglVertex3f(x + (radius * s1), y + height, z + (radius * c1));
        dglVertex3f(x + (radius * s1), y, z + (radius * c1));
        dglVertex3f(x + (radius * s2), y, z + (radius * c2));
        dglVertex3f(x + (radius * s1), y + height, z + (radius * c1));
        dglVertex3f(x + (radius * s2), y + height, z + (radius * c2));
    }

    dglEnd();

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);
}

//
// R_DrawBoundingBox
//

void R_DrawBoundingBox(bbox_t bbox, byte r, byte g, byte b)
{
    if(!gLevel.loaded)
        return;

    if(!R_FrustumTestBox(bbox))
        return;

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    dglColor4ub(r, g, b, 255);

    if(!bWireframe)
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
    
    if(!bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);
}

//
// R_DrawOrigin
//

void R_DrawOrigin(vec3_t origin, float size)
{
    GL_SetState(GLSTATE_TEXTURE0, false);

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);
    dglBegin(GL_LINES);

    // x
    dglColor4ub(255, 0, 0, 255);
    dglVertex3f(origin[0], origin[1], origin[2]);
    dglVertex3f(origin[0] + size, origin[1], origin[2]);
    // y
    dglColor4ub(0, 255, 0, 255);
    dglVertex3f(origin[0], origin[1], origin[2]);
    dglVertex3f(origin[0], origin[1] + size, origin[2]);
    // z
    dglColor4ub(0, 0, 255, 255);
    dglVertex3f(origin[0], origin[1], origin[2]);
    dglVertex3f(origin[0], origin[1], origin[2] + size);

    dglEnd();
    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);

    GL_SetState(GLSTATE_TEXTURE0, true);
}

//
// R_DrawNormals
//

void R_DrawNormals(mdlsection_t *section)
{
    unsigned int i;

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    dglBegin(GL_LINES);

    for(i = 0; i < section->numverts; i++)
    {
        float x, y, z;

        x = section->xyz[i][0];
        y = section->xyz[i][1];
        z = section->xyz[i][2];

        dglColor4ub(0, 0, 255, 255);
        dglVertex3f(x, y, z);
        glColor4ub(0, 255, 0, 255);
        dglVertex3f(
            x + (8 * section->normals[i*3+0]),
            y + (8 * section->normals[i*3+1]),
            z + (8 * section->normals[i*3+2]));
    }

    dglEnd();

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);
}
