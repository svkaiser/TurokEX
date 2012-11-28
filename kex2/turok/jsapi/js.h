// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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

#ifndef _JSAPI_H_
#define _JSAPI_H_

#ifndef _WINDOWS
#define _WINDOWS
#endif

#ifndef _X86_
#define _X86 1
#endif

#ifndef JSFILE
#define JSFILE
#endif

#ifndef WIN32
#define WIN32
#endif

#ifndef XP_WIN
#define XP_WIN
#endif

void J_ExecBuffer(char *buffer);
void J_Shutdown(void);
void J_Init(void);

#endif