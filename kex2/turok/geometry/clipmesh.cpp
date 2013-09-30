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
#include "zone.h"
#include "clipmesh.h"
#include "actor.h"
#include "renderSystem.h"

enum {
    scClipMesh_type = 0,
    scClipMesh_end
};

static const sctokens_t clipMeshTokens[scClipMesh_end+1] = {
    { scClipMesh_type,           "type"                 },
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
                Z_Free(this->cmGroups[i].points);
                this->cmGroups[i].points = NULL;
            }
            if(this->cmGroups[i].indices) {
                Z_Free(this->cmGroups[i].indices);
                this->cmGroups[i].indices = NULL;
            }
            if(this->cmGroups[i].triangles) {
                Z_Free(this->cmGroups[i].triangles);
                this->cmGroups[i].triangles = NULL;
            }
        }

        this->numGroups = 0;
        Z_Free(this->cmGroups);
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

    origin                  = bbox.Center();
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Z_Calloc(sizeof(cmGroup_t) * numGroups, PU_CM, NULL);
    cmGroup                 = &cmGroups[0];
    cmGroup->numPoints      = 8;
    cmGroup->numIndices     = 36;
    cmGroup->numTriangles   = 12;
    cmGroup->points         = (kexVec3*)Z_Calloc(sizeof(kexVec3) * cmGroup->numPoints, PU_CM, NULL);
    cmGroup->indices        = (word*)Z_Calloc(sizeof(word) * cmGroup->numIndices, PU_CM, NULL);
    cmGroup->triangles      = (kexTri*)Z_Calloc(sizeof(kexTri) * cmGroup->numTriangles, PU_CM, NULL);
    word *indices           = cmGroup->indices;
    kexVec3 *points         = cmGroup->points;
    
    indices[ 0] = 3; indices[ 1] = 1; indices[ 2] = 0;
    indices[ 3] = 5; indices[ 4] = 7; indices[ 5] = 4;
    indices[ 6] = 1; indices[ 7] = 4; indices[ 8] = 0;
    indices[ 9] = 6; indices[10] = 5; indices[11] = 1;
    indices[12] = 7; indices[13] = 6; indices[14] = 2;
    indices[15] = 3; indices[16] = 0; indices[17] = 4;
    indices[18] = 3; indices[19] = 2; indices[20] = 1;
    indices[21] = 5; indices[22] = 6; indices[23] = 7;
    indices[24] = 6; indices[25] = 1; indices[26] = 2;
    indices[27] = 7; indices[28] = 2; indices[29] = 3;
    indices[30] = 3; indices[31] = 4; indices[32] = 7;
    indices[33] = 1; indices[34] = 5; indices[35] = 4;

    kexBBox newBox = bbox + -bbox.Radius();
    
    points[0].x = newBox.max[0];
    points[0].y = newBox.min[1];
    points[0].z = newBox.min[2];
    points[1].x = newBox.max[0];
    points[1].y = newBox.min[1];
    points[1].z = newBox.max[2];
    points[2].x = newBox.min[0];
    points[2].y = newBox.min[1];
    points[2].z = newBox.max[2];
    points[3]   = newBox.min;
    points[4].x = newBox.max[0];
    points[4].y = newBox.max[1];
    points[4].z = newBox.min[2];
    points[5]   = newBox.max;
    points[6].x = newBox.min[0];
    points[6].y = newBox.max[1];
    points[6].z = newBox.max[2];
    points[7].x = newBox.min[0];
    points[7].y = newBox.max[1];
    points[7].z = newBox.min[2];
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

    origin                  = bbox.Center();
    s                       = bbox.max - origin;
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Z_Calloc(sizeof(cmGroup_t) * numGroups, PU_CM, NULL);
    cmGroup                 = &cmGroups[0];
    cmGroup->numPoints      = 4;
    cmGroup->numIndices     = 12;
    cmGroup->numTriangles   = 4;
    cmGroup->points         = (kexVec3*)Z_Calloc(sizeof(kexVec3) * cmGroup->numPoints, PU_CM, NULL);
    cmGroup->indices        = (word*)Z_Calloc(sizeof(word) * cmGroup->numIndices, PU_CM, NULL);
    cmGroup->triangles      = (kexTri*)Z_Calloc(sizeof(kexTri) * cmGroup->numTriangles, PU_CM, NULL);
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

    origin                  = bbox.Center();
    s                       = bbox.max - origin;
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Z_Calloc(sizeof(cmGroup_t) * numGroups, PU_CM, NULL);
    cmGroup                 = &cmGroups[0];
    cmGroup->numPoints      = 6;
    cmGroup->numIndices     = 24;
    cmGroup->numTriangles   = 8;
    cmGroup->points         = (kexVec3*)Z_Calloc(sizeof(kexVec3) * cmGroup->numPoints, PU_CM, NULL);
    cmGroup->indices        = (word*)Z_Calloc(sizeof(word) * cmGroup->numIndices, PU_CM, NULL);
    cmGroup->triangles      = (kexTri*)Z_Calloc(sizeof(kexTri) * cmGroup->numTriangles, PU_CM, NULL);
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

    origin                  = bbox.Center();
    numGroups               = 1;
    cmGroups                = (cmGroup_t*)Z_Calloc(sizeof(cmGroup_t) * numGroups, PU_CM, NULL);
    cmGroup                 = &cmGroups[0];
    cmGroup->numPoints      = 20;
    cmGroup->numIndices     = 108;
    cmGroup->numTriangles   = 36;
    cmGroup->points         = (kexVec3*)Z_Calloc(sizeof(kexVec3) * cmGroup->numPoints, PU_CM, NULL);
    cmGroup->indices        = (word*)Z_Calloc(sizeof(word) * cmGroup->numIndices, PU_CM, NULL);
    cmGroup->triangles      = (kexTri*)Z_Calloc(sizeof(kexTri) * cmGroup->numTriangles, PU_CM, NULL);
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
// kexClipMesh::CreateMeshFromModel
//

void kexClipMesh::CreateMeshFromModel(void) {
    const kmodel_t *model = owner->Model();

    if(model == NULL) {
        return;
    }

    // TODO - support variants and child nodes
    mdlmesh_t *mesh = &model->nodes[0].meshes[0];

    origin      = owner->BoundingBox().Center();
    numGroups   = mesh->numsections;
    cmGroups    = (cmGroup_t*)Z_Calloc(sizeof(cmGroup_t) * numGroups, PU_CM, NULL);

    for(unsigned int i = 0; i < mesh->numsections; i++) {
        mdlsection_t *sec       = &mesh->sections[i];
        cmGroup_t *cmGroup      = &cmGroups[i];
        cmGroup->numIndices     = sec->numtris;
        cmGroup->numPoints      = sec->numverts;
        cmGroup->numTriangles   = sec->numtris / 3;
        cmGroup->points         = (kexVec3*)Z_Calloc(sizeof(kexVec3) * cmGroup->numPoints, PU_CM, NULL);
        cmGroup->indices        = (word*)Z_Calloc(sizeof(word) * cmGroup->numIndices, PU_CM, NULL);
        cmGroup->triangles      = (kexTri*)Z_Calloc(sizeof(kexTri) * cmGroup->numTriangles, PU_CM, NULL);

        for(unsigned int k = 0; k < cmGroup->numIndices; k++) {
            cmGroup->indices[k] = sec->tris[k];
        }

        for(unsigned int v = 0; v < cmGroup->numPoints; v++) {
            cmGroup->points[v].x = sec->xyz[v][0];
            cmGroup->points[v].y = sec->xyz[v][1];
            cmGroup->points[v].z = sec->xyz[v][2];
        }
    }
}

//
// kexClipMesh::Transform
//

void kexClipMesh::Transform(void) {
    if(owner == NULL) {
        return;
    }

    kexMatrix mtx = owner->Matrix();

    for(unsigned int g = 0; g < numGroups; g++) {
        cmGroup_t *cmGroup = &cmGroups[g];

        for(unsigned int i = 0; i < cmGroup->numPoints; i++) {
            cmGroup->points[i] |= mtx;
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
        CreateBox(owner->BoundingBox());
        break;
    case CMT_TETRAHEDRON:
        CreateTetrahedron(owner->BoundingBox());
        break;
    case CMT_OCTAHEDRON:
        CreateOctahedron(owner->BoundingBox());
        break;
    case CMT_DODECAHEDRON:
        CreateDodecahedron(owner->BoundingBox());
        break;
    case CMT_MESH:
        CreateMeshFromModel();
        break;
    case CMT_CONVEXHULL:    // TODO
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

            tri->point[0] = &cmGroup->points[cmGroup->indices[i * 3 + 0]];
            tri->point[1] = &cmGroup->points[cmGroup->indices[i * 3 + 1]];
            tri->point[2] = &cmGroup->points[cmGroup->indices[i * 3 + 2]];
            tri->plane.SetNormal(
                *tri->point[0],
                *tri->point[1],
                *tri->point[2]);
            tri->plane.SetDistance(*tri->point[0]);

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
                        if(pt1 == nPt1 && pt2 == nPt2) {
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

bool kexClipMesh::Trace(kexPhysics *physics,
                        const kexVec3 &start,
                        const kexVec3 &end,
                        const kexVec3 &dir) {
    kexPluecker r;
    float frac = 1;
    r.SetRay(start, dir);

    float radius = static_cast<kexWorldActor*>(physics->GetOwner())->Radius();

    for(unsigned int i = 0; i < numGroups; i++) {
        cmGroup_t *cmGroup = &cmGroups[i];

        for(unsigned int j = 0; j < cmGroup->numTriangles; j++) {
            if(dir.Dot(cmGroup->triangles[j].plane.Normal()) >= 0) {
                continue;
            }

            kexPluecker l;
            byte bits = 0;

            kexTri *tri = &cmGroup->triangles[j];

            for(int k = 0; k < 3; k++) {
                kexVec3 *pt1 = tri->point[(1+k)%3];
                kexVec3 *pt2 = tri->point[(0+k)%3];

                kexVec3 edge;

                edge.x = pt1->x - pt2->x;
                edge.y = pt1->y - pt2->y;
                edge.z = pt1->z - pt2->z;

                l.SetLine(*pt1, *pt2);
                float pd = l.InnerProduct(r);
                float r = (float)sqrt(edge.UnitSq()*(radius*radius));
                float rpd = pd - r;

                bits |= (FLOATSIGNBIT(rpd) << k);
            }

            if(bits != 0x7) {
                continue;
            }

            float d = tri->point[0]->Dot(tri->plane.Normal());

            float d1 = start.Dot(tri->plane.Normal()) - (d + radius);
            float d2 = end.Dot(tri->plane.Normal()) - (d + radius);

            if(d1 <= d2)
                continue;

            if(d1 < 0)
                continue;

            if(d2 > 0)
                continue;

            frac = (d1 / (d1 - d2));
            if(frac < 0 || frac >= physics->traceInfo.fraction)
                continue;

            physics->traceInfo.fraction = frac;
            physics->traceInfo.hitNormal = tri->plane.Normal();
            physics->traceInfo.hitMesh = this;
            physics->traceInfo.hitTri = tri;
            physics->traceInfo.hitVector = (start + ((end - start) * frac));
        }
    }

    return (frac != 1);
}

//
// kexClipMesh::DebugDraw
//

void kexClipMesh::DebugDraw(void) {
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

        dglColor4ub(
            0xFF - (0x50 * (i & 3)),
            0xFF * (i & 1),
            0x50 * (i & 3),
            192);

        dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3),
            reinterpret_cast<float*>(&cmGroup->points[0].x));
        dglDrawElements(GL_TRIANGLES, cmGroup->numIndices,
            GL_UNSIGNED_SHORT, cmGroup->indices);

        dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        dglDrawElements(GL_TRIANGLES, cmGroup->numIndices,
            GL_UNSIGNED_SHORT, cmGroup->indices);

        dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    dglEnableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_ALPHATEST, false);
    renderSystem.SetState(GLSTATE_LIGHTING, true);
}
