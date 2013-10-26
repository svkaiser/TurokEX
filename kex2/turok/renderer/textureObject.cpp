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
#include "zone.h"
#include "mathlib.h"
#include "renderSystem.h"
#include "textureObject.h"

kexCvar cvarGamma("gl_gamma", CVF_FLOAT|CVF_CONFIG, "0", "TODO");
kexCvar cvarGLFilter("gl_filter", CVF_INT|CVF_CONFIG, "0", "Texture filter mode");
kexCvar cvarGLAnisotropic("gl_anisotropic", CVF_INT|CVF_CONFIG, "0", "TODO");

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

typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} palette_t;

enum tga_type {
    TGA_TYPE_INDEXED        = 1,
    TGA_TYPE_RGB            = 2,
    TGA_TYPE_BW             = 3,
    TGA_TYPE_RLE_INDEXED    = 9,
    TGA_TYPE_RLE_RGB        = 10,
    TGA_TYPE_RLE_BW         = 11
};

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
    this->jsObject = NULL;
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
        if(fileSystem.OpenFile(file, &data, PU_STATIC) == 0 &&
            fileSystem.ReadExternalTextFile(file, &data) <= 0) {
            return NULL;
        }
    }
    else if(fileSystem.OpenFile(file, &data, PU_STATIC) == 0) {
        return NULL;
    }

    if(strstr(file, ".tga")) {
        out = LoadFromTGA(data);
    }
    else {
        common.Warning("kexTexture::LoadFromFile(%s) - Unknown file format\n", file);
    }

    Z_Free(data);
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
    data        = (byte*)Z_Calloc(height * width * 3, PU_STATIC, 0);
    
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
                data = (byte*)Z_Calloc(tga.width * tga.height * 3, PU_STATIC, 0);
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
                data = (byte*)Z_Calloc(tga.width * tga.height * 4, PU_STATIC, 0);
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
        
        data = (byte*)Z_Calloc(tga.width * tga.height * bitStride, PU_STATIC, 0);
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
// kexTexture::PadImage
//

void kexTexture::PadImage(byte **data) {
    if(origwidth == width && origheight == height) {
        return;
    }
    
    int bitStride = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *pad = (byte*)Z_Calloc(width * height * bitStride, PU_STATIC, 0);

    for(int y = 0; y < origheight; y++) {
        memcpy(pad + y * width * bitStride, *data + y * origwidth * bitStride,
            origwidth * bitStride);
    }
    
    Z_Free(*data);
    *data = pad;
}

//
// kexTexture::VerticalFlipImage
//

void kexTexture::VerticalFlipImage(byte **data) {
    int bitStride = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *buffer = (byte*)Z_Calloc((width * height) * bitStride, PU_STATIC, 0);
    byte *tmp = *data;
    int col = (width * bitStride);

    for(int i = 0; i < height / 2; i++) {
        memcpy(buffer, &tmp[i * col], col);
        memcpy(&tmp[i * col], &tmp[(height - (i + 1)) * col], col);
        memcpy(&tmp[(height - (i + 1)) * col], buffer, col);
    }

    Z_Free(buffer);
}

//
// kexTexture::Upload
//

void kexTexture::Upload(byte *data, texClampMode_t clamp, texFilterMode_t filter) {
    if(data == NULL) {
        return;
    }
    origwidth = width;
    origheight = height;

    width = kexMath::RoundPowerOfTwo(width);
    height = kexMath::RoundPowerOfTwo(height);

    clampMode = clamp;
    filterMode = filter;

    dglGenTextures(1, &texid);
    dglBindTexture(GL_TEXTURE_2D, texid);

    PadImage(&data);

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA8 : GL_RGB8,
        width,
        height,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        (byte*)data);

    SetParameters();

    bLoaded = true;
    dglBindTexture(GL_TEXTURE_2D, 0);
}

//
// kexTexture::Bind
//

void kexTexture::Bind(void) {
    int unit = renderSystem.glState.currentUnit;
    dtexture currentTexture = renderSystem.glState.textureUnits[unit].currentTexture;

    if(texid == currentTexture) {
        return;
    }

    dglBindTexture(GL_TEXTURE_2D, texid);
    renderSystem.glState.textureUnits[unit].currentTexture = texid;
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
