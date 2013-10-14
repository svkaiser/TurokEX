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

#ifndef __CLIPMESH_H__
#define __CLIPMESH_H__

#include "mathlib.h"
#include "script.h"
#include "triangle.h"
#include "physics.h"

typedef enum {
    CMT_NONE            = 0,
    CMT_BOX             = 1,
    CMT_TETRAHEDRON     = 2,
    CMT_OCTAHEDRON      = 3,
    CMT_DODECAHEDRON    = 4,
    CMT_CONVEXHULL      = 5,
    CMT_MESH            = 6,
    CMT_CYLINDER        = 7,
    CMT_CUSTOM          = 8
} clipMeshType_t;

class kexWorldActor;

class kexClipMesh {
public:
                            kexClipMesh(void);
                            ~kexClipMesh(void);

    void                    Parse(kexLexer *lexer);
    void                    DebugDraw(void);
    void                    CreateShape(void);
    void                    Transform(void);
    bool                    Trace(kexPhysics *physics,
                                  const kexVec3 &start,
                                  const kexVec3 &end,
                                  const kexVec3 &dir);

    const clipMeshType_t    GetType(void) const { return type; }
    void                    SetType(const clipMeshType_t _type) { type = _type; }
    kexWorldActor           *GetOwner(void) { return owner; }
    void                    SetOwner(kexWorldActor *actor) { owner = actor; }

private:
    void                    CreateBox(const kexBBox &bbox);
    void                    CreateTetrahedron(const kexBBox &bbox);
    void                    CreateOctahedron(const kexBBox &bbox);
    void                    CreateDodecahedron(const kexBBox &bbox);
    void                    CreateCylinder(const kexBBox &bbox);
    void                    CreateMeshFromModel(void);

    typedef struct {
        unsigned int        numPoints;    
        kexVec3             *points;
        unsigned int        numIndices;
        word                *indices;
        unsigned int        numTriangles;
        kexTri              *triangles;
    } cmGroup_t;

    unsigned int            numGroups;
    cmGroup_t               *cmGroups;
    kexVec3                 origin;
    clipMeshType_t          type;
    kexWorldActor           *owner;
};

#endif
