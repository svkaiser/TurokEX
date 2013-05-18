// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _OPENGL_H_
#define _OPENGL_H_

#include "shared.h"
#include "keywds.h"
#include "dgl.h"

extern int DGL_CLAMP;

typedef struct
{
    float   x;
    float   y;
    float   z;
    float   tu;
    float   tv;
    byte    r;
    byte    g;
    byte    b;
    byte    a;
    float   nx;
    float   ny;
    float   nz;
} vtx_t;

//
// dglGetColorf
//

d_inline void dglGetColorf(rcolor color, float* argb)
{
    argb[3] = (float)((color >> 24) & 0xff) / 255.0f;
    argb[2] = (float)((color >> 16) & 0xff) / 255.0f;
    argb[1] = (float)((color >> 8) & 0xff) / 255.0f;
    argb[0] = (float)(color & 0xff) / 255.0f;
}

//
// dglSetVertexColor
//

d_inline void dglSetVertexColor(vtx_t *v, rcolor c, word count)
{
    int i;

    for(i = 0; i < count; i++)
        *(rcolor*)&v[i].r = c;
}

#define GLSTATE_BLEND       0
#define GLSTATE_CULL        1
#define GLSTATE_TEXTURE0    2
#define GLSTATE_TEXTURE1    3
#define GLSTATE_TEXTURE2    4
#define GLSTATE_TEXTURE3    5
#define GLSTATE_ALPHATEST   6
#define GLSTATE_TEXGEN_S    7
#define GLSTATE_TEXGEN_T    8
#define GLSTATE_DEPTHTEST   9

#define MAX_COORD 32768.0f

//
// TEXTURES
//

typedef struct texture_s
{
    char name[MAX_FILEPATH];
    unsigned int width;
    unsigned int height;
    unsigned int origwidth;
    unsigned int origheight;
    int clampmode;
    dtexture texid;
    struct texture_s *next;
} texture_t;

void Tex_Init(void);
void Tex_Shutdown(void);
kbool Tex_IsDefault(texture_t *tex);
int Tex_PadDims(int n);
texture_t *Tex_Alloc(const char *name, byte *data,
                     int width, int height, int clampmode);
texture_t *Tex_Find(const char *name);
texture_t *Tex_CacheTextureFile(const char *name,
                                int clampmode, kbool masked);

//
// MAIN
//
void GL_SetOrtho(void);
void GL_SwapBuffers(void);
void GL_SetState(int bit, kbool enable);
void GL_SetTextureFilter(void);
void GL_BindTexture(texture_t *texture);
void GL_BindTextureName(const char *name);
void GL_SetTextureUnit(int unit, kbool enable);
void GL_SetVertexPointer(vtx_t *vtx);
void GL_Triangle(int v0, int v1, int v2);
void GL_Vertex(float x, float y, float z,
               float tu, float tv,
               float nx, float ny, float nz,
               byte r, byte g, byte b, byte a);
void GL_DrawElements(unsigned int count, vtx_t *vtx);
void GL_DrawElements2(void);
void GL_ClearView(float *clear);
void GL_Register(void);
void GL_Init(void);
dtexture GL_ScreenToTexture(void);

//
// DRAW
//
void Draw_Init(void);
void Draw_SetBigTextColor(byte r1, byte g1, byte b1, byte r2, byte g2, byte b2);
float Draw_BigText(float x, float y, byte alpha, kbool centered,
                   float scale, const char* string);
void Draw_ShadowedText(float x, float y, byte alpha, kbool centered,
                       float scale, const char* string);
void Draw_Pic(const char *pic, float x, float y, byte alpha, float scale);
void Draw_Tile(const char *pic, float x, float y,
               float tx1, float ty1, float tx2, float ty2,
               float width, float height,
               byte r, byte g, byte b, byte alpha);
float Draw_Text(float x, float y, rcolor color,
                float scale, const char* string, ...);

//
// IMAGES
//
void Img_LoadTGA(const char *name, byte **output,
                 int *width, int *height, kbool masked);

#endif