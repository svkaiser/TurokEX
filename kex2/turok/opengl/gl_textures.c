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
// DESCRIPTION: Texture management
//
//-----------------------------------------------------------------------------

#include "gl.h"
#include "common.h"
#include "zone.h"
#include "kernel.h"

texture_t *tex_hashlist[MAX_TEXTURE_HASH_LIST];

//
// Tex_HashFileName
//

static unsigned int Tex_HashFileName(const char *name)
{
    unsigned int hash   = 1315423911;
    unsigned int i      = 0;
    char *str           = (char*)name;

    for(i = 0; i < strlen(name)-1 && *str != '\0'; str++, i++)
    {
        hash ^= ((hash << 5) + toupper((int)*str) + (hash >> 2));
    }

    return hash & (MAX_TEXTURE_HASH_LIST-1);
}

//
// Tex_PadDims
//

#define MAXTEXSIZE	2048
#define MINTEXSIZE	1

int Tex_PadDims(int n)
{
    int mask = 1;
    
    while(mask < 0x40000000)
    {
        if(n == mask || (n & (mask-1)) == n)
            return mask;
        
        mask <<= 1;
    }
    return n;
}

//
// Tex_Alloc
//

texture_t *Tex_Alloc(const char *name, byte *data, int width, int height, int clampmode)
{
    texture_t *texture;
    unsigned int hash;
    unsigned int y;
    byte *pad;

    if(strlen(name) >= MAX_FILEPATH)
    {
        Com_Error("Tex_Alloc: \"%s\" is too long", name);
    }

    texture = Z_Calloc(sizeof(texture_t), PU_TEXTURE, 0);
    strcpy(texture->name, name);
    texture->origwidth = width;
    texture->origheight = height;
    texture->width = Tex_PadDims(width);
    texture->height = Tex_PadDims(height);
    texture->clampmode = clampmode;

    dglGenTextures(1, &texture->texid);
    dglBindTexture(GL_TEXTURE_2D, texture->texid);

    pad = Z_Calloc(texture->width * texture->height * 4, PU_STATIC, 0);

    for(y = 0; y < texture->origheight; y++)          
    {
        memcpy(pad + y * texture->width * 4,
            ((byte*)data) + y * texture->origwidth * 4, texture->origwidth * 4);
    }

    dglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width,
        texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (byte*)pad);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmode);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmode);

    GL_SetTextureFilter();
    Z_Free(pad);

    dglBindTexture(GL_TEXTURE_2D, 0);

    hash = Tex_HashFileName(name);
    texture->next = tex_hashlist[hash];
    tex_hashlist[hash] = texture;

    return texture;
}

//
// Tex_Find
//

texture_t *Tex_Find(const char *name)
{
    texture_t *texture;
    unsigned int hash;

    hash = Tex_HashFileName(name);

    for(texture = tex_hashlist[hash]; texture; texture = texture->next)
    {
        if(!strcmp(name, texture->name))
        {
            return texture;
        }
    }

    return NULL;
}

//
// Tex_CacheTextureFile
//

texture_t *Tex_CacheTextureFile(const char *name, int clampmode, kbool masked)
{
    texture_t *texture;

    texture = Tex_Find(name);

    if(texture == NULL)
    {
        int width;
        int height;
        byte *data;

        Img_LoadTGA(name, &data, &width, &height, masked);

        if(data == NULL)
        {
            return NULL;
        }

        texture = Tex_Alloc(name, data, width, height, clampmode);
        Z_Free(data);
    }

    return texture;
}