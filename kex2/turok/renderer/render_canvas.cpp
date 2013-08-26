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
// DESCRIPTION: Canvas System
//
//-----------------------------------------------------------------------------

#include "SDL_opengl.h"
#include "common.h"
#include "gl.h"
#include "actor.h"
#include "render.h"
#include "client.h"

void R_SetupClipFrustum(void);
kbool R_FrustumTestPlane(plane_t *plane);

//
// Canvas_SetDrawColor
//

void Canvas_SetDrawColor(canvas_t *canvas, byte r, byte g, byte b)
{
    int i;

    for(i = 0; i < 4; i++)
    {
        canvas->drawColor[i][0] = r;
        canvas->drawColor[i][1] = g;
        canvas->drawColor[i][2] = b;
    }
}

//
// Canvas_SetDrawAlpha
//

void Canvas_SetDrawAlpha(canvas_t *canvas, byte a)
{
    int i;

    for(i = 0; i < 4; i++)
        canvas->drawColor[i][3] = a;
}

//
// Canvas_SetDrawScale
//

void Canvas_SetDrawScale(canvas_t *canvas, float scale)
{
	canvas->scale = scale;
}

//
// Canvas_SetTextureTile
//

void Canvas_SetTextureTile(canvas_t *canvas,
                           float u1, float u2, float t1, float t2)
{
    canvas->drawCoord[0] = u1;
    canvas->drawCoord[1] = u2;
    canvas->drawCoord[2] = t1;
    canvas->drawCoord[3] = t2;
}

//
// Canvas_SetFont
//

void Canvas_SetFont(canvas_t *canvas, font_t *font)
{
    canvas->font = font;
}

//
// Canvas_DrawTile
//

void Canvas_DrawTile(canvas_t *canvas, texture_t *texture,
                     float x, float y, float w, float h)
{
    float u1    = canvas->drawCoord[0];
    float u2    = canvas->drawCoord[1];
    float t1    = canvas->drawCoord[2];
    float t2    = canvas->drawCoord[3];
    byte r[4];
    byte g[4];
    byte b[4];
    byte a[4];
    int i;

    for(i = 0; i < 4; i++)
    {
        r[i] = canvas->drawColor[i][0];
        g[i] = canvas->drawColor[i][1];
        b[i] = canvas->drawColor[i][2];
        a[i] = canvas->drawColor[i][3];
    }

    if(canvas->scale <= 0.01f)
        canvas->scale = 1;

	w *= canvas->scale;
	h *= canvas->scale;

    GL_Vertex(x+0, y+0, 0, u1, t1, 0, 0, 0, r[0], g[0], b[0], a[0]);
    GL_Vertex(x+w, y+0, 0, u2, t1, 0, 0, 0, r[1], g[1], b[1], a[1]);
    GL_Vertex(x+0, y+h, 0, u1, t2, 0, 0, 0, r[2], g[2], b[2], a[2]);
    GL_Vertex(x+w, y+h, 0, u2, t2, 0, 0, 0, r[3], g[3], b[3], a[3]);
    GL_Triangle(0, 1, 2);
    GL_Triangle(2, 1, 3);
    GL_BindTexture(texture);
    GL_DrawElements2();
}

//
// Canvas_DrawFixedTile
//

void Canvas_DrawFixedTile(canvas_t *canvas, texture_t *texture,
                     float x, float y)
{
    float ratiox;
    float ratioy;
    float rx;
    float ry;
    float rw;
    float rh;

    ratiox = (float)FIXED_WIDTH / sysMain.VideoWidth();
    ratioy = (float)FIXED_HEIGHT / sysMain.VideoHeight();
    rx = x / ratiox;
    rw = (float)texture->width / ratiox;
    ry = y / ratioy;
    rh = (float)texture->height / ratioy;

    Canvas_DrawTile(canvas, texture, rx, ry, rw, rh);
}

//
// Canvas_DrawString
//

void Canvas_DrawString(canvas_t *canvas, const char *string,
                       float x, float y, kbool center)
{
    float width;
    float height;
    byte r[4];
    byte g[4];
    byte b[4];
    byte a[4];
    byte old_r, old_g, old_b;
    int tri;
    unsigned int i;
    unsigned int len;
    kbool enterTag;

    if(canvas->font == NULL)
        return;

    if(canvas->scale <= 0.01f)
        canvas->scale = 1;

    if(center)
        x -= Font_StringWidth(canvas->font, string, canvas->scale, 0) * 0.5f;

    width       = (float)canvas->font->width;
    height      = (float)canvas->font->height;

    for(i = 0; i < 4; i++)
    {
        r[i] = canvas->drawColor[i][0];
        g[i] = canvas->drawColor[i][1];
        b[i] = canvas->drawColor[i][2];
        a[i] = canvas->drawColor[i][3];
    }

    tri         = 0;
    enterTag    = false;
    old_r       = r[0];
    old_g       = g[0];
    old_b       = b[0];
    len         = strlen(string);

    GL_BindTextureName(canvas->font->texture);

    // TODO - TEMP
    //dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    for(i = 0; i < len; i++)
    {
        char ch         = string[i];
        atlas_t *atlas  = &canvas->font->atlas[ch];
        float vx1       = x;
        float vy1       = y;
        float vx2       = vx1 + atlas->w * canvas->scale;
        float vy2       = vy1 + atlas->h * canvas->scale;
        float tx1       = (atlas->x / width) + 0.001f;
        float tx2       = (tx1 + atlas->w / width) - 0.002f;
        float ty1       = (atlas->y / height);
        float ty2       = (ty1 + atlas->h / height);
        char *check     = (char*)string+i;

        if(!strncmp(check, "<color=>", 7))
        {
            unsigned int rgbStart;
            char val[3];
            int k;

            enterTag = true;
            rgbStart = i+7;

            for(i = rgbStart; i < len; i++)
            {
                if(string[i] == ',')
                {
                    byte cr;
                    strncpy(val, string+rgbStart, i-rgbStart);
                    cr = atoi(val);
                    for(k = 0; k < 4; k++)
                        r[k] = cr;

                    break;
                }
            }

            if(i >= (len-1))
                return;

            rgbStart = i+1;

            for(i = rgbStart; i < len; i++)
            {
                if(string[i] == ',')
                {
                    byte cg;
                    strncpy(val, string+rgbStart, i-rgbStart);
                    cg = atoi(val);
                    for(k = 0; k < 4; k++)
                        g[k] = cg;
                    break;
                }
            }

            if(i >= (len-1))
                return;

            rgbStart = i+1;

            for(i = rgbStart; i < len; i++)
            {
                if(string[i] == '>')
                {
                    byte cb;
                    strncpy(val, string+rgbStart, i-rgbStart);
                    cb = atoi(val);
                    for(k = 0; k < 4; k++)
                        b[k] = cb;
                    break;
                }
            }

            continue;
        }
        else if(!strncmp(check, "</color>", 7))
        {
            r[0] = old_r;
            g[0] = old_g;
            b[0] = old_b;

            i += 7;
            continue;
        }

        GL_Vertex(vx1, vy1, 0, tx1, ty1, 0, 0, 0, r[0], g[0], b[0], a[0]);
        GL_Vertex(vx2, vy1, 0, tx2, ty1, 0, 0, 0, r[1], g[1], b[1], a[1]);
        GL_Vertex(vx1, vy2, 0, tx1, ty2, 0, 0, 0, r[2], g[2], b[2], a[2]);
        GL_Vertex(vx2, vy2, 0, tx2, ty2, 0, 0, 0, r[3], g[3], b[3], a[3]);

        GL_Triangle(0+tri, 1+tri, 2+tri);
        GL_Triangle(2+tri, 1+tri, 3+tri);

        x += atlas->w * canvas->scale;
        tri += 4;
    }

    GL_DrawElements2();
}

//
// Canvas_DrawFixedString
//

void Canvas_DrawFixedString(canvas_t *canvas, const char *string, float x, float y, kbool center)
{
    float ratiox;
    float ratioy;
    float rx;
    float ry;
    float oldScale;

    ratiox = (float)FIXED_WIDTH / sysMain.VideoWidth();
    ratioy = (float)FIXED_HEIGHT / sysMain.VideoHeight();
    rx = x / ratiox;
    ry = y / ratioy;

    oldScale = canvas->scale;
    canvas->scale /= ratiox;
    Canvas_DrawString(canvas, string, rx, ry, center);
    canvas->scale = oldScale;
}

//
// Canvas_DrawActor
//

void Canvas_DrawActor(canvas_t *canvas, gActor_t *actor)
{
    mtx_t mtx;

    if(actor->bHidden)
        return;

    Actor_UpdateTransform(actor);

    GL_SetState(GLSTATE_LIGHTING, true);

    // setup projection matrix
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(sysMain.VideoWidth(), sysMain.VideoHeight(), 45, 32, -1);

    // setup modelview matrix
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    Mtx_IdentityY(mtx, DEG2RAD(-90));

    if(actor->model)
    {
        dglPushMatrix();
        dglMultMatrixf(actor->matrix);
        dglPushMatrix();
        dglMultMatrixf(mtx);

        if(bWireframe)
            dglColor4ub(192, 192, 192, 255);

        R_TraverseDrawNode(actor, &actor->model->nodes[0], &actor->animState);

        dglPopMatrix();
        dglPopMatrix();
    }

    GL_SetState(GLSTATE_LIGHTING, false);

    if(showorigin)
    {
        vec3_t vec;

        Vec_Set3(vec, 0, 0, 0);

        dglPushMatrix();
        dglMultMatrixf(actor->matrix);
        R_DrawOrigin(vec, 32.0f);
        dglPopMatrix();
    }
}

//
// DrawPlaneOutline
//

static void DrawPlaneOutline(plane_t *plane, int idx, float zoom)
{
    rcolor outlineColor;
    vec3_t pos;
    kbool drawOk = false;
    plane_t * pl;

    if(!plane)
        return;

    pl = plane->link[idx];

    if(!pl || plane->area_id != pl->area_id)
    {
        gArea_t *area;
        drawOk = true;

        area = Map_GetArea(plane);

        if(area->triggerSound)
            outlineColor = RGBA(255, 0, 160, 255);
        else if(plane->flags & CLF_ONESIDED)
            outlineColor = RGBA(128, 96, 64, 255);
        else if(area->flags & AAF_WATER)
            outlineColor = RGBA(0, 0, 255, 255);
        else if(area->flags & AAF_TELEPORT)
            outlineColor = RGBA(255, 0, 160, 255);
        else if(!pl)
            outlineColor = RGBA(0, 255, 0, 255);
        else
        {
            area = Map_GetArea(pl);

            if(area->triggerSound)
                outlineColor = RGBA(255, 0, 160, 255);
            else if(pl->flags & CLF_ONESIDED)
                outlineColor = RGBA(128, 96, 64, 255);
            else if(area->flags & AAF_WATER)
                outlineColor = RGBA(0, 0, 255, 255);
            else if(area->flags & AAF_TELEPORT)
                outlineColor = RGBA(255, 0, 160, 255);
            else if(pl->flags & (CLF_FRONTNOCLIP|CLF_SLOPETEST|CLF_CLIMB))
                outlineColor = RGBA(224, 224, 224, 255);
            else
                drawOk = false;
        }
    }

    if(drawOk)
    {
        float c[4];

        dglGetColorf(outlineColor, c);

        if(plane->flags & CLF_CHECKHEIGHT)
        {
            c[3] = 1.0f - 
                ((float)fabs(client.playerActor->origin[1] -
                Plane_GetDistance(plane, client.playerActor->origin)) / 512);

            if(c[3] < 0) c[3] = 0;
            if(c[3] > 1) c[3] = 1;
        }

        dglColor4fv(c);

        Vec_Sub(pos, plane->points[(idx+0)%3], client.playerActor->origin);
        dglVertex3f(pos[0], zoom, pos[2]);
        Vec_Sub(pos, plane->points[(idx+1)%3], client.playerActor->origin);
        dglVertex3f(pos[0], zoom, pos[2]);
    }
}

//
// RecursiveDrawPlaneOutline
//

static void RecursiveDrawPlaneOutline(plane_t *plane, float zoom)
{
    plane_t *p;
    float dx, dz;

    p = plane;

    dx = (plane->points[0][0] + plane->points[1][0] + plane->points[2][0]) / 3;
    dz = (plane->points[0][2] + plane->points[1][2] + plane->points[2][2]) / 3;

    dx = client.playerActor->origin[0] - dx;
    dz = client.playerActor->origin[2] - dz;

    if(dx*dx+dz*dz >= 4194304.0f)
        return;

    while(1)
    {
        p->flags |= CLF_MAPPED;

        if(p->link[0] && !(p->link[0]->flags & CLF_MAPPED))
            RecursiveDrawPlaneOutline(p->link[0], zoom);
        
        DrawPlaneOutline(p, 0, zoom);

        if(p->link[1] && !(p->link[1]->flags & CLF_MAPPED))
            RecursiveDrawPlaneOutline(p->link[1], zoom);

        DrawPlaneOutline(p, 1, zoom);
        DrawPlaneOutline(p, 2, zoom);

        if(p->link[2] == NULL)
            break;

        p = p->link[2];

        if(p->flags & CLF_MAPPED)
            break;
    }
}

//
// RecursiveClearMappedFlags
//

static void RecursiveClearMappedFlags(plane_t *plane)
{
    plane_t *p = plane;

    do
    {
        p->flags &= ~CLF_MAPPED;

        if(p->link[0] && p->link[0]->flags & CLF_MAPPED)
            RecursiveClearMappedFlags(p->link[0]);

        if(p->link[1] && p->link[1]->flags & CLF_MAPPED)
            RecursiveClearMappedFlags(p->link[1]);

        p = p->link[2];

    } while(p && p->flags & CLF_MAPPED);
}

//
// Canvas_DrawPlaneOutlines
//

void Canvas_DrawPlaneOutlines(canvas_t *canvas)
{
    gActor_t *camera;
    plane_t *plane;
    float zoom;

    camera = client.player->actor;
    plane = Map_IndexToPlane(camera->plane);

    GL_BindTextureName("textures/white.tga");
    dglEnable(GL_ALPHA_TEST);
    GL_SetState(GLSTATE_DEPTHTEST, 0);
    GL_SetState(GLSTATE_BLEND, 1);
    dglDisable(GL_FOG);

    // setup projection matrix
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
    Mtx_ViewFrustum(sysMain.VideoWidth(), sysMain.VideoHeight(), 45, 32, -1);

    zoom = -2048;

    // setup modelview matrix
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglRotatef(90, 1, 0, 0);
    dglRotatef(180, 0, 1, 0);

    dglColor4ub(255, 255, 0, 255);
    dglBegin(GL_LINES);
    dglVertex3f(0, zoom, 0);
    dglVertex3f(-16.0f, zoom, -48.0f);
    dglVertex3f(0, zoom, 0);
    dglVertex3f(16.0f, zoom, -48.0f);
    dglEnd();

    dglColor4ub(0, 255, 0, 255);
    dglPushMatrix();
    dglRotatef(RAD2DEG(-camera->angles[0]), 0, 1, 0);
    dglBegin(GL_LINES);
    RecursiveDrawPlaneOutline(plane, zoom);
    dglEnd();
    dglPopMatrix();

    dglDisable(GL_ALPHA_TEST);
    GL_SetState(GLSTATE_BLEND, 0);
    GL_SetState(GLSTATE_DEPTHTEST, 1);
    dglEnable(GL_FOG);

    RecursiveClearMappedFlags(plane);
}
