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
// DESCRIPTION: Camera code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"

//
// R_RenderCameraView
//

void R_RenderCameraView(void)
{
    vec3_t org;
    vec3_t pos;
    vec4_t yaw;
    vec4_t pitch;
    vec4_t roll;
    vec4_t vroll;
    vec4_t rot;
    mtx_t mtx;
    gActor_t *camera;
    float zfar;

    camera = client.player->camera;

    if(camera == NULL)
        return;

    // set view origin
    Vec_Set3(org, camera->origin[0], camera->origin[1], camera->origin[2]);

    // setup projection matrix
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();

    // TODO - TEMP
    zfar = -1;
    if(dglIsEnabled(GL_FOG))
        dglGetFloatv(GL_FOG_END, &zfar);

    Mtx_ViewFrustum(video_width, video_height, cvarClientFOV.GetFloat(), 0.1f, zfar);

    // setup modelview matrix
    dglMatrixMode(GL_MODELVIEW);
    Mtx_Identity(mtx);
    Vec_SetQuaternion(yaw, -camera->angles[0] + M_PI, 0, 1, 0);
    Vec_SetQuaternion(pitch, camera->angles[1], 1, 0, 0);
    Vec_SetQuaternion(roll, camera->angles[2], 0,
        (float)sin(camera->angles[1]), (float)cos(camera->angles[1]));
    Vec_MultQuaternion(vroll, yaw, roll);
    Vec_MultQuaternion(rot, vroll, pitch);
    Mtx_ApplyRotation(rot, mtx);
    Vec_TransformToWorld(mtx, org, pos);
    Mtx_Copy(camera->matrix, mtx);
    Mtx_AddTranslation(mtx, -pos[0], -pos[1], -pos[2]);

    // load view matrix
    dglLoadMatrixf(mtx);
}
