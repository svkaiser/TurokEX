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
// DESCRIPTION: Menu system
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "SDL.h"

#include "common.h"
#include "gl.h"
#include "zone.h"
#include "kernel.h"
#include "client.h"
#include "menu.h"

/*short       menu_itemOn;
kbool       menuactive;
menu_t*     menu_current;

static short        itemSelected;
static menu_t*      oldmenu;
static menu_t*      initialmenu;

//------------------------------------------------------------------------
//
// MENU CVARS
//
//------------------------------------------------------------------------

CVAR_CMD(m_menumouse, 1)
{
    SDL_ShowCursor(cvar->value < 1);
    if(cvar->value <= 0)
        itemSelected = -1;
}

CVAR_CMD(m_cursorscale, 8)
{
    if(cvar->value < 0)
        cvar->value = 0;

    if(cvar->value > 50)
        cvar->value = 50;
}

//
// Menu_GetActiveItems
//

int Menu_GetActiveItems(menu_t *menu)
{
    int numitems;
    int i;

    numitems = 0;

    for(i = 0; i < menu->numitems; i++)
    {
        if(menu->menuitems[i].status == MIS_HIDDEN)
        {
            continue;
        }

        numitems++;
    }

    return numitems;
}

//
// Menu_Clear
//

void Menu_Clear(void)
{
    IN_CenterMouse();

    menuactive = false;
    menu_current->lastOn = menu_itemOn;
    menu_current->opacity = 0.0f;
    menu_current->status = MS_READY;

    if(oldmenu)
    {
        oldmenu->opacity = 0.0f;
        oldmenu->status = MS_READY;
        oldmenu = NULL;
    }
}

//
// Menu_Set
//

void Menu_Set(menu_t *menu)
{
    if(menu_current != menu)
    {
        oldmenu = menu_current;
        oldmenu->lastOn = menu_itemOn;
        oldmenu->status = MS_FADEOUT;
        oldmenu->opacity = 1.0f;
    }

    menu_current = menu;
    menu_current->status = MS_FADEIN;
    menu_current->opacity = 0.0f;

    menu_itemOn = menu_current->lastOn;
}

//
// Menu_Ticker
//

void Menu_Ticker(void)
{
    if(menuactive == false)
    {
        return;
    }

    if(oldmenu && oldmenu->status == MS_FADEOUT)
    {
        oldmenu->opacity -= 0.05f;
        if(oldmenu->opacity < 0.0f)
        {
            oldmenu->opacity = 0.0f;
            oldmenu = NULL;
        }
    }

    if(menu_current && menu_current->status == MS_FADEIN)
    {
        menu_current->opacity += 0.05f;
        if(menu_current->opacity > 1.0f)
        {
            menu_current->opacity = 1.0f;
            menu_current->status = MS_READY;
        }
    }
}

//
// Menu_Responder
//

kbool Menu_Responder(event_t* ev)
{
    if(ev->type == ev_keydown)
    {
        switch(ev->data1)
        {
        case SDLK_ESCAPE:
            if(!menuactive)
            {
                menuactive = true;
                Menu_Set(initialmenu);
                return true;
            }
            else if(menu_current->prevMenu == NULL)
            {
                Menu_Clear();
                return true;
            }
            else
            {
                Menu_Set(menu_current->prevMenu);
                return true;
            }
        }
    }

    if(menu_current->responder && menu_current->status == MS_READY)
    {
        return menu_current->responder(ev);
    }

    return false;
}

//
// Menu_DrawFillBox
//

void Menu_DrawFillBox(float x, float y, float w, float h,
                  int bordersize, int tintside,
                  byte r, byte g, byte b, byte a)
{
    byte r1, r2;
    byte g1, g2;
    byte b1, b2;
    float ratiox;
    float ratioy;
    float rx;
    float ry;
    float rw;
    float rh;

    ratiox = (float)FIXED_WIDTH / video_width;
    ratioy = (float)FIXED_HEIGHT / video_height;
    rx = x / ratiox;
    rw = w / ratiox;
    ry = y / ratioy;
    rh = h / ratioy;

    dglDisable(GL_TEXTURE_2D);
    GL_SetState(GLSTATE_BLEND, 1);

    if(tintside)
    {
        r1 = r>>1;
        g1 = g>>1;
        b1 = b>>1;

        r2 = r<<1;
        g2 = g<<1;
        b2 = b<<1;
    }
    else
    {
        r1 = r<<1;
        g1 = g<<1;
        b1 = b<<1;

        r2 = r>>1;
        g2 = g>>1;
        b2 = b>>1;
    }

    // body frame
    dglColor4ub(r, g, b, a);
    dglRectf(rx, ry, rw, rh);

    // border
    dglColor4ub(r2, g2, b2, a);
    dglRectf(rx, ry, rw, ry + bordersize);

    // border
    dglColor4ub(r1, g1, b1, a);
    dglRectf(rw - bordersize, ry, rw, rh);

    // border
    dglColor4ub(r1, g1, b1, a);
    dglRectf(rx, rh - bordersize, rw, rh);

    // border
    dglColor4ub(r2, g2, b2, a);
    dglRectf(rx, ry, rx + bordersize, rh);

    dglEnable(GL_TEXTURE_2D);
    GL_SetState(GLSTATE_BLEND, 0);
}

//
// Menu_DrawCursor
//

void Menu_DrawCursor(float x, float y, float w, float h)
{
    static int cur_flashtic = 0;
    byte alpha;
    rcolor color;
    float ratiox;
    float ratioy;
    float rx;
    float ry;
    float rw;
    float rh;
    float sw;
    float sh;
    vtx_t vtx[36];
    int i;
    int j;
    int v;

    ratiox = (float)FIXED_WIDTH / video_width;
    ratioy = (float)FIXED_HEIGHT / video_height;
    rx = x / ratiox;
    rw = w / ratiox;
    ry = y / ratioy;
    rh = h / ratioy;
    sw = rw / 3.0f;
    sh = rh / 3.0f;

    GL_SetVertexPointer(vtx);

    dglDisable(GL_TEXTURE_2D);
    GL_SetState(GLSTATE_BLEND, 1);

    memset(vtx, 0, sizeof(vtx_t) * 36);

    dglSetVertexColor(vtx, 0x00002020, 36);

    alpha = 160 + (int)(cos((float)(cur_flashtic++) * 0.125f) * 95.0f);
    color = RGBA(0x7C, 0x67, 0x42, alpha);

    *(rcolor*)&vtx[3].r = color;
    *(rcolor*)&vtx[6].r = color;
    *(rcolor*)&vtx[7].r = color;
    *(rcolor*)&vtx[10].r = color;
    *(rcolor*)&vtx[13].r = color;
    *(rcolor*)&vtx[15].r = color;
    *(rcolor*)&vtx[16].r = color;
    *(rcolor*)&vtx[17].r = color;
    *(rcolor*)&vtx[18].r = color;
    *(rcolor*)&vtx[19].r = color;
    *(rcolor*)&vtx[20].r = color;
    *(rcolor*)&vtx[22].r = color;
    *(rcolor*)&vtx[25].r = color;
    *(rcolor*)&vtx[28].r = color;
    *(rcolor*)&vtx[29].r = color;
    *(rcolor*)&vtx[32].r = color;
    
    v = 0;

    for(j = 0; j < 3; j++)
    {
        for(i = 0; i < 3; i++)
        {
            vtx[0+v].x = rx + (sw * i);
            vtx[0+v].y = ry + (sh * j);
            vtx[1+v].x = rx + (sw * (i+1));
            vtx[1+v].y = ry + (sh * j);
            vtx[2+v].x = rx + (sw * i);
            vtx[2+v].y = ry + (sh * (j+1));
            vtx[3+v].x = rx + (sw * (i+1));
            vtx[3+v].y = ry + (sh * (j+1));

            GL_Triangle(0+v, 1+v, 2+v);
            GL_Triangle(3+v, 2+v, 1+v);

            v += 4;
        }
    }

    GL_DrawElements(36, vtx);

    dglEnable(GL_TEXTURE_2D);
    GL_SetState(GLSTATE_BLEND, 0);
}

//
// Menu_Drawer
//

void Menu_Drawer(void)
{
    if(menuactive == false)
    {
        return;
    }

    if(menu_current->drawer)
    {
        menu_current->drawer(menu_current);
    }

    if(oldmenu)
    {
        if(oldmenu->drawer)
        {
            oldmenu->drawer(oldmenu);
        }
    }
}

//
// Menu_Init
//

void Menu_Init(void)
{
    //Cvar_Register(&m_menumouse);
    //Cvar_Register(&m_cursorscale);

    oldmenu = NULL;
    initialmenu = &menu_main;
    menu_current = initialmenu;
    menuactive = false;
    menu_itemOn = 0;
    itemSelected = -1;
}*/
