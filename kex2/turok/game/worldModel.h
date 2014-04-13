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

#ifndef __WORLDMODEL_H__
#define __WORLDMODEL_H__

#include "common.h"
#include "worldObject.h"
#include "renderModel.h"
#include "clipmesh.h"

class kexClipMesh;

//-----------------------------------------------------------------------------
//
// kexWorldModel
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexWorldModel, kexWorldObject);
public:
                                    kexWorldModel(void);
                                    ~kexWorldModel(void);

    virtual void                    LocalTick(void);
    virtual void                    Tick(void);
    virtual void                    Parse(kexLexer *lexer);
    virtual void                    UpdateTransform(void);

    void                            Spawn(void);
    void                            SetModel(const char *modelFile);
    void                            SetModel(const kexStr &modelFile);

    kexClipMesh                     &ClipMesh(void) { return clipMesh; }
    const kexModel_t                *Model(void) const { return model; }

    // TODO - need some sort of skin system
    kexMaterial                     **materials;
    bool                            bTraced;
    int                             validcount;

    kexLinklist<kexWorldModel>      worldLink;
    kexSDNodeRef<kexWorldModel>     renderNode;

protected:
    void                            AllocateMaterials(void);

    kexClipMesh                     clipMesh;
    kexModel_t                      *model;
END_CLASS();

#endif
