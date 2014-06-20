// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012-2014 Samuel Villarreal
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

#ifndef __SHADERPROG_H__
#define __SHADERPROG_H__

#include "keymap.h"
#include "getter.h"

typedef enum {
    RST_VERTEX          = 0,
    RST_FRAGMENT,
    RST_TOTAL
} rShaderType_t;

typedef enum {
    RSP_DIFFUSE_COLOR   = 0,
    RSP_VIEW_WIDTH,
    RSP_VIEW_HEIGHT,
    RSP_GENERIC_PARAM1,
    RSP_GENERIC_PARAM2,
    RSP_GENERIC_PARAM3,
    RSP_GENERIC_PARAM4,
    RSP_TOTAL
} rShaderGlobalParams_t;

typedef GLhandleARB	rhandle;

class kexMaterial;
class kexShaderManager;

#define REGISTER_PARAM(g)                                       \
template<typename type>                                         \
static void RegisterParam(const char *name, type *obj, g cb) {  \
    kexGetterBase *getter = new kexGetter<type>(name, obj, cb); \
                                                                \
    customParameters.Push(getter);                              \
}

class kexShaderObj {
public:
                                kexShaderObj(void);
                                ~kexShaderObj(void);
                                
    void                        InitProgram(void);
    void                        Init(void);
    void                        InitFromDefinition(kexKeyMap *def);
    void                        Compile(const char *name, rShaderType_t type);
    bool                        Link(void);
    void                        Enable(void);
    void                        Delete(void);
    void                        SetUniform(const char *name, const int val);
    void                        SetUniform(const char *name, const int *val, const int size);
    void                        SetUniform(const char *name, const float val);
    void                        SetUniform(const char *name, kexVec2 &val);
    void                        SetUniform(const char *name, kexVec3 &val);
    void                        SetUniform(const char *name, kexVec3 *val, const int size);
    void                        SetUniform(const char *name, kexVec4 &val);
    void                        SetUniform(const char *name, kexMatrix &val, bool bTranspose = false);
    void                        CommitGlobalUniforms(const kexMaterial *material = NULL);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, const int val);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, const float val);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, kexVec2 &val);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, kexVec3 &val);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, kexVec4 &val);
    void                        SetGlobalUniform(const rShaderGlobalParams_t param, kexMatrix &val);
    
                                REGISTER_PARAM(GETTER_FLOAT(type));
                                REGISTER_PARAM(GETTER_INT(type));
                                REGISTER_PARAM(GETTER_BOOL(type));
                                REGISTER_PARAM(GETTER_VEC2(type));
                                REGISTER_PARAM(GETTER_VEC3(type));
                                REGISTER_PARAM(GETTER_VEC4(type));
    
    rhandle                     &Program(void) { return programObj; }
    rhandle                     &VertexProgram(void) { return vertexProgram; }
    rhandle                     &FragmentProgram(void) { return fragmentProgram; }
    const bool                  HasErrors(void) const { return bHasErrors; }
    const bool                  IsLoaded(void) const { return bLoaded; }
    
    void                        ResetValidCount(void) { validCount = 0; }

    static kexShaderManager     manager;
    
    filepath_t                  fileName;

private:
    void                        DumpErrorLog(const rhandle handle);
    
    static kexArray<kexGetterBase*> customParameters;
    
    rhandle                     programObj;
    rhandle                     vertexProgram;
    rhandle                     fragmentProgram;
    bool                        bHasErrors;
    bool                        bLoaded;
    int                         globalParams[RSP_TOTAL];
    int                         *customParametersLocal;
    int                         validCount;
};

#undef REGISTER_PARAM

class kexShaderManager : public kexResourceManager<kexShaderObj> {
public:
    kexShaderObj                *OnLoad(const char *file);
};

#endif
