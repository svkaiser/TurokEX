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
static kbool showcollision = false;

static double viewMatrix[16];
static double projMatrix[16];
static float frustum[6][4];

#define CALCMATRIX(a, b, c, d, e, f, g, h)  \
    (float)(viewMatrix[a] * projMatrix[b] + \
    viewMatrix[c] * projMatrix[d] +         \
    viewMatrix[e] * projMatrix[f] +         \
    viewMatrix[g] * projMatrix[h])

//
// R_SetupClipFrustum
//

void R_SetupClipFrustum(void)
{
   mtx_t clip;

   dglGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
   dglGetDoublev(GL_MODELVIEW_MATRIX, viewMatrix);

   clip[0]  = CALCMATRIX(0, 0, 1, 4, 2, 8, 3, 12);
   clip[1]  = CALCMATRIX(0, 1, 1, 5, 2, 9, 3, 13);
   clip[2]  = CALCMATRIX(0, 2, 1, 6, 2, 10, 3, 14);
   clip[3]  = CALCMATRIX(0, 3, 1, 7, 2, 11, 3, 15);

   clip[4]  = CALCMATRIX(4, 0, 5, 4, 6, 8, 7, 12);
   clip[5]  = CALCMATRIX(4, 1, 5, 5, 6, 9, 7, 13);
   clip[6]  = CALCMATRIX(4, 2, 5, 6, 6, 10, 7, 14);
   clip[7]  = CALCMATRIX(4, 3, 5, 7, 6, 11, 7, 15);

   clip[8]  = CALCMATRIX(8, 0, 9, 4, 10, 8, 11, 12);
   clip[9]  = CALCMATRIX(8, 1, 9, 5, 10, 9, 11, 13);
   clip[10] = CALCMATRIX(8, 2, 9, 6, 10, 10, 11, 14);
   clip[11] = CALCMATRIX(8, 3, 9, 7, 10, 11, 11, 15);

   clip[12] = CALCMATRIX(12, 0, 13, 4, 14, 8, 15, 12);
   clip[13] = CALCMATRIX(12, 1, 13, 5, 14, 9, 15, 13);
   clip[14] = CALCMATRIX(12, 2, 13, 6, 14, 10, 15, 14);
   clip[15] = CALCMATRIX(12, 3, 13, 7, 14, 11, 15, 15);

   // Right plane
   frustum[0][0] = clip[ 3] - clip[ 0];
   frustum[0][1] = clip[ 7] - clip[ 4];
   frustum[0][2] = clip[11] - clip[ 8];
   frustum[0][3] = clip[15] - clip[12];

   // Left plane
   frustum[1][0] = clip[ 3] + clip[ 0];
   frustum[1][1] = clip[ 7] + clip[ 4];
   frustum[1][2] = clip[11] + clip[ 8];
   frustum[1][3] = clip[15] + clip[12];

   // Bottom plane
   frustum[2][0] = clip[ 3] + clip[ 1];
   frustum[2][1] = clip[ 7] + clip[ 5];
   frustum[2][2] = clip[11] + clip[ 9];
   frustum[2][3] = clip[15] + clip[13];

   // Top plane
   frustum[3][0] = clip[ 3] - clip[ 1];
   frustum[3][1] = clip[ 7] - clip[ 5];
   frustum[3][2] = clip[11] - clip[ 9];
   frustum[3][3] = clip[15] - clip[13];

   // Far plane
   frustum[4][0] = clip[ 3] - clip[ 2];
   frustum[4][1] = clip[ 7] - clip[ 6];
   frustum[4][2] = clip[11] - clip[10];
   frustum[4][3] = clip[15] - clip[14];

   // Near plane
   frustum[5][0] = clip[ 3] + clip[ 2];
   frustum[5][1] = clip[ 7] + clip[ 6];
   frustum[5][2] = clip[11] + clip[10];
   frustum[5][3] = clip[15] + clip[14];
}

//
// R_FrustrumTestBox
//

kbool R_FrustrumTestBox(bbox_t box)
{
   int p;

   for(p = 0; p < 6; p++)
   {
       if(frustum[p][0] * box.min[0] + frustum[p][1] * box.min[1] + frustum[p][2] * box.min[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.max[0] + frustum[p][1] * box.min[1] + frustum[p][2] * box.min[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.min[0] + frustum[p][1] * box.max[1] + frustum[p][2] * box.min[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.max[0] + frustum[p][1] * box.max[1] + frustum[p][2] * box.min[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.min[0] + frustum[p][1] * box.min[1] + frustum[p][2] * box.max[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.max[0] + frustum[p][1] * box.min[1] + frustum[p][2] * box.max[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.min[0] + frustum[p][1] * box.max[1] + frustum[p][2] * box.max[2] + frustum[p][3] > 0)
           continue;
       if(frustum[p][0] * box.max[0] + frustum[p][1] * box.max[1] + frustum[p][2] * box.max[2] + frustum[p][3] > 0)
           continue;

      return false;
   }

   return true;
}

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
// R_DrawBoundingBox
//

void R_DrawBoundingBox(bbox_t bbox, byte r, byte g, byte b)
{
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
        -client.localactor.origin[1] - 51.2f,
        -client.localactor.origin[2]);
    Mtx_RotateX(mtx, -client.localactor.yaw + (180 * M_RAD));
    Mtx_RotateZ(mtx, client.localactor.pitch);
    dglLoadMatrixf(mtx);

    R_SetupClipFrustum();

    if(g_currentmap != NULL && !showcollision)
    {
        unsigned int i;
        unsigned int j;

        for(i = 0; i < g_currentmap->numinstances; i++)
        {
            instance_t *inst = &g_currentmap->instances[i];

            if(inst == NULL)
            {
                continue;
            }

            for(j = 0; j < inst->numstatics; j++)
            {
                object_t *obj = &inst->statics[j];

                if(obj == NULL)
                {
                    continue;
                }

                dglPushMatrix();
                dglMultMatrixf(obj->matrix);

                if(R_FrustrumTestBox(obj->box))
                {
                    R_DrawTestModel(obj->mdlpath);
                }

                dglPopMatrix();
            }
        }
    }

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
    if(Cmd_GetArgc() < 1)
    {
        return;
    }

    showcollision ^= 1;
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
