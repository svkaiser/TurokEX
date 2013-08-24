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
// DESCRIPTION: FX Rendering
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "gl.h"
#include "render.h"
#include "fx.h"
#include "client.h"

kexCvar cvarRenderFxTexture("r_fxtexture", CVF_BOOL|CVF_CONFIG, "1", "TODO");

extern int numFxCount;

typedef struct
{
    fx_t *fx;
} fxDisplay_t;

#define MAX_FX_DISPLAYS 2048

static fxDisplay_t fxDisplayList[MAX_FX_DISPLAYS];

//
// SortSprites
//

static int SortSprites(const void *a, const void *b)
{
    fx_t *xa = ((fxDisplay_t*)a)->fx;
    fx_t *xb = ((fxDisplay_t*)b)->fx;

    return (int)(xb->dist - xa->dist);
}

//
// R_DrawFX
//

void R_DrawFX(void)
{
    int fxDisplayNum;
    int i;

    numFxCount = 0;

    if(client.playerActor == NULL)
        return;

    GL_SetState(GLSTATE_BLEND, true);
    GL_SetState(GLSTATE_ALPHATEST, true);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);

    dglAlphaFunc(GL_GEQUAL, 0.01f);

    if(!bWireframe)
        dglEnableClientState(GL_COLOR_ARRAY);

    memset(fxDisplayList, 0, sizeof(fxDisplay_t) * MAX_FX_DISPLAYS);

    bool bShowTexture = cvarRenderFxTexture.GetBool();

    for(fxRover = fxRoot.next, fxDisplayNum = 0; fxRover != &fxRoot; fxRover = fxRover->next)
    {
        if(fxDisplayNum >= MAX_FX_DISPLAYS)
            break;

        if(fxRover == NULL)
            continue;

        if(fxRover->restart > 0)
            continue;

        fxDisplayList[fxDisplayNum++].fx = fxRover;
    }

    qsort(fxDisplayList, fxDisplayNum, sizeof(fxDisplay_t), SortSprites);
    numFxCount = fxDisplayNum;

    for(i = 0; i < fxDisplayNum; i++)
    {
        mtx_t mtx;
        mtx_t scalemtx;
        mtx_t finalmtx;
        fx_t *fx;
        float scale;
        float w;
        float h;
        float y;
        fxinfo_t *fxinfo;
        texture_t *texture;

        fx = fxDisplayList[i].fx;
        fxinfo = fx->info;

        if(fxinfo == NULL)
            continue;

        if(fx->bStale)
            continue;

        scale = fx->scale * 0.01f;

        Mtx_IdentityZ(scalemtx, fx->rotation_offset + DEG2RAD(180));

        if(fxinfo->screen_offset_x != 0 || fxinfo->screen_offset_y != 0)
        {
            vec3_t svec;

            Vec_Set3(svec, fxinfo->screen_offset_x, fxinfo->screen_offset_y, 0);
            Mtx_ApplyVector(scalemtx, svec);
        }

        Mtx_Scale(scalemtx, scale, scale, scale);

        switch(fxinfo->drawtype)
        {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            Mtx_IdentityY(mtx, DEG2RAD(90));
            GL_SetState(GLSTATE_CULL, false);
            break;
        case VFX_DRAWBILLBOARD:
            {
                vec4_t rot;

                Vec_SetQuaternion(rot, client.player->camera->angles[0], 0, 1, 0);
                Mtx_ApplyRotation(rot, mtx);
                GL_SetState(GLSTATE_CULL, true);
            }
            break;
        default:
            Mtx_ApplyRotation(client.player->camera->rotation, mtx);
            GL_SetState(GLSTATE_CULL, true);
            break;
        }

        texture = fx->textures[fx->frame];

        y = fx->origin[1];

        if(fxinfo->bOffsetFromFloor && fx->plane != NULL)
        {
            float dist = Plane_GetDistance(fx->plane, fx->origin) + 3.42f;

            if(dist >= fx->origin[1])
                y = dist;

            if(fxinfo->drawtype == VFX_DRAWBILLBOARD)
                y += (float)texture->height;
        }

        Mtx_MultiplyRotation(finalmtx, scalemtx, mtx);
        Mtx_AddTranslation(finalmtx, fx->origin[0], y, fx->origin[2]);

        dglPushMatrix();
        dglMultMatrixf(finalmtx);

        w = (float)texture->origwidth;
        h = (float)texture->height;

        GL_Vertex(-w, -h, 0, 0, 1, 0, 0, 0,
            fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex(w, -h, 0, 1, 1, 0, 0, 0,
            fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex(-w, h, 0, 0, 0, 0, 0, 0,
            fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex(w, h, 0, 1, 0, 0, 0, 0,
            fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);

        GL_Triangle(0, 1, 2);
        GL_Triangle(2, 1, 3);

        GL_SetState(GLSTATE_DEPTHTEST, fxinfo->bDepthBuffer);

        if(bWireframe)
        {
            GL_BindTextureName("textures/white.tga");
            dglColor4ub(192, 0, 0, 255);
        }
        else {
            if(bShowTexture)
                GL_BindTexture(texture);
            else
                GL_BindTextureName("textures/white.tga");
        }

        GL_DrawElements2();
        dglPopMatrix();

        if(showorigin)
        {
            if(sqrt(fx->rotation[0]*fx->rotation[0]+
                fx->rotation[1]*fx->rotation[1]+
                fx->rotation[2]*fx->rotation[2]+
                fx->rotation[3]*fx->rotation[3]) != 0.0f)
            {
                mtx_t mtx;
                vec3_t vec;

                Vec_Set3(vec, 0, 0, 0);
                Mtx_ApplyRotation(fx->rotation, mtx);
                Mtx_AddTranslation(mtx, fx->origin[0], fx->origin[1], fx->origin[2]);
                dglPushMatrix();
                dglMultMatrixf(mtx);
                R_DrawOrigin(vec, 8.0f);
                dglPopMatrix();
            }
            else
                R_DrawOrigin(fx->origin, 8.0f);
        }
    }

    GL_SetState(GLSTATE_DEPTHTEST, true);

    if(!bWireframe)
        dglDisableClientState(GL_COLOR_ARRAY);
}
