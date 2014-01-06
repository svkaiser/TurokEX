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
#include "world.h"

DECLARE_CLASS(kexArea, kexObject)

unsigned int kexArea::id = 0;

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
    this->worldID           = 0;

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

    scriptManager.Engine()->RegisterEnum("EnumWaterLevelType");
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_INVALID", WLT_INVALID);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_OVER", WLT_OVER);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_BETWEEN", WLT_BETWEEN);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_UNDER", WLT_UNDER);

    scriptManager.RegisterMethod("kArea", "int GetWaterLevel(const kVec3 &in, const float)",
        asMETHODPR(kexArea, GetWaterLevel, (const kexVec3&, const float), waterLevelType_t));
}

//
// kexArea::Enter
//

void kexArea::Enter(void) {
    scriptComponent.CallFunction(scriptComponent.onEnter);
    if(flags & AAF_EVENT) {
        localWorld.TriggerActor(targetID);
    }
}

//
// kexArea::GetWaterLevel
//

waterLevelType_t kexArea::GetWaterLevel(const kexVec3 &origin, const float height) {
    if(flags & AAF_WATER) {
        if(height + origin[1] >= waterplane) {
            if(origin[1] < waterplane) {
                return WLT_BETWEEN;
            }
            else {
                return WLT_OVER;
            }
        }

        return WLT_UNDER;
    }

    return WLT_INVALID;
}

//
// kexArea::Setup
//

void kexArea::Setup(void) {
    kexHashKey *key;
    bool flag;

    worldID = kexArea::id++;

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

    if((key = keyMap.Find("component"))) {
        scriptComponent.Construct(key->GetString());
        scriptComponent.CallFunction(scriptComponent.onSpawn);
    }
}
