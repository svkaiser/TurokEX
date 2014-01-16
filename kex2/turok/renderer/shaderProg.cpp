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
//
// DESCRIPTION: Shader Program Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "fileSystem.h"
#include "memHeap.h"
#include "renderSystem.h"
#include "shaderProg.h"

//
// kexShaderObj::kexShaderObj
//

kexShaderObj::kexShaderObj(void) {
    this->programObj        = 0;
    this->vertexProgram     = 0;
    this->fragmentProgram   = 0;
}

//
// kexShaderObj::~kexShaderObj
//

kexShaderObj::~kexShaderObj(void) {
}

//
// kexShaderObj::InitProgram
//

void kexShaderObj::InitProgram(void) {
    programObj = dglCreateProgramObjectARB();
}

//
// kexShaderObj::Enable
//

void kexShaderObj::Enable(void) {
    dglUseProgramObjectARB(programObj);
}

//
// kexShaderObj::Compile
//

void kexShaderObj::Compile(const char *name, rShaderType_t type) {
    byte *data;
    rhandle *handle;
    
    if(cvarDeveloper.GetBool()) {
        if(fileSystem.OpenFile(name, &data, hb_static) == 0 &&
            fileSystem.ReadExternalTextFile(name, &data) <= 0) {
            return;
        }
    }
    else {
        if(fileSystem.OpenFile(name, &data, hb_static) == 0) {
            return;
        }
    }
    
    if(type == RST_VERTEX) {
        vertexProgram = dglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        handle = &vertexProgram;
    }
    else if(type == RST_FRAGMENT) {
        fragmentProgram = dglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        handle = &fragmentProgram;
    }
    else {
        Mem_Free(data);
        return;
    }
    
    dglShaderSourceARB(*handle, 1, (const GLcharARB**)&data, NULL);
    dglCompileShaderARB(*handle);
    dglAttachObjectARB(programObj, *handle);

    if(type == RST_FRAGMENT) {
        int loc;

        loc = dglGetUniformLocationARB(programObj, "diffuse1");
        if(loc != -1) {
            dglUniform1iARB(loc, 0);
        }
        loc = dglGetUniformLocationARB(programObj, "diffuse2");
        if(loc != -1) {
            dglUniform1iARB(loc, 1);
        }
    }
    
    Mem_Free(data);
}

//
// kexShaderObj::Link
//

void kexShaderObj::Link(void) {
    int linked;
    
    dglLinkProgramARB(programObj);
    dglGetObjectParameterivARB(programObj, GL_OBJECT_LINK_STATUS_ARB, &linked);
    
    if(!linked) {
        int logLength;
        
        dglGetObjectParameterivARB(programObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
        
        if(logLength > 0) {
            int cw;
            char *log;
            
            log = (char*)Mem_Alloca(logLength);
            
            dglGetInfoLogARB(programObj, logLength, &cw, log);
            common.Warning("%s\n", log);
        }
    }
    
    dglUseProgramObjectARB(0);
}
