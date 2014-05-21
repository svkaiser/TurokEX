// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: RGB(A) image management
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "fileSystem.h"
#include "renderBackend.h"
#include "textureObject.h"
#include "image.h"
#include "binFile.h"
#include "fbo.h"

kexCvar cvarGamma("gl_gamma", CVF_FLOAT|CVF_CONFIG, "1", "TODO");

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
// kexImageManager::kexImageManager
//

kexImageManager::kexImageManager(void) {
    this->data = NULL;
    this->width = 0;
    this->height = 0;
    this->origwidth = 0;
    this->origheight = 0;
    this->colorMode = TCR_RGBA;
}

//
// kexImageManager::~kexImageManager
//

kexImageManager::~kexImageManager(void) {
    if(data) {
        delete[] data;
    }
}

//
// kexImageManager::GetRGBGamma
//

byte kexImageManager::GetRGBGamma(int c) {
    float f = cvarGamma.GetFloat();
    
    if(f == 1.0f) {
        return c;
    }
    
    return (byte)MIN(kexMath::Pow((float)c, (1.0f + (0.01f * f))), 255);
}

//
// kexImageManager::Alloc
//

void kexImageManager::Alloc(void) {
    int size = width * height * (colorMode == TCR_RGBA ? 4 : 3);

    data = new byte[size];

    if(width != origwidth || height != origheight) {
        memset(data, 0, size);
    }
}

//
// kexImageManager::LoadFromFile
//

void kexImageManager::LoadFromFile(const char *file) {
    byte *fileData;
    
    strcpy(filePath, file);
    
    if(cvarDeveloper.GetBool()) {
        if(fileSystem.OpenFile(file, &fileData, hb_static) == 0 &&
            fileSystem.ReadExternalTextFile(file, &fileData) <= 0) {
            return;
        }
    }
    else if(fileSystem.OpenFile(file, &fileData, hb_static) == 0) {
        return;
    }

    if(strstr(file, ".tga")) {
        LoadFromTGA(fileData);
    }
    else if(strstr(file, ".bmp")) {
        LoadFromBMP(fileData);
    }
    else {
        data = NULL;
        common.Warning("kexImageManager::LoadFromFile(%s) - Unknown file format\n", file);
    }

    Mem_Free(fileData);
}

//
// kexImageManager::LoadFromScreenBuffer
//

void kexImageManager::LoadFromScreenBuffer(void) {
    int pack;
    int col;
    int x;
    int y;
    
    origwidth   = renderBackend.ViewWidth();
    origheight  = renderBackend.ViewHeight();
    width       = origwidth;
    height      = origheight;
    x           = renderBackend.WindowX();
    y           = renderBackend.WindowY();
    col         = (width * 3);
    colorMode   = TCR_RGB;
    
    Alloc();
    
    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);
}

//
// kexImageManager::LoadFromFrameBuffer
//

void kexImageManager::LoadFromFrameBuffer(kexFBO &fbo) {
    int pack;
    int col;
    
    origwidth   = fbo.Width();
    origheight  = fbo.Height();
    width       = origwidth;
    height      = origheight;
    col         = (width * 3);
    colorMode   = TCR_RGB;
    
    Alloc();

    fbo.Bind();
    
    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    fbo.UnBind();
}

//
// kexImageManager::LoadFromTGA
//

void kexImageManager::LoadFromTGA(byte *input) {
    byte *tgafile;
    byte *rover;
    tgaheader_t tga;
    byte tmp[2];
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
    tga.cmap_start  = sysMain.SwapLE16(*((short*)tmp));
    rover += 2;
    tmp[0]          = rover[0];
    tmp[1]          = rover[1];
    tga.cmap_len    = sysMain.SwapLE16(*((short*)tmp));
    rover += 2;
    tga.cmap_bits   = *rover++;
    tga.xorigin     = sysMain.SwapLE16(*((short*)rover));
    rover += 2;
    tga.yorigin     = sysMain.SwapLE16(*((short*)rover));
    rover += 2;
    tga.width       = sysMain.SwapLE16(*((short*)rover));
    rover += 2;
    tga.height      = sysMain.SwapLE16(*((short*)rover));
    rover += 2;
    tga.pixel_bits  = *rover++;
    tga.flags       = *rover++;

    if(tga.infolen != 0) {
        rover += tga.infolen;
    }

    origwidth = tga.width;
    origheight = tga.height;
    width = kexMath::RoundPowerOfTwo(origwidth);
    height = kexMath::RoundPowerOfTwo(origheight);

    switch(tga.type) {
    case TGA_TYPE_INDEXED:
        switch(tga.pixel_bits) {
        case 8:
            p = (palette_t*)rover;

            switch(tga.cmap_bits) {
            case 24:
                colorMode = TCR_RGB;
                Alloc();
                rover += (3 * tga.cmap_len);

                for(r = tga.height - 1; r >= 0; r--) {
                    data_r = data + r * width * 3;
                    for(c = 0; c < tga.width; c++) {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        rover++;
                    }
                }
                break;
            case 32:
                colorMode = TCR_RGBA;
                Alloc();
                rover += (4 * tga.cmap_len);

                for(r = tga.height - 1; r >= 0; r--) {
                    data_r = data + r * width * 4;
                    for(c = 0; c < tga.width; c++) {
                        *data_r++ = GetRGBGamma(p[*rover].b);
                        *data_r++ = GetRGBGamma(p[*rover].g);
                        *data_r++ = GetRGBGamma(p[*rover].r);
                        *data_r++ = p[*rover].a;
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
        
        Alloc();

        for(r = tga.height - 1; r >= 0; r--) {
            data_r = data + r * width * bitStride;
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
}

//
// kexImageManager::WriteTGA
//

void kexImageManager::WriteTGA(kexBinFile &binFile) {
    tgaheader_t tga;

    memset(&tga, 0, sizeof(tgaheader_t));

    tga.type = 2;
    tga.cmap_bits = 0;
    tga.has_cmap = 0;
    tga.cmap_len = 0;
    tga.pixel_bits = colorMode == TCR_RGB ? 24 : 32;
    tga.flags = 8;
    tga.width = origwidth;
    tga.height = origheight;

    binFile.Write8(tga.infolen);
    binFile.Write8(tga.has_cmap);
    binFile.Write8(tga.type);
    binFile.Write16(tga.cmap_start);
    binFile.Write16(tga.cmap_len);
    binFile.Write8(tga.cmap_bits);
    binFile.Write16(tga.yorigin);
    binFile.Write16(tga.xorigin);
    binFile.Write16(tga.width);
    binFile.Write16(tga.height);
    binFile.Write8(tga.pixel_bits);
    binFile.Write8(tga.flags);

    for(int i = 0; i < (tga.width * tga.height); i++) {
        binFile.Write8(data[i * 3 + 2]);
        binFile.Write8(data[i * 3 + 1]);
        binFile.Write8(data[i * 3 + 0]);

        if(colorMode == TCR_RGBA) {
            binFile.Write8(data[i * 3 + 4]);
        }
    }
}

//
// kexImageManager::LoadFromBMP
//

void kexImageManager::LoadFromBMP(byte *input) {
    byte *rover = input;
    bmpheader_t bmp;

    bmp.id[0]       = *rover++;
    bmp.id[1]       = *rover++;
    bmp.fileSize    = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.u1          = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.dataOffset  = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.headerSize  = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.width       = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.height      = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.planes      = sysMain.SwapLE16(*(short*)rover); rover += 2;
    bmp.bits        = sysMain.SwapLE16(*(short*)rover); rover += 2;
    bmp.compression = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.dataSize    = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.hRes        = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.vRes        = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.colors1     = sysMain.SwapLE32(*(long*)rover); rover += 4;
    bmp.colors2     = sysMain.SwapLE32(*(long*)rover); rover += 4;

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

    origwidth = cols;
    origheight = rows;
    width = kexMath::RoundPowerOfTwo(origwidth);
    height = kexMath::RoundPowerOfTwo(origheight);

    if(bmp.bits != 32) {
        bitStride = 3;
        colorMode = TCR_RGB;
    }
    else {
        bitStride = 4;
        colorMode = TCR_RGBA;
    }

    Alloc();

    for(int y = rows-1; y >= 0; y--) {
        byte *buf = data + (y * width * bitStride);

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
}

//
// kexImageManager::FlipVertical
//

void kexImageManager::FlipVertical(void) {
    if(data == NULL) {
        return;
    }
    
    int bitStride   = (colorMode == TCR_RGBA) ? 4 : 3;
    byte *buffer    = new byte[(width * height) * bitStride];
    int col         = (width * bitStride);
    int offset1;
    int offset2;

    for(int i = 0; i < height / 2; i++) {
        for(int j = 0; j < col; j++) {
            offset1 = (i * col) + j;
            offset2 = ((height - (i + 1)) * col) + j;

            buffer[j] = data[offset1];
            data[offset1] = data[offset2];
            data[offset2] = buffer[j];
        }
    }

    delete[] buffer;
}
