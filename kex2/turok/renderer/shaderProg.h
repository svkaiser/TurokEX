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

typedef enum {
    RST_VERTEX      = 0,
    RST_FRAGMENT,
    RST_TOTAL
} rShaderType_t;

typedef GLhandleARB	rhandle;

class kexShaderObj {
public:
                                kexShaderObj(void);
                                ~kexShaderObj(void);
                                
    void                        InitProgram(void);
    void                        Compile(const char *name, rShaderType_t type);
    void                        Link(void);
    void                        Enable(void);
    
    rhandle                     &Program(void) { return programObj; }
    rhandle                     &VertexProgram(void) { return vertexProgram; }
    rhandle                     &FragmentProgram(void) { return fragmentProgram; }

private:
    rhandle                     programObj;
    rhandle                     vertexProgram;
    rhandle                     fragmentProgram;
};

#endif
