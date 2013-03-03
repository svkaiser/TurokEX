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
#include "common.h"
#include "gl.h"
#include "kernel.h"
#include "render.h"
#include "mathlib.h"
#include "client.h"
#include "level.h"
#include "zone.h"
#include "game.h"
#include "js.h"

CVAR_EXTERNAL(cl_fov);
CVAR(r_fog, 1);

static kbool showcollision = false;
static kbool showbbox = false;

static float wpn_thudoffset = 0;
static mtx_t mtx_rotation;

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
// R_FrustumTestBox
//

kbool R_FrustumTestBox(bbox_t box)
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
// R_FrustumTestPlane
//

kbool R_FrustumTestPlane(plane_t *plane)
{
   int p;
   int i;

   for(p = 0; p < 6; p++)
   {
       for(i = 0; i < 3; i++)
       {
           if( frustum[p][0] * plane->points[i][0] + 
               frustum[p][1] * plane->points[i][1] + 
               frustum[p][2] * plane->points[i][2] +
               frustum[p][3] > 0 ||
               frustum[p][0] * plane->points[i][0] + 
               frustum[p][1] * plane->height[i]    + 
               frustum[p][2] * plane->points[i][2] +
               frustum[p][3] > 0)
           {
               break;
           }
       }

       if(i != 3)
       {
           continue;
       }
       
       return false;
   }

   return true;
}

//
// R_DrawSection
//

void R_DrawSection(mdlsection_t *section, char *texture)
{
    texture_t *tex;
    char *texturepath;
    rcolor color;

    if(texture == NULL)
    {
        texturepath = section->texpath;
    }
    else
    {
        texturepath = texture;
    }

    GL_SetState(GLSTATE_CULL, !(section->flags & MDF_NOCULLFACES));
    GL_SetState(GLSTATE_BLEND, section->flags & (MDF_MASKED|MDF_TRANSPARENT1));
    GL_SetState(GLSTATE_ALPHATEST, section->flags & (MDF_MASKED|MDF_TRANSPARENT1));
    GL_SetState(GLSTATE_TEXGEN_S, section->flags & MDF_SHINYSURFACE);
    GL_SetState(GLSTATE_TEXGEN_T, section->flags & MDF_SHINYSURFACE);

    if(section->flags & MDF_COLORIZE)
    {
        color = section->color1;
    }
    else
    {
        color = COLOR_WHITE;
    }

    if(section->flags & MDF_MASKED)
    {
        dglAlphaFunc(GL_GEQUAL, 0.6525f);
    }
    else
    {
        dglAlphaFunc(GL_GEQUAL, 0.01f);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float), section->normals);
    dglTexCoordPointer(2, GL_FLOAT, 0, section->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), section->xyz);

    tex = Tex_CacheTextureFile(texturepath, GL_REPEAT,
        section->flags & MDF_MASKED);

    if(tex)
    {
        GL_SetState(GLSTATE_TEXTURE0, true);
        GL_BindTexture(tex);
    }
    else
    {
        GL_SetState(GLSTATE_TEXTURE0, false);
        color = section->color1;
    }

    if(section->flags & MDF_TRANSPARENT1)
    {
        color = color & 0xffffff;
        color |= (160 << 24);
    }

    dglColor4ubv((byte*)&color);

    dglDrawElements(GL_TRIANGLES, section->numtris, GL_UNSIGNED_SHORT, section->tris);
}

//
// R_TraverseDrawNode
//

void R_TraverseDrawNode(kmodel_t *model, mdlnode_t *node,
                          char **textures, int variant, animstate_t *animstate)
{
    unsigned int i;
    mtx_t mtx;
    anim_t *anim;
    anim_t *prevanim;
    int frame;
    int nextframe;
    float delta;

    if(animstate != NULL)
    {
        dglPushMatrix();

        anim = animstate->track.anim;
        prevanim = animstate->prevtrack.anim;
        frame = animstate->track.frame;
        nextframe = animstate->track.nextframe;
        delta = animstate->deltatime;

        if(anim && frame < (int)anim->numframes)
        {
            vec4_t rot;
            vec3_t pos;
            vec4_t r1, r2;
            vec3_t t1, t2;
            vec4_t rot_cur;
            vec4_t rot_next;
            vec3_t pos_cur;
            vec3_t pos_next;
            int nodenum;

            nodenum = node - model->nodes;

            Mdl_GetAnimRotation(r1, anim, nodenum, frame);
            Mdl_GetAnimRotation(r2, anim, nodenum, nextframe);
            Mdl_GetAnimTranslation(t1, anim, nodenum, frame);
            Mdl_GetAnimTranslation(t2, anim, nodenum, nextframe);

            if(!(animstate->flags & ANF_BLEND))
            {
                Vec_Copy4(rot_cur, r1);
                Vec_Copy4(rot_next, r2);
                Vec_Copy3(pos_cur, t1);
                Vec_Copy3(pos_next, t2);
            }
            else
            {
                vec4_t r3;
                vec3_t t3;

                frame = animstate->prevtrack.frame;
                nextframe = animstate->prevtrack.nextframe;

                Mdl_GetAnimRotation(r3, prevanim, nodenum, frame);
                Mdl_GetAnimTranslation(t3, prevanim, nodenum, frame);

                Vec_Slerp(rot_cur, delta, r3, r1);
                Vec_Slerp(rot_next, delta, r3, r2);
                Vec_Lerp3(pos_cur, delta, t3, t1);
                Vec_Lerp3(pos_next, delta, t3, t2);
            }

            Vec_Slerp(rot, delta, rot_cur, rot_next);
            Vec_Lerp3(pos, delta, pos_cur, pos_next);
            Mtx_ApplyRotation(rot, mtx);
            Mtx_AddTranslation(mtx, pos[0], pos[1], pos[2]);
            dglMultMatrixf(mtx);
        }
    }

    if(node->nummeshes > 0)
    {
        for(i = 0; i < node->meshes[variant].numsections; i++)
        {
            mdlsection_t *section = &node->meshes[variant].sections[i];
            char *texturepath = NULL;

            if(textures != NULL)
            {
                if(textures[i][0] != '-')
                    texturepath = textures[i];
            }

            R_DrawSection(section, texturepath);
        }
    }

    for(i = 0; i < node->numchildren; i++)
    {
        R_TraverseDrawNode(model,
            &model->nodes[node->children[i]], textures, variant, animstate);
    }

    if(animstate != NULL)
    {
        dglPopMatrix();
    }
}

//
// R_MorphModel
//

typedef struct
{
    char    *model;
    int     speed;
    float   time;
    int     tics;
    int     frame;
} morphmodel_t;

static morphmodel_t morphmodels[2] =
{
    { "models/mdl500/mdl500.kmesh", 4, 0, 0, 0 },
    { "models/mdl643/mdl643.kmesh", 8, 0, 0, 0 }
};

static void R_MorphModel(morphmodel_t *morph)
{
    kmodel_t *model;
    unsigned int i;
    unsigned int varcount;
    mdlmesh_t *dst;
    mdlmesh_t *src1;
    mdlmesh_t *src2;

    if(!(model = Mdl_Find(morph->model)))
    {
        return;
    }

    varcount = model->nodes[0].numvariants;
    
    if(varcount <= 0)
    {
        return;
    }

    if(morph->tics <= client.tics)
    {
        morph->time = 0;
        morph->tics = client.tics + morph->speed;
        morph->frame++;
    }

    dst  = &model->nodes[0].meshes[0];
    src1 = &model->nodes[0].meshes[(morph->frame % (varcount-2)) + 2];
    src2 = &model->nodes[0].meshes[((morph->frame + 1) % (varcount-2)) + 2];

    for(i = 0; i < dst->numsections; i++)
    {
        unsigned int j;
        mdlsection_t *s1 = &dst->sections[i];
        mdlsection_t *s2 = &src1->sections[i];
        mdlsection_t *s3 = &src2->sections[i];

        for(j = 0; j < s1->numverts; j++)
        {
            Vec_Lerp3(s1->xyz[j], morph->time, s2->xyz[j], s3->xyz[j]);
        }
    }

    morph->time += (1/(float)morph->speed);
}

//
// R_DrawStatics
//

void R_DrawStatics(void)
{
    unsigned int i;

    if(showcollision || !gLevel.loaded)
        return;

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gb = &gLevel.gridBounds[i];
        unsigned int j;

        for(j = 0; j < gb->numStatics; j++)
        {
            gActor_t *actor = &gb->statics[j];

            if(actor->bHidden)
                continue;

            if(!R_FrustumTestBox(actor->bbox))
                continue;

            dglPushMatrix();
            dglMultMatrixf(actor->matrix);

            R_TraverseDrawNode(actor->model, &actor->model->nodes[0],
                actor->textureSwaps, actor->variant, NULL);

            dglPopMatrix();
        }
    }
}

//
// R_DrawSkies
//

static void R_DrawSkies(void)
{
    static float sky_cloudpanx = 0;
    static float sky_cloudpany = 0;
    static float sky_offset = 0;
    moveframe_t *frame;
    int i;
    area_t *area;
    vtx_t vtx[12];
    texture_t *tex;
    mtx_t mi;
    mtx_t mt;
    mtx_t mtx;

    frame = &client.moveframe;

    if(frame->plane == NULL || showcollision)
    {
        return;
    }

    area = Map_GetArea(frame->plane);

    if(!(area->flags & AAF_DRAWSKY))
    {
        return;
    }

    // TODO - handle a better way of loading sky textures per level
    tex = Tex_CacheTextureFile(kva("maps/map%02d/mapsky%02d_00.tga",
        g_currentmap - kmaps, g_currentmap - kmaps), GL_REPEAT, false);

    // TODO - TEMP
    if(Tex_IsDefault(tex))
    {
        return;
    }

    // set up sky offset relative to identity matrix
    sky_offset = (((area->skyheight - 5.4f) * 10.24f) -
        sky_offset) * 0.1f + sky_offset;

    GL_SetVertexPointer(vtx);

    // reset some states
    GL_SetState(GLSTATE_CULL, false);
    GL_SetState(GLSTATE_BLEND, true);
    GL_SetState(GLSTATE_ALPHATEST, true);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);
    GL_SetState(GLSTATE_TEXTURE0, true);

    GL_BindTexture(tex);

    // update panning skies
    sky_cloudpanx = (client.tics) * 0.768f * -0.0005f;
    sky_cloudpany = (client.tics) * 1.536f * -0.0005f;

    if(sky_cloudpanx > 12288) sky_cloudpanx = sky_cloudpanx - 12288;
    if(sky_cloudpany > 12288) sky_cloudpany = sky_cloudpany - 12288;

    // setup persistent vertex data
    for(i = 0; i < 12; i += 3)
    {
        vtx[i+0].r = vtx[i+0].g = vtx[i+0].b =
        vtx[i+1].r = vtx[i+1].g = vtx[i+1].b =
        vtx[i+2].r = vtx[i+2].g = vtx[i+2].b = 0xff;
        vtx[i+0].a = 0;
        vtx[i+1].a = 0;

        vtx[i+2].tu = 2;
        vtx[i+2].tv = 3;
        vtx[i+2].x = 0;
        vtx[i+2].z = 0;
    }

    // setup texture coordinates (4 x 6)
    vtx[ 0].tu = 0; vtx[ 0].tv = 0;
    vtx[ 1].tu = 4; vtx[ 1].tv = 0;
    vtx[ 3].tu = 0; vtx[ 3].tv = 0;
    vtx[ 4].tu = 0; vtx[ 4].tv = 6;
    vtx[ 6].tu = 4; vtx[ 6].tv = 6;
    vtx[ 7].tu = 4; vtx[ 7].tv = 0;
    vtx[ 9].tu = 4; vtx[ 9].tv = 6;
    vtx[10].tu = 0; vtx[10].tv = 6;

    // setup vertex coordinates (768 x 1024)
    vtx[ 0].x = -768; vtx[ 0].z = -1024;
    vtx[ 1].x =  768; vtx[ 1].z = -1024;
    vtx[ 3].x = -768; vtx[ 3].z = -1024;
    vtx[ 4].x = -768; vtx[ 4].z =  1024;
    vtx[ 6].x =  768; vtx[ 6].z =  1024;
    vtx[ 7].x =  768; vtx[ 7].z = -1024;
    vtx[ 9].x =  768; vtx[ 9].z =  1024;
    vtx[10].x = -768; vtx[10].z =  1024;

    // new projection
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(video_width, video_height, cl_fov.value, 0.1f);

    // new model matrix
    dglMatrixMode(GL_MODELVIEW);
    Mtx_Identity(mi);
    Mtx_Multiply(mtx, mi, mtx_rotation);
    Mtx_RotateY(mtx, frame->yaw);
    dglLoadMatrixf(mtx);

    // new texture matrix
    dglMatrixMode(GL_TEXTURE);
    dglPushMatrix();
    Mtx_SetTranslation(mtx,
        sky_cloudpanx + (frame->origin[0] * 0.0005f),
        sky_cloudpany + (frame->origin[2] * 0.0005f),
        0);
    dglMultMatrixf(mtx);
    dglPushMatrix();
    Mtx_IdentityZ(mi, -frame->yaw);
    Mtx_SetTranslation(mt, -2, -3, 0);
    Mtx_Multiply(mtx, mt, mi);
    dglMultMatrixf(mtx);

    // setup remaining vertex data for 1st layer
    for(i = 0; i < 12; i += 3)
    {
        vtx[i+2].a = 200;
        vtx[i+0].y = vtx[i+1].y = vtx[i+2].y = sky_offset -
            (frame->origin[1] - 79.872f) * 0.5f;
        GL_Triangle(i+0, i+1, i+2);
    }

    GL_DrawElements(12, vtx);

    // push another texture matrix for 2nd layer
    dglPushMatrix();
    Mtx_SetTranslation(mtx, 1, 1.5f, 0);
    dglMultMatrixf(mtx);

    // update vertex data for 2nd layer
    for(i = 0; i < 12; i += 3)
    {
        vtx[i+2].a = 127;
        vtx[i+0].tu /= 2; vtx[i+0].tv /= 2;
        vtx[i+1].tu /= 2; vtx[i+1].tv /= 2;
        vtx[i+2].tu /= 2; vtx[i+2].tv /= 2;
        vtx[i+0].y = vtx[i+1].y = vtx[i+2].y = sky_offset -
            (frame->origin[1] + 266.24f) * 0.5f;
        GL_Triangle(i+0, i+1, i+2);
    }

    GL_DrawElements(12, vtx);

    // pop texture matrix stacks
    dglPopMatrix();
    dglPopMatrix();
    dglPopMatrix();
}

//
// R_DrawFrame
//

void R_DrawFrame(void)
{
    J_RunObjectEvent(JS_EV_RENDER, "event_PreRender");
    R_SetupClipFrustum();

    dglCullFace(GL_BACK);
    dglEnable(GL_DEPTH_TEST);
    dglDisableClientState(GL_COLOR_ARRAY);

    if(showcollision)
        R_DrawCollision();

    R_DrawStatics();

    dglCullFace(GL_FRONT);
    J_RunObjectEvent(JS_EV_RENDER, "event_OnRender");

    dglEnableClientState(GL_COLOR_ARRAY);
    dglAlphaFunc(GL_GEQUAL, 0.01f);

    R_DrawSkies();

    if(!showcollision)
        dglDisable(GL_FOG);

    dglDisable(GL_DEPTH_TEST);

    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_BLEND, false);
    GL_SetState(GLSTATE_ALPHATEST, false);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);

    J_RunObjectEvent(JS_EV_RENDER, "event_PostRender");

    R_MorphModel(&morphmodels[0]);
    R_MorphModel(&morphmodels[1]);
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
// FCmd_ShowBoundingBox
//

static void FCmd_ShowBoundingBox(void)
{
    if(Cmd_GetArgc() < 1)
    {
        return;
    }

    showbbox ^= 1;
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
    Cmd_AddCommand("showbbox", FCmd_ShowBoundingBox);

    Cvar_Register(&r_fog);

    Mdl_Init();
}
