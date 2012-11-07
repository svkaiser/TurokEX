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
#include "game.h"

CVAR_EXTERNAL(cl_fov);
static kbool showcollision = false;
static kbool showbbox = false;

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
// R_FrustrumTestPlane
//

kbool R_FrustrumTestPlane(plane_t *plane)
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
// R_SetupViewFrame
//

static void R_SetupViewFrame(actor_t *actor)
{
    static float cam_roll;
    mtx_t mtx;
    vec4_t yaw;
    vec4_t pitch;
    vec4_t roll;
    vec4_t vroll;
    vec4_t rot;
    float bob_x;
    float bob_y;
    float d;
    float amt;
    vec3_t org;
    vec3_t pos;
    vec3_t dir;
    float angle;

    // roll view camera if strafing left or right

    // create the directional vector
    Vec_Add(dir, actor->origin, actor->velocity);
    Vec_Sub(dir, dir, actor->origin);

    // get angle of direction
    angle = Ang_Diff(actor->yaw + M_PI,
        Ang_VectorToAngle(dir) + M_PI);

    // get normalized direction vector
    Vec_Copy3(dir, actor->velocity);
    Vec_Normalize3(dir);

    // clamp angle between -90 and 90
    Ang_Clamp(&angle);

    if(angle > DEG2RAD(90))
    {
        angle = M_PI - angle;
    }

    if(angle < -DEG2RAD(90))
    {
        angle = -M_PI - angle;
    }

    d = Vec_Unit2(actor->velocity) * 0.05f;

    if(actor->terriantype == TT_WATER_UNDER)
    {
        cam_roll *= 0.935f;
        amt = 0.4f;
    }
    else
    {
        cam_roll *= 0.9f;
        amt = 0.0625f;
    }

    // interpolate view roll
    cam_roll = (((angle * amt) * Vec_Unit2(dir)) - cam_roll) * d + cam_roll;

    // clamp roll due to stupid floating point precision
    if(cam_roll < 0.001f && cam_roll > -0.001f)
    {
        cam_roll = 0;
    }

    bob_x = 0;
    bob_y = 0;

    if(actor->terriantype != TT_WATER_UNDER && (actor->origin[1] +
        actor->velocity[1]) -
        Plane_GetDistance(actor->plane, actor->origin) < 4)
    {
        // calculate bobbing
        d = Vec_Unit2(actor->velocity);

        if(d > 0.005f)
        {
            bob_x = (float)sin(client.tics * 0.3250f) * d * 0.0025f;
            bob_y = (float)sin(client.tics * 0.1625f) * d * 0.0025f;
        }
    }
    else if(actor->terriantype == TT_WATER_SURFACE ||
        actor->terriantype == TT_WATER_UNDER)
    {
        bob_x = (float)sin(client.tics * 0.035f) * 0.0150f;
        bob_y = (float)sin(client.tics * 0.025f) * 0.0107f;
    }

    // set view origin
    Vec_Set3(org,
        actor->origin[0],
        actor->origin[1] + (actor->object.centerheight + actor->object.viewheight),
        actor->origin[2]);

    // setup projection matrix
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(video_width, video_height, cl_fov.value, 0.1f);

    // setup modelview matrix
    dglMatrixMode(GL_MODELVIEW);
    Mtx_Identity(mtx);
    Vec_SetQuaternion(yaw, -actor->yaw + M_PI - bob_y, 0, 1, 0);
    Vec_SetQuaternion(pitch, actor->pitch + bob_x, 1, 0, 0);
    Vec_SetQuaternion(roll, cam_roll, 0, 0, 1);
    Vec_MultQuaternion(vroll, yaw, roll);
    Vec_MultQuaternion(rot, vroll, pitch);
    Mtx_ApplyRotation(rot, mtx);
    Mtx_ApplyToVector(mtx, org, pos);
    Mtx_AddTranslation(mtx, -pos[0], -pos[1], -pos[2]);

    // load view matrix
    dglLoadMatrixf(mtx);
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
    float lerp;

    if(animstate != NULL)
    {
        dglPushMatrix();

        anim = animstate->anim;
        prevanim = animstate->prevanim;
        frame = animstate->frame;
        nextframe = animstate->nextframe;
        lerp = animstate->lerptime;

        if(anim)
        {
            if(frame < (int)anim->numframes)
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

                if(prevanim == NULL)
                {
                    Vec_Copy4(rot_cur, r1);
                    Vec_Copy4(rot_next, r2);
                    Vec_Copy3(pos_cur, t1);
                    Vec_Copy3(pos_next, t2);
                }
                else
                {
                    vec4_t r3, r4;
                    vec3_t t3, t4;
                    int prevframe;
                    int prevnextframe;

                    prevframe = animstate->prevframe;
                    prevnextframe = animstate->prevnextframe;

                    Mdl_GetAnimRotation(r3, prevanim, nodenum, prevframe);
                    Mdl_GetAnimRotation(r4, prevanim, nodenum, prevnextframe);
                    Mdl_GetAnimTranslation(t3, prevanim, nodenum, prevframe);
                    Mdl_GetAnimTranslation(t4, prevanim, nodenum, prevnextframe);

                    Vec_Slerp(rot_cur, lerp, r3, r1);
                    Vec_Slerp(rot_next, lerp, r4, r2);
                    Vec_Lerp3(pos_cur, lerp, t3, t1);
                    Vec_Lerp3(pos_next, lerp, t4, t2);
                }

                Vec_Slerp(rot, lerp, rot_cur, rot_next);
                Vec_Lerp3(pos, lerp, pos_cur, pos_next);
                Mtx_ApplyRotation(rot, mtx);
                Mtx_AddTranslation(mtx, pos[0], pos[1], pos[2]);
                dglMultMatrixf(mtx);
            }
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
                {
                    texturepath = textures[i];
                }
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
// R_DrawObject
//

void R_DrawObject(object_t *object)
{
    kmodel_t *model;
    int var;

    if(!(model = Mdl_Load(object->mdlpath)))
    {
        return;
    }

    // TODO - temp
    if(object->type == OT_MINIPORTAL ||
        object->type == OT_WATER)
    {
        var = 0;
    }
    else
    {
        var = object->variant;
    }

    R_TraverseDrawNode(model, &model->nodes[0],
        object->textureswaps, var, NULL);
}

//
// R_DrawInstances
//

static void R_DrawInstances(void)
{
    if(g_currentmap != NULL && !showcollision)
    {
        unsigned int i;
        unsigned int j;

        for(i = 0; i < g_currentmap->numinstances; i++)
        {
            instance_t *inst = &g_currentmap->instances[i];

            if(inst == NULL || inst->statics == NULL)
            {
                continue;
            }

            for(j = 0; j < inst->numstatics; j++)
            {
                object_t *obj;
                
                obj = &inst->statics[j];

                if(obj == NULL)
                {
                    continue;
                }

                dglPushMatrix();
                dglMultMatrixf(obj->matrix);

                if(R_FrustrumTestBox(obj->box))
                {
                    R_DrawObject(obj);
                }

                dglPopMatrix();

                if(showbbox)
                {
                    byte r, g, b;

                    r = 255; g = 255; b = 0;
                    R_DrawBoundingBox(obj->box, r, g, b);
                }
            }

            if(inst->specials == NULL)
            {
                continue;
            }

            for(j = 0; j < inst->numspecials; j++)
            {
                object_t *obj;
                kbool item = false;
                
                obj = &inst->specials[j];

                if(obj == NULL)
                {
                    continue;
                }

                item = (Obj_GetClassType(obj) == OC_PICKUP);
                
                dglPushMatrix();
                dglMultMatrixf(obj->matrix);

                if(R_FrustrumTestBox(obj->box))
                {
                    if(item)
                    {
                        dglPushMatrix();
                        dglRotatef((float)(client.tics % 360), 0, 1, 0);
                    }

                    R_DrawObject(obj);

                    if(item)
                    {
                        dglPopMatrix();
                    }
                }

                dglPopMatrix();

                if(showbbox)
                {
                    byte r, g, b;
                    
                    r = 0; g = 255; b = 0;
                    R_DrawBoundingBox(obj->box, r, g, b);
                }
            }
        }
    }
}

//
// R_DrawViewWeapon
//

void R_DrawViewWeapon(weapon_t *weapon)
{
    kmodel_t *model;
    mtx_t mtx_transform;
    mtx_t mtx_final;
    mtx_t mtx_rot;
    mtx_t mtx_pos;
    mtx_t mtx_flip;
    vec4_t yaw;
    vec4_t pitch;
    vec4_t aim;

    if(!(model = weapon->model))
    {
        return;
    }

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(video_width, video_height, 45, 32);
    dglMatrixMode(GL_MODELVIEW);
    Mtx_Identity(mtx_pos);
    Mtx_Identity(mtx_flip);
    Mtx_Scale(mtx_flip, -1, 1, 1);
    Mtx_Transpose(mtx_pos);
    Mtx_Multiply(mtx_transform, mtx_pos, mtx_flip);
    Vec_SetQuaternion(yaw, weapon->yaw, 0, 0, 1);
    Vec_SetQuaternion(pitch, weapon->pitch, 1, 0, 0);
    Vec_MultQuaternion(aim, pitch, yaw);
    Mtx_ApplyRotation(aim, mtx_rot);
    Mtx_ApplyVector(mtx_transform, weapon->origin);
    Mtx_Multiply(mtx_final, mtx_rot, mtx_transform);
    dglLoadMatrixf(mtx_final);

    R_TraverseDrawNode(model, &model->nodes[0],
        NULL, 0, &weapon->animstate);
}

//
// R_DrawFrame
//

void R_DrawFrame(void)
{
    GL_ClearView(0xFF3f3f3f);
    
    R_SetupViewFrame(&client.localactor);
    R_SetupClipFrustum();

    dglCullFace(GL_BACK);
    dglEnable(GL_DEPTH_TEST);
    dglDisableClientState(GL_COLOR_ARRAY);

    R_DrawInstances();

    if(showcollision)
    {
        R_DrawCollision();
    }

    dglCullFace(GL_FRONT);

    // TODO - TEMP
    R_DrawViewWeapon(&weapons[wp_pulse]);

    dglEnableClientState(GL_COLOR_ARRAY);
    dglDisable(GL_DEPTH_TEST);

    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_BLEND, false);
    GL_SetState(GLSTATE_ALPHATEST, false);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);

    GL_SetOrtho();

    // underwater overlay
    if(client.localactor.flags & AF_SUBMERGED)
    {
        GL_SetState(GLSTATE_TEXTURE0, false);
        GL_SetState(GLSTATE_BLEND, true);
        dglColor4ub(0, 36, 190, 160);
        dglRectf(0, 0, (float)video_width, (float)video_height);
        GL_SetState(GLSTATE_TEXTURE0, true);
        GL_SetState(GLSTATE_BLEND, false);
    }

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

    Mdl_Init();
}
