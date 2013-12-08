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
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#ifndef EDITOR
#include "common.h"
#else
#include "editorCommon.h"
#endif
#include "binFile.h"
#include "fileSystem.h"
#include "collisionMap.h"
#include "renderSystem.h"

#define CM_ID_HEADER    0
#define CM_ID_DATASIZE  1
#define CM_ID_POINTS    2
#define CM_ID_SECTORS   3

#define STEPHEIGHT      12.0f

kexHeapBlock kexCollisionMap::hb_collisionMap("collision map", false, NULL, NULL);

//
// kexSector::kexSector
//

kexSector::kexSector(void) {
    this->link[0]   = NULL;
    this->link[1]   = NULL;
    this->link[2]   = NULL;
    this->area      = NULL;
    this->bTraced   = false;
}

//
// kexSector::Wall
//

bool kexSector::Wall(void) {
    if(!(flags & CLF_SLOPETEST)) {
        return (flags & CLF_FRONTNOCLIP) != 0;
    }

    return (lowerTri.plane.b <= 0.5f);
}

//
// kexSector::kexSector
//

bool kexSector::InRange(const kexVec3 &origin) {
    kexVec3 org = origin;

    org[1] -= (org[1] - lowerTri.GetDistance(org));
    return lowerTri.PointInRange(org, 0);
}

//
// kexSector::CheckHeight
//

bool kexSector::CheckHeight(const kexVec3 &pos) {
    if(Wall()) {
        float y = pos[1] + 16.384f;

        if( y > *lowerTri.point[0][1] &&
            y > *lowerTri.point[1][1] &&
            y > *lowerTri.point[2][1]) {
                return false;
        }
    }
    return true;
}

//
// kexSector::IntersectEdge
//

bool kexSector::IntersectEdge(cMapTrace_t *trace, const kexVec3 pt1, const kexVec3 pt2) {
    float x;
    float z;
    float dx;
    float dz;
    float d;

    x = pt2[2] - pt1[2];
    z = pt1[0] - pt2[0];

    d = x * (trace->end[0] - trace->start[0]) +
        z * (trace->end[2] - trace->start[2]);

    if(d < 0) {
        dx = pt1[0] - trace->end[0];
        dz = pt1[2] - trace->end[2];

        d = -(x * dx + z * dz) / d;

        if(d >= 0 && d < 1 && d < trace->result->fraction) {
            trace->result->fraction = d;
            trace->result->position.Lerp(trace->start, trace->end, d);
            trace->result->normal.Set(x, 0, z);
            trace->result->normal.Normalize();
            return true;
        }
    }

    return false;
}

//
// kexSector::CrossEdge
//

kexSector *kexSector::CrossEdge(cMapTrace_t *trace, const int edge) {
    kexSector *next = link[edge];

    if(next == NULL) {
        return NULL;
    }

    cMapTraceResult_t *result = trace->result;
    kexTri *curTri = &lowerTri;
    kexTri *nextTri = &next->lowerTri;

    // don't cross blocking sectors
    if(next->flags & CLF_BLOCK && !(next->flags & CLF_TOGGLE)) {
        result->contactSector = next;
        return NULL;
    }

    if(Wall()) {
        if(!next->Wall()) {
            return next;
        }

        if(flags & CLF_CLIMB && !(next->flags & CLF_CLIMB) &&
            curTri->GetDistance(result->position) + 1.024f > result->position[1]) {
                return NULL;
        }
    }

    // moving in and out of water
    if(flags & CLF_WATER && !(next->flags & CLF_WATER) &&
        trace->flags & PF_NOEXITWATER) {
            return NULL;
    }

    if(!(flags & CLF_WATER) && next->flags & CLF_WATER &&
        trace->flags & PF_NOENTERWATER) {
            return NULL;
    }

    // crossing into a wall or a very steep slope
    if(next->Wall() && !Wall()) {
        float dist1 = nextTri->GetDistance(trace->end);
        float dist2 = curTri->GetDistance(trace->start);

        // handle steps and drop-offs
        if(dist1 <= dist2) {
            float len = (nextTri->point[0]->y +
                nextTri->point[1]->y +
                nextTri->point[2]->y) / 3;

            if(len < 0) {
                len = -len;
            }

            if(!next->CheckHeight(trace->end) && len >= STEPHEIGHT &&
                !(trace->flags & PF_DROPOFF)) {
                return NULL;
            }

            // able to step off into this plane
            return next;
        }

        // special case for planes flagged to block
        // from the front side. these will be treated as
        // solid walls. direction of ray must be facing
        // towards the plane
        if(next->CheckHeight(trace->end) &&
            nextTri->plane.IsFacing(trace->direction.ToYaw())) {
                result->contactSector = next;
                return NULL;
        }
    }

    return next;
}

//
// kexCollisionMap::kexCollisionMap
//

kexCollisionMap::kexCollisionMap(void) {
    this->bLoaded   = false;
    this->points[0] = NULL;
    this->points[1] = NULL;
    this->sectors   = NULL;
    this->areas     = NULL;
}

//
// kexCollisionMap::~kexCollisionMap
//

kexCollisionMap::~kexCollisionMap(void) {
}

//
// kexCollisionMap::Load
//

void kexCollisionMap::Load(const char *name) {
    kexBinFile binFile;
    kexSector *sec;
    float *pointPtrs;
    int pt[3];
    int edge[3];
    int i;
    int j;
    int area_id;

    if(!binFile.Open(name)) {
        common.Warning("kexCollisionMap::Load: %s not found\n", name);
        return;
    }

    pointPtrs = (float*)binFile.GetOffset(CM_ID_POINTS, NULL, &numPoints);
    binFile.GetOffset(CM_ID_SECTORS, NULL, &numSectors);

    if(numSectors <= 0) {
        binFile.Close();
        return;
    }

    points[0] = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    points[1] = (kexVec3*)Mem_Calloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    indices = (word*)Mem_Calloc((sizeof(word) * numSectors) * 3,
        kexCollisionMap::hb_collisionMap);

    for(i = 0; i < numPoints; i++) {
        points[0][i].Set(
            pointPtrs[i * 4 + 0],
            pointPtrs[i * 4 + 1],
            pointPtrs[i * 4 + 2]);
        points[1][i].Set(
            pointPtrs[i * 4 + 0],
            pointPtrs[i * 4 + 3],
            pointPtrs[i * 4 + 2]);
    }

    sectors = (kexSector*)Mem_Calloc(sizeof(kexSector) * numSectors,
        kexCollisionMap::hb_collisionMap);

    for(i = 0; i < numSectors; i++) {
        sec = &sectors[i];

        area_id = binFile.Read16();
        sec->flags = binFile.Read16();

        for(j = 0; j < 3; j++) {
            pt[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            edge[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            sec->lowerTri.point[j] = reinterpret_cast<kexVec3*>(&points[0][pt[j]]);
            sec->upperTri.point[j] = reinterpret_cast<kexVec3*>(&points[1][pt[j]]);
            sec->lowerTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].lowerTri : NULL;
            sec->upperTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].upperTri : NULL;
            sec->link[j] = (edge[j] != -1) ? &sectors[edge[j]] : NULL;
        }

        indices[i * 3 + 0] = pt[0];
        indices[i * 3 + 1] = pt[1];
        indices[i * 3 + 2] = pt[2];

        // build plane for lower triangle
        sec->lowerTri.plane.SetNormal(
            *sec->lowerTri.point[0],
            *sec->lowerTri.point[1],
            *sec->lowerTri.point[2]);
        sec->lowerTri.plane.SetDistance(*sec->lowerTri.point[0]);
        sec->lowerTri.SetBounds();

        // build plane for upper triangle
        sec->upperTri.plane.SetNormal(
            *sec->upperTri.point[0],
            *sec->upperTri.point[1],
            *sec->upperTri.point[2]);
        sec->upperTri.plane.SetDistance(*sec->upperTri.point[0]);
        sec->upperTri.SetBounds();
    }

    binFile.Close();
    bLoaded = true;
}

//
// kexCollisionMap::PointInSector
//

kexSector *kexCollisionMap::PointInSector(const kexVec3 &origin) {
    float dist;
    float curdist = 0;
    kexSector *sector = NULL;
    bool ok = false;

    for(int i = 0; i < numSectors; i++) {
        kexSector *s;

        s = &sectors[i];

        if(s->InRange(origin)) {
            dist = origin[1] - s->lowerTri.GetDistance(origin);

            if(s->flags & CLF_ONESIDED && dist < -16) {
                continue;
            }

            if(dist < 0) {
                dist = -dist;
            }

            if(ok) {
                if(dist < curdist) {
                    curdist = dist;
                    sector = s;
                }
            }
            else {
                sector = s;
                curdist = dist;
                ok = true;
            }
        }
    }

    return sector;
}

//
// kexCollisionMap::TraverseSectors
//

void kexCollisionMap::TraverseSectors(cMapTrace_t *trace, kexSector *sector) {
    int i;
    kexSector *next = NULL;
    kexVec3 pt1;
    kexVec3 pt2;

    if(sector == NULL) {
        return;
    }

    trace->result->sector = sector;

    for(i = 0; i < 3; i++) {
        pt1 = *sector->lowerTri.point[(i+0)%3];
        pt2 = *sector->lowerTri.point[(i+1)%3];

        if(sector->IntersectEdge(trace, pt1, pt2)) {
            next = sector->CrossEdge(trace, i);
        }
    }

    if(next != NULL) {
        if(next->Wall()) {
            kexTri *tri     = &next->lowerTri;
            float distStart = tri->plane.Distance(trace->start) - tri->plane.d;
            float distEnd   = tri->plane.Distance(trace->end) - tri->plane.d;

            if(!(distStart <= distEnd || distStart < 0 || distEnd > 0)) {
                float frac = (distStart / (distStart - distEnd));

                if(frac >= 0 && frac <= 1 && frac < trace->result->fraction) {
                    kexVec3 hit = trace->start.Lerp(trace->end, frac);

                    if(next->CheckHeight(hit)) {
                        trace->result->position = hit;
                        trace->result->fraction = frac;
                        trace->result->contactSector = next;
                        return;
                    }
                }
            }
        }

        TraverseSectors(trace, next);
    }
}

//
// kexCollisionMap::Trace
//

void kexCollisionMap::Trace(cMapTraceResult_t *result,
                            const kexVec3 &start, const kexVec3 &end,
                            kexSector *sector,
                            const int flags) {
    cMapTrace_t trace;

    if(bLoaded == false || sector == NULL) {
        return;
    }

    trace.start     = start;
    trace.end       = end;
    trace.sector    = sector;
    trace.flags     = flags;
    trace.result    = result;
    trace.direction = (end - start).Normalize();

    trace.result->position      = end;
    trace.result->fraction      = 1.0f;
    trace.result->normal        = trace.direction;
    trace.result->sector        = sector;
    trace.result->contactSector = NULL;

    TraverseSectors(&trace, sector);
}

//
// kexCollisionMap::DebugDraw
//

void kexCollisionMap::DebugDraw(void) {
    if(numSectors <= 0 || numPoints <= 0 || sectors == NULL) {
        return;
    }

    renderSystem.SetState(GLSTATE_CULL, true);
    renderSystem.SetState(GLSTATE_TEXTURE0, false);
    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.SetState(GLSTATE_ALPHATEST, true);
    renderSystem.SetState(GLSTATE_LIGHTING, false);

    dglDisableClientState(GL_NORMAL_ARRAY);
    dglDisableClientState(GL_TEXTURE_COORD_ARRAY);

    for(int i = 0; i < numSectors; i++) {
        kexSector *sector = &sectors[i];
        kexTri *tri = &sector->lowerTri;

        if(sector->bTraced == false) {
            continue;
        }

        dglColor4ub(0xFF, 0xFF, 0xFF, 192);
        dglBegin(GL_TRIANGLES);
        dglVertex3f((*tri->point[0]).x, (*tri->point[0]).y, (*tri->point[0]).z);
        dglVertex3f((*tri->point[1]).x, (*tri->point[1]).y, (*tri->point[1]).z);
        dglVertex3f((*tri->point[2]).x, (*tri->point[2]).y, (*tri->point[2]).z);
        dglEnd();

        sector->bTraced = false;
    }

    dglColor4ub(255, 128, 128, 128);
    dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3),
        reinterpret_cast<float*>(points[0]));
    dglDrawElements(GL_TRIANGLES, numSectors * 3,
        GL_UNSIGNED_SHORT, indices);

    dglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    dglColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    dglDrawElements(GL_TRIANGLES, numSectors * 3,
        GL_UNSIGNED_SHORT, indices);

    dglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    dglEnableClientState(GL_NORMAL_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);

    renderSystem.SetState(GLSTATE_TEXTURE0, true);
    renderSystem.SetState(GLSTATE_BLEND, false);
    renderSystem.SetState(GLSTATE_ALPHATEST, false);
    renderSystem.SetState(GLSTATE_LIGHTING, true);
}
