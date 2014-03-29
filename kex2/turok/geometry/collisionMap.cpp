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
#include "world.h"
#include "fileSystem.h"
#include "collisionMap.h"
#include "renderSystem.h"

#define CM_ID_HEADER    0
#define CM_ID_DATASIZE  1
#define CM_ID_POINTS    2
#define CM_ID_SECTORS   3
#define CM_ID_AREAS     4

#define STEPHEIGHT      12.0f
#define CEILING_EXPAND  10.24f

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
// kexSector::GetID
//

const int kexSector::GetID(void) const {
    return this - localWorld.CollisionMap().sectors;
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

        if( y > lowerTri.point[0]->y &&
            y > lowerTri.point[1]->y &&
            y > lowerTri.point[2]->y) {
                return false;
        }
    }
    return true;
}

//
// kexSector::IntersectEdge
//

bool kexSector::IntersectEdge(cMapTrace_t *trace, const int edgeNum) {
    kexVec2 cp;
    kexVec2 tStart;
    kexVec2 tEnd;
    kexVec2 pt1;
    kexVec2 pt2;
    float d;

    pt1 = *lowerTri.point[(edgeNum+0)%3];
    pt2 = *lowerTri.point[(edgeNum+1)%3];

    tStart = trace->start;
    tEnd = trace->end;

    d = cp.Cross(pt1, pt2).Dot(tEnd - tStart);

    if(d < 0) {
        d = 1.0f + (cp.Dot(pt1 - tEnd) / d);

        if(d < 1 && d < trace->result->fraction) {
            trace->result->fraction = d;
            trace->result->position = trace->start.Lerp(trace->end, d);
            trace->result->normal.Set(cp[0], 0, cp[1]);
            trace->result->normal.Normalize();
            return true;
        }
    }

    return false;
}

//
// kexSector::Trace
//

bool kexSector::Trace(cMapTrace_t *trace, const bool bTestCeiling) {
    kexTri *tri = bTestCeiling ? &upperTri : &lowerTri;
    float distStart;
    float distEnd;
    float dist;

    if(tri->plane.Distance(trace->direction) >= 0) {
        return false;
    }

    dist = tri->plane.d;

    if(bTestCeiling) {
        dist -= (trace->height * tri->plane.Normal().y);
    }

    distStart = tri->plane.Distance(trace->start) - dist;
    distEnd   = tri->plane.Distance(trace->end) - dist;

    if(!(distStart <= distEnd || distStart < 0 || distEnd > 0)) {
        float frac = (distStart / (distStart - distEnd));

        if(frac >= 0 && frac <= 1 && frac < trace->result->fraction) {
            kexVec3 hit = trace->start.Lerp(trace->end, frac);
            float expand = 0;

            if(bTestCeiling) {
                expand = CEILING_EXPAND;
            }

            if(tri->PointInRange(hit, expand)) {
                trace->result->position = hit;
                trace->result->fraction = frac;
                trace->result->normal = tri->plane.Normal();
                trace->result->contactSector = this;
                return true;
            }
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

    if(next->flags & CLF_CHECKHEIGHT) {
        if((next->upperTri.GetDistance(result->position) - trace->height) < result->position[1]) {
            return NULL;
        }
    }

    if(Wall()) {
        if(next->flags & CLF_CLIMB) {
            return next;
        }

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
    if(next->Wall() && !Wall() && !(next->flags & CLF_CLIMB)) {
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
        if(next->CheckHeight(result->position) &&
            nextTri->plane.IsFacing(trace->direction.ToYaw())) {
                return NULL;
        }
    }

    return next;
}

//
// kexCollisionMap::kexCollisionMap
//

kexCollisionMap::kexCollisionMap(void) {
    this->bLoaded       = false;
    this->points[0]     = NULL;
    this->points[1]     = NULL;
    this->sectors       = NULL;
    this->numPoints     = 0;
    this->numSectors    = 0;
    this->numAreas      = 0;

    this->areas.Empty();
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
    kexArea *area;
    float *pointPtrs;
    int pt[3];
    int edge[3];
    int i;
    int j;
    int area_id;
    kexStr key;
    kexStr value;
    int len;
    int numKeys;

    if(!binFile.Open(name)) {
        common.Warning("kexCollisionMap::Load: %s not found\n", name);
        return;
    }

    binFile.GetOffset(CM_ID_AREAS, NULL, &numAreas);

    if(numAreas <= 0) {
        binFile.Close();
        common.Warning("kexCollisionMap::Load: couldn't load %s\n", name);
        return;
    }

    renderSystem.DrawLoadingScreen("Loading Areas...");

    for(i = 0; i < numAreas; i++) {
        if(!(area = static_cast<kexArea*>(localWorld.ConstructObject("kexArea")))) {
            continue;
        }

        numKeys = binFile.Read16();

        for(j = 0; j < numKeys; j++) {
            len = binFile.Read16();
            if(len <= 0) {
                // value should also be blank as well
                binFile.Read16();
                continue;
            }

            key = binFile.ReadString();

            len = binFile.Read16();
            if(len <= 0) {
                continue;
            }

            value = binFile.ReadString();
            area->keyMap.Add(key.c_str(), value.c_str());
            area->scriptComponent.SetID(i);
        }

        area->Setup();
        areas.Push(area);
    }

    bLoaded = true;

    pointPtrs = (float*)binFile.GetOffset(CM_ID_POINTS, NULL, &numPoints);
    binFile.GetOffset(CM_ID_SECTORS, NULL, &numSectors);

    if(numSectors <= 0 || numPoints <= 0) {
        binFile.Close();
        return;
    }

    points[0] = (kexVec3*)Mem_Malloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    points[1] = (kexVec3*)Mem_Malloc(sizeof(kexVec3) * numPoints,
        kexCollisionMap::hb_collisionMap);
    indices = (word*)Mem_Malloc((sizeof(word) * numSectors) * 3,
        kexCollisionMap::hb_collisionMap);

    renderSystem.DrawLoadingScreen("Loading Points...");

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

    renderSystem.DrawLoadingScreen("Loading Sectors...");

    for(i = 0; i < numSectors; i++) {
        sec = &sectors[i];

        area_id = binFile.Read16();

        if(area_id >= 0 && area_id < numAreas) {
            sec->area = areas[area_id];
        }

        sec->flags = binFile.Read16();

        for(j = 0; j < 3; j++) {
            pt[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            edge[j] = binFile.Read16();
        }
        for(j = 0; j < 3; j++) {
            sec->lowerTri.point[j] = reinterpret_cast<kexVec3*>(&points[0][pt[j]]);
            sec->upperTri.point[j] = reinterpret_cast<kexVec3*>(&points[1][pt[2 - j]]);
            sec->lowerTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].lowerTri : NULL;
            sec->upperTri.edgeLink[j] = (edge[j] != -1) ? &sectors[edge[j]].upperTri : NULL;
            sec->link[j] = (edge[j] != -1) ? &sectors[edge[j]] : NULL;
        }

        indices[i * 3 + 0] = pt[0];
        indices[i * 3 + 1] = pt[1];
        indices[i * 3 + 2] = pt[2];

        // build plane for lower triangle
        sec->lowerTri.Set(
            sec->lowerTri.point[0],
            sec->lowerTri.point[1],
            sec->lowerTri.point[2]);
        sec->lowerTri.id = kexTri::globalID++;
        sec->lowerTri.SetBounds();

        // build plane for upper triangle
        sec->upperTri.Set(
            sec->upperTri.point[0],
            sec->upperTri.point[1],
            sec->upperTri.point[2]);
        sec->upperTri.id = kexTri::globalID++;
        sec->lowerTri.SetBounds();
    }

    binFile.Close();

    renderSystem.DrawLoadingScreen("Setting Up Sector Stacks...");
    SetupSectorStackList();
}

//
// kexCollisionMap::Unload
//

void kexCollisionMap::Unload(void) {
    int i;
    kexArea *area;

    if(!bLoaded) {
        return;
    }

    if(areas.Length() > 0) {
        for(i = 0; i < numAreas; i++) {
            area = areas[i];

            delete area;
            areas[i] = NULL;
        }
    }

    for(i = 0; i < numSectors; i++) {
        sectors[i].stacks.Empty();
    }

    areas.Empty();
    Mem_Purge(kexCollisionMap::hb_collisionMap);
}

//
// kexCollisionMap::SetupSectorStackList
//

void kexCollisionMap::SetupSectorStackList(void) {
    kexSector *sector;
    kexBBox box1;
    kexBBox box2;

    for(int i = 0; i < numSectors; i++) {
        sector = &sectors[i];

        box1 = sector->lowerTri.bounds;
        box1.min.y = 0;
        box1.max.y = 0;

        for(int j = 0; j < numSectors; j++) {
            kexSector *check = &sectors[j];
            kexTri *tri = &check->lowerTri;

            if(check == sector) {
                continue;
            }
            if(!(check->flags & CLF_ONESIDED)) {
                continue;
            }
            if(tri->plane.Distance(*sector->lowerTri.point[0]) - tri->plane.d > 0) {
                continue;
            }

            box2 = tri->bounds;
            box2.min.y = 0;
            box2.max.y = 0;

            if(box1.IntersectingBox(box2)) {
                sector->stacks.Push(check);
            }
        }
    }
}

//
// kexCollisionMap::PointInSector
//

kexSector *kexCollisionMap::PointInSector(const kexVec3 &origin) {
    float dist;
    float curdist = 0;
    kexSector *sector = NULL;
    bool ok = false;

    if(bLoaded == false) {
        return NULL;
    }

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
// kexCollisionMap::RecursiveChangeHeight
//

void kexCollisionMap::RecursiveChangeHeight(kexSector *sector, float destHeight,
                                            unsigned int areaID) {
    kexSector *sec;
    int i;

    if(sector == NULL) {
        return;
    }

    sec = sector;

    for(i = 0; i < 3; i++) {
        if(sec->link[i] == NULL) {
            continue;
        }

        sec->link[i]->lowerTri.Set(
            sec->link[i]->lowerTri.point[0],
            sec->link[i]->lowerTri.point[1],
            sec->link[i]->lowerTri.point[2]);
    }

    while(sec->area->WorldID() == areaID) {
        if( sec->lowerTri.point[0]->y == destHeight &&
            sec->lowerTri.point[1]->y == destHeight &&
            sec->lowerTri.point[2]->y == destHeight) {
                break;
        }

        for(i = 0; i < 3; i++) {
            sec->lowerTri.point[i]->y = destHeight;
        }

        if(sec->link[0]) {
            RecursiveChangeHeight(sec->link[0], destHeight, areaID);
        }

        if(sec->link[1]) {
            RecursiveChangeHeight(sec->link[1], destHeight, areaID);
        }

        if(sec->link[2] == NULL) {
            break;
        }

        sec = sec->link[2];

        for(i = 0; i < 3; i++) {
            if(sec->link[i] == NULL) {
                continue;
            }

            sec->link[i]->lowerTri.Set(
                sec->link[i]->lowerTri.point[0],
                sec->link[i]->lowerTri.point[1],
                sec->link[i]->lowerTri.point[2]);
        }
    }
}

//
// kexCollisionMap::TraverseSectors
//

void kexCollisionMap::TraverseSectors(cMapTrace_t *trace, kexSector *sector) {
    int i;
    kexSector *next = NULL;

    if(sector == NULL) {
        return;
    }

    trace->result->sector = sector;
    trace->result->fraction = 1;

    if(sector->flags & CLF_CHECKHEIGHT && sector->Trace(trace, true)) {
        // made contact
        return;
    }
    if(sector->Trace(trace, false)) {
        // made contact
        return;
    }

    for(i = 0; i < 3; i++) {
        if(sector->IntersectEdge(trace, i)) {
            next = sector->CrossEdge(trace, i);
        }
    }

    if(next != NULL) {
        TraverseSectors(trace, next);
    }
    else {
        trace->result->bClippedEdge = true;
    }
}

//
// kexCollisionMap::Trace
//

void kexCollisionMap::Trace(cMapTraceResult_t *result,
                            const kexVec3 &start, const kexVec3 &end,
                            kexSector *sector,
                            const int flags,
                            const float height) {
    cMapTrace_t trace;
    kexVec3 pos;
    kexSector *s;

    trace.result = result;

    trace.result->position      = end;
    trace.result->fraction      = 1.0f;
    trace.result->sector        = sector;
    trace.result->contactSector = NULL;
    trace.result->bClippedEdge  = false;

    if(bLoaded == false || sector == NULL) {
        return;
    }

    trace.start     = start;
    trace.end       = end;
    trace.sector    = sector;
    trace.flags     = flags;
    trace.height    = height;
    trace.direction = (end - start).Normalize();

    trace.result->normal = trace.direction;

    if(!sector->Trace(&trace, false)) {
        TraverseSectors(&trace, sector);
    }

    for(unsigned int i = 0; i < trace.result->sector->stacks.Length(); i++) {
        s = trace.result->sector->stacks[i];
        pos = trace.result->position;
        
        if((pos[1] - s->lowerTri.GetDistance(pos)) >= 0 && s->InRange(pos)) {
            trace.result->sector = s;
        }
    }
}

//
// kexCollisionMap::PlayerCrossAreas
//

void kexCollisionMap::PlayerCrossAreas(kexSector *enter, kexSector *exit) {
    kexArea *area;

    if(enter == exit) {
        return;
    }

    if(exit != NULL && enter != NULL) {
        area = exit->area;
        if(area != enter->area) {
            if(area != NULL) {
                area->scriptComponent.CallFunction(area->scriptComponent.onExit);
            }
            area = enter->area;
            if(area != NULL) {
                area->Enter();
            }
        }
    }
}

//
// kexCollisionMap::RecursiveToggleBlock
//

void kexCollisionMap::RecursiveToggleBlock(kexSector *sector, bool bToggle,
                                           unsigned int areaID) {
    if(sector == NULL) {
        return;
    }

    kexSector *sec = sector;

    while(sec->area->WorldID() == areaID) {
        if(bToggle) {
            if(sec->flags & CLF_TOGGLE) {
                break;
            }

            sec->flags |= CLF_TOGGLE;
        }
        else {
            if(!(sec->flags & CLF_TOGGLE)) {
                break;
            }

            sec->flags &= ~CLF_TOGGLE;
        }

        if(sec->link[0] != NULL) {
            RecursiveToggleBlock(sec->link[0], bToggle, areaID);
        }

        if(sec->link[1] != NULL) {
            RecursiveToggleBlock(sec->link[1], bToggle, areaID);
        }

        if(sec->link[2] == NULL) {
            break;
        }

        sec = sec->link[2];
    }
}

//
// kexCollisionMap::ToggleBlock
//

void kexCollisionMap::ToggleBlock(const kexVec3 pos, bool bToggle) {
    kexSector *sector;

    if(!(sector = PointInSector(pos))) {
        return;
    }

    if(!(sector->flags & CLF_BLOCK)) {
        return;
    }

    RecursiveToggleBlock(sector, bToggle, sector->area->WorldID());
}
