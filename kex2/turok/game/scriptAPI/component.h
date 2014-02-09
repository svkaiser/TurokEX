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
    virtual void            Deconstruct(void);
    virtual bool            CallConstructor(const char *decl);

    int                     PrepareFunction(const char *decl);
    int                     PrepareFunction(asIScriptFunction *func);
    void                    SetCallArgument(const int arg, int val);
    void                    SetCallArgument(const int arg, byte val);
    void                    SetCallArgument(const int arg, float val);
    void                    SetCallArgument(const int arg, bool val);
    void                    SetCallArgument(const int arg, void *val);
    bool                    ExecuteFunction(int state);
    void                    FinishFunction(int state);
    void                    FinishFunction(int state, int *val);
    void                    FinishFunction(int state, byte *val);
    void                    FinishFunction(int state, float *val);
    void                    FinishFunction(int state, bool *val);
    void                    FinishFunction(int state, void **val);

    bool                    Spawn(const char *className);
    bool                    CallFunction(asIScriptFunction *func);
    kexScriptObjHandle      &Handle(void) { return objHandle; }
    const asIObjectType     *ScriptType(void) const { return type; }
    asIScriptObject         *ScriptObject(void) { return obj; }
    void                    SetOwner(kexObject *kobj) { objHandle.owner = kobj; }
    void                    Clear(void) { obj = NULL; type = NULL; }
    void                    Release(void) { if(obj) { obj->Release(); } }
    kexObject               *GetOwner(void) const { return static_cast<kexObject*>(objHandle.owner); }

protected:
    asIObjectType           *type;
    kexScriptObjHandle      objHandle;
    asIScriptObject         *obj;

private:
    asIScriptModule         *mod;
    kexStr                  name;
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

    asIScriptFunction       *onThink;
    asIScriptFunction       *onLocalThink;
    asIScriptFunction       *onSpawn;
    asIScriptFunction       *onTouch;
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

    asIScriptFunction       *onThink;
    asIScriptFunction       *onLocalThink;
    asIScriptFunction       *onSpawn;
    asIScriptFunction       *onEnter;
    asIScriptFunction       *onExit;

private:
    unsigned int            id;
};

#endif
