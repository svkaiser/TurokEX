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

#ifndef __RENDERIMGUI_H__
#define __RENDERIMGUI_H__

#include "imgui.h"

class kexRenderImgui {
public:
                            kexRenderImgui(void);
                            ~kexRenderImgui(void);
    
    void                    Draw(const int width, const int height);

private:
    static const int        CIRCLE_VERTS = 8*4;
    static const unsigned   TEMP_COORD_COUNT = 100;
    
    void                    DrawPolygon(const float *coords, unsigned int numCoords, float r, rcolor col);
    void                    DrawRect(const float x, const float y, const float w, const float h,
                                     const float fth, rcolor col);
    void                    DrawRoundRect(const float x, const float y, const float w, const float h,
                                     const float r, const float fth, rcolor col);
    void                    DrawLine(const float x0, const float y0, const float x1, const float y1,
                                     const float r, const float fth, rcolor col);
    void                    DrawText(const float x, const float y, const char *text,
                                     const int align, rcolor color);
    
    float                   circleVerts[CIRCLE_VERTS*2];
    float                   tempCoords[TEMP_COORD_COUNT*2];
    float                   tempNormals[TEMP_COORD_COUNT*2];
};

extern kexRenderImgui renderImgui;

#endif
