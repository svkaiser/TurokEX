// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: Texture object class. Handles all opengl
//              texture binding and uploading as well as
//              all RGB(A) manipulations such as scaling,
//              image flipping, etc
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "fileSystem.h"
#include "memHeap.h"
#include "mathlib.h"
#include "renderSystem.h"
#include "textureObject.h"

kexCvar cvarGamma("gl_gamma", CVF_FLOAT|CVF_CONFIG, "0", "TODO");
kexCvar cvarGLFilter("gl_filter", CVF_INT|CVF_CONFIG, "0", "Texture filter mode");
kexCvar cvarGLAnisotropic("gl_anisotropic", CVF_INT|CVF_CONFIG, "0", "TODO");

kexHeapBlock kexTexture::hb_texture("texture", false, NULL, NULL);

//
// ------------------------------------------------------
//
// common palette structure
//
// ------------------------------------------------------
//
typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} palette_t;

//
// ------------------------------------------------------
//
// tga structure
//
// ------------------------------------------------------
//

typedef struct {
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

enum tga_type {
    TGA_TYPE_INDEXED        = 1,
    TGA_TYPE_RGB            = 2,
    TGA_TYPE_BW             = 3,
    TGA_TYPE_RLE_INDEXED    = 9,
    TGA_TYPE_RLE_RGB        = 10,
    TGA_TYPE_RLE_BW         = 11
};

//
// ------------------------------------------------------
//
// bmp structure
//
// ------------------------------------------------------
//

typedef struct {
    char id[2];
    ulong fileSize;
    ulong u1;
    ulong dataOffset;
    ulong headerSize;
    ulong width;
    ulong height;
    word planes;
    word bits;
    ulong compression;
    ulong dataSize;
    ulong hRes;
    ulong vRes;
    ulong colors1;
    ulong colors2;
    palette_t palette[256];
} bmpheader_t;

//
// kexTexture::kexTexture
//

kexTexture::kexTexture(void) {
    this->width = 0;
    this->height = 0;
    this->origwidth = 0;
    this->origheight = 0;
    this->clampMode = TC_CLAMP;
    this->filterMode = TF_LINEAR;
    this->colorMode = TCR_RGBA;
    this->texid = 0;
    this->bLoaded = false;
    this->bMasked = false;
    this->next = NULL;
}

//
// kexTexture::~kexTexture
//

kexTexture::~kexTexture(void) {
    Delete();
}

//
// kexTexture::GetRGBGamma
//

byte kexTexture::GetRGBGamma(int c) {
    float f = cvarGamma.GetFloat();
    
    if(f == 1.0f) {
        return c;
    }
    
    return (byte)MIN(kexMath::Pow((float)c, (1.0f + (0.01f * f))), 255);
}

//
// kexTexture::SetParameters
//

void kexTexture::SetParameters(void) {
    unsigned int clamp;
    unsigned int filter;

    switch(clampMode) {
    case TC_CLAMP:
        clamp = GL_CLAMP_TO_EDGE;
        break;
    case TC_REPEAT:
        clamp = GL_REPEAT;
        break;
    default:
        return;
    }

    switch(filterMode) {
    case TF_LINEAR:
        filter = GL_LINEAR;
        break;
    case TF_NEAREST:
        filter = GL_NEAREST;
        break;
    default:
        return;
    }

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

    if(has_GL_EXT_texture_filter_anisotropic) {
        if(cvarGLAnisotropic.GetInt()) {
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, renderSystem.MaxAnisotropic());
        }
        else {
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
        }
    }
}

//
// kexTexture::LoadFromFile
//

byte *kexTexture::LoadFromFile(const char *file) {
    byte *data;
    byte *out;
    
    if(cvarDeveloper.GetBool()) {
        if(fileSystem.OpenFile(file, &data, hb_static) == 0 &&
            fileSystem.ReadExternalTextFile(file, &data) <= 0) {
            return NULL;
        }
    }
    else if(fileSystem.OpenFile(file, &data, hb_static) == 0) {
        return NULL;
    }

    if(strstr(file, ".tga")) {
        out = LoadFromTGA(data);
    }
    else if(strstr(file, ".bmp")) {
        out = LoadFromBMP(data);
    }
    else {
        common.Warning("kexTexture::LoadFromFile(%s) - Unknown file format\n", file);
    }

    Mem_Free(data);
    return out;
}

//
// kexTexture::LoadFromScreenBuffer
//

byte *kexTexture::LoadFromScreenBuffer(void) {
    byte* data;
    int pack;
    int col;
    int width;
    int height;
    int x;
    int y;

    width       = renderSystem.ViewWidth();
    height      = renderSystem.ViewHeight();
    x           = renderSystem.WindowX();
    y           = renderSystem.WindowY();
    col         = (width * 3);
    data        = (byte*)Mem_Calloc(height * width * 3, hb_static);
    
    colorMode   = TCR_RGB;
    bMasked     = false;

    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    VerticalFlipImage(&data);

    return data;
}

//
// kexTexture::LoadFromTGA
//

byte *kexTexture::LoadFromTGA(byte *input) {
    byte *tgafile;
    byte *rover;
    tgaheader_t tga;
    byte tmp[2];
    byte *data;
    byte *data_r;
    int r;
    int c;
    palette_t *p;
    int bitStride = 0;

    tgafile = input;
    rover = tgafile;
    data = NULL;

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

    if(tga.infolen != 0) {
        rover += tga.infolen;
    }

    width = tga.width;
    height = tga.height;

    switch(tga.type) {
    case TGA_TYPE_INDEXED:
        switch(tga.pixel_bits) {
        case 8:
            p = (palette_t*)rover;

            switch(tga.cmap_bits) {
            case 24:
                data = (byte*)Mem_Calloc(tga.width * tga.height * 3, hb_static);
                rover += (3 * tga.cmap_len);
                colorMode = TCR_RGB;

                for(r = tga.height - 1; r >= 0; r--) {
                    data_r = data + r * tga.width * 3;
                    for(c = 0; c < tga.width; c++) {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        rover++;
                    }
                }
                break;
            case 32:
                data = (byte*)Mem_Calloc(tga.width * tga.height * 4, hb_static);
                rover += (4 * tga.cmap_len);
                colorMode = TCR_RGBA;

                for(r = tga.height - 1; r >= 0; r--) {
                    data_r = data + r * tga.width * 4;
                    for(c = 0; c < tga.width; c++) {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        *data_r++ = bMasked ? p[*rover].a : 0xff;
                        rover++;
                    }
                }
                break;
            default:
                common.Error("%i-bit color map not supported for %s", tga.cmap_bits, filePath);
                break;
            }
            break;
        case 24:
            common.Error("24 bits (indexed) is not supported for %s", filePath);
        case 32:
            common.Error("32 bits (indexed) is not supported for %s", filePath);
            break;
        default:
            common.Error("unknown pixel bit for %s", filePath);
            break;
        }
        break;
    case TGA_TYPE_RGB:
        if(tga.pixel_bits == 32) {
            colorMode = TCR_RGBA;
            bitStride = 4;
        }
        else {
            colorMode = TCR_RGB;
            bitStride = 3;
        }
        
        data = (byte*)Mem_Calloc(tga.width * tga.height * bitStride, hb_static);

        for(r = tga.height - 1; r >= 0; r--) {
            data_r = data + r * tga.width * bitStride;
            for(c = 0; c < tga.width; c++) {
                switch(tga.pixel_bits) {
                case 24:
                    *data_r++ = GetRGBGamma(rover[2]);
                    *data_r++ = GetRGBGamma(rover[1]);
                    *data_r++ = GetRGBGamma(rover[0]);
                    rover += 3;
                    break;
                case 32:
                    *data_r++ = GetRGBGamma(rover[2]);
                    *data_r++ = GetRGBGamma(rover[1]);
                    *data_r++ = GetRGBGamma(rover[0]);
                    *data_r++ = rover[3];
                    rover += 4;
                    break;
                default:
                    common.Error("unknown pixel bit for %s", filePath);
                    break;
                }
            }
        }
        break;
    case TGA_TYPE_BW:
        common.Error("grayscale images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_INDEXED:
        common.Error("RLE indexed images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_RGB:
        common.Error("RLE images is not supported for %s", filePath);
        break;
    case TGA_TYPE_RLE_BW:
        common.Error("RLE grayscale images is not supported for %s", filePath);
        break;
    default:
        common.Error("%s has unknown tga type", filePath);
        break;
    }

    return data;
}

//
// kexTexture::LoadFromBMP
//

byte *kexTexture::LoadFromBMP(byte *input) {
    byte *data = NULL;
    byte *rover = input;
    bmpheader_t bmp;

    bmp.id[0]       = *rover++;
    bmp.id[1]       = *rover++;
    bmp.fileSize    = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.u1          = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.dataOffset  = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.headerSize  = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.width       = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.height      = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.planes      = Com_SwapLE16(*(short*)rover); rover += 2;
    bmp.bits        = Com_SwapLE16(*(short*)rover); rover += 2;
    bmp.compression = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.dataSize    = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.hRes        = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.vRes        = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.colors1     = Com_SwapLE32(*(long*)rover); rover += 4;
    bmp.colors2     = Com_SwapLE32(*(long*)rover); rover += 4;

    if(bmp.bits == 8) {
        memcpy(bmp.palette, rover, sizeof(palette_t) * 256);
        rover += (sizeof(palette_t) * 256);
    }

    if(bmp.id[0] != 'B' && bmp.id[1] != 'M') {
        common.Error("bitmap (%s) has unknown header ID ('BM' only supported\n", filePath);
    }
    if(bmp.compression != 0) {
        common.Error("compression not supported for bitmap (%s)\n", filePath);
    }
    if(bmp.bits < 8) {
        common.Error("monochrome and 4-bit pixels not supported for bitmap (%s)\n", filePath);
    }

    int bitStride = 0;

    int cols = kexMath::Abs(bmp.width);
    int rows = kexMath::Abs(bmp.height);

    int numPixels = cols * rows;

    width = cols;
    height = rows;

    if(bmp.bits != 32) {
        bitStride = 3;
        colorMode = TCR_RGB;
    }
    else {
        bitStride = 4;
        colorMode = TCR_RGBA;
    }

    data = (byte*)Mem_Calloc(cols * rows * bitStride, hb_static);

    for(int y = rows-1; y >= 0; y--) {
        byte *buf = data + (y * cols * bitStride);

        for(int x = 0; x < cols; x++) {
            byte rgba[4];
            word rgb16;
            int palIdx;

            switch(bmp.bits) {
            case 8:
                palIdx = *rover++;
                *buf++ = bmp.palette[palIdx].b;
                *buf++ = bmp.palette[palIdx].g;
                *buf++ = bmp.palette[palIdx].r;
                break;
            case 16:
                rgb16 = *(word*)buf; buf += 2;
                *buf++ = (rgb16 & (31 << 10)) >> 7;
                *buf++ = (rgb16 & (31 << 5)) >> 2;
                *buf++ = (rgb16 & 31) << 3;
                break;
            case 24:
                rgba[2] = *rover++;
                rgba[1] = *rover++;
                rgba[0] = *rover++;
                *buf++ = rgba[0];
                *buf++ = rgba[1];
                *buf++ = rgba[2];
                break;
            case 32:
                rgba[2] = *rover++;
                rgba[1] = *rover++;
                rgba[0] = *rover++;
                rgba[3] = *rover++;
                *buf++ = rgba[0];
                *buf++ = rgba[1];
                *buf++ = rgba[2];
                *buf++ = rgba[3];
                break;
            default:
                common.Error("bitmap (%s) has unknown pixel format (%i)\n", filePath, bmp.bits);
                break;
            }
        }
    }

    return data;
}

//
// kexTexture::PadImage
//

byte *kexTexture::PadImage(byte **data) {
    if(origwidth == width && origheight == height) {
        return *data;
    }
    
    int bitStride = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *pad = (byte*)Mem_Calloc(width * height * bitStride, hb_static);

    for(int y = 0; y < origheight; y++) {
        memcpy(pad + y * width * bitStride, *data + y * origwidth * bitStride,
            origwidth * bitStride);
    }
    
    Mem_Free(*data);
    return pad;
}

//
// kexTexture::VerticalFlipImage
//

void kexTexture::VerticalFlipImage(byte **data) {
    int bitStride = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *buffer = (byte*)Mem_Calloc((width * height) * bitStride, hb_static);
    byte *tmp = *data;
    int col = (width * bitStride);

    for(int i = 0; i < height / 2; i++) {
        memcpy(buffer, &tmp[i * col], col);
        memcpy(&tmp[i * col], &tmp[(height - (i + 1)) * col], col);
        memcpy(&tmp[(height - (i + 1)) * col], buffer, col);
    }

    Mem_Free(buffer);
}

//
// kexTexture::Upload
//

void kexTexture::Upload(byte **data, texClampMode_t clamp, texFilterMode_t filter) {
    if(*data == NULL) {
        return;
    }
    origwidth = width;
    origheight = height;

    width = kexMath::RoundPowerOfTwo(width);
    height = kexMath::RoundPowerOfTwo(height);

    clampMode = clamp;
    filterMode = filter;

    dglGenTextures(1, &texid);

    if(texid == 0) {
        // renderer is not initialized yet
        return;
    }

    dglBindTexture(GL_TEXTURE_2D, texid);

    *data = PadImage(data);

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA8 : GL_RGB8,
        width,
        height,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        (byte*)*data);

    SetParameters();

    bLoaded = true;
    dglBindTexture(GL_TEXTURE_2D, 0);
}

//
// kexTexture::Bind
//

void kexTexture::Bind(void) {
    dtexture tid = texid;

    if(bLoaded == false) {
        // we may have attempted to cache this texture before the renderer was
        // initialized so try reloading it
        byte *data = LoadFromFile(filePath);
        Upload(&data, clampMode, filterMode);

        if(data != NULL) {
            Mem_Free(data);
        }

        if(bLoaded == false || texid == 0) {
            // fall back to default texture
            tid = *renderSystem.defaultTexture.TextureID();
        }
        else {
            tid = texid;
        }
    }

    int unit = renderSystem.glState.currentUnit;
    dtexture currentTexture = renderSystem.glState.textureUnits[unit].currentTexture;

    if(tid == currentTexture) {
        return;
    }

    dglBindTexture(GL_TEXTURE_2D, tid);
    renderSystem.glState.textureUnits[unit].currentTexture = tid;
}

//
// kexTexture::Delete
//

void kexTexture::Delete(void) {
    if(texid == 0 || !bLoaded) {
        return;
    }

    dglDeleteTextures(1, &texid);
    texid = 0;
    bLoaded = false;
}
