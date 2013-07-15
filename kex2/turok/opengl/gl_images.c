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
// DESCRIPTION: Texture image handling
//
//-----------------------------------------------------------------------------

#include <math.h>

#include "common.h"
#include "type.h"
#include "gl.h"
#include "zone.h"
#include "kernel.h"

CVAR_EXTERNAL(developer);

CVAR_CMD(gl_gamma, 0)
{
    //GL_DumpTextures();
}

//////////////////////////////////////
// TGA FORMAT HANDLING
//////////////////////////////////////

typedef struct
{
    byte infolen;
    byte has_cmap;
    byte type;
    short cmap_start;
    short cmap_len;
    byte cmap_bits;
    short xorigin;
    short yorigin;
    short width;
    short height;
    byte pixel_bits;
    byte flags;
} tgaheader_t;

typedef struct
{
    byte r;
    byte g;
    byte b;
    byte a;
} palette_t;

enum tga_type
{
    TGA_TYPE_INDEXED        = 1,
    TGA_TYPE_RGB            = 2,
    TGA_TYPE_BW             = 3,
    TGA_TYPE_RLE_INDEXED    = 9,
    TGA_TYPE_RLE_RGB        = 10,
    TGA_TYPE_RLE_BW         = 11
};

//
// Img_GetRGBGamma
//

d_inline static byte Img_GetRGBGamma(int c)
{
    return (byte)MIN(pow((float)c, (1.0f + (0.01f * gl_gamma.value))), 255);
}

//
// Img_LoadTGA
//

void Img_LoadTGA(const char *name, byte **output, int *width, int *height, kbool masked)
{
    byte *tgafile;
    byte *rover;
    tgaheader_t tga;
    byte tmp[2];
    byte *data;
    byte *data_r;

    if(developer.value)
    {
        if(KF_OpenFileCache(name, &tgafile, PU_STATIC) == 0 &&
            KF_ReadTextFile(name, &tgafile) <= 0)
        {
            *output = NULL;
            return;
        }
    }
    else if(KF_OpenFileCache(name, &tgafile, PU_STATIC) == 0)
    {
        *output = NULL;
        return;
    }

    rover = tgafile;

    tga.infolen = *rover++;
	tga.has_cmap = *rover++;
	tga.type = *rover++;
	
    tmp[0]          = rover[0];
    tmp[1]          = rover[1];
    tga.cmap_start  = Com_SwapLE16(*((short*)tmp));
    rover += 2;
    tmp[0]          = rover[0];
    tmp[1]          = rover[1];
    tga.cmap_len    = Com_SwapLE16(*((short*)tmp));
    rover += 2;
    tga.cmap_bits   = *rover++;
    tga.xorigin     = Com_SwapLE16(*((short*)rover));
    rover += 2;
    tga.yorigin     = Com_SwapLE16(*((short*)rover));
    rover += 2;
    tga.width       = Com_SwapLE16(*((short*)rover));
    rover += 2;
    tga.height      = Com_SwapLE16(*((short*)rover));
    rover += 2;
    tga.pixel_bits  = *rover++;
    tga.flags       = *rover++;

    if(tga.infolen != 0)
    {
        rover += tga.infolen;
    }

    if(width)   *width  = tga.width;
    if(height)  *height = tga.height;

    switch(tga.type)
    {
    case TGA_TYPE_INDEXED:
        switch(tga.pixel_bits)
        {
        case 8:
            {
                int r;
                int c;
                palette_t *p;

                data = (byte*)Z_Calloc(tga.width * tga.height * 4, PU_STATIC, 0);
                *output = data;
                p = (palette_t*)rover;

                switch(tga.cmap_bits)
                {
                case 24:
                    rover += (3 * tga.cmap_len);

                    for(r = tga.height - 1; r >= 0; r--)
                    {
                        data_r = data + r * tga.width * 4;
                        for(c = 0; c < tga.width; c++)
                        {
                            *data_r++ = Img_GetRGBGamma(p[*rover].b);
                            *data_r++ = Img_GetRGBGamma(p[*rover].g);
                            *data_r++ = Img_GetRGBGamma(p[*rover].r);
                            *data_r++ = 0xff;
                            rover++;
                        }
                    }
                    break;
                case 32:
                    rover += (4 * tga.cmap_len);

                    for(r = tga.height - 1; r >= 0; r--)
                    {
                        data_r = data + r * tga.width * 4;
                        for(c = 0; c < tga.width; c++)
                        {
                            *data_r++ = Img_GetRGBGamma(p[*rover].b);
                            *data_r++ = Img_GetRGBGamma(p[*rover].g);
                            *data_r++ = Img_GetRGBGamma(p[*rover].r);
                            *data_r++ = masked ? p[*rover].a : 0xff;
                            rover++;
                        }
                    }
                    break;
                default:
                    Com_Error("%i-bit color map not supported for %s", tga.cmap_bits, name);
                    break;
                }
            }
            break;
        case 24:
            Com_Error("24 bits (indexed) is not supported for %s", name);
        case 32:
            Com_Error("32 bits (indexed) is not supported for %s", name);
            break;
        default:
            Com_Error("unknown pixel bit for %s", name);
            break;
        }
        break;
    case TGA_TYPE_RGB:
        {
            int r;
            int c;

            data = (byte*)Z_Calloc(tga.width * tga.height * 4, PU_STATIC, 0);
            *output = data;

            for(r = tga.height - 1; r >= 0; r--)
            {
                data_r = data + r * tga.width * 4;
                for(c = 0; c < tga.width; c++)
                {
                    switch(tga.pixel_bits)
                    {
                    case 24:
                        *data_r++ = Img_GetRGBGamma(rover[2]);
                        *data_r++ = Img_GetRGBGamma(rover[1]);
                        *data_r++ = Img_GetRGBGamma(rover[0]);
                        *data_r++ = 0xff;
                        rover += 3;
                        break;
                    case 32:
                        *data_r++ = Img_GetRGBGamma(rover[2]);
                        *data_r++ = Img_GetRGBGamma(rover[1]);
                        *data_r++ = Img_GetRGBGamma(rover[0]);
                        *data_r++ = rover[3];
                        rover += 4;
                        break;
                    default:
                        Com_Error("unknown pixel bit for %s", name);
                        break;
                    }
                }
            }
        }
        break;
    case TGA_TYPE_BW:
        Com_Error("grayscale images is not supported for %s", name);
        break;
    case TGA_TYPE_RLE_INDEXED:
        Com_Error("RLE indexed images is not supported for %s", name);
        break;
    case TGA_TYPE_RLE_RGB:
        Com_Error("RLE images is not supported for %s", name);
        break;
    case TGA_TYPE_RLE_BW:
        Com_Error("RLE grayscale images is not supported for %s", name);
        break;
    default:
        Com_Error("%s has unknown tga type", name);
        break;
    }

    Z_Free(tgafile);
}

//
// Img_WriteTGA
//

void Img_WriteTGA(const char *filename, byte *data, int width, int height)
{
    byte *buffer;
    byte header[18];
    int i;
    int c;
    FILE *f;

    buffer = malloc(width*height*3);
    memset(header, 0, 18);
    header[2] = 2;      // uncompressed type
    header[12] = width&255;
    header[13] = width>>8;
    header[14] = height&255;
    header[15] = height>>8;
    header[16] = 24;    // pixel size

    // swap rgb to bgr
    c = width * height * 3;

    for(i = 0; i < c; i += 3)
    {
        buffer[i] = data[i+2];       // blue
        buffer[i+1] = data[i+1];     // green
        buffer[i+2] = data[i+0];     // red
    }

    f = fopen(filename, "wb");
    fwrite(header, 1, 18, f);
    fwrite(buffer, 1, c, f);
    fclose(f);

    free(buffer);
}
