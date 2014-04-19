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

#ifndef __RENDER_MAIN_H__
#define __RENDER_MAIN_H__

class kexMaterial;
class kexSector;
class kexTri;

typedef struct {
    unsigned int            flags;
    unsigned int            numVerts;
    unsigned int            numIndices;
    kexVec3                 *vertices;
    float                   *coords;
    float                   *normals;
    byte                    *rgb;
    word                    *indices;
    kexMaterial             *material;
} surface_t;

#define MAX_FX_DISPLAYS 2048

class kexFx;

typedef struct {
    kexFx                   *fx;
} fxDisplay_t;

class kexRenderer {
public:
                            kexRenderer(void);
                            ~kexRenderer(void);

    void                    Init(void);
    void                    DrawSurface(const surface_t *surface, kexMaterial *material);
    void                    DrawWireFrameSurface(const surface_t *surface, const rcolor color);
    void                    Draw(void);
    void                    DrawFX(const fxDisplay_t *fxList, const int count);
    
    const surface_t         *currentSurface;

private:
    void                    ProcessMotionBlur(void);

    kexMaterial             *motionBlurMaterial;
    kexMaterial             *wireframeMaterial;
    kexMatrix               prevMVMatrix;
};

extern kexRenderer renderer;

#endif
