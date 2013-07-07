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

#include "types.h"
#include "common.h"
#include "pak.h"
#include "rnc.h"
#include "zone.h"

#define CHUNK_DIRECTORY_TEXTURE     16

#define CHUNK_TEX_COUNT             0
#define CHUNK_TEX_OFFSET(x)         (4 + (x * 4))

#define CHUNK_TEX_SUBCOUNT          0
#define CHUNK_TEXINFO               4
#define CHUNK_TEXHEADER             8
#define CHUNK_TEXDATA_OFFSET(x)     (4 + (x * 4))

#define CHUNK_PAL_SUBCOUNT          0
#define CHUNK_PALDATA_OFFSET(x)     (4 + (x * 4))
#define CHUNK_PALHEADER             12

static byte *textures;
static int numtextures;
static int curtexture = 0;

short texindexes[2000];

typedef struct
{
    short flags;
    byte wshift;
    byte hshift;
    byte unknown[4];
} turoktexture_t;

typedef struct
{
    short u1;
    short u2;
    short w1;
    short h1;
    int w2;
    int h2;
} hudtexture_t;

typedef struct
{
    unsigned int address;
    const char *name;
} hudlist_t;

typedef struct
{
    byte infolen;
    byte has_cmap;
    byte type;
    short cmap_start;
    short cmap_len;
    byte cmap_bits;
    short yorigin;
    short xorigin;
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
} dPalette_t;

static void ConvertPalette(dPalette_t* palette, word* data, int indexes, dboolean fxTexture)
{
    int i;
    short val;
    byte *p = (byte*)data;
    
    for(i = 0; i < indexes; i++)
    {
        // Read the next packed short from the input buffer.
        val = *(short*)p;
        p += 2;
        val = Com_Swap16(val);
        
        // Unpack and expand to 8bpp, then flip from BGR to RGB.
        

        if(fxTexture)
        {
            /*palette[i].r = (val & 0xF800) >> 8;
            palette[i].g = (val & 0xF800) >> 8;
            palette[i].b = (val & 0xF800) >> 8;*/
            palette[i].r = (val & 0x003E) << 2;
            palette[i].g = (val & 0x07C0) >> 3;
            palette[i].b = (val & 0xF800) >> 8;
            palette[i].a = (val & 0x00FF);
        }
        else
        {
            palette[i].r = (val & 0x003E) << 2;
            palette[i].g = (val & 0x07C0) >> 3;
            palette[i].b = (val & 0xF800) >> 8;
            palette[i].a = (val & 0x0001) ? 0xFF : 0;
        }
    }
}

void AddTexture(byte *data, int size, const char *path)
{
    tgaheader_t tga;
    turoktexture_t *tex;
    byte *texheader;
    byte *palheader;
    int texcount;
    int palcount;
    int i;
    int row;
    int col;
    byte *buffer;
    byte *rover;

    memset(&tga, 0, sizeof(tgaheader_t));
    tex = (turoktexture_t*)Com_GetCartData(data, CHUNK_TEXINFO, 0);

    tga.type = 1;
    tga.cmap_bits = 32;
    tga.has_cmap = 1;
    tga.cmap_len = 256;
    tga.pixel_bits = 8;
    tga.flags = 8;
    tga.width = (1 << tex->wshift);
    tga.height = (1 << tex->hshift);

    texheader = Com_GetCartData(data, CHUNK_TEXHEADER, 0);
    palheader = Com_GetCartData(data, CHUNK_PALHEADER, 0);
    texcount = Com_GetCartOffset(texheader, CHUNK_TEX_SUBCOUNT, 0);
    palcount = Com_GetCartOffset(palheader, CHUNK_PAL_SUBCOUNT, 0);

    if(texcount <= 1 && palcount > 1)
        texindexes[curtexture] = palcount;
    else
        texindexes[curtexture] = texcount;

    for(i = 0; i < texindexes[curtexture]; i++)
    {
        byte *texdata;
        byte *paldata;
        int curtex;
        int texsize;
        int palsize;
        dPalette_t pal[256];
        char name[256];

        buffer = Com_Alloc(size * 16);
        rover = buffer;

        memset(&pal, 0, sizeof(dPalette_t) * 256);
        sprintf(name, "%s_%02d.tga", path, i);

        curtex = i;
        if(curtex >= texcount)
            curtex = (texcount-1);
        if(curtex < 0)
            curtex = 0;

        texdata = Com_GetCartData(texheader, CHUNK_TEXDATA_OFFSET(curtex), &texsize);

        if(!palcount)
        {
            tga.type = 2;
            tga.cmap_bits = 0;
            tga.has_cmap = 0;
            tga.cmap_len = 0;
            tga.pixel_bits = 32;
            tga.flags = 8;
        }
        else
        {
            if(palcount > 1)
            {
                paldata = Com_GetCartData(palheader,
                    CHUNK_PALDATA_OFFSET(i), &palsize);
            }
            else if(palcount > 0)
            {
                paldata = Com_GetCartData(palheader,
                    CHUNK_PALDATA_OFFSET(0), &palsize);
            }

            ConvertPalette(pal, (word*)paldata,
                (palsize / 2), (tex->flags & 0x2));
        }

        Com_WriteMem8(rover, tga.infolen);
        Com_WriteMem8(rover, tga.has_cmap);
        Com_WriteMem8(rover, tga.type);
        Com_WriteMem16(rover, tga.cmap_start);
        Com_WriteMem16(rover, tga.cmap_len);
        Com_WriteMem8(rover, tga.cmap_bits);
        Com_WriteMem16(rover, tga.yorigin);
        Com_WriteMem16(rover, tga.xorigin);
        Com_WriteMem16(rover, tga.width);
        Com_WriteMem16(rover, tga.height);
        Com_WriteMem8(rover, tga.pixel_bits);
        Com_WriteMem8(rover, tga.flags);

        if(palcount && tga.has_cmap)
        {
            if(tex->flags & 0x1)
            {
                for(row = (16-1); row >= 0; row--)
                {
                    for(col = (16-1); col >= 0; col--)
                    {
                        Com_WriteMem8(rover, pal[row * 16 + col].r);
                        Com_WriteMem8(rover, pal[row * 16 + col].g);
                        Com_WriteMem8(rover, pal[row * 16 + col].b);
                        Com_WriteMem8(rover, pal[row * 16 + col].a);
                    }
                }
            }
            else
            {
                for(col = 255; col >= 0; col--)
                {
                    Com_WriteMem8(rover, pal[col].r);
                    Com_WriteMem8(rover, pal[col].g);
                    Com_WriteMem8(rover, pal[col].b);
                    Com_WriteMem8(rover, pal[col].a);
                }
            }
        }

        if(tex->flags & 0x1 && palcount)
        {
            for(row = (tga.height - 1); row >= 0; row--)
            {
                for(col = 0; col < (tga.width >> 1); col++)
                {
                    byte t1;
                    byte t2;

                    t1 = (texdata[row * (tga.width >> 1) + col] & 0xf);
                    t2 = (texdata[row * (tga.width >> 1) + col] >> 4);

                    Com_WriteMem8(rover, 0xff - t2);
                    Com_WriteMem8(rover, 0xff - t1);
                }
            }
        }
        else
        {
            for(row = tga.height - 1; row >= 0; row--)
            {
                for(col = 0; col < tga.width; col++)
                {
                    byte *data_rover;

                    if(tex->flags & 0x4 && !palcount)
                    {
                        short val;
                        short *tmp;
                        int r, g, b, a;

                        data_rover = texdata + row * tga.width * 2;

                        tmp = (short*)data_rover;
                        val = tmp[col];
                        r = (Com_Swap16(val) & 0x003E) << 2;
                        g = (Com_Swap16(val) & 0x07C0) >> 3;
                        b = (Com_Swap16(val) & 0xF800) >> 8;
                        a = (Com_Swap16(val) & 0x0001);

                        Com_WriteMem8(rover, r);
                        Com_WriteMem8(rover, g);
                        Com_WriteMem8(rover, b);
                        Com_WriteMem8(rover, a);
                    }
                    else
                    {
                        Com_WriteMem8(rover, 0xff - texdata[row * tga.width + col]);
                    }
                }
            }
        }

        PK_AddFile(name, buffer, rover - buffer, true);
        Com_Free(&buffer);
    }

    curtexture++;
}

void TX_StoreTextures(void)
{
    int i;

    textures = Com_GetCartData(cartfile, CHUNK_DIRECTORY_TEXTURE, 0);
    numtextures = Com_GetCartOffset(textures, CHUNK_TEX_COUNT, 0);

    PK_AddFolder("textures/");
    StoreExternalFile("default.tga", "textures/default.tga");
    StoreExternalFile("black.tga", "textures/black.tga");
    StoreExternalFile("white.tga", "textures/white.tga");

    for(i = 0; i < numtextures; i++)
    {
        byte *data;
        byte *texdata;
        int size;
        int outsize;

        data = Com_GetCartData(textures, CHUNK_TEX_OFFSET(i), &size);
        texdata = RNC_ParseFile(data, size, &outsize);
        AddTexture(texdata, outsize, va("textures/tex%04d", curtexture));

        Com_Free(&texdata);
    }
}

#define BIGFONT_COUNT     47

static void ProcessBigFonts(void)
{
    byte *bigfonts;
    tgaheader_t tga;
    byte *data;
    byte *rover;
    int len;
    int i;
    int j;
    int x;
    int y;
    int next;

    bigfonts = (cartfile + 0x100FA8);
    len = sizeof(tgaheader_t) + ((256 * 256) * 4);
    data = Com_Alloc(len);
    rover = data;

    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 0;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = 32;
    tga.flags = 8;
    tga.width = 256;
    tga.height = 256;

    Com_WriteMem8(rover, tga.infolen);
    Com_WriteMem8(rover, tga.has_cmap);
    Com_WriteMem8(rover, tga.type);
    Com_WriteMem16(rover, tga.cmap_start);
    Com_WriteMem16(rover, tga.cmap_len);
    Com_WriteMem8(rover, tga.cmap_bits);
    Com_WriteMem16(rover, tga.yorigin);
    Com_WriteMem16(rover, tga.xorigin);
    Com_WriteMem16(rover, tga.width);
    Com_WriteMem16(rover, tga.height);
    Com_WriteMem8(rover, tga.pixel_bits);
    Com_WriteMem8(rover, tga.flags);

    x = 0;
    y = 0;

    for(next = 0; next < BIGFONT_COUNT; next++)
    {
        if(x >= 256)
        {
            x = 0;
            y -= 36;
        }

        for(i = 0; i < 32; i++)
        {
            byte* src;
            byte* dst;

            src = &bigfonts[(i + (next * 32)) * 16];
            dst = &rover[(((255 - i) + y)*4 * 256) + (x*4)];

            for(j = 0; j < 16; j++)
            {
                byte t1 = (src[j] & 0xf);
                byte t2 = (src[j] >> 4);

                if(t2 == 0)
                {
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                }
                else
                {
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 0xff);
                }
                
                if(t1 == 0)
                {
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                }
                else
                {
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 0xff);
                }
            }
        }

        x += 32;
    }

    PK_AddFile("fonts/bigfont.tga", data, len, true);
    Com_Free(&data);
}

#define SMALLFONT_COUNT     65

static void ProcessSmallFonts(void)
{
    byte *smallfonts;
    tgaheader_t tga;
    byte *data;
    byte *rover;
    int len;
    int i;
    int j;
    int x;
    int y;
    int next;

    smallfonts = (cartfile + 0x9C088);
    len = sizeof(tgaheader_t) + ((256 * 64) * 4);
    data = Com_Alloc(len);
    rover = data;

    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 0;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = 32;
    tga.flags = 8;
    tga.width = 256;
    tga.height = 64;

    Com_WriteMem8(rover, tga.infolen);
    Com_WriteMem8(rover, tga.has_cmap);
    Com_WriteMem8(rover, tga.type);
    Com_WriteMem16(rover, tga.cmap_start);
    Com_WriteMem16(rover, tga.cmap_len);
    Com_WriteMem8(rover, tga.cmap_bits);
    Com_WriteMem16(rover, tga.yorigin);
    Com_WriteMem16(rover, tga.xorigin);
    Com_WriteMem16(rover, tga.width);
    Com_WriteMem16(rover, tga.height);
    Com_WriteMem8(rover, tga.pixel_bits);
    Com_WriteMem8(rover, tga.flags);

    x = 0;
    y = 0;

    for(next = 0; next < SMALLFONT_COUNT; next++)
    {
        if(x >= 256)
        {
            x = 0;
            y -= 8;
        }

        for(i = 0; i < 8; i++)
        {
            byte* src;
            byte* dst;

            src = &smallfonts[(i + (next * 8)) * 8];
            dst = &rover[(((63 - i) + y)*4 * 256) + (x*4)];

            for(j = 0; j < 8; j++)
            {
                byte t1 = (src[j] & 0xf);
                byte t2 = (src[j] >> 4);

                if(t2 == 0)
                {
                    Com_WriteMem8(dst, 255);
                    Com_WriteMem8(dst, 255);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                }
                else
                {
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 15 + (16 * t2));
                    Com_WriteMem8(dst, 0xff);
                }
                
                if(t1 == 0)
                {
                    Com_WriteMem8(dst, 255);
                    Com_WriteMem8(dst, 255);
                    Com_WriteMem8(dst, 0);
                    Com_WriteMem8(dst, 0);
                }
                else
                {
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 15 + (16 * t1));
                    Com_WriteMem8(dst, 0xff);
                }
            }
        }

        x += 16;
    }

    PK_AddFile("fonts/smallfont.tga", data, len, true);
    Com_Free(&data);
}

#define HUDGFX_COUNT    61

void ProcessHudGfx(unsigned int address, const char *name)
{
    byte *start;
    hudtexture_t *hud;
    tgaheader_t tga;
    int len;
    byte *data;
    byte *rover;
    int w;
    int h;

    start = (cartfile + address);
    hud = (hudtexture_t*)start;
    len = sizeof(tgaheader_t) + ((hud->w1 * hud->h1) * 4);
    data = Com_Alloc(len);
    rover = data;

    start += sizeof(hudtexture_t);

    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 0;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = 32;
    tga.flags = 8;
    tga.width = hud->w1;
    tga.height = hud->h1;

    Com_WriteMem8(rover, tga.infolen);
    Com_WriteMem8(rover, tga.has_cmap);
    Com_WriteMem8(rover, tga.type);
    Com_WriteMem16(rover, tga.cmap_start);
    Com_WriteMem16(rover, tga.cmap_len);
    Com_WriteMem8(rover, tga.cmap_bits);
    Com_WriteMem16(rover, tga.yorigin);
    Com_WriteMem16(rover, tga.xorigin);
    Com_WriteMem16(rover, tga.width);
    Com_WriteMem16(rover, tga.height);
    Com_WriteMem8(rover, tga.pixel_bits);
    Com_WriteMem8(rover, tga.flags);

    for(h = 0; h < hud->h1; h++)
    {
        for(w = 0; w < hud->w1; w++)
        {
            short val;
            short *tmp;
            int r, g, b, a;

            tmp = (short*)start;
            val = tmp[(hud->w1 * ((hud->h1-1) - h)) + w];
            r = (Com_Swap16(val) & 0x003E) << 2;
            g = (Com_Swap16(val) & 0x07C0) >> 3;
            b = (Com_Swap16(val) & 0xF800) >> 8;
            a = (Com_Swap16(val) & 0x0001);

            Com_WriteMem8(rover, r);
            Com_WriteMem8(rover, g);
            Com_WriteMem8(rover, b);

            if(a != 0)
            {
                Com_WriteMem8(rover, 0xff);
            }
            else
            {
                Com_WriteMem8(rover, 0);
            }
        }
    }

    PK_AddFile(name, data, len, true);
    Com_Free(&data);
}

static const hudlist_t hudlist[HUDGFX_COUNT] =
{
    { 0xD9AB8, "hud/h_num0.tga" },
    { 0xD9D88, "hud/h_num1.tga" },
    { 0xDA058, "hud/h_num2.tga" },
    { 0xDA328, "hud/h_num3.tga" },
    { 0xDA5F8, "hud/h_num4.tga" },
    { 0xDA8C8, "hud/h_num5.tga" },
    { 0xDAB98, "hud/h_num6.tga" },
    { 0xDAE68, "hud/h_num7.tga" },
    { 0xDB138, "hud/h_num8.tga" },
    { 0xDB408, "hud/h_num9.tga" },
    { 0xDB6D8, "hud/h_minus.tga" },
    { 0xE1518, "hud/h_health.tga" },
    { 0xE2398, "hud/h_armor.tga" },
    { 0xE3368, "hud/h_turok.tga" },
    { 0xE5918, "hud/h_coin.tga" },
    { 0xE6A68, "hud/h_bar_air.tga" },
    { 0xEEA78, "hud/h_bar_boss.tga" },
    { 0xF6E88, "hud/h_bar1.tga" },
    { 0xF8CF8, "hud/h_walk.tga" },
    { 0xF9F08, "hud/h_sprint.tga" },
    { 0xFB118, "hud/h_bar2.tga" },
    { 0xFCF88, "hud/h_lens1.tga" },
    { 0xFEF98, "hud/h_lens2.tga" },
    { 0xDD498, "hud/h_key1.tga" },
    { 0xDDCA8, "hud/h_key2.tga" },
    { 0xDE4B8, "hud/h_key3.tga" },
    { 0xDECC8, "hud/h_key4.tga" },
    { 0xDF4D8, "hud/h_key5.tga" },
    { 0xDFCE8, "hud/h_key6.tga" },
    { 0xE04F8, "hud/h_key7.tga" },
    { 0xE0D08, "hud/h_key8.tga" },
    { 0xDB9A8, "hud/h_plaque1.tga" },
    { 0xCFEC8, "hud/h_pause.tga" },
    { 0xD1E18, "hud/h_gameover.tga" },
    { 0xD5A28, "hud/h_options.tga" },
    { 0xD8078, "hud/h_slider.tga" },
    { 0xD8AA8, "hud/h_plaque2.tga" },
    { 0xA4818, "hud/h_w_knife.tga" },
    { 0xA6178, "hud/h_w_bow.tga" },
    { 0xA9588, "hud/h_w_pistol.tga" },
    { 0xAAA18, "hud/h_w_rifle.tga" },
    { 0xADE28, "hud/h_w_shotgun.tga" },
    { 0xB0C38, "hud/h_w_autoshot.tga" },
    { 0xB3C48, "hud/h_w_glauncher.tga" },
    { 0xB7658, "hud/h_w_mlauncher.tga" },
    { 0xBAE68, "hud/h_w_minigun.tga" },
    { 0xBEE78, "hud/h_w_pulse.tga" },
    { 0xC1088, "hud/h_w_alien.tga" },
    { 0xC4698, "hud/h_w_charge.tga" },
    { 0xC7AA8, "hud/h_w_fusion.tga" },
    { 0xCACB8, "hud/h_w_scepter.tga" },
    { 0x9D0C8, "hud/h_a_arrow1.tga" },
    { 0x9DDE8, "hud/h_a_arrow2.tga" },
    { 0x9EA58, "hud/h_a_clip.tga" },
    { 0x9F328, "hud/h_a_shells1.tga" },
    { 0xA01A8, "hud/h_a_shells2.tga" },
    { 0xA0F78, "hud/h_a_cell.tga" },
    { 0xA1B68, "hud/h_a_mags.tga" },
    { 0xA2518, "hud/h_a_grenades.tga" },
    { 0xA3248, "hud/h_a_missiles.tga" },
    { 0xA3F48, "hud/h_a_power.tga" }
};

void TX_StoreFonts(void)
{
    int i;

    PK_AddFolder("fonts/");
    StoreExternalFile("confont.tga", "fonts/confont.tga");

    Com_GetCartFile(
        "PC Turok Executable File (Turok.exe) \0*Turok.exe\0All Files (*.*)\0*.*\0",
        "Locate Turok.exe");

    if(cartfile == NULL)
        return;

    ProcessBigFonts();
    ProcessSmallFonts();

    PK_AddFolder("hud/");

    for(i = 0; i < HUDGFX_COUNT; i++)
    {
        ProcessHudGfx(hudlist[i].address, hudlist[i].name);
    }

    Com_CloseCartFile();
}

