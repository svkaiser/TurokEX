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

#ifndef __WORLD_H__
#define __WORLD_H__

#include "common.h"
#include "array.h"
#include "linkedlist.h"
#include "camera.h"
#include "fx.h"

typedef struct gridBound_s {
    kexBBox                             box;
    kexLinklist<kexWorldActor>          staticActors;
    kexLinklist<kexWorldActor>          linkedActors;
    kexPtrArray<struct gridBound_s*>    linkedBounds;
    bool                                bTraced;
} gridBound_t;

class kexWorld {
public:
                                        kexWorld(void);
                                        ~kexWorld(void);

    void                                Init(void);
    void                                Tick(void);
    void                                LocalTick(void);
    void                                Load(const char *mapFile);
    void                                Unload(void);
    const char                          *GetMapFileFromID(const int id);
    kexObject                           *ConstructObject(const char *className);
    kexWorldActor                       *ConstructActor(const char *className);
    void                                AddActor(kexWorldActor *actor);
    kexWorldActor                       *SpawnActor(const char *className, const char *component,
                                            const kexVec3 &origin, const kexAngle &angles);
    kexWorldActor                       *SpawnActor(kexStr &className, kexStr &component,
                                            kexVec3 &origin, kexAngle &angles);
    kexFx                               *SpawnFX(const char *name, kexActor *source, kexVec3 &velocity,
                                            kexVec3 &origin, kexQuat &rotation);
    void                                SpawnFX(const kexStr &str, kexActor *source, kexVec3 &velocity,
                                            kexVec3 &origin, kexQuat &rotation);
    void                                RemoveActor(kexWorldActor *actor);
    void                                SpawnLocalPlayer(void);
    void                                Trace(traceInfo_t *trace);
    void                                PlaySound(const char *name);
    void                                PlaySound(const kexStr &name);
    void                                SetFogRGB(float r, float g, float b);

    bool                                IsLoaded(void) const { return bLoaded; }
    float                               DeltaTime(void) { return deltaTime; }
    kexCamera                           *Camera(void) { return &camera; };
    kexVec3                             &GetGravity(void) { return gravity; }
    void                                SetGravity(const kexVec3 &in) { gravity = in; }
    const int                           GetTicks(void) const { return ticks; }
    float                               GetFogNear(void) { return currentFogNear; }
    void                                SetFogNear(float n) { fogNear = n; }
    float                               GetFogFar(void) { return currentFogFar; }
    void                                SetFogFar(float f) { fogFar = f; }
    float                               *GetCurrentFogRGB(void) { return currentFogRGB; }
    bool                                FogEnabled(void) { return bEnableFog; }
    void                                ToggleFog(bool toggle) { bEnableFog = toggle; }

    kexLinklist<kexWorldActor>          actors;
    kexLinklist<kexFx>                  fxList;
    kexPtrArray<gridBound_t*>           gridBounds;

    kexWorldActor                       *actorRover;
    kexFx                               *fxRover;

    // TEMP
    kexVec4                             worldLightOrigin;
    kexVec4                             worldLightColor;
    kexVec4                             worldLightAmbience;
    kexVec4                             worldLightModelAmbience;
    //

    static void                         InitObject(void);

private:
    void                                ParseGridBound(kexLexer *lexer);
    void                                LinkGridBounds(void);

    bool                                bLoaded;
    bool                                bReadyUnload;
    int                                 mapID;
    int                                 nextMapID;
    kexStr                              title;
    kexCamera                           camera;
    kexVec3                             gravity;
    int                                 ticks;
    float                               time;
    float                               deltaTime;
    bool                                bEnableFog;
    float                               fogNear;
    float                               fogFar;
    float                               fogRGB[4];
    float                               currentFogNear;
    float                               currentFogFar;
    float                               currentFogRGB[4];
};

extern kexWorld localWorld;

#endif
