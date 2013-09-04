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

#ifndef __SCRIPT_SYS_H__
#define __SCRIPT_SYS_H__

#include "angelscript.h"

class kexScriptManager {
public:
                        kexScriptManager(void);
                        ~kexScriptManager(void);

    void                Init(void);
    void                Shutdown(void);
    void                CallExternalScript(const char *file, const char *function);

    asIScriptEngine     *Engine(void) { return engine; }
    asIScriptContext    *Context(void) { return ctx; }

private:
    void                RegisterBasicTypes(void);
    void                RegisterObjects(void);

    static void         MessageCallback(const asSMessageInfo *msg, void *param);

    asIScriptEngine     *engine;
    asIScriptContext    *ctx;
};

extern kexScriptManager scriptManager;

class kexScriptObjSystem {
public:
    static void         Init(void);
    static void         Printf(const kexStr &str);
    static void         CPrintf(rcolor color, const kexStr &str);
    static void         Warning(const kexStr &str);
    static void         DPrintf(const kexStr &str);
    static void         Error(const kexStr &str);
    static const int    GetMS(void);
};

class kexScriptObjHandle  {
public:
                        kexScriptObjHandle();
                        kexScriptObjHandle(const kexScriptObjHandle &other);
                        kexScriptObjHandle(void *ref, asIObjectType *type);
                        ~kexScriptObjHandle();

    kexScriptObjHandle  &operator=(const kexScriptObjHandle &other);
    void                Set(void *ref, asIObjectType *type);

    bool                operator==(const kexScriptObjHandle &o) const;
    bool                operator!=(const kexScriptObjHandle &o) const;
    bool                Equals(void *ref, int typeId) const;

    void                Cast(void **outRef, int typeId);
    asIObjectType       *GetType(void);

    static void         Init(void);
    static void         ObjectConstruct(kexScriptObjHandle *self) { new(self)kexScriptObjHandle(); }
    static void         ObjectConstruct(kexScriptObjHandle *self, const kexScriptObjHandle &other) {
                            new(self)kexScriptObjHandle(other);
                        }
    static void         ObjectConstruct(kexScriptObjHandle *self, void *ref, int typeID) {
                            new(self)kexScriptObjHandle(ref, typeID);
                        }
    static void         ObjectDeconstruct(kexScriptObjHandle *self) { self->~kexScriptObjHandle(); }

protected:
    void                ReleaseHandle(void);
    void                AddRefHandle(void);

    // These shouldn't be called directly by the 
    // application as they requires an active context
    kexScriptObjHandle(void *ref, int typeId);
    kexScriptObjHandle &Assign(void *ref, int typeId);

    void                *m_ref;
    asIObjectType       *m_type;
};

#endif
