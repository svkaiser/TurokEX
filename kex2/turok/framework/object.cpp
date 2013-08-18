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
// DESCRIPTION: RTTI Object Class
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "object.h"

//
// kexRTTI::kexRTTI
//

kexRTTI::kexRTTI(const char *classname, const char *supername,
    kexObject *(*Create)(void)) {
    this->classname = classname;
    this->supername = supername;
    this->Create = Create;
    this->type_id = ++kexObject::roverID;
    this->super = kexObject::Get(supername);
    
    if(kexObject::root == NULL) {
        this->next = NULL;
        kexObject::root = this;
    }
    else {
        this->next = kexObject::root;
        kexObject::root = this;
    }
}

//
// kexRTTI::~kexRTTI
//

kexRTTI::~kexRTTI(void) {
    Destroy();
}

//
// kexRTTI::Init
//

void kexRTTI::Init(void) {
}

//
// kexRTTI::Destroy
//

void kexRTTI::Destroy(void) {
}

//
// kexRTTI::InstanceOf
//

bool kexRTTI::InstanceOf(const kexRTTI *objInfo) const {
    return type_id == objInfo->type_id;
}

DECLARE_ABSTRACT_CLASS(kexObject, NULL)

kexRTTI *kexObject::root = NULL;
bool kexObject::bInitialized = false;
int kexObject::roverID = 0;

//
// kexObject::~kexObject
//

kexObject::~kexObject(void) {
}

//
// kexObject::Create
//

kexObject *kexObject::Create(const char *name) {
    const kexRTTI *info;

    if(name == NULL || !(info = kexObject::Get(name)))
        return NULL;
        
    return info->Create();
}

//
// kexObject::Init
//

void kexObject::Init(void) {
    for(kexRTTI *oi = kexObject::root; oi != NULL; oi = oi->next) {
        oi->Init();
    }
    
    bInitialized = true;
    command.Add("listRuntimeClasses", kexObject::ListClasses);
    common.Printf("Runtime Object Initialized\n");
}

//
// kexObject::Shutdown
//

void kexObject::Shutdown(void) {
    for(kexRTTI *oi= kexObject::root; oi != NULL; oi = oi->next) {
        oi->Destroy();
    }
    
    bInitialized = false;
}

//
// kexObject::Get
//

kexRTTI *kexObject::Get(const char *classname) {
    if(classname == NULL)
        return NULL;
    for(kexRTTI *oi= kexObject::root; oi != NULL; oi = oi->next) {
        if(!strcmp(oi->classname, classname))
            return oi;
    }
    return NULL;
}

//
// kexObject::InstanceOf
//

bool kexObject::InstanceOf(const kexRTTI *objInfo) const {
    return GetInfo()->type_id == objInfo->type_id;
}

//
// kexObject::GetClassName
//

const char *kexObject::GetClassName(void) const {
    return GetInfo()->classname;
}

//
// kexObject::GetSuperName
//

const char *kexObject::GetSuperName(void) const {
    return GetInfo()->supername;
}

//
// kexObject::ListClasses
//

void kexObject::ListClasses(void) {
    common.CPrintf(COLOR_GREEN, "-------------- Runtime Classes ---------------\n");
    for(kexRTTI *oi= kexObject::root; oi != NULL; oi = oi->next) {
        common.Printf("%s %s %i\n", oi->classname, oi->supername, oi->type_id);
    }
    common.CPrintf(COLOR_GREEN, "----------------------------------------------\n\n");
}
