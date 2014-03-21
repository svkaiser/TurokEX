// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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

#include "common.h"
#include "fileSystem.h"
#include "memHeap.h"
#include "renderSystem.h"
#include "material.h"

unsigned int kexMaterial::matID = 0;
unsigned int kexMaterial::bindID = 0;

//
// kexMaterial::kexMaterial
//

kexMaterial::kexMaterial(void) {
}

//
// kexMaterial::~kexMaterial
//

kexMaterial::~kexMaterial(void) {
}

//
// kexMaterial::Init
//

void kexMaterial::Init(void) {
    this->genID         = ++kexMaterial::matID;
    this->next          = NULL;
    this->flags         = 0;
    this->cullType      = MCT_NONE;
    this->units         = 0;
    this->genID         = 0;
    this->bShaderErrors = false;
    
    for(int i = 0; i < MAX_SAMPLER_UNITS; i++) {
        this->samplers[i].param     = "";
        this->samplers[i].unit      = 0;
        this->samplers[i].texture   = NULL;
        this->samplers[i].clamp     = MC_CLAMP;
        this->samplers[i].filter    = MF_LINEAR;
        this->samplers[i].scroll[0] = 0;
        this->samplers[i].scroll[1] = 0;
        this->samplers[i].scale[0]  = 0;
        this->samplers[i].scale[1]  = 0;
        this->samplers[i].scale[2]  = 0;
        this->samplers[i].rotate    = 0;
    }
}

//
// kexMaterial::Sampler
//

matSampler_t *kexMaterial::Sampler(const int which) {
    assert(which >= 0 && which < MAX_SAMPLER_UNITS);
    return &samplers[which];
}

//
// kexMaterial::ParseParam
//

void kexMaterial::ParseParam(kexLexer *lexer) {
    matParamType_t paramType;
    char *name;
    
    paramType = (matParamType_t)lexer->GetNumber();
    lexer->GetString();
    name = lexer->StringToken();
    
    switch(paramType) {
        case MPT_FLOAT:
        {
            matParam_t<float> param;
            param.param = name;
            param.value = (float)lexer->GetFloat();
            params.floatParams.Push(param);
        }
            break;
        case MPT_INT:
        {
            matParam_t<int> param;
            param.param = name;
            param.value = lexer->GetNumber();
            params.intParams.Push(param);
        }
            break;
        case MPT_VEC2:
        {
            matParam_t<kexVec2> param;
            param.param = name;
            param.value.x = (float)lexer->GetFloat();
            param.value.z = (float)lexer->GetFloat();
            
            params.vec2Params.Push(param);
        }
            break;
        case MPT_VEC3:
        {
            matParam_t<kexVec3> param;
            param.param = name;
            param.value.x = (float)lexer->GetFloat();
            param.value.y = (float)lexer->GetFloat();
            param.value.z = (float)lexer->GetFloat();
            
            params.vec3Params.Push(param);
        }
            break;
        case MPT_VEC4:
        {
            matParam_t<kexVec4> param;
            param.param = name;
            param.value.x = (float)lexer->GetFloat();
            param.value.y = (float)lexer->GetFloat();
            param.value.z = (float)lexer->GetFloat();
            param.value.w = (float)lexer->GetFloat();
            
            params.vec4Params.Push(param);
        }
            break;
        case MPT_MAT4:
            // TODO
            break;
        default:
            parser.Error("kexMaterial::ParseParam - Unknown param type: %i\n", paramType);
            break;
    }
}

//
// kexMaterial::ParseSampler
//

void kexMaterial::ParseSampler(kexLexer *lexer) {
    kexStr texFile;
    kexStr paramName;
    unsigned int unit;
    matSampler_t *sampler;

    lexer->GetString();
    paramName = lexer->StringToken();
    
    unit = lexer->GetNumber();
    
    lexer->GetString();
    texFile = lexer->StringToken();
    
    sampler = Sampler(unit);
    sampler->param = paramName;
    sampler->filter = MF_LINEAR;
    sampler->clamp = MC_CLAMP;
    sampler->texture = renderSystem.CacheTexture(texFile.c_str(), TC_CLAMP);
    
    shaderObj.SetUniform(sampler->param, sampler->unit);
    
    if(unit+1 > units) {
        units = unit+1;
    }
    
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();
    
    while(lexer->TokenType() != TK_RBRACK) {
        if(lexer->Matches("filter")) {
            lexer->Find();
            if(lexer->Matches("linear")) {
                sampler->filter = MF_LINEAR;
            }
            else if(lexer->Matches("nearest")) {
                sampler->filter = MF_NEAREST;
            }
        }
        else if(lexer->Matches("wrap")) {
            lexer->Find();
            if(lexer->Matches("clamp")) {
                sampler->clamp = MC_CLAMP;
            }
            else if(lexer->Matches("repeat")) {
                sampler->clamp = MC_REPEAT;
            }
            else if(lexer->Matches("mirrored")) {
                sampler->clamp = MC_MIRRORED;
            }
        }
        else if(lexer->Matches("scroll")) {
            sampler->scroll[0] = (float)lexer->GetFloat();
            sampler->scroll[1] = (float)lexer->GetFloat();
        }
        else if(lexer->Matches("scale")) {
            sampler->scale[0] = (float)lexer->GetFloat();
            sampler->scale[1] = (float)lexer->GetFloat();
            sampler->scale[1] = (float)lexer->GetFloat();
        }
        else if(lexer->Matches("rotate")) {
            sampler->rotate = (float)lexer->GetFloat();
        }
        
        lexer->Find();
    }
}

//
// kexMaterial::Parse
//

void kexMaterial::Parse(kexLexer *lexer) {
    shaderObj.InitProgram();
    
    while(lexer->CheckState()) {
        lexer->Find();
        
        switch(lexer->TokenType()) {
            case TK_EOF:
                return;
            case TK_IDENIFIER:
                if(lexer->Matches("fragmentProgram")) {
                    lexer->GetString();
                    shaderObj.Compile(lexer->StringToken(), RST_FRAGMENT);
                }
                else if(lexer->Matches("vertexProgram")) {
                    lexer->GetString();
                    shaderObj.Compile(lexer->StringToken(), RST_VERTEX);
                }
                else if(lexer->Matches("fullbright")) {
                    flags |= MTF_FULLBRIGHT;
                }
                else if(lexer->Matches("solid")) {
                    flags |= MTF_SOLID;
                }
                else if(lexer->Matches("masked")) {
                    flags |= MTF_MASKED;
                }
                else if(lexer->Matches("nodraw")) {
                    flags |= MTF_NODRAW;
                }
                else if(lexer->Matches("transparent")) {
                    flags |= MTF_TRANSPARENT;
                }
                else if(lexer->Matches("nofog")) {
                    flags |= MTF_NOFOG;
                }
                else if(lexer->Matches("cullType")) {
                    lexer->Find();
                    
                    if(lexer->Matches("back")) {
                        cullType = MCT_BACK;
                    }
                    else if(lexer->Matches("front")) {
                        cullType = MCT_FRONT;
                    }
                    else if(lexer->Matches("none")) {
                        cullType = MCT_NONE;
                    }
                }
                else if(lexer->Matches("shaderParam")) {
                    ParseParam(lexer);
                }
                else if(lexer->Matches("sampler")) {
                    ParseSampler(lexer);
                }
                break;
            default:
                break;
        }
    }
    
    if(!shaderObj.Link()) {
        bShaderErrors = true;
    }
}

//
// kexMaterial::Bind
//

void kexMaterial::Bind(void) {
    bool bBlend;
    
    if(genID == kexMaterial::bindID) {
        return;
    }
    
    kexMaterial::bindID = genID;
    shaderObj.Enable();
    
    if(cullType != MCT_NONE) {
        renderSystem.SetState(GLSTATE_CULL, true);
        renderSystem.SetCull(cullType == MCT_FRONT ? GLCULL_FRONT : GLCULL_BACK);
    }
    else {
        renderSystem.SetState(GLSTATE_CULL, false);
    }
    
    bBlend = ((flags & (MTF_MASKED|MTF_TRANSPARENT)) != 0);
    
    renderSystem.SetState(GLSTATE_BLEND, bBlend);
    renderSystem.SetState(GLSTATE_ALPHATEST, bBlend);

    // TODO
}

//
// kexMaterialManager::kexMaterialManager
//

kexMaterialManager::kexMaterialManager(void) {
}

//
// kexMaterialManager::~kexMaterialManager
//

kexMaterialManager::~kexMaterialManager(void) {
}

//
// kexMaterialManager::LoadMaterial
//

kexMaterial *kexMaterialManager::LoadMaterial(const char *file) {
    kexMaterial *material;
    
    if(file == NULL) {
        return NULL;
    }
    else if(file[0] == 0) {
        return NULL;
    }
    
    if(!(material = materials.Find(file))) {
        kexLexer *lexer;
        
        if(!(lexer = parser.Open(file))) {
            if(!(lexer = parser.Open("materials/default.kmat"))) {
                return NULL;
            }
            
            common.Warning("kexMaterialManager::LoadMaterial: %s not found\n", file);
        }
        
        material = materials.Add(file);
        material->Init();
        
        strncpy(material->fileName, file, MAX_FILEPATH);
        
        material->Parse(lexer);
        
        // we're done with the file
        parser.Close();
    }
    
    return material;
}

//
// kexMaterialManager::Shutdown
//

void kexMaterialManager::Shutdown(void) {
}
