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
#include "system.h"
#include "render.h"
#include "mathlib.h"
#include "client.h"
#include "level.h"
#include "zone.h"
#include "game.h"
#include "js.h"
#include "fx.h"
#include "ai.h"
#include "debug.h"
#include "parse.h"

kexCvar cvarRenderFog("r_fog", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarRenderCull("r_cull", CVF_BOOL|CVF_CONFIG, "1", "TODO");

static kbool showcollision = false;
static kbool showbbox = false;
static kbool showgrid = false;
static kbool showpnmls = false;
static kbool showradius = false;
static kbool shownodes = false;

kbool bWireframe = false;

static float grid_high_y = 0;
static float grid_low_y = 0;

int numFxCount = 0;

static float wpn_thudoffset = 0;
static mtx_t mtx_rotation;

static mtx_t viewMatrix;
static mtx_t projMatrix;
static float frustum[6][4];

kbool showorigin = false;

//
// R_SetupClipFrustum
//

void R_SetupClipFrustum(void)
{
   mtx_t clip;

   dglGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
   dglGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

   Mtx_Multiply(clip, viewMatrix, projMatrix);

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
        GL_SetState(GLSTATE_LIGHTING, false);
    }
    else
    {
        color = COLOR_WHITE;
        GL_SetState(GLSTATE_LIGHTING, true);
    }

    if(section->flags & MDF_MASKED)
    {
        dglAlphaFunc(GL_GEQUAL, 0.6525f);
    }
    else
    {
        dglAlphaFunc(GL_GEQUAL, 0.01f);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float)*3, section->normals);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, section->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), section->xyz);

    tex = Tex_CacheTextureFile(texturepath, GL_REPEAT,
        section->flags & MDF_MASKED);

    if(!bWireframe)
    {
        if(tex)
        {
            GL_SetState(GLSTATE_LIGHTING, true);
            GL_BindTexture(tex);
        }
        else
        {
            GL_SetState(GLSTATE_LIGHTING, false);
            GL_BindTextureName("textures/white.tga");
            color = section->color1;
        }

        if(section->flags & MDF_TRANSPARENT1)
        {
            GL_SetState(GLSTATE_LIGHTING, false);
            color = color & 0xffffff;
            color |= (160 << 24);
        }

        dglColor4ubv((byte*)&color);
    }
    else
    {
        GL_SetState(GLSTATE_LIGHTING, false);
        GL_BindTextureName("textures/white.tga");
    }

    dglDrawElements(GL_TRIANGLES, section->numtris, GL_UNSIGNED_SHORT, section->tris);
}

//
// R_TraverseDrawNode
//

void R_TraverseDrawNode(gActor_t *actor, mdlnode_t *node, animstate_t *animstate)
{
    unsigned int i;
    mtx_t mtx;
    anim_t *anim;
    anim_t *prevanim;
    int frame;
    int nextframe;
    float delta;
    int nodenum;
    kmodel_t *model;

    model = actor->model;
    nodenum = node - model->nodes;

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
            vec4_t rCur;

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
                vec4_t r4;
                vec3_t t3;
                vec3_t t4;

                frame = animstate->prevtrack.frame;
                nextframe = animstate->prevtrack.nextframe;

                Mdl_GetAnimRotation(r3, prevanim, nodenum, frame);
                Mdl_GetAnimRotation(r4, prevanim, nodenum, nextframe);
                Mdl_GetAnimTranslation(t3, prevanim, nodenum, frame);
                Mdl_GetAnimTranslation(t4, prevanim, nodenum, nextframe);

                Vec_Slerp(rot_cur, delta, r3, r1);
                Vec_Slerp(rot_next, delta, r4, r2);
                Vec_Lerp3(pos_cur, delta, t3, t1);
                Vec_Lerp3(pos_next, delta, t4, t2);
            }

            Vec_Slerp(rot, delta, rot_cur, rot_next);
            Vec_Lerp3(pos, delta, pos_cur, pos_next);

            if(nodenum == 0)
            {
                if(animstate->flags & ANF_ROOTMOTION)
                {
                    if(nextframe >= frame && animstate->frametime > 0)
                    {
                        mtx_t mtx;
                        vec3_t offs;
                        vec3_t dir;

                        // TODO
                        Mtx_IdentityY(mtx, DEG2RAD(-90));
                        Mtx_Scale(mtx, -1, 1, 1);

                        dir[0] = pos_next[0] - pos_cur[0];
                        dir[1] = pos_next[1] - pos_cur[1];
                        dir[2] = pos_next[2] - pos_cur[2];

                        Vec_TransformToWorld(mtx, dir, offs);

                        animstate->baseOffset = -pos_cur[2] * actor->scale[1];

                        animstate->rootMotion[0] = offs[0] * actor->scale[0];
                        animstate->rootMotion[1] = offs[1] * actor->scale[1];
                        animstate->rootMotion[2] = offs[2] * actor->scale[2];

                        Vec_Scale(animstate->rootMotion, animstate->rootMotion,
                            (60.0f / animstate->frametime));
                    }
                }

                if(animstate->flags & ANF_ROOTMOTION ||
                    animstate->prevflags & ANF_ROOTMOTION)
                    Vec_Set3(pos, 0, 0, pos[2]);
            }

            Vec_Copy4(rCur, rot);
            Vec_MultQuaternion(rot, rCur, actor->nodeOffsets_r[nodenum]);

            Mtx_ApplyRotation(rot, mtx);
            Mtx_AddTranslation(mtx, pos[0], pos[1], pos[2]);
            dglMultMatrixf(mtx);

            if(shownodes)
            {
                vec3_t tmp;
                Vec_Copy3(tmp, pos_cur);
                tmp[0] *= actor->scale[0];
                tmp[1] *= actor->scale[1];
                tmp[2] *= actor->scale[2];
                GL_SetState(GLSTATE_LIGHTING, false);
                R_DrawOrigin(tmp, 4.0f);
                GL_SetState(GLSTATE_LIGHTING, true);
                // restore the color if in wireframe mode
                if(bWireframe)
                    dglColor4ub(192, 192, 192, 255);
            }
        }
    }

    if(node->nummeshes > 0)
    {
        unsigned int var;

        if(actor->variant >= (int)node->numvariants)
            var = 0;
        else
            var = node->variants[actor->variant];

        if(var >= node->nummeshes)
            var = 0;

        for(i = 0; i < node->meshes[var].numsections; i++)
        {
            mdlsection_t *section = &node->meshes[var].sections[i];
            char *texturepath = NULL;

            if(actor->textureSwaps != NULL)
            {
                char *meshTexture = actor->textureSwaps[nodenum][var][i];

                if(meshTexture != NULL && meshTexture[0] != '-')
                    texturepath = meshTexture;
            }

            R_DrawSection(section, texturepath);
        }
    }

    for(i = 0; i < node->numchildren; i++)
    {
        R_TraverseDrawNode(actor,
            &model->nodes[node->children[i]], animstate);
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
    /*kmodel_t *model;
    unsigned int i;
    unsigned int varcount;
    mdlmesh_t *dst;
    mdlmesh_t *src1;
    mdlmesh_t *src2;

    if(!(model = Kmesh_Find(morph->model)))
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

    morph->time += (1/(float)morph->speed);*/
}

//
// R_DrawActors
//

void R_DrawActors(void)
{
    mtx_t mtx;

    if(!gLevel.loaded)
        return;

    Mtx_IdentityY(mtx, DEG2RAD(-90));
    Mtx_Scale(mtx, -1, 1, 1);

    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gActor_t *actor = gLevel.actorRover;
        bbox_t box;

        if(actor->bHidden)
            continue;

        if(cvarRenderCull.GetBool() && actor->cullDistance != 0)
        {
            if(Vec_Length3(client.LocalPlayer().camera->origin,
                actor->origin) >= actor->cullDistance)
            {
                actor->bCulled = true;
                continue;
            }
        }

        Vec_Copy3(box.min, actor->bbox.min);
        Vec_Copy3(box.max, actor->bbox.max);
        Vec_Mult(box.min, box.min, actor->scale);
        Vec_Mult(box.max, box.max, actor->scale);
        Vec_Add(box.min, box.min, actor->origin);
        Vec_Add(box.max, box.max, actor->origin);

        if(actor != client.LocalPlayer().camera->owner)
        {
            actor->bCulled = !R_FrustumTestBox(box);

            if(actor->bCulled)
                continue;

            if(actor->model)
            {
                dglPushMatrix();
                dglMultMatrixf(actor->matrix);
                dglPushMatrix();

                // TODO - find a better solution for this
                if(!actor->bTouch && !(actor->classFlags & AC_FX))
                    dglMultMatrixf(mtx);

                if(bWireframe)
                    dglColor4ub(192, 192, 192, 255);

                R_TraverseDrawNode(actor, &actor->model->nodes[0], &actor->animState);

                dglPopMatrix();
                dglPopMatrix();
            }
        }

        if(showbbox || showorigin || showradius)
        {
            GL_SetState(GLSTATE_LIGHTING, false);

            if(showbbox)
            {
                if(actor->bTouch)
                    R_DrawBoundingBox(box, 0, 255, 0);
                else
                    R_DrawBoundingBox(box, 255, 0, 0);
            }

            if(showorigin && actor != client.LocalPlayer().camera->owner)
            {
                vec3_t vec;

                dglPushMatrix();
                dglMultMatrixf(actor->matrix);
                Vec_Set3(vec, 0, 0, 0);
                R_DrawOrigin(vec, 32.0f);
                Vec_Set3(vec, 0, actor->centerHeight+actor->viewHeight, 0);
                R_DrawOrigin(vec, 16.0f);
                dglPopMatrix();
            }
            if(showradius && actor != client.LocalPlayer().camera->owner)
            {
                R_DrawRadius(
                    actor->origin[0],
                    actor->origin[1],
                    actor->origin[2],
                    actor->radius,
                    actor->baseHeight,
                    255, 128, 128);

                R_DrawRadius(
                    actor->origin[0],
                    actor->origin[1],
                    actor->origin[2],
                    actor->radius * 0.5f,
                    actor->height + actor->viewHeight,
                    128, 128, 255);

                R_DrawRadius(
                    actor->origin[0],
                    actor->origin[1],
                    actor->origin[2],
                    actor->radius * 0.5f,
                    actor->viewHeight,
                    128, 255, 128);
            }

            GL_SetState(GLSTATE_LIGHTING, true);
        }
    }
}

//
// R_DrawStatics
//

void R_DrawStatics(void)
{
    unsigned int i;

    if(!gLevel.loaded)
        return;

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gb = &gLevel.gridBounds[i];
        unsigned int j;

        if(showgrid)
        {
            bbox_t box;

            box.min[0] = gb->minx;
            box.min[1] = grid_low_y;
            box.min[2] = gb->minz;
            box.max[0] = gb->maxx;
            box.max[1] = grid_high_y;
            box.max[2] = gb->maxz;

            GL_SetState(GLSTATE_LIGHTING, false);
            R_DrawBoundingBox(box, 224, 244, 224);
            GL_SetState(GLSTATE_LIGHTING, true);
        }

        for(j = 0; j < gb->numStatics; j++)
        {
            gActor_t *actor = &gb->statics[j];
            bbox_t box;

            if(actor->bHidden || showcollision)
                continue;

            if(cvarRenderCull.GetBool() && actor->cullDistance != 0)
            {
                if(Vec_Length3(client.LocalPlayer().camera->origin,
                    actor->origin) >= actor->cullDistance)
                {
                    actor->bCulled = true;
                    continue;
                }
            }

            Vec_Copy3(box.min, actor->bbox.min);
            Vec_Copy3(box.max, actor->bbox.max);

            box.min[0] += actor->origin[0];
            box.min[1] += actor->origin[1];
            box.min[2] += actor->origin[2];
            box.max[0] += actor->origin[0];
            box.max[1] += actor->origin[1];
            box.max[2] += actor->origin[2];

            actor->bCulled = !R_FrustumTestBox(box);

            if(actor->bCulled)
                continue;

            if(showgrid)
            {
                if(box.max[1] > grid_high_y)
                    grid_high_y = box.max[1];

                if(box.min[1] < grid_low_y)
                    grid_low_y = box.min[1];
            }

            dglPushMatrix();
            dglMultMatrixf(actor->matrix);

            if(bWireframe)
            {
                if(!actor->bTouch)
                    dglColor4ub(0, 224, 224, 255);
                else
                    dglColor4ub(224, 224, 0, 255);
            }

            R_TraverseDrawNode(actor, &actor->model->nodes[0], NULL);

            dglPopMatrix();

            if(showbbox || showradius)
            {
                GL_SetState(GLSTATE_LIGHTING, false);

                if(showbbox)
                {
                    if(actor->bTouch)
                        R_DrawBoundingBox(box, 0, 255, 0);
                    else
                        R_DrawBoundingBox(box, 255, 255, 0);
                }
                if(showradius && actor->bCollision)
                {
                    R_DrawRadius(actor->origin[0], actor->origin[1], actor->origin[2],
                        actor->radius, actor->height, 255, 128, 128);
                }

                GL_SetState(GLSTATE_LIGHTING, true);
            }
        }
    }
}

//
// R_DrawSkies
//

static void R_DrawSkies(void)
{
    /*static float sky_cloudpanx = 0;
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
    Mtx_ViewFrustum(sysMain.VideoWidth(), sysMain.VideoHeight(), cl_fov.value, 0.1f);

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
    dglPopMatrix();*/
}

//
// R_SetupWorldLight
//

static void R_SetupWorldLight(void)
{
    if(bWireframe || !gLevel.loaded)
        return;

    GL_SetState(GLSTATE_LIGHTING, true);

    dglEnable(GL_LIGHT0);
    dglEnable(GL_COLOR_MATERIAL);
    dglColorMaterial(GL_FRONT, GL_DIFFUSE);
    dglColorMaterial(GL_BACK, GL_DIFFUSE);
    dglLightfv(GL_LIGHT0, GL_POSITION, gLevel.worldLightOrigin);
    dglMaterialfv(GL_FRONT, GL_DIFFUSE, gLevel.worldLightColor);
    dglMaterialfv(GL_FRONT, GL_AMBIENT, gLevel.worldLightAmbience);
    dglLightModelfv(GL_LIGHT_MODEL_AMBIENT, gLevel.worldLightModelAmbience);
}

//
// R_DrawFrame
//

void R_DrawFrame(void)
{
    client.LocalPlayer().PlayerEvent("onPreRender");

    if(showcollision)
        dglDisable(GL_FOG);

    R_RenderCameraView();
    R_SetupClipFrustum();

    dglCullFace(GL_BACK);
    GL_SetState(GLSTATE_DEPTHTEST, true);

    if(showpnmls)
        R_DrawPlaneNormals();

    if(showcollision)
        R_DrawCollision();

    dglDisableClientState(GL_COLOR_ARRAY);

    if(bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    R_SetupWorldLight();
    R_DrawStatics();
    R_DrawActors();

    GL_SetState(GLSTATE_LIGHTING, false);

    dglCullFace(GL_FRONT);
    R_DrawFX();

    client.LocalPlayer().PlayerEvent("onRender");

    dglEnableClientState(GL_COLOR_ARRAY);
    dglAlphaFunc(GL_GEQUAL, 0.01f);

    R_DrawSkies();

    if(!showcollision)
        dglDisable(GL_FOG);

    if(bWireframe)
        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GL_SetState(GLSTATE_DEPTHTEST, false);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_BLEND, false);
    GL_SetState(GLSTATE_ALPHATEST, false);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);
    GL_SetState(GLSTATE_LIGHTING, false);
    
    dglDisable(GL_LIGHT0);
    dglDisable(GL_COLOR_MATERIAL);

    GL_SetOrtho();

    client.LocalPlayer().PlayerEvent("onPostRender");

    R_MorphModel(&morphmodels[0]);
    R_MorphModel(&morphmodels[1]);
}

//
// R_ScreenShot
//
extern kexCvar cvarBasePath;
static void R_ScreenShot(void)
{
    static char name[256];
    int shotnum;
    byte *buff;

    shotnum = 0;
    while(shotnum < 1000)
    {
        sprintf(name, "%s\\sshot%03d.tga", cvarBasePath.GetValue(), shotnum);
        if(access(name, 0) != 0)
            break;
        shotnum++;
    }

    if(shotnum >= 1000)
        return;

    if((sysMain.VideoHeight() % 2))  // height must be power of 2
        return;

    if(!(buff = GL_GetScreenBuffer(0, 0, sysMain.VideoWidth(), sysMain.VideoHeight(), false)))
        return;

    Img_WriteTGA(name, buff, sysMain.VideoWidth(), sysMain.VideoHeight());
    common.Printf("Saved Screenshot %s\n", name);
}

//
// R_FinishFrame
//

void R_FinishFrame(void)
{
    inputSystem.UpdateGrab();
    GL_SwapBuffers();
}

//
// FCmd_ShowCollision
//

static void FCmd_ShowCollision(void)
{
    if(command.GetArgc() < 1)
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
    if(command.GetArgc() < 1)
    {
        return;
    }

    showbbox ^= 1;
}

//
// FCmd_ShowGrid
//

static void FCmd_ShowGrid(void)
{
    if(command.GetArgc() < 1)
    {
        return;
    }

    showgrid ^= 1;
}

//
// FCmd_ShowOrigin
//

static void FCmd_ShowOrigin(void)
{
    if(command.GetArgc() < 1)
        return;

    showorigin ^= 1;
}

//
// FCmd_ShowPlaneNormals
//

static void FCmd_ShowPlaneNormals(void)
{
    if(command.GetArgc() < 1)
        return;

    showpnmls ^= 1;
}

//
// FCmd_ShowRadius
//

static void FCmd_ShowRadius(void)
{
    if(command.GetArgc() < 1)
        return;

    showradius ^= 1;
}

//
// FCmd_ShowWireframe
//

static void FCmd_ShowWireFrame(void)
{
    if(command.GetArgc() < 1)
        return;

    bWireframe ^= 1;
}

//
// FCmd_ShowNodes
//

static void FCmd_ShowNodes(void)
{
    if(command.GetArgc() < 1)
        return;

    shownodes ^= 1;
}

//
// FCmd_Screenshot
//

static void FCmd_Screenshot(void)
{
    R_ScreenShot();
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
    //command.Add("showcollision", FCmd_ShowCollision);
    command.Add("showbbox", FCmd_ShowBoundingBox);
    command.Add("showgrid", FCmd_ShowGrid);
    command.Add("showorigin", FCmd_ShowOrigin);
    command.Add("showpnmls", FCmd_ShowPlaneNormals);
    command.Add("showradius", FCmd_ShowRadius);
    command.Add("drawwireframe", FCmd_ShowWireFrame);
    command.Add("shownodes", FCmd_ShowNodes);
    command.Add("screenshot", FCmd_Screenshot);

    Debug_RegisterPerfStatVar((float*)&numFxCount, "FX Count", false);

    Mdl_Init();
    FX_Init();
}
