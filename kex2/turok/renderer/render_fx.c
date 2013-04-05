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

//
// R_DrawFX
//

void R_DrawFX(void)
{
    if(client.playerActor == NULL)
        return;

    GL_SetState(GLSTATE_CULL, true);
    GL_SetState(GLSTATE_BLEND, true);
    GL_SetState(GLSTATE_ALPHATEST, true);
    GL_SetState(GLSTATE_TEXGEN_S, false);
    GL_SetState(GLSTATE_TEXGEN_T, false);

    dglEnableClientState(GL_COLOR_ARRAY);

    for(fxRover = fxRoot.next; fxRover != &fxRoot; fxRover = fxRover->next)
    {
        // TODO - PLACEHOLDER/TESTING
        mtx_t mtx;
        mtx_t scalemtx;
        mtx_t finalmtx;
        fx_t *fx;
        float scale;
        int lifetime;
        int alpha;
        fxinfo_t *fxinfo;
        texture_t *texture;

        if(fxRover == NULL)
            continue;

        fx = fxRover;
        fxinfo = fx->info;

        if(fxinfo->bOffsetFromFloor)
        {
            float dist = Plane_GetDistance(fx->plane, fx->origin) + 3.42f;

            if(dist >= fx->origin[1])
                fx->origin[1] = dist;
        }

        scale = fx->scale * 0.01f;

        Mtx_IdentityZ(scalemtx, fx->rotation_offset + DEG2RAD(90));
        Mtx_Scale(scalemtx, scale, scale, scale);

        switch(fxinfo->drawtype)
        {
        case VFX_DRAWFLAT:
            Mtx_IdentityY(mtx, DEG2RAD(90));
            break;
        default:
            Mtx_ApplyRotation(client.playerActor->rotation, mtx);
            break;
        }

        texture = fx->textures[fx->frame];

        Mtx_MultiplyRotation(finalmtx, scalemtx, mtx);
        Mtx_AddTranslation(finalmtx, fx->origin[0],
            fx->origin[1] + texture->origheight, fx->origin[2]);

        dglPushMatrix();
        dglMultMatrixf(finalmtx);

        GL_Vertex(-(float)texture->origwidth, -(float)texture->height,
            0, 0, 0, 0, 0, 0, fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex((float)texture->origwidth, -(float)texture->height,
            0, 1, 0, 0, 0, 0, fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex(-(float)texture->origwidth, (float)texture->height,
            0, 0, 1, 0, 0, 0, fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);
        GL_Vertex((float)texture->origwidth, (float)texture->height,
            0, 1, 1, 0, 0, 0, fx->color1[0], fx->color1[1], fx->color1[2], fx->color1[3]);

        GL_Triangle(0, 1, 2);
        GL_Triangle(2, 1, 3);

        GL_BindTexture(texture);
        GL_DrawElements2();
        dglPopMatrix();

        if(fx->bAnimate)
        {
            if(fx->frametime < client.time && fxinfo->numTextures > 1)
            {
                fx->frametime = client.time + fxinfo->animspeed;
                fx->frame = (fx->frame + 1) % (fxinfo->numTextures - 1);

                if((fx->frame + 1) == (fxinfo->numTextures-1) &&
                    fxinfo->animtype == VFX_ANIMONETIME)
                {
                    fx->bAnimate = false;
                }
            }
        }

        fx->scale = ((fx->scale * fx->scale_dest) - fx->scale) *
            (2 * client.runtime) + fx->scale;

        lifetime = (fxinfo->lifetime.value - (int)fx->lifetime);

        alpha = fx->color1[3];

        if(lifetime < fxinfo->fadein_time)
        {
            alpha += (255 / (fxinfo->fadein_time + 1)) >> 2;

            if(alpha > 0xff)
                alpha = 0xff;
        }

        if(fx->lifetime < fxinfo->fadeout_time)
        {
            alpha = (int)(255 * fx->lifetime / (fxinfo->fadeout_time + 1));

            if(alpha < 0)
                alpha = 0;
        }

        fx->color1[3] = alpha;
        fx->color2[3] = alpha;

        fx->lifetime -= (client.runtime * 16);
        if(fx->lifetime < 0)
            FX_Kill(fx);
    }

    dglDisableClientState(GL_COLOR_ARRAY);
}
