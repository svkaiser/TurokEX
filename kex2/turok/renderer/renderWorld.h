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

class kexRenderWorld {
public:
                    kexRenderWorld(void);

    void            RenderScene(void);

    static void     Init(void);

    bool            bShowBBox;
    bool            bShowGrid;
    bool            bShowNormals;
    bool            bShowRadius;
    bool            bShowNodes;
    bool            bShowOrigin;
    bool            bWireframe;
    bool            bShowClipMesh;

private:
    void            SetupGlobalLight(void);
    void            DrawStaticActors(void);
    void            DrawActors(void);
    void            DrawFX(void);
    void            DrawSurface(const mdlsection_t *section, const char *texturePath);
    void            TraverseDrawActorNode(const kexWorldActor *actor,
                        const mdlnode_t *node, animstate_t *animstate);

    void            DrawBoundingBox(const kexBBox &bbox, byte r, byte g, byte b);

    kexWorld        *world;
};

extern kexRenderWorld renderWorld;

#endif
