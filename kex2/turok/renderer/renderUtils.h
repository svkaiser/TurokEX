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

#ifndef __RENDER_UTILS_H__
#define __RENDER_UTILS_H__

#define DEBUG_LINE_TOP  32

class kexRenderUtils {
public:
    static void     DrawBoundingBox(const kexBBox &bbox,
                                    const byte r, const byte g, const byte b);
    static void     DrawFilledBoundingBox(const kexBBox &bbox,
                                          const byte r, const byte g, const byte b);
    static void     DrawRadius(float x, float y, float z,
                               float radius, float height,
                               const byte r, const byte g, const byte b);
    static void     DrawOrigin(float x, float y, float z, float size);
    static void     DrawSphere(float x, float y, float z, float radius,
                               const byte r, const byte g, const byte b);
    static void     DrawLine(const kexVec3 &p1, const kexVec3 &p2,
                             const byte r, const byte g, const byte b);
    static void     PrintStatsText(const char *title, const char *s, ...);
    
    static void     ClearDebugLine(void) { debugLineNum = DEBUG_LINE_TOP; }
    static void     AddDebugLineSpacing(void) { debugLineNum += 16; }
    
    static float    debugLineNum;
};

#endif
