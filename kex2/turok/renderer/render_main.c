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
#include "client.h"
#include "level.h"
#include "zone.h"

CVAR_EXTERNAL(cl_fov);
static kbool showcollision = /*false*/ true; // TEMP

//
// R_DrawSection
//

void R_DrawSection(mdlsection_t *section)
{
    if(section->flags & MDF_NOCULLFACES)
    {
        GL_SetState(GLSTATE_CULL, false);
    }

    if(section->flags & MDF_MASKED)
    {
        GL_SetState(GLSTATE_BLEND, 1);
        dglEnable(GL_ALPHA_TEST);
    }

    if(section->flags & MDF_SHINYSURFACE)
    {
        dglEnable(GL_TEXTURE_GEN_S);
        dglEnable(GL_TEXTURE_GEN_T);
    }

    if(section->flags & MDF_COLORIZE)
    {
        dglColor4ubv((byte*)&section->color1);
    }
    else
    {
        dglColor4ub(255, 255, 255, 255);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float), section->normals);
    dglTexCoordPointer(2, GL_FLOAT, 0, section->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), section->xyz);

    GL_BindTexture(Tex_CacheTextureFile(
        section->texpath, GL_REPEAT, section->flags & MDF_MASKED));

    if(has_GL_EXT_compiled_vertex_array)
    {
        dglLockArraysEXT(0, (section->numtris / 3) * 2);
    }

    dglDrawElements(GL_TRIANGLES, section->numtris, GL_UNSIGNED_SHORT, section->tris);

    if(has_GL_EXT_compiled_vertex_array)
    {
        dglUnlockArraysEXT();
    }

    if(section->flags & MDF_MASKED)
    {
        GL_SetState(GLSTATE_BLEND, 0);
        dglDisable(GL_ALPHA_TEST);
    }

    if(section->flags & MDF_SHINYSURFACE)
    {
        dglDisable(GL_TEXTURE_GEN_S);
        dglDisable(GL_TEXTURE_GEN_T);
    }

    if(section->flags & MDF_NOCULLFACES)
    {
        GL_SetState(GLSTATE_CULL, true);
    }
}

//
// R_DrawTestModel
//

static void R_DrawTestModel(const char *file)
{
    kmodel_t *model;
    unsigned int i;

    if(!(model = Mdl_Load(file)))
    {
        return;
    }

    dglCullFace(GL_BACK);
    dglEnable(GL_DEPTH_TEST);

    dglDisableClientState(GL_COLOR_ARRAY);

    for(i = 0; i < model->nodes[0].meshes[0].numsections; i++)
    {
        R_DrawSection(&model->nodes[0].meshes[0].sections[i]);
    }

    dglEnableClientState(GL_COLOR_ARRAY);
    dglDisable(GL_DEPTH_TEST);
    dglCullFace(GL_FRONT);
}

//
// R_DrawCollision
//

static void R_DrawCollision(void)
{
    unsigned int i;

    if(g_currentmap == NULL)
    {
        return;
    }

    dglEnable(GL_ALPHA_TEST);

    GL_SetState(GLSTATE_TEXTURE0, false);
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);

    for(i = 0; i < g_currentmap->numplanes; i++)
    {
        plane_t *p = &g_currentmap->planes[i];

        dglBegin(GL_TRIANGLES);

        if(p->flags & CLF_UNKNOWN65536)
            dglColor4ub(0, 0, 0, 192);
        else if(p->flags & CLF_UNKNOWN32768)
            dglColor4ub(224, 208, 128, 80);
        else if(p->flags & CLF_ENDLESSPIT)
            dglColor4ub(255, 255, 0, 80);
        else if(p->flags & CLF_UNKNOWN8192)
            dglColor4ub(255, 128, 0, 80);
        else if(p->flags & CLF_DAMAGE_LAVA)
            dglColor4ub(255, 0, 0, 80);
        else if(p->flags & CLF_UNKNOWN2048)
            dglColor4ub(0, 64, 96, 80);
        else if(p->flags & CLF_UNKNOWN1024)
            dglColor4ub(64, 0, 96, 80);
        else if(p->flags & CLF_HIDDEN)
            dglColor4ub(0, 255, 0, 80);
        else if(p->flags & CLF_UNKNOWN256)
            dglColor4ub(64, 128, 64, 80);
        else if(p->flags & CLF_UNKNOWN128)
            dglColor4ub(0, 64, 64, 80);
        else if(p->flags & CLF_CHECKHEIGHT)
            dglColor4ub(0, 0, 255, 80);
        else if(p->flags & CLF_ONESIDED)
            dglColor4ub(0, 255, 255, 80);
        else if(p->flags & CLF_UNKNOWN16)
            dglColor4ub(255, 0, 255, 80);
        else if(p->flags & CLF_NOSOLIDWALL)
            dglColor4ub(255, 255, 255, 80);
        else if(p->flags & CLF_UNKNOWN4)
            dglColor4ub(255, 96, 255, 80);
        else if(p->flags & CLF_UNKNOWN2)
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

        p->flags &= ~CLF_DAMAGE_LAVA;
    }

    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, false);

    dglDisable(GL_ALPHA_TEST);
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
    Mtx_Identity(mtx);
    Mtx_SetTranslation(mtx,
        -client.localactor.origin[0],
        -client.localactor.origin[1] - 30.72f,
        -client.localactor.origin[2]);
    Mtx_RotateX(mtx, -client.localactor.yaw + (180 * M_RAD));
    Mtx_RotateZ(mtx, client.localactor.pitch);
    dglLoadMatrixf(mtx);

    //R_DrawTestModel("models/mdl320/mdl320.kmesh");

    if(showcollision)
    {
        R_DrawCollision();
    }

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
// FCmd_ShowCollision
//

static void FCmd_ShowCollision(void)
{
    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    showcollision = atoi(Cmd_GetArgv(1));
}

//
// R_Shutdown
//

void R_Shutdown(void)
{
    Z_FreeTags(PU_MODEL, PU_MODEL);
}

//
// R_Init
//

void R_Init(void)
{
    Cmd_AddCommand("showcollision", FCmd_ShowCollision);

    Mdl_Init();
}
