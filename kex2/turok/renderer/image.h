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

#ifndef __IMAGE_H__
#define __IMAGE_H__

typedef enum {
    TCR_RGB     = 0,
    TCR_RGBA    = 1
} texColorMode_t;

class kexFBO;

class kexImageManager {
public:
                            kexImageManager(void);
                            ~kexImageManager(void);

    void                    LoadFromFile(const char *file);
    void                    LoadFromScreenBuffer(void);
    void                    LoadFromFrameBuffer(kexFBO &fbo);
    void                    FlipVertical(void);
    void                    WriteTGA(kexBinFile &binFile);
    
    const byte              *Data(void) const { return data; }
    const int               Width(void) const { return width; }
    const int               Height(void) const { return height; }
    const int               OriginalWidth(void) const { return origwidth; }
    const int               OriginalHeight(void) const { return origheight; }
    const texColorMode_t    ColorMode(void) const { return colorMode; }

private:
    void                    LoadFromTGA(byte *input);
    void                    LoadFromBMP(byte *input);
    byte                    GetRGBGamma(int c);
    void                    Alloc(void);
    
    byte                    *data;
    int                     width;
    int                     height;
    int                     origwidth;
    int                     origheight;
    texColorMode_t          colorMode;
    filepath_t              filePath;
};

#endif
