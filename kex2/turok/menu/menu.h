// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _MENU_H_
#define _MENU_H_

#include "type.h"
#include "client.h"

//------------------------------------------------------------------------
//
// MENU TYPEDEFS
//
//------------------------------------------------------------------------

typedef enum
{
    MIS_HIDDEN      = -3,
    MIS_ENTEROK     = -2,
    MIS_DISABLED    = -1,
    MIS_NOCURSOR    =  0,
    MIS_OK          =  1,
    MIS_ARROWSOK    =  2,
    MIS_SLIDER      =  3
} menuitemstatus_e;

typedef struct
{
    menuitemstatus_e    status;
    char                name[64];
    void                (*routine)(int choice);
} menuitem_t;

typedef struct
{
    cvar_t  *mitem;
    float   mdefault;
} menudefault_t;

typedef struct
{
    int     item;
    float   width;
    cvar_t  *mitem;
} menuthermobar_t;

typedef enum
{
    MS_READY    = 0,
    MS_FADEIN   = 1,
    MS_FADEOUT  = 2
} menustatus_e;

typedef struct menu_s
{
    kbool               (*responder)(event_t*);
    short               numitems;                       // # of menu items
    struct menu_s       *prevMenu;                      // previous menu
    menuitem_t          *menuitems;                     // menu items
    void                (*drawer)(struct menu_s *menu); // draw routine
    short               lastOn;                         // last item user was on in menu
    menudefault_t       *defaultitems;                  // pointer to default values for cvars
    short               numpageitems;                   // number of items to display per page
    short               menupageoffset;
    char                **hints;
    menuthermobar_t     *thermobars;
    float               opacity;
    menustatus_e        status;
} menu_t;

extern short    menu_itemOn;
extern kbool    menuactive;
extern menu_t*  menu_current;

int Menu_GetActiveItems(menu_t *menu);
void Menu_Set(menu_t *menu);
void Menu_DrawFillBox(float x, float y, float w, float h, int bordersize, int tintside,
                  byte r, byte g, byte b, byte a);
void Menu_DrawCursor(float x, float y, float w, float h);
kbool Menu_Responder(event_t* ev);
void Menu_Ticker(void);
void Menu_Drawer(void);
void Menu_Init(void);

//------------------------------------------------------------------------
//
// MENU ITEMS
//
//------------------------------------------------------------------------

extern menu_t menu_main;
extern menu_t menu_options;

#endif