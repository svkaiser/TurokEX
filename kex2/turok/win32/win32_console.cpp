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
// DESCRIPTION:
//	Win32 Console
//
//-----------------------------------------------------------------------------

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>

#include "SDL.h"
#include "common.h"
#include "system.h"

#define EDIT_ID     100
#define ENTER_ID    102
#define QUIT_ID     103
#define INPUT_ID    104

HWND        hwndMain;           //Main window handle
HWND        hwndBuffer;
HWND        hwndButtonEnter;
HWND        hwndButtonReady;
HWND        hwndButtonQuit;
HWND        hwndInputLine;
HFONT       hfBufferFont;
HFONT       hfButtonFont;
HBRUSH      hbrEditBackground;
HDC         hdc = 0;
HINSTANCE   hwndMainInst;       //Main window instance

char        kexSysConsoleClass[] = "KexSysConsole";

//
// Sys_ShowConsole
//

void Sys_ShowConsole(kbool show)
{
    ShowWindow(hwndMain, show ? SW_SHOW : SW_HIDE);
}

//
// Sys_DestroyConsole
//

void Sys_DestroyConsole(void)
{
    if(hwndMain)
    {
        Sys_ShowConsole(false);
        CloseWindow(hwndMain);
        DestroyWindow(hwndMain);
        hwndMain = 0;
    }
}

//
// Sys_ConsoleProc
//

LRESULT CALLBACK Sys_ConsoleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch(msg) 
    {    
    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
        case QUIT_ID:
            Sys_DestroyConsole();
            SDL_Quit();
            exit(0);
            break;
            
        case ENTER_ID:
            break;
        }
        break;
        
        case WM_SETFOCUS:
        case WM_SHOWWINDOW:
            UpdateWindow(hwndMain);
            break;
            
        case WM_CREATE:
            hbrEditBackground = CreateSolidBrush(RGB(0, 64, 0));
            break;
            
        case WM_CTLCOLORSTATIC:
            if((HWND) lParam == hwndBuffer)
            {
                SetBkColor((HDC) wParam, RGB(0, 64, 0));
                SetTextColor((HDC) wParam, RGB(0x00, 0xff, 0x00));
                
                return (long)hbrEditBackground;
            }
            break;
                
        default:
            return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}

//
// Sys_SpawnConsole
//

void Sys_SpawnConsole(void)
{
    WNDCLASSEX  wcx;
    RECT        rect;
    int         swidth;
    int         sheight;
    
    hwndMainInst = GetModuleHandle(NULL);
    memset(&wcx, 0, sizeof(WNDCLASSEX));

    wcx.cbSize          = sizeof(WNDCLASSEX);
    wcx.style           = CS_OWNDC;
    wcx.lpfnWndProc     = (WNDPROC)Sys_ConsoleProc;
    wcx.cbClsExtra      = 0;
    wcx.cbWndExtra      = 0;
    wcx.hInstance       = hwndMainInst;
    wcx.hIcon           = NULL;
    wcx.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground   = (HBRUSH)COLOR_WINDOW;
    wcx.lpszClassName   = kexSysConsoleClass;
    wcx.hIconSm         = NULL;
    
    if(!RegisterClassEx(&wcx))
        return;
    
    rect.left   = 0;
    rect.right  = 384;
    rect.top    = 0;
    rect.bottom = 480;

    AdjustWindowRect(&rect, WS_POPUPWINDOW, FALSE);
    
    hdc     = GetDC(GetDesktopWindow());
    swidth  = GetDeviceCaps(hdc, HORZRES);
    sheight = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(GetDesktopWindow(), hdc);
    
    // Create the window
    hwndMain = CreateWindowEx(
        0,                          //Extended window style
        kexSysConsoleClass,         // Window class name
        NULL,                       // Window title
        WS_POPUPWINDOW,             // Window style
        (swidth - 400) / 2,
        (sheight - 480 ) / 2 ,
        rect.right - rect.left + 1,
        rect.bottom - rect.top + 1, // Width and height of the window
        NULL,                       // HWND of the parent window (can be null also)
        NULL,                       // Handle to menu
        hwndMainInst,               // Handle to application instance
        NULL                        // Pointer to window creation data
        );
    
    if(!hwndMain)
        return;
    
    hwndBuffer = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | 
        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        6, 16, 370, 400,
        hwndMain, 
        (HMENU)EDIT_ID,
        hwndMainInst, NULL);
    
    hwndButtonEnter = CreateWindow("button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        5, 445, 64, 24,
        hwndMain, 
        (HMENU)ENTER_ID,
        hwndMainInst, NULL);
    SendMessage(hwndButtonEnter, WM_SETTEXT, 0, (LPARAM) "enter");
    
    hwndButtonQuit = CreateWindow("button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        312, 445, 64, 24,
        hwndMain, 
        (HMENU)QUIT_ID,
        hwndMainInst, NULL);
    SendMessage(hwndButtonQuit, WM_SETTEXT, 0, (LPARAM) "quit");

    hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
        6, 420, 370, 20,
        hwndMain, 
        (HMENU)INPUT_ID,
        hwndMainInst, NULL);
    
    hdc = GetDC(hwndMain);
    hfBufferFont = CreateFont(-MulDiv( 8, GetDeviceCaps( hdc, LOGPIXELSY), 72),
        0,
        0,
        0,
        FW_LIGHT,
        0,
        0,
        0,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        FF_MODERN | FIXED_PITCH,
        "Courier New");
    
    ReleaseDC(hwndMain,hdc);
    
    SendMessage(hwndBuffer, WM_SETFONT, (WPARAM) hfBufferFont, 0);
    
    Sys_ShowConsole(1);
    UpdateWindow(hwndMain);
}

//
// Sys_Printf
//

#define CONSOLE_BUFFER_SIZE		16384

void Sys_Printf(const char *string)
{
    char buffer[CONSOLE_BUFFER_SIZE*2];
    char *b = buffer;
    const char *msg;
    int bufLen;
    int i = 0;
    static unsigned long s_totalChars;

    //
    // if the message is REALLY long, use just the last portion of it
    //
    if(strlen(string) > CONSOLE_BUFFER_SIZE - 1)
    {
        msg = string + strlen(string) - CONSOLE_BUFFER_SIZE + 1;
    }
    else
    {
        msg = string;
    }

    //
    // copy into an intermediate buffer
    //
    while(msg[i] && ((b - buffer) < sizeof(buffer) - 1 ))
    {
        if(msg[i] == '\n' && msg[i+1] == '\r')
        {
            b[0] = '\r';
            b[1] = '\n';
            b += 2;
            i++;
        }
        else if(msg[i] == '\r')
        {
            b[0] = '\r';
            b[1] = '\n';
            b += 2;
        }
        else if(msg[i] == '\n')
        {
            b[0] = '\r';
            b[1] = '\n';
            b += 2;
        }
        else
        {
            *b= msg[i];
            b++;
        }

        i++;
    }

    *b = 0;
    bufLen = b - buffer;
    s_totalChars += bufLen;

    //
    // replace selection instead of appending if we're overflowing
    //
    if(s_totalChars > 0x7fff)
    {
        SendMessage(hwndBuffer, EM_SETSEL, 0, -1 );
        s_totalChars = bufLen;
    }

    //
    // put this text into the windows console
    //
    SendMessage(hwndBuffer, EM_LINESCROLL, 0, 0xffff);
    SendMessage(hwndBuffer, EM_SCROLLCARET, 0, 0 );
    SendMessage(hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer);
}

//
// Sys_UpdateConsole
//

void Sys_UpdateConsole(void)
{
    MSG msg;

    if(GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//
// Sys_Error
//

void Sys_Error(const char *string)
{
    MSG msg;

    Sys_ShowConsole(true);
    Sys_Printf("\n********* ERROR *********\n");
    Sys_Printf(string);

    while(1)
    {
        while(GetMessage(&msg,NULL,0,0)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        sysMain.Sleep(100);
    }
}

#endif
