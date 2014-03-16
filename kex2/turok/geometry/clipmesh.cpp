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
// DESCRIPTION: Clip meshes
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "clipmesh.h"
#include "actor.h"
#include "renderSystem.h"
#include "renderWorld.h"
#include "stanHull.h"

kexHeapBlock kexClipMesh::hb_clipMesh("clip mesh", false, NULL, NULL);

enum {
    scClipMesh_type = 0,
    scClipMesh_end
};

static const sctokens_t clipMeshTokens[scClipMesh_end+1] = {
    { scClipMesh_type,          "type"                  },
    { -1,                       NULL                    }
};

//
// kexClipMesh::kexClipMesh
//

kexClipMesh::kexClipMesh(void) {
    this->numGroups = 0;
    this->owner     = NULL;
    this->cmGroups  = NULL;
    this->type      = CMT_NONE;

    this->origin.Set(0, 0, 0);
}

//
// kexClipMesh::~kexClipMesh
//

kexClipMesh::~kexClipMesh(void) {
    if(this->cmGroups) {
        for(unsigned int i = 0; i < this->numGroups; i++) {
            this->cmGroups[i].numPoints     = 0;
            this->cmGroups[i].numIndices    = 0;
            this->cmGroups[i].numTriangles  = 0;
            if(this->cmGroups[i].points) {
                Mem_Free(this->cmGroups[i].points);
                this->cmGroups[i].points = NULL;
            }
            if(this->cmGroups[i].indices) {
                Mem_Free(this->cmGroups[i].indices);
                this->cmGroups[i].indices = NULL;
            }
            if(this->cmGroups[i].triangles) {
                Mem_Free(this->cmGroups[i].triangles);
                this->cmGroups[i].triangles = NULL;
            }
        }

        this->numGroups = 0;
        Mem_Free(this->cmGroups);
    }
}

//
// kexClipMesh::Parse
//

void kexClipMesh::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(clipMeshTokens, lexer->Token())) {
        case scClipMesh_type:
            type = (clipMeshType_t)lexer->GetNumber();
            break;
        default:
            if(lexer->TokenType() == TK_IDENIFIER) {
                parser.Error("kexClipMesh::Parse: unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }

        lexer->Find();
    }
}

//
// kexClipMesh::CreateBox
//

void kexClipMesh::CreateBox(const kexBBox &bbox) {
    cmGroup_t *cmGroup;

    origin                  = bbox.Center() - owner->GetOrigin();
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);
    cmGroup                 = &cmGroups[0];

    AllocateCmGroup(cmGroup, 8, 36);

    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;
    
    indices[ 0] = 0; indices[ 1] = 1; indices[ 2] = 3;
    indices[ 3] = 4; indices[ 4] = 7; indices[ 5] = 5;
    indices[ 6] = 0; indices[ 7] = 4; indices[ 8] = 1;
    indices[ 9] = 1; indices[10] = 5; indices[11] = 6;
    indices[12] = 2; indices[13] = 6; indices[14] = 7;
    indices[15] = 4; indices[16] = 0; indices[17] = 3;
    indices[18] = 1; indices[19] = 2; indices[20] = 3;
    indices[21] = 7; indices[22] = 6; indices[23] = 5;
    indices[24] = 2; indices[25] = 1; indices[26] = 6;
    indices[27] = 3; indices[28] = 2; indices[29] = 7;
    indices[30] = 7; indices[31] = 4; indices[32] = 3;
    indices[33] = 4; indices[34] = 5; indices[35] = 1;
    
    points[0].x = bbox.max[0];
    points[0].y = bbox.min[1];
    points[0].z = bbox.min[2];
    points[1].x = bbox.max[0];
    points[1].y = bbox.min[1];
    points[1].z = bbox.max[2];
    points[2].x = bbox.min[0];
    points[2].y = bbox.min[1];
    points[2].z = bbox.max[2];
    points[3]   = bbox.min;
    points[4].x = bbox.max[0];
    points[4].y = bbox.max[1];
    points[4].z = bbox.min[2];
    points[5]   = bbox.max;
    points[6].x = bbox.min[0];
    points[6].y = bbox.max[1];
    points[6].z = bbox.max[2];
    points[7].x = bbox.min[0];
    points[7].y = bbox.max[1];
    points[7].z = bbox.min[2];
}

//
// kexClipMesh::CreateTetrahedron
//

void kexClipMesh::CreateTetrahedron(const kexBBox &bbox) {
    float c1 = 0.4714045207f;
    float c2 = 0.8164965809f;
    float c3 = -0.3333333333f;
    kexVec3 s;
    cmGroup_t *cmGroup;

    origin                  = bbox.Center() - owner->GetOrigin();
    s                       = bbox.max - origin;
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);
    cmGroup                 = &cmGroups[0];

    AllocateCmGroup(cmGroup, 4, 12);

    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;

    indices[ 0] = 0; indices[ 1] = 1; indices[ 2] = 2;
    indices[ 3] = 0; indices[ 4] = 2; indices[ 5] = 3;
    indices[ 6] = 0; indices[ 7] = 3; indices[ 8] = 1;
    indices[ 9] = 1; indices[10] = 3; indices[11] = 2;

    points[0] = origin + kexVec3(0, 0, s.z);
    points[1] = origin + kexVec3(2.0f * c1 * s.x, 0, c3 * s.z);
    points[2] = origin + kexVec3(-c1 * s.x, c2 * s.y, c3 * s.z);
    points[3] = origin + kexVec3(-c1 * s.x, -c2 * s.y, c3 * s.z);
}

//
// kexClipMesh::CreateOctahedron
//

void kexClipMesh::CreateOctahedron(const kexBBox &bbox) {
    kexVec3 s;
    cmGroup_t *cmGroup;

    origin                  = bbox.Center() - owner->GetOrigin();
    s                       = bbox.max - origin;
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);
    cmGroup                 = &cmGroups[0];
    cmGroup->numPoints      = 6;
    cmGroup->numIndices     = 24;
    cmGroup->numTriangles   = 8;
    cmGroup->points         = (kexVec3*)Mem_Malloc(sizeof(kexVec3) * cmGroup->numPoints, kexClipMesh::hb_clipMesh);
    cmGroup->indices        = (word*)Mem_Malloc(sizeof(word) * cmGroup->numIndices, kexClipMesh::hb_clipMesh);
    cmGroup->triangles      = (kexTri*)Mem_Malloc(sizeof(kexTri) * cmGroup->numTriangles, kexClipMesh::hb_clipMesh);
    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;

    indices[ 0] = 4; indices[ 1] = 0; indices[ 2] = 2;
    indices[ 3] = 4; indices[ 4] = 2; indices[ 5] = 1;
    indices[ 6] = 4; indices[ 7] = 1; indices[ 8] = 3;
    indices[ 9] = 4; indices[10] = 3; indices[11] = 0;
    indices[12] = 5; indices[13] = 2; indices[14] = 0;
    indices[15] = 5; indices[16] = 1; indices[17] = 2;
    indices[18] = 5; indices[19] = 3; indices[20] = 1;
    indices[21] = 5; indices[22] = 0; indices[23] = 3;

    points[0] = origin + kexVec3(s.x, 0, 0);
    points[1] = origin + kexVec3(-s.x, 0, 0);
    points[2] = origin + kexVec3(0, s.y, 0);
    points[3] = origin + kexVec3(0, -s.y, 0);
    points[4] = origin + kexVec3(0, 0, s.z);
    points[5] = origin + kexVec3(0, 0, -s.z);
}

//
// kexClipMesh::CreateDodecahedron
//

void kexClipMesh::CreateDodecahedron(const kexBBox &bbox) {
    cmGroup_t *cmGroup;

    origin                  = bbox.Center() - owner->GetOrigin();
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);
    cmGroup                 = &cmGroups[0];

    AllocateCmGroup(cmGroup, 20, 108);

    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;

    indices[ 0] = 0;   indices[ 1] = 8;  indices[ 2] = 4;
    indices[ 3] = 8;   indices[ 4] = 9;  indices[ 5] = 4;
    indices[ 6] = 0;   indices[ 7] = 12; indices[ 8] = 8;
    indices[ 9] = 12;  indices[10] = 13; indices[11] = 1;
    indices[12] = 0;   indices[13] = 16; indices[14] = 2;
    indices[15] = 16;  indices[16] = 17; indices[17] = 2;
    indices[18] = 8;   indices[19] = 1;  indices[20] = 5;
    indices[21] = 1;   indices[22] = 18; indices[23] = 5;
    indices[24] = 12;  indices[25] = 2;  indices[26] = 3;
    indices[27] = 2;   indices[28] = 10; indices[29] = 3;
    indices[30] = 16;  indices[31] = 4;  indices[32] = 6;
    indices[33] = 4;   indices[34] = 14; indices[35] = 6;
    indices[36] = 9;   indices[37] = 5;  indices[38] = 14;
    indices[39] = 5;   indices[40] = 15; indices[41] = 14;
    indices[42] = 6;   indices[43] = 11; indices[44] = 2;
    indices[45] = 11;  indices[46] = 10; indices[47] = 2;
    indices[48] = 3;   indices[49] = 19; indices[50] = 1;
    indices[51] = 19;  indices[52] = 18; indices[53] = 1;
    indices[54] = 7;   indices[55] = 15; indices[56] = 18;
    indices[57] = 15;  indices[58] = 5;  indices[59] = 18;
    indices[60] = 7;   indices[61] = 11; indices[62] = 14;
    indices[63] = 11;  indices[64] = 6;  indices[65] = 14;
    indices[66] = 7;   indices[67] = 19; indices[68] = 10;
    indices[69] = 19;  indices[70] = 3;  indices[71] = 10;
    indices[72] = 16;  indices[73] = 0;  indices[74] = 4;
    indices[75] = 12;  indices[76] = 0;  indices[77] = 2;
    indices[78] = 9;   indices[79] = 8;  indices[80] = 5;
    indices[81] = 13;  indices[82] = 12; indices[83] = 3;
    indices[84] = 17;  indices[85] = 16; indices[86] = 6;
    indices[87] = 4;   indices[88] = 9;  indices[89] = 14;
    indices[90] = 17;  indices[91] = 6;  indices[92] = 2;
    indices[93] = 13;  indices[94] = 3;  indices[95] = 1;
    indices[96] = 19;  indices[97] = 7;  indices[98] = 18;
    indices[99] = 15;  indices[100] = 7; indices[101] = 14;
    indices[102] = 11; indices[103] = 7; indices[104] = 10;
    indices[105] = 12; indices[106] = 1; indices[107] = 8;

    float a[3], b[3], c[3];
    float s;

    a[0] = a[1] = a[2] = 0.5773502691896257f;
    b[0] = b[1] = b[2] = 0.3568220897730899f;
    c[0] = c[1] = c[2] = 0.9341723589627156f;

    float d = 0.5f / c[0];

    s = (bbox.max.x - bbox.min.x) * d;
    a[0] *= s;
    a[1] *= s;
    a[2] *= s;
    s = (bbox.max.y - bbox.min.y) * d;
    b[0] *= s;
    b[1] *= s;
    b[2] *= s;
    s = (bbox.max.z - bbox.min.z) * d;
    c[0] *= s;
    c[1] *= s;
    c[2] *= s;

    points[ 0].Set(origin[0] + a[0], origin[1] + a[1], origin[2] - a[2]);
    points[ 1].Set(origin[0] + a[0], origin[1] - a[1], origin[2] - a[2]);
    points[ 2].Set(origin[0] + a[0], origin[1] + a[1], origin[2] + a[2]);
    points[ 3].Set(origin[0] + a[0], origin[1] - a[1], origin[2] + a[2]);
    points[ 4].Set(origin[0] - a[0], origin[1] + a[1], origin[2] - a[2]);
    points[ 5].Set(origin[0] - a[0], origin[1] - a[1], origin[2] - a[2]);
    points[ 6].Set(origin[0] - a[0], origin[1] + a[1], origin[2] + a[2]);
    points[ 7].Set(origin[0] - a[0], origin[1] - a[1], origin[2] + a[2]);
    points[ 8].Set(origin[0] + b[0], origin[1]       , origin[2] - c[2]);
    points[ 9].Set(origin[0] - b[0], origin[1]       , origin[2] - c[2]);
    points[10].Set(origin[0] + b[0], origin[1]       , origin[2] + c[2]);
    points[11].Set(origin[0] - b[0], origin[1]       , origin[2] + c[2]);
    points[12].Set(origin[0] + c[2], origin[1] + b[1], origin[2]);
    points[13].Set(origin[0] + c[2], origin[1] - b[1], origin[2]);
    points[14].Set(origin[0] - c[2], origin[1] + b[1], origin[2]);
    points[15].Set(origin[0] - c[2], origin[1] - b[1], origin[2]);
    points[16].Set(origin[0]       , origin[1] + c[1], origin[2] - b[0]);
    points[17].Set(origin[0]       , origin[1] + c[1], origin[2] + b[0]);
    points[18].Set(origin[0]       , origin[1] - c[1], origin[2] - b[0]);
    points[19].Set(origin[0]       , origin[1] - c[1], origin[2] + b[0]);
}

//
// kexClipMesh::CreateCylinder
//

void kexClipMesh::CreateCylinder(const kexBBox &bbox) {
    cmGroup_t *cmGroup;
    float r;
    float h;

    origin                  = bbox.Center() - owner->GetOrigin();
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);
    cmGroup                 = &cmGroups[0];

    AllocateCmGroup(cmGroup, 16, 84);

    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;
    kexVec3 scale           = owner->GetScale();

    points[ 0].Set(0.000000f, 0.000000f, -1.000000f);
    points[ 1].Set(0.000000f, 1.000000f, -1.000000f);
    points[ 2].Set(0.707107f, 0.000000f, -0.707107f);
    points[ 3].Set(0.707107f, 1.000000f, -0.707107f);
    points[ 4].Set(1.000000f, 0.000000f, 0.000000f);
    points[ 5].Set(1.000000f, 1.000000f, 0.000000f);
    points[ 6].Set(0.707107f, 0.000000f, 0.707107f);
    points[ 7].Set(0.707107f, 1.000000f, 0.707107f);
    points[ 8].Set(-0.000000f, 0.000000f, 1.000000f);
    points[ 9].Set(-0.000000f, 1.000000f, 1.000000f);
    points[10].Set(-0.707107f, 0.000000f, 0.707107f);
    points[11].Set(-0.707107f, 1.000000f, 0.707107f);
    points[12].Set(-1.000000f, 0.000000f, -0.000000f);
    points[13].Set(-1.000000f, 1.000000f, -0.000000f);
    points[14].Set(-0.707107f, 0.000000f, -0.707107f);
    points[15].Set(-0.707107f, 1.000000f, -0.707107f);

    r = owner->Radius();
    h = owner->Height();

    for(unsigned int i = 0; i < cmGroup->numPoints; i++) {
        points[i].x *= r * (1 / scale.x);
        points[i].y *= h * (1 / scale.y);
        points[i].z *= r * (1 / scale.z);
    }

    indices[ 0] = 0;  indices[ 1] = 1;  indices[ 2] = 3;
    indices[ 3] = 2;  indices[ 4] = 3;  indices[ 5] = 5;
    indices[ 6] = 4;  indices[ 7] = 5;  indices[ 8] = 7;
    indices[ 9] = 6;  indices[10] = 7;  indices[11] = 9;
    indices[12] = 8;  indices[13] = 9;  indices[14] = 11;
    indices[15] = 10; indices[16] = 11; indices[17] = 13;
    indices[18] = 11; indices[19] = 15; indices[20] = 13;
    indices[21] = 14; indices[22] = 15; indices[23] = 1;
    indices[24] = 12; indices[25] = 13; indices[26] = 15;
    indices[27] = 2;  indices[28] = 4;  indices[29] = 6;
    indices[30] = 2;  indices[31] = 0;  indices[32] = 3;
    indices[33] = 4;  indices[34] = 2;  indices[35] = 5;
    indices[36] = 6;  indices[37] = 4;  indices[38] = 7;
    indices[39] = 8;  indices[40] = 6;  indices[41] = 9;
    indices[42] = 10; indices[43] = 8;  indices[44] = 11;
    indices[45] = 12; indices[46] = 10; indices[47] = 13;
    indices[48] = 1;  indices[49] = 15; indices[50] = 11;
    indices[51] = 1;  indices[52] = 11; indices[53] = 5;
    indices[54] = 9;  indices[55] = 7;  indices[56] = 11;
    indices[57] = 3;  indices[58] = 1;  indices[59] = 5;
    indices[60] = 7;  indices[61] = 5;  indices[62] = 11;
    indices[63] = 0;  indices[64] = 14; indices[65] = 1;
    indices[66] = 14; indices[67] = 12; indices[68] = 15;
    indices[69] = 0;  indices[70] = 2;  indices[71] = 6;
    indices[72] = 10; indices[73] = 6;  indices[74] = 8;
    indices[75] = 0;  indices[76] = 6;  indices[77] = 10;
    indices[78] = 14; indices[79] = 0;  indices[80] = 10;
    indices[81] = 12; indices[82] = 14; indices[83] = 10;
}

//
// kexClipMesh::CreateMeshFromModel
//

void kexClipMesh::CreateMeshFromModel(void) {
    const kexModel_t *model = owner->Model();

    if(model == NULL) {
        return;
    }

    // TODO - support variants and child nodes
    surfaceGroup_t *group = &model->nodes[0].surfaceGroups[0];
    surface_t *surface;
    cmGroup_t *cmGroup;

    origin = owner->Bounds().Center();
    numGroups = group->numSurfaces;

    if(numGroups <= 0) {
        return;
    }

    cmGroups = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);

    for(unsigned int i = 0; i < numGroups; i++) {
        surface = &group->surfaces[i];
        cmGroup = &cmGroups[i];

        AllocateCmGroup(cmGroup, surface->numVerts, surface->numIndices);

        for(unsigned int k = 0; k < cmGroup->numIndices; k++) {
            cmGroup->indices[k] = surface->indices[k];
        }

        for(unsigned int v = 0; v < cmGroup->numPoints; v++) {
            cmGroup->points[v].x = surface->vertices[v][0];
            cmGroup->points[v].y = surface->vertices[v][1];
            cmGroup->points[v].z = surface->vertices[v][2];
        }
    }
}

//
// kexClipMesh::CreateConvexHull
//

void kexClipMesh::CreateConvexHull(void) {
    const kexModel_t *model = owner->Model();

    if(model == NULL) {
        return;
    }

    surfaceGroup_t *group;
    cmGroup_t *cmGroup;
    HullLibrary hl;
    HullResult result;
    HullError err;

    // TODO - support variants and child nodes
    group = &model->nodes[0].surfaceGroups[0];

    if(group->numSurfaces <= 0) {
        return;
    }

    origin = owner->Bounds().Center();
    numGroups = 1;

    cmGroups = (cmGroup_t*)Mem_Malloc(sizeof(cmGroup_t) * numGroups, kexClipMesh::hb_clipMesh);

    if(group->numSurfaces > 1) {
        unsigned int i;
        PxF32 *verts;
        int totalVerts = 0;
        int totalCopyVerts = 0;

        for(i = 0; i < group->numSurfaces; i++) {
            totalVerts += group->surfaces[i].numVerts;
        }

        verts = (PxF32*)Mem_Malloc(totalVerts * (sizeof(PxF32) * 3), hb_static);

        for(i = 0; i < group->numSurfaces; i++) {
            memcpy(&verts[totalCopyVerts], reinterpret_cast<PxF32*>(group->surfaces[i].vertices),
                (sizeof(PxF32) * 3) * group->surfaces[i].numVerts);

            totalCopyVerts += (group->surfaces[i].numVerts * 3);
        }

        HullDesc desc(QF_TRIANGLES, totalVerts, verts ,sizeof(PxF32) * 3);
        err = hl.CreateConvexHull(desc, result);

        Mem_Free(verts);
    }
    else {
        HullDesc desc(QF_TRIANGLES, group->surfaces[0].numVerts,
            reinterpret_cast<PxF32*>(group->surfaces[0].vertices) ,sizeof(PxF32) * 3);
        err = hl.CreateConvexHull(desc, result);
    }

    if(err == QE_OK) {
        cmGroup = &cmGroups[0];

        AllocateCmGroup(cmGroup, result.mNumOutputVertices, result.mNumIndices);

        for(unsigned int k = 0; k < cmGroup->numIndices; k++) {
            cmGroup->indices[k] = result.mIndices[k];
        }

        float *p;

        for(unsigned int v = 0; v < cmGroup->numPoints; v++) {
            p = &result.mOutputVertices[v*3];

            cmGroup->points[v].x = p[0];
            cmGroup->points[v].y = p[1];
            cmGroup->points[v].z = p[2];
        }
    }

    hl.ReleaseResult(result);
}

//
// kexClipMesh::AllocateCmGroup
//

void kexClipMesh::AllocateCmGroup(cmGroup_t *group, const int numPoints, const int numIndices) {
    group->numPoints    = numPoints;
    group->numIndices   = numIndices;
    group->numTriangles = numIndices / 3;
    group->points       = (kexVec3*)Mem_Malloc(sizeof(kexVec3) * group->numPoints, kexClipMesh::hb_clipMesh);
    group->indices      = (word*)Mem_Malloc(sizeof(word) * group->numIndices, kexClipMesh::hb_clipMesh);
    group->triangles    = (kexTri*)Mem_Malloc(sizeof(kexTri) * group->numTriangles, kexClipMesh::hb_clipMesh);
}

//
// kexClipMesh::Transform
//

void kexClipMesh::Transform(void) {
    if(owner == NULL || type == CMT_NONE) {
        return;
    }

    kexMatrix mtx = owner->Matrix();

    for(unsigned int g = 0; g < numGroups; g++) {
        cmGroup_t *cmGroup = &cmGroups[g];

        for(unsigned int i = 0; i < cmGroup->numPoints; i++) {
            if(type != CMT_BOX) {
                cmGroup->points[i] |= mtx;
            }

            for(unsigned int i = 0; i < cmGroup->numTriangles; i++) {
                kexTri *tri = &cmGroup->triangles[i];
                
                tri->plane.SetNormal(
                    *tri->point[0],
                    *tri->point[1],
                    *tri->point[2]);

                tri->plane.SetDistance(*tri->point[0]);
                tri->SetBounds();
                tri->SetPlueckerEdges();
            }
        }
    }
}

//
// kexClipMesh::CreateShape
//

void kexClipMesh::CreateShape(void) {
    if(owner == NULL) {
        return;
    }

    switch(type) {
    case CMT_BOX:
        CreateBox(owner->Bounds());
        break;
    case CMT_TETRAHEDRON:
        CreateTetrahedron(owner->Bounds());
        break;
    case CMT_OCTAHEDRON:
        CreateOctahedron(owner->Bounds());
        break;
    case CMT_DODECAHEDRON:
        CreateDodecahedron(owner->Bounds());
        break;
    case CMT_MESH:
        CreateMeshFromModel();
        break;
    case CMT_CYLINDER:
        CreateCylinder(owner->Bounds());
        break;
    case CMT_CONVEXHULL:
        CreateConvexHull();
        break;
    case CMT_CUSTOM:        // TODO
        return;
    default:
        return;
    }

    for(unsigned int g = 0; g < numGroups; g++) {
        cmGroup_t *cmGroup = &cmGroups[g];

        // setup triangle data
        for(unsigned int i = 0; i < cmGroup->numTriangles; i++) {
            kexTri *tri = &cmGroup->triangles[i];

            tri->id = kexTri::globalID++;
            tri->Set(&cmGroup->points[cmGroup->indices[i * 3 + 0]],
                     &cmGroup->points[cmGroup->indices[i * 3 + 1]],
                     &cmGroup->points[cmGroup->indices[i * 3 + 2]]);

            // link triangle edges
            for(int j = 0; j < 3; j++) {
                if(tri->edgeLink[j] != NULL)
                    continue;

                kexVec3 *pt1 = tri->point[j];
                kexVec3 *pt2 = tri->point[(j+1)%3];
                bool ok = false;

                // scan through all triangles for matching edges
                for(unsigned int k = 0; k < cmGroup->numTriangles; k++) {
                    // don't check itself
                    if(k == i) {
                        continue;
                    }

                    kexTri *nTri = &cmGroup->triangles[k];

                    for(int n = 0; n < 3; n++) {
                        kexVec3 *nPt1 = nTri->point[n];
                        kexVec3 *nPt2 = nTri->point[(n+1)%3];

                        // points share an edge so link it
                        if(pt1 == nPt2 && pt2 == nPt1) {
                            tri->edgeLink[j] = nTri;
                            ok = true;
                            break;
                        }
                    }

                    if(ok == true) {
                        break;
                    }
                }
            }
        }
    }
}

//
// kexClipMesh::Trace
//

bool kexClipMesh::Trace(traceInfo_t *trace) {
    float frac = 1;
    float r = 0;
    kexTri *tri;
    float dist;
    float distStart;
    float distEnd;
    kexVec3 hit;
    kexVec3 offset;
    cmGroup_t *cmGroup;

    for(unsigned int i = 0; i < numGroups; i++) {
        cmGroup = &cmGroups[i];

        for(unsigned int j = 0; j < cmGroup->numTriangles; j++) {
            tri = &cmGroup->triangles[j];

            // direction must be facing the plane
            if(tri->plane.Distance(trace->dir) >= 0) {
                continue;
            }

            if(trace->bUseBBox) {
                offset.x = tri->plane.a < 0 ? trace->localBBox.max.x : trace->localBBox.min.x;
                offset.y = tri->plane.b < 0 ? trace->localBBox.max.y : trace->localBBox.min.y;
                offset.z = tri->plane.c < 0 ? trace->localBBox.max.z : trace->localBBox.min.z;

                r = -offset.Dot(tri->plane.Normal());
            }

            dist = tri->plane.d + r;

            distStart = tri->plane.Distance(trace->start) - dist;
            distEnd = tri->plane.Distance(trace->end) - dist;

            if(distStart <= distEnd || distStart < 0 || distEnd > 0) {
                continue;
            }

            frac = (distStart / (distStart - distEnd));

            if(frac > 1) {
                continue;
            }

            if(frac < 0) {
                if(trace->bUseBBox == false) {
                    continue;
                }
            }

            // check if something closer was hit
            if(frac >= trace->fraction) {
                continue;
            }

            hit = trace->start.Lerp(trace->end, frac);

            // check if hit vector lies within the triangle's edges
            if(!tri->PointInRange(hit, 0.1f)) {
                if(trace->bUseBBox) {
                    kexPluecker bp[8];
                    kexVec3 bMin;
                    kexVec3 bMax;
                    int bit[3];

                    bMin = trace->bbox.min;
                    bMax = trace->bbox.max;

                    bit[0] = bit[1] = bit[2] = 0;

                    // setup pluecker coordinates from each bounding box point
                    bp[0].SetRay(kexVec3(bMin.x, bMin.y, bMin.z), trace->dir);
                    bp[1].SetRay(kexVec3(bMin.x, bMax.y, bMin.z), trace->dir);
                    bp[2].SetRay(kexVec3(bMax.x, bMin.y, bMin.z), trace->dir);
                    bp[3].SetRay(kexVec3(bMax.x, bMax.y, bMin.z), trace->dir);
                    bp[4].SetRay(kexVec3(bMin.x, bMin.y, bMax.z), trace->dir);
                    bp[5].SetRay(kexVec3(bMin.x, bMax.y, bMax.z), trace->dir);
                    bp[6].SetRay(kexVec3(bMax.x, bMin.y, bMax.z), trace->dir);
                    bp[7].SetRay(kexVec3(bMax.x, bMax.y, bMax.z), trace->dir);

                    for(int k = 0; k < 3; k++) {
                        for(int l = 0; l < 8; l++) {
                            float d = bp[l].InnerProduct(tri->plEdge[k]);
                            bit[k] |= FLOATSIGNBIT(d) << l;
                        }
                    }

                    // abort if bounding box didn't make contact with one of the edges
                    if(bit[0] == 0xff || bit[1] == 0xff || bit[2] == 0xff) {
                        continue;
                    }
                }
                else {
                    continue;
                }
            }

            trace->fraction = frac;
            trace->hitNormal = tri->plane.Normal();
            trace->hitMesh = this;
            trace->hitTri = tri;
            trace->hitVector = hit;
        }
    }

    return (frac != 1);
}

//
// kexClipMesh::DebugDraw
//

void kexClipMesh::DebugDraw(void) {
    static rcolor colors[6] = {
        RGBA(255, 128, 128, 128),
        RGBA(255, 96, 255, 128),
        RGBA(192, 192, 192, 128),
        RGBA(255, 128, 0, 128),
        RGBA(255, 255, 0, 128),
        RGBA(0, 64, 96, 128)
    };
    if(owner == NULL || type == CMT_NONE) {
        return;
    }
    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_ALPHATEST, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    dglDisableClientState(GL_NORMAL_ARRAY);
    dglDisableClientState(GL_TEXTURE_COORD_ARRAY);

    for(unsigned int i = 0; i < numGroups; i++) {
        cmGroup_t *cmGroup = &cmGroups[i];

        for(unsigned int j = 0; j < cmGroup->numTriangles; j++) {
            kexTri *tri = &cmGroup->triangles[j];

            if(tri->bTraced == false) {
                continue;
            }

            dglColor4ub(0xFF, 0xFF, 0xFF, 192);
            dglBegin(GL_TRIANGLES);
            dglVertex3f((*tri->point[0]).x, (*tri->point[0]).y, (*tri->point[0]).z);
            dglVertex3f((*tri->point[1]).x, (*tri->point[1]).y, (*tri->point[1]).z);
            dglVertex3f((*tri->point[2]).x, (*tri->point[2]).y, (*tri->point[2]).z);
            dglEnd();

            tri->bTraced = false;
        }

        dglColor4ubv((byte*)&colors[i%6]);

        dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3),
            reinterpret_cast<float*>(&cmGroup->points[0].x));
        dglDrawElements(GL_TRIANGLES, cmGroup->numIndices,
            GL_UNSIGNED_SHORT, cmGroup->indices);

        renderSystem.SetPolyMode(GLPOLY_LINE);
        dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        dglDrawElements(GL_TRIANGLES, cmGroup->numIndices,
            GL_UNSIGNED_SHORT, cmGroup->indices);

        renderSystem.SetPolyMode(GLPOLY_FILL);
    }

    dglEnableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_ALPHATEST, false);
    renderSystem.SetState(GLSTATE_LIGHTING, true);
#if 0
    for(unsigned int i = 0; i < numGroups; i++) {
        cmGroup_t *cmGroup = &cmGroups[i];

        for(unsigned int j = 0; j < cmGroup->numTriangles; j++) {
            renderWorld.DrawBoundingBox(cmGroup->triangles[j].bounds, 255, 255, 0);
        }
    }
#endif
}
