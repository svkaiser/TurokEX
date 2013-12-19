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
#include "collisionMap.h"
#include "areas.h"

DECLARE_CLASS(kexArea, kexObject)

//
// kexArea::kexArea
//

kexArea::kexArea(void) {
    this->waterplane        = 0;
    this->targetID          = -1;
    this->triggerSound      = NULL;
    this->fSurfaceID        = -1;
    this->cSurfaceID        = -1;
    this->wSurfaceID        = -1;
    this->globalFogZFar     = 1024;
    this->flags             = 0;

    this->globalFogRGB.Clear();
    this->scriptComponent.SetOwner(this);
}

//
// kexArea::InitObject
//

void kexArea::InitObject(void) {
    kexScriptManager::RegisterRefObjectNoCount<kexArea>("kArea");
    scriptManager.Engine()->RegisterObjectProperty("kArea", "kKeyMap key",
        asOFFSET(kexArea, keyMap));
    scriptManager.Engine()->RegisterObjectProperty("kArea", "float waterplane",
        asOFFSET(kexArea, waterplane));
    scriptManager.Engine()->RegisterObjectProperty("kArea", "int targetID",
        asOFFSET(kexArea, targetID));
    scriptManager.Engine()->RegisterObjectProperty("kArea", "float fogZFar",
        asOFFSET(kexArea, globalFogZFar));
    scriptManager.Engine()->RegisterObjectProperty("kArea", "kVec3 fogRGB",
        asOFFSET(kexArea, globalFogRGB));
    scriptManager.Engine()->RegisterObjectProperty("kArea", "ref @obj",
        asOFFSET(kexArea, scriptComponent.Handle()));
}

//
// kexArea::Setup
//

void kexArea::Setup(void) {
    kexHashKey *key;
    bool flag;

    keyMap.GetFloat("waterlevel", waterplane);
    keyMap.GetInt("targetID", targetID);
    keyMap.GetFloat("globalfog_zfar", globalFogZFar);
    keyMap.GetVector("globalfog_rgb", globalFogRGB);
    globalFogRGB /= 255.0f;

    keyMap.GetBool("bDamage", flag);
    if(flag) {
        flags |= AAF_DAMAGE;
    }

    keyMap.GetBool("bToggle", flag);
    if(flag) {
        flags |= AAF_TOGGLE;
    }

    keyMap.GetBool("bClimb", flag);
    if(flag) {
        flags |= AAF_CLIMB;
    }

    keyMap.GetBool("bWater", flag);
    if(flag) {
        flags |= AAF_WATER;
    }

    keyMap.GetBool("bTrigger", flag);
    if(flag) {
        flags |= AAF_EVENT;
    }

    keyMap.GetBool("bRepeatable", flag);
    if(flag) {
        flags |= AAF_REPEATABLE;
    }

    if((key = keyMap.Find("component"))) {
        scriptComponent.Construct(key->GetString());
        scriptComponent.CallFunction(scriptComponent.onSpawn);
    }
}
