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
//
// DESCRIPTION: Main program
//
//-----------------------------------------------------------------------------

#ifdef _WIN32
#include <crtdbg.h>
#endif

#include "SDL.h"
#include "system.h"
#include "common.h"

//
// main
//

#ifndef _MSC_VER
    #define Kex_Main main
#else
#ifndef _DEBUG
#include <windows.h>
#include "SDL_syswm.h"

extern int __cdecl I_W32ExceptionHandler(PEXCEPTION_POINTERS ep);
int Kex_Main(int argc, char *argv[]);

int main(int argc, char **argv) {
    __try {
        Kex_Main(argc, argv);
    }
    __except(I_W32ExceptionHandler(GetExceptionInformation())) {
        common.Error("Exception caught in main: see CRASHLOG.TXT for info\n");
    }
    
    return 0;
}
#else
    #define Kex_Main main
#endif  /*_DEBUG*/

#endif  /*_WIN32*/

int Kex_Main(int argc, char *argv[]) {
#ifdef _WIN32
    _CrtSetDbgFlag(0);
#endif
    sysMain.Main(argc, argv);
    return 0;
}
