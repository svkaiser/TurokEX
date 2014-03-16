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

#ifndef __RENDER_WORLD_H__
#define __RENDER_WORLD_H__

#include "world.h"

#define MAX_FX_DISPLAYS 2048

class kexRenderWorld {
public:
                    kexRenderWorld(void);

    void            RenderScene(void);
    void            DrawBoundingBox(const kexBBox &bbox, byte r, byte g, byte b);
    void            DrawRadius(float x, float y, float z, float radius, float height,
                               byte r, byte g, byte b);
    void            DrawOrigin(float x, float y, float z, float size);
    void            DrawWorldNode(worldNode_t *node);
    void            DrawAreaNode(void);

    static void     Init(void);
    static int      SortSprites(const void *a, const void *b);

    bool            bShowBBox;
    bool            bShowGrid;
    bool            bShowNormals;
    bool            bShowRadius;
    bool            bShowNodes;
    bool            bShowOrigin;
    bool            bWireframe;
    bool            bShowClipMesh;
    bool            bShowCollisionMap;

    int             showWorldNode;
    int             showAreaNode;

private:
    void            SetupGlobalFog(void);
    void            SetupGlobalLight(void);
    void            DrawActors(void);
    void            DrawStaticActors(void);
    void            DrawViewActors(void);
    void            DrawFX(void);
    void            DrawSurface(const surface_t *surface, const char *texturePath);
    void            TraverseDrawActorNode(kexActor *actor,
                        const modelNode_t *node, kexAnimState *animState);

    kexWorld        *world;
    bool            bDrawTris;

    typedef struct {
        kexFx       *fx;
    } fxDisplay_t;

    fxDisplay_t     fxDisplayList[MAX_FX_DISPLAYS];
};

extern kexRenderWorld renderWorld;

#endif
