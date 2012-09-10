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
// DESCRIPTION: Menu items
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include "common.h"
#include "client.h"
#include "gl.h"
#include "menu.h"

//
// Menu_DrawCenteredItems
//

static void Menu_DrawCenteredItems(menu_t *menu, float y,
                                   float opacity, float spacing)
{
    int i;
    float row;

    row = y;

    for(i = 0; i < menu->numitems; i++)
    {
        if(menu->menuitems[i].status == MIS_HIDDEN)
        {
            continue;
        }

        if(menu_itemOn == i && menu == menu_current)
        {
            Menu_DrawCursor(78, row - spacing + 2, 164, 15);
        }

        opacity = (menu->opacity * 255.0f) / 255.0f;

        Draw_SetBigTextColor(180, 180, 124, 77, 63, 42);
        Draw_ShadowedText(160, row, (byte)(255.0f * opacity),
            true, 0.35f, menu->menuitems[i].name);

        row += spacing;
    }
}

//
// Menu_DefaultDrawer
//

static void Menu_DefaultDrawer(menu_t *menu)
{
    float y;
    float row;
    float height;
    float opacity;

    height = (float)(15 * Menu_GetActiveItems(menu) + 8);
    row = 178.0f - height / 2;
    y = row;
    opacity = (menu->opacity * 150.0f) / 255.0f;

    Menu_DrawFillBox(70, y, 250, y + height, 2, 0, 0, 0, 0, (byte)(255.0f * opacity));
    Menu_DrawCenteredItems(menu, y + 16.0f, opacity, 15.0f);
}

//
// Menu_DefaultResponder
//

static kbool Menu_DefaultResponder(event_t *ev)
{
    if(ev->type == ev_keydown)
    {
        switch(ev->data1)
        {
        case SDLK_DOWN:
            do
            {
                if(menu_itemOn + 1 > menu_current->numitems-1)
                {
                    menu_itemOn = 0;
                }
                else
                {
                    menu_itemOn++;
                }
            } while(menu_current->menuitems[menu_itemOn].status == MIS_DISABLED ||
                menu_current->menuitems[menu_itemOn].status == MIS_HIDDEN);
            return true;

        case SDLK_UP:
            do
            {
                if(!menu_itemOn)
                {
                    menu_itemOn = menu_current->numitems-1;
                }
                else
                {
                    menu_itemOn--;
                }
            } while(menu_current->menuitems[menu_itemOn].status == MIS_DISABLED ||
                menu_current->menuitems[menu_itemOn].status == MIS_HIDDEN);
            return true;

        case SDLK_RETURN:
            if(menu_current->menuitems[menu_itemOn].routine)
            {
                menu_current->menuitems[menu_itemOn].routine(1);
            }
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------
//
// MAIN MENU
//
//------------------------------------------------------------------------

static void Menu_MainChoice(int choice);
static void Menu_SetupCheatCode(void);

enum
{
    item_start = 0,
    item_load,
    item_options,
    item_training,
    item_entercheat,
    item_cheatmenu,
    item_exitgame,
    item_mainend
} itemmain_e;

menuitem_t mitem_main[item_mainend] =
{
    { MIS_OK,       "start game",   Menu_MainChoice },
    { MIS_OK,       "load game",    Menu_MainChoice },
    { MIS_OK,       "options",      Menu_MainChoice },
    { MIS_OK,       "training",     Menu_MainChoice },
    { MIS_OK,       "enter cheat",  Menu_MainChoice },
    { MIS_HIDDEN,   "cheat menu",   Menu_MainChoice },
    { MIS_OK,       "quit",         NULL }
};

menu_t menu_main =
{
    Menu_DefaultResponder,  // responder
    item_mainend,           // numitems
    NULL,                   // previous menu
    mitem_main,             // menu item
    Menu_DefaultDrawer,     // drawer
    0,                      // last item
    NULL,                   // default items
    -1,                     // numpageitems
    0,                      // menupageoffset
    NULL,                   // hints
    NULL,                   // thermobars
    0.0f,                   // opacity
    MS_READY                // initial state
};

//
// Menu_MainChoice
//

static void Menu_MainChoice(int choice)
{
    switch(menu_itemOn)
    {
    case item_start:
        break;
    case item_load:
        break;
    case item_options:
        Menu_Set(&menu_options);
        break;
    case item_training:
        break;
    case item_entercheat:
        Menu_SetupCheatCode();
        break;
    case item_cheatmenu:
        break;
    case item_exitgame:
        break;
    }
}

//------------------------------------------------------------------------
//
// OPTIONS MENU
//
//------------------------------------------------------------------------

static void Menu_DrawOptions(menu_t *menu);
static void Menu_OptionChoice(int choice);

enum
{
    item_controls = 0,
    item_mouse,
    item_setup,
    item_sound,
    item_display,
    item_network,
    item_optionexit,
    item_optionend
} itemoption_e;

menuitem_t mitem_options[item_optionend] =
{
    { MIS_OK,       "controls", Menu_OptionChoice },
    { MIS_OK,       "mouse",    Menu_OptionChoice },
    { MIS_OK,       "setup",    Menu_OptionChoice },
    { MIS_OK,       "sound",    Menu_OptionChoice },
    { MIS_OK,       "display",  Menu_OptionChoice },
    { MIS_OK,       "network",  Menu_OptionChoice },
    { MIS_OK,       "exit",     Menu_OptionChoice }
};

menu_t menu_options =
{
    Menu_DefaultResponder,  // responder
    item_optionend,         // numitems
    &menu_main,             // previous menu
    mitem_options,          // menu item
    Menu_DrawOptions,       // drawer
    0,                      // last item
    NULL,                   // default items
    -1,                     // numpageitems
    0,                      // menupageoffset
    NULL,                   // hints
    NULL,                   // thermobars
    0.0f,                   // opacity
    MS_READY                // initial state
};

//
// Menu_OptionChoice
//

static void Menu_OptionChoice(int choice)
{
    switch(menu_itemOn)
    {
    case item_controls:
        break;
    case item_mouse:
        break;
    case item_setup:
        break;
    case item_sound:
        break;
    case item_display:
        break;
    case item_network:
        break;
    case item_optionexit:
        Menu_Set(menu_current->prevMenu);
        break;
    }
}

//
// Menu_DrawOptions
//

static void Menu_DrawOptions(menu_t *menu)
{
    float y;
    float row;
    float height;
    float opacity;

    height = (float)(15 * Menu_GetActiveItems(menu) + 8);
    row = 144.0f - height / 2;
    y = row;
    opacity = (menu->opacity * 150.0f) / 255.0f;

    Menu_DrawFillBox(56, 26, 264, 208, 2, 0, 0, 0, 0, (byte)(255.0f * opacity));
    Menu_DrawCenteredItems(menu, y + 16.0f, opacity, 15.0f);

    Draw_Pic("hud/h_options.tga", 266, 112, (byte)(255.0f * opacity), 1.0f);
}

//------------------------------------------------------------------------
//
// ENTER CHEAT CODE MENU
//
//------------------------------------------------------------------------

static char *cheatchars = "bcdfghjklmnpqrst";
static void Menu_CheatCodeDrawer(menu_t *menu);

menuitem_t mitem_cheatcode[19];

menu_t menu_cheatcode =
{
    Menu_DefaultResponder,  // responder
    19,                     // numitems
    &menu_main,             // previous menu
    mitem_cheatcode,        // menu item
    Menu_CheatCodeDrawer,   // drawer
    0,                      // last item
    NULL,                   // default items
    -1,                     // numpageitems
    0,                      // menupageoffset
    NULL,                   // hints
    NULL,                   // thermobars
    0.0f,                   // opacity
    MS_READY                // initial state
};

//
// Menu_CheatCodeChoice
//

static void Menu_CheatCodeChoice(int choice)
{
    switch(menu_itemOn)
    {
    case 16:
        break;
    case 17:
        break;
    case 18:
        Menu_Set(menu_current->prevMenu);
        break;
    }
}

//
// Menu_SetupCheatCode
//

static void Menu_SetupCheatCode(void)
{
    char *str = cheatchars;
    int i;

    for(i = 0; i < 16; i++)
    {
        mitem_cheatcode[i].status = MIS_OK;
        mitem_cheatcode[i].name[0] = *str++;
        mitem_cheatcode[i].routine = Menu_CheatCodeChoice;
    }

    mitem_cheatcode[16].status = MIS_OK;
    strcpy(mitem_cheatcode[16].name, "delete");
    mitem_cheatcode[16].routine = Menu_CheatCodeChoice;
    mitem_cheatcode[17].status = MIS_OK;
    strcpy(mitem_cheatcode[17].name, "enter");
    mitem_cheatcode[17].routine = Menu_CheatCodeChoice;
    mitem_cheatcode[18].status = MIS_OK;
    strcpy(mitem_cheatcode[18].name, "exit");
    mitem_cheatcode[18].routine = Menu_CheatCodeChoice;

    Menu_Set(&menu_cheatcode);
}

//
// Menu_CheatCodeDrawer
//

static void Menu_CheatCodeDrawer(menu_t *menu)
{
    float opacity;
    int i;
    int j;
    char c[2];
    float x;
    float y;

    opacity = (menu->opacity * 180.0f) / 255.0f;

    Menu_DrawFillBox(40, 25, 280, 215, 2, 0, 0, 0, 0, (byte)(255.0f * opacity));
    Draw_SetBigTextColor(200, 0, 200, 0, 200, 0);
    Draw_ShadowedText(160, 42, (byte)(255.0f * menu->opacity), true, 0.35f, "enter cheat code");

    Draw_SetBigTextColor(200, 200, 138, 86, 71, 47);

    y = 83;

    for(j = 0; j < 4; j++)
    {
        x = 124;
        
        for(i = 0; i < 4; i++)
        {
            int item = (i + (4 * j));

            if(menu_itemOn == item && menu == menu_current)
            {
                Menu_DrawCursor(x - 12, y - 18, 24, 18);
            }

            c[0] = cheatchars[item];
            c[1] = 0;
            Draw_ShadowedText(x, y, (byte)(255.0f * menu->opacity),
                true, 0.5f, c);

            x += 24.0f;
        }

        y += 18.0f;
    }

    if(menu_itemOn >= 16 && menu == menu_current)
    {
        switch(menu_itemOn)
        {
        case 16:
            Menu_DrawCursor(56, 150, 208, 18);
            break;
        case 17:
            Menu_DrawCursor(56, 170, 208, 18);
            break;
        case 18:
            Menu_DrawCursor(56, 190, 208, 18);
            break;
        default:
            break;
        }
    }

    Draw_ShadowedText(160, 166, (byte)(255.0f * menu->opacity),
        true, 0.5f, mitem_cheatcode[16].name);
    Draw_ShadowedText(160, 186, (byte)(255.0f * menu->opacity),
        true, 0.5f, mitem_cheatcode[17].name);
    Draw_ShadowedText(160, 206, (byte)(255.0f * menu->opacity),
        true, 0.5f, mitem_cheatcode[18].name);
}

