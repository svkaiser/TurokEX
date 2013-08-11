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
// DESCRIPTION: Shadow Rendering
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "gl.h"
#include "render.h"
#include "client.h"


typedef struct
{
    int     i2;
    int     facing;
} edgeDef_t;

#define	MAX_EDGE_DEFS	32
#define MAX_VERTICES    4000
#define MAX_INDEXES     (6*MAX_VERTICES)

static word numIndexes = 0;
static word indexes[MAX_INDEXES];
static word numVertexes = 0;
static vtx_t vtx[MAX_VERTICES];
static vec3_t shadowVerts[MAX_VERTICES];

static edgeDef_t    edgeDefs[MAX_VERTICES][MAX_EDGE_DEFS];
static int          numEdgeDefs[MAX_VERTICES];
static int          facing[MAX_INDEXES/3];

//
// R_AddEdgeDef
//

static void R_AddEdgeDef(int i1, int i2, int facing)
{
    int c;
    
    c = numEdgeDefs[i1];

    if(c == MAX_EDGE_DEFS)
        return;		// overflow

    edgeDefs[i1][c].i2 = i2;
    edgeDefs[i1][c].facing = facing;
    
    numEdgeDefs[i1]++;
}

//
// R_DrawShadowEdges
//

static void R_DrawShadowEdges(mdlsection_t *section)
{
    unsigned int i;
    unsigned int j;
    unsigned int c;
    unsigned int numTris;
    int i2;
    int o1;
    int o2;
    int o3;

    for(i = 0; i < section->numverts; i++)
    {
        c = numEdgeDefs[i];
        for(j = 0; j < c; j++)
        {
            if(!edgeDefs[i][j].facing)
                continue;

            i2 = edgeDefs[i][j].i2;

            dglBegin(GL_TRIANGLE_STRIP);
            dglVertex3fv(section->xyz[i]);
            dglVertex3fv(shadowVerts[i]);
            dglVertex3fv(section->xyz[i2]);
            dglVertex3fv(shadowVerts[i2]);
            dglEnd();
        }
    }

    numTris = section->numtris / 3;

    for(i = 0; i < numTris; i++)
    {
        if(!facing[i])
            continue;

        o1 = section->tris[i*3+0];
        o2 = section->tris[i*3+1];
        o3 = section->tris[i*3+2];

        dglBegin(GL_TRIANGLES);
        dglVertex3fv(section->xyz[o1]);
        dglVertex3fv(section->xyz[o2]);
        dglVertex3fv(section->xyz[o3]);
        dglEnd();

        dglBegin(GL_TRIANGLES);
        dglVertex3fv(shadowVerts[o3]);
        dglVertex3fv(shadowVerts[o2]);
        dglVertex3fv(shadowVerts[o1]);
        dglEnd();
    }
}

//
// R_ShadowFill
//

void R_ShadowFill(void)
{
    dglEnable(GL_STENCIL_TEST);
    dglStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFF);
    dglStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    dglDisable(GL_CLIP_PLANE0);

    GL_SetState(GLSTATE_DEPTHTEST, true);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);
    GL_SetState(GLSTATE_ALPHATEST, true);
    GL_BindTextureName("textures/white.tga");

    dglPushMatrix();
    dglLoadIdentity();
    dglColor4f(0, 0, 0, 0.5f);
    dglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    dglBegin(GL_QUADS);
	dglVertex3f(-100, 100, -10);
	dglVertex3f(100, 100, -10);
	dglVertex3f(100, -100, -10);
	dglVertex3f(-100, -100, -10);
	dglEnd();

    dglColor4f(1, 1, 1, 1);
    dglDisable(GL_STENCIL_TEST);
    dglPopMatrix();
    dglEnable(GL_CLIP_PLANE0);
}

//
// R_CreateShadows
//

void R_CreateShadows(mdlsection_t *section)
{
    unsigned int i;
    unsigned int numTris;
    vec3_t worldorg;
    vec3_t lightDir;
    vec3_t lightOrg;
    float groundDist;
    gActor_t *camera;
    
    camera = client.player->camera;

    //if(numVertexes >= (MAX_VERTICES / 2))
        //return;

    Vec_Set3(lightOrg, 1, 1, 1);
    lightOrg[1] = 0;
    Vec_Normalize3(lightOrg);

    Vec_Set3(lightDir, lightOrg[0]*0.3f, 1, lightOrg[2]*0.3f);

    for(i = 0; i < section->numverts; i++)
    {
        vec3_t xyz;

        //Vec_Set3(xyz, vtx[i].x, vtx[i].y, vtx[i].z);
        Vec_Copy3(xyz, section->xyz[i]);
        Vec_Add(worldorg, xyz, camera->origin);
        groundDist = worldorg[1] - /*shadowPlane*/-128.0f;
        groundDist += 16.0f;

        shadowVerts[i][0] = xyz[0] + (lightDir[0] * -groundDist);
        shadowVerts[i][1] = xyz[1] + (lightDir[1] * -groundDist);
        shadowVerts[i][2] = xyz[2] + (lightDir[2] * -groundDist);
    }

    memset(numEdgeDefs, 0, sizeof(int) * section->numverts);
    numTris = section->numtris / 3;

    for(i = 0; i < numTris; i++)
    {
        int i1, i2, i3;
        vec3_t d1, d2, normal;
        float *v1, *v2, *v3;
        float d;

        i1 = section->tris[i*3+0];
        i2 = section->tris[i*3+1];
        i3 = section->tris[i*3+2];

        v1 = section->xyz[i1];
        v2 = section->xyz[i2];
        v3 = section->xyz[i3];

        Vec_Sub(d1, v2, v1);
        Vec_Sub(d2, v3, v1);
        Vec_Cross(normal, d1, d2);

        d = Vec_Dot(normal, lightDir);

        if(d > 0)
            facing[i] = 1;
        else
            facing[i] = 0;

        R_AddEdgeDef(i1, i2, facing[i]);
        R_AddEdgeDef(i2, i3, facing[i]);
        R_AddEdgeDef(i3, i1, facing[i]);
    }

    GL_BindTextureName("textures/white.tga");
    dglBlendFunc(GL_ONE, GL_ZERO);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);
    GL_SetState(GLSTATE_ALPHATEST, false);

    if(bWireframe)
    {
        dglColor3f(1, 0, 0);
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        dglColor3f(0.2f, 0.2f, 0.2f);
        dglColorMask(0, 0, 0, 0);
        dglEnable(GL_STENCIL_TEST);
        dglDepthMask(GL_FALSE);
        dglStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
    }

    dglDepthFunc(GL_LESS);

    // front
    dglCullFace(GL_FRONT);
    dglStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
    R_DrawShadowEdges(section);

    // back
    dglCullFace(GL_BACK);
    dglStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
    R_DrawShadowEdges(section);

    dglDepthFunc(GL_LEQUAL);

    // restore states
    dglCullFace(GL_BACK);
    dglDepthMask(GL_TRUE);

    if(!bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    dglColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
