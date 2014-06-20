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

#ifndef __CPUVERTEXLIST_H__
#define __CPUVERTEXLIST_H__

#define CPU_VERT_MAXSIZE        512

#define CPU_VERT_NUM_XYZ        CPU_VERT_MAXSIZE * 3
#define CPU_VERT_NUM_COORDS     CPU_VERT_MAXSIZE * 2
#define CPU_VERT_NUM_NORMALS    CPU_VERT_MAXSIZE * 3
#define CPU_VERT_NUM_COLOR      CPU_VERT_MAXSIZE * 4
#define CPU_VERT_NUM_INDICES    CPU_VERT_MAXSIZE * 3

class kexMaterial;

class kexCpuVertList {
public:
                            kexCpuVertList(void);
                            ~kexCpuVertList(void);

    void                    Reset(void);
    void                    BindDrawPointers(void);
    void                    AddTriangle(int v0, int v1, int v2);
    void                    AddVertex(float x, float y, float z, float s, float t,
                                      byte r, byte g, byte b, byte a);
    void                    AddVertex(float x, float y, float z, float s, float t, byte *rgba);
    void                    AddLine(float x1, float y1, float z1,
                                    float x2, float y2, float z2,
                                    byte r, byte g, byte b, byte a);
    void                    AddLine(float x1, float y1, float z1,
                                    float x2, float y2, float z2,
                                    byte r1, byte g1, byte b1, byte a1,
                                    byte r2, byte g2, byte b2, byte a2);
    void                    DrawElements(const bool bClearCount = true);
    void                    DrawElements(const kexMaterial *material, const bool bClearCount = true);
    void                    DrawElementsNoShader(const bool bClearCount = true);
    void                    DrawLineElements(void);

private:
    word                    indiceCount;
    word                    vertexCount;
    word                    drawIndices[CPU_VERT_NUM_INDICES];
    float                   drawVertices[CPU_VERT_NUM_XYZ];
    float                   drawTexCoords[CPU_VERT_NUM_COORDS];
    float                   drawNormals[CPU_VERT_NUM_NORMALS];
    byte                    drawRGB[CPU_VERT_NUM_COLOR];
    
    word                    *roverIndices;
    float                   *roverVertices;
    float                   *roverTexCoords;
    byte                    *roverRGB;
};

extern kexCpuVertList cpuVertList;

#endif
