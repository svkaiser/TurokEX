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
// R_DrawObject
//

void R_DrawObject(object_t *object)
{
    kmodel_t *model;

    if(!(model = Mdl_Load(object->mdlpath)))
    {
        return;
    }

    dglCullFace(GL_BACK);
    dglEnable(GL_DEPTH_TEST);
    dglDisableClientState(GL_COLOR_ARRAY);

    Mdl_TraverseDrawNode(model, &model->nodes[0],
        object->textureswaps);

    dglEnableClientState(GL_COLOR_ARRAY);
    dglDisable(GL_DEPTH_TEST);
    dglCullFace(GL_FRONT);
    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_TEXTURE0, true);
    GL_SetState(GLSTATE_BLEND, false);
    dglDisable(GL_ALPHA_TEST);
    dglDisable(GL_TEXTURE_GEN_S);
    dglDisable(GL_TEXTURE_GEN_T);
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

            if(inst == NULL)
            {
                continue;
            }

            if(inst->statics == NULL)
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
// R_DrawFrame
//

void R_DrawFrame(void)
{
    GL_ClearView(0xFF3f3f3f);
    
    R_SetupViewFrame(&client.localactor);
    R_SetupClipFrustum();
    R_DrawInstances();

    if(showcollision)
    {
        R_DrawCollision();
    }

    GL_SetOrtho();

    // underwater overlay
    if(client.localactor.terriantype == TT_WATER_UNDER)
    {
        GL_SetState(GLSTATE_TEXTURE0, false);
        GL_SetState(GLSTATE_BLEND, true);
        dglColor4ub(0, 36, 190, 160);
        dglRectf(0, 0, (float)video_width, (float)video_height);
        GL_SetState(GLSTATE_TEXTURE0, true);
        GL_SetState(GLSTATE_BLEND, false);
    }
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
