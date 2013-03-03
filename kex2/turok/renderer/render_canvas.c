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
#include "render.h"

//
// Canvas_SetDrawColor
//

void Canvas_SetDrawColor(canvas_t *canvas, byte r, byte g, byte b)
{
    canvas->drawColor[0] = r;
    canvas->drawColor[1] = g;
    canvas->drawColor[2] = b;
}

//
// Canvas_SetDrawAlpha
//

void Canvas_SetDrawAlpha(canvas_t *canvas, byte a)
{
    canvas->drawColor[3] = a;
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
    byte r      = canvas->drawColor[0];
    byte g      = canvas->drawColor[1];
    byte b      = canvas->drawColor[2];
    byte a      = canvas->drawColor[3];

    if(canvas->scale <= 0)
        canvas->scale = 1;

    GL_Vertex(x+0, y+0, 0, u1, t1, 0, 0, 0, r, g, b, a);
    GL_Vertex(x+w, y+0, 0, u2, t1, 0, 0, 0, r, g, b, a);
    GL_Vertex(x+0, y+h, 0, u1, t2, 0, 0, 0, r, g, b, a);
    GL_Vertex(x+w, y+h, 0, u2, t2, 0, 0, 0, r, g, b, a);
    GL_Triangle(0, 1, 2);
    GL_Triangle(2, 1, 3);
    GL_BindTexture(texture);
    GL_DrawElements2();
}

//
// Canvas_DrawString
//

void Canvas_DrawString(canvas_t *canvas, const char *string, float x, float y)
{
    float width;
    float height;
    byte r, g, b, a;
    byte old_r, old_g, old_b;
    int tri;
    unsigned int i;
    unsigned int len;
    kbool enterTag;

    if(canvas->font == NULL)
        return;

    if(canvas->scale <= 0)
        canvas->scale = 1;

    width       = (float)canvas->font->width;
    height      = (float)canvas->font->height;
    r           = canvas->drawColor[0];
    g           = canvas->drawColor[1];
    b           = canvas->drawColor[2];
    a           = canvas->drawColor[3];
    tri         = 0;
    enterTag    = false;
    old_r       = r;
    old_g       = g;
    old_b       = b;
    len         = strlen(string);

    GL_BindTextureName(canvas->font->texture);

    // TODO - TEMP
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

            enterTag = true;
            rgbStart = i+7;

            for(i = rgbStart; i < len; i++)
            {
                if(string[i] == ',')
                {
                    strncpy(val, string+rgbStart, i-rgbStart);
                    r = atoi(val);
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
                    strncpy(val, string+rgbStart, i-rgbStart);
                    g = atoi(val);
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
                    strncpy(val, string+rgbStart, i-rgbStart);
                    b = atoi(val);
                    break;
                }
            }

            continue;
        }
        else if(!strncmp(check, "</color>", 7))
        {
            r = old_r;
            g = old_g;
            b = old_b;

            i += 7;
            continue;
        }

        GL_Vertex(vx1, vy1, 0, tx1, ty1, 0, 0, 0, r, g, b, a);
        GL_Vertex(vx2, vy1, 0, tx2, ty1, 0, 0, 0, r, g, b, a);
        GL_Vertex(vx1, vy2, 0, tx1, ty2, 0, 0, 0, r, g, b, a);
        GL_Vertex(vx2, vy2, 0, tx2, ty2, 0, 0, 0, r, g, b, a);

        GL_Triangle(0+tri, 1+tri, 2+tri);
        GL_Triangle(2+tri, 1+tri, 3+tri);

        x += atlas->w * canvas->scale;
        tri += 4;
    }

    GL_DrawElements2();
}

