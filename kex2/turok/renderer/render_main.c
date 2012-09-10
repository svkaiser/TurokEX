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
// DESCRIPTION: Renderer System
//
//-----------------------------------------------------------------------------

#include "SDL_opengl.h"
#include "gl.h"
#include "kernel.h"
#include "render.h"
#include "common.h"
#include "mathlib.h"

CVAR_EXTERNAL(cl_fov);

//
// R_DrawTestCube
//

static void R_DrawTestCube(void)
{
    GL_SetState(GLSTATE_CULL, false);
    GL_BindTextureName("textures/default.tga");
    dglEnable(GL_DEPTH_TEST);

    dglColor4ub(255, 255, 255, 255);
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(-32, 32, 32);
    dglTexCoord2f(1, 0);
    dglVertex3f(32, 32, 32);
    dglTexCoord2f(1, 1);
    dglVertex3f(32, -32, 32);
    dglTexCoord2f(0, 1);
    dglVertex3f(-32, -32, 32);
    dglEnd();
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(32, 32, 32);
    dglTexCoord2f(1, 0);
    dglVertex3f(32, 32, -32);
    dglTexCoord2f(1, 1);
    dglVertex3f(32, -32, -32);
    dglTexCoord2f(0, 1);
    dglVertex3f(32, -32, 32);
    dglEnd();
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(32, 32, -32);
    dglTexCoord2f(1, 0);
    dglVertex3f(-32, 32, -32);
    dglTexCoord2f(1, 1);
    dglVertex3f(-32, -32, -32);
    dglTexCoord2f(0, 1);
    dglVertex3f(32, -32, -32);
    dglEnd();
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(-32, 32, -32);
    dglTexCoord2f(1, 0);
    dglVertex3f(-32, 32, 32);
    dglTexCoord2f(1, 1);
    dglVertex3f(-32, -32, 32);
    dglTexCoord2f(0, 1);
    dglVertex3f(-32, -32, -32);
    dglEnd();
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(-32, 32, -32);
    dglTexCoord2f(1, 0);
    dglVertex3f(32, 32, -32);
    dglTexCoord2f(1, 1);
    dglVertex3f(32, 32, 32);
    dglTexCoord2f(0, 1);
    dglVertex3f(-32, 32, 32);
    dglEnd();
    dglBegin(GL_POLYGON);
    dglTexCoord2f(0, 0);
    dglVertex3f(-32, -32, 32);
    dglTexCoord2f(1, 0);
    dglVertex3f(32, -32, 32);
    dglTexCoord2f(1, 1);
    dglVertex3f(32, -32, -32);
    dglTexCoord2f(0, 1);
    dglVertex3f(-32, -32, -32);
    dglEnd();

    GL_SetState(GLSTATE_CULL, true);
    dglDisable(GL_DEPTH_TEST);
}

//
// R_DrawTestModel
//

static void R_DrawTestModel(const char *file)
{
    kmodel_t *model;
    float *coords;
    vec3_t *vtx;
    float *normals;
    word *tris;
    int count;
    unsigned int i;

    if(!(model = Mdl_Load(file)))
    {
        return;
    }

    dglCullFace(GL_BACK);
    dglEnable(GL_DEPTH_TEST);
    dglColor4ub(255, 255, 255, 255);

    dglDisableClientState(GL_COLOR_ARRAY);

    for(i = 0; i < model->nodes[0].meshes[0].numsections; i++)
    {
        coords  = model->nodes[0].meshes[0].sections[i].coords;
        vtx     = model->nodes[0].meshes[0].sections[i].xyz;
        normals = model->nodes[0].meshes[0].sections[i].normals;
        tris    = model->nodes[0].meshes[0].sections[i].tris;

        dglTexCoordPointer(2, GL_FLOAT, 0, coords);
        dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), vtx);
        dglNormalPointer(GL_FLOAT, 0, normals);

        GL_BindTexture(Tex_CacheTextureFile(
            model->nodes[0].meshes[0].sections[i].texpath, GL_REPEAT, false));

        count = model->nodes[0].meshes[0].sections[i].numtris;

        if(has_GL_EXT_compiled_vertex_array)
            dglLockArraysEXT(0, (count / 3) * 2);

        dglDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, tris);

        if(has_GL_EXT_compiled_vertex_array)
            dglUnlockArraysEXT();
    }

    dglEnableClientState(GL_COLOR_ARRAY);
    dglDisable(GL_DEPTH_TEST);
    dglCullFace(GL_FRONT);
}

//
// R_DrawFrame
//

void R_DrawFrame(void)
{
    mtx_t mtx;

    GL_ClearView(0xFF3f3f3f);
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(video_width, video_height, cl_fov.value, 0.1f);
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    Mtx_Identity(mtx);
    Mtx_SetTranslation(mtx, 0, -1024, -1500);
    dglMultMatrixf(mtx);

    R_DrawTestCube();
    R_DrawTestModel("models/mdl320/mdl320.kmesh");

    GL_SetOrtho();
}

//
// R_FinishFrame
//

void R_FinishFrame(void)
{
    IN_UpdateGrab();
    GL_SwapBuffers();
}

//
// R_Init
//

void R_Init(void)
{
    Mdl_Init();
}
