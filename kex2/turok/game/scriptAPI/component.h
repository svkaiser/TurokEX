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

#ifndef __SCRIPT_COMPONENT_H__
#define __SCRIPT_COMPONENT_H__

#include "scriptAPI/scriptSystem.h"

//-----------------------------------------------------------------------------
//
// kexComponent
//
//-----------------------------------------------------------------------------

class kexComponent {
public:
                            kexComponent(void);
                            ~kexComponent(void);

    virtual void            Construct(const char *className) = 0;
    virtual bool            CallConstructor(const char *decl);

    bool                    Spawn(const char *className);
    bool                    CallFunction(asIScriptFunction *func);
    bool                    CallFunction(const char *decl, int *val);
    bool                    CallFunction(asIScriptFunction *func, void *object, bool *val);
    kexScriptObjHandle      &Handle(void) { return objHandle; }
    const asIObjectType     *ScriptType(void) const { return type; }
    const asIScriptObject   *ScriptObject(void) const { return obj; }
    void                    SetOwner(kexObject *kobj) { objHandle.owner = kobj; }
    kexObject               *GetOwner(void) const { return static_cast<kexObject*>(objHandle.owner); }

    asIScriptFunction       *onThink;
    asIScriptFunction       *onLocalThink;
    asIScriptFunction       *onSpawn;

protected:
    asIObjectType           *type;
    kexScriptObjHandle      objHandle;
    asIScriptObject         *obj;

private:
    asIScriptModule         *mod;
};

//-----------------------------------------------------------------------------
//
// kexActorComponent
//
//-----------------------------------------------------------------------------

class kexActorComponent : public kexComponent {
    public:
                            kexActorComponent(void);
                            ~kexActorComponent(void);

    virtual void            Construct(const char *className);

    static void             Init(void);

    asIScriptFunction       *onTouch;
    asIScriptFunction       *onDamage;
    asIScriptFunction       *onTrigger;
};

//-----------------------------------------------------------------------------
//
// kexAreaComponent
//
//-----------------------------------------------------------------------------

class kexAreaComponent : public kexComponent {
    public:
                            kexAreaComponent(void);
                            ~kexAreaComponent(void);

    virtual void            Construct(const char *className);
    virtual bool            CallConstructor(const char *decl);

    void                    SetID(const int val) { id = val; }
    const int               GetID(void) const { return id; }

    static void             Init(void);

    asIScriptFunction       *onEnter;
    asIScriptFunction       *onExit;

private:
    unsigned int            id;
};

#endif
