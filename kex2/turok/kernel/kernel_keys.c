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
// DESCRIPTION: Key input handling and binding
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include "common.h"
#include "kernel.h"
#include "zone.h"

char keycode[2][MAX_KEYS];

typedef struct cmdlist_s
{
    char *command;
    struct cmdlist_s *next;
} cmdlist_t;

typedef struct
{
    cmdlist_t *cmds;
    char *name;
} keycmd_t;

static keycmd_t keycmds[MAX_KEYS];
static kbool shiftdown = false;
static kbool keydown[MAX_KEYS];

typedef struct
{
    int		code;
    char	*name;
} keyinfo_t;

static keyinfo_t Keys[] =
{
    { SDLK_RIGHT,       "Right" },
    { SDLK_LEFT,        "Left" },
    { SDLK_UP,          "Up" },
    { SDLK_DOWN,        "Down" },
    { SDLK_ESCAPE,      "Escape" },
    { SDLK_RETURN,      "Enter" },
    { SDLK_TAB,         "Tab" },
    { SDLK_BACKSPACE,   "Backsp" },
    { SDLK_PAUSE,       "Pause" },
    { SDLK_LSHIFT,      "Shift" },
    { SDLK_LALT,        "Alt" },
    { SDLK_LCTRL,       "Ctrl" },
    { SDLK_PLUS,        "+" },
    { SDLK_MINUS,       "-" },
    { SDLK_CAPSLOCK,    "Caps" },
    { SDLK_INSERT,      "Ins" },
    { SDLK_DELETE,      "Del" },
    { SDLK_HOME,        "Home" },
    { SDLK_END,         "End" },
    { SDLK_PAGEUP,      "PgUp" },
    { SDLK_PAGEDOWN,    "PgDn" },
    { ';',              ";" },
    { '\'',             "'" },
    { '#',              "#" },
    { '\\',             "\\" },
    { ',',              "," },
    { '.',              "." },
    { '/',              "/" },
    { '[',              "[" },
    { ']',              "]" },
    { '*',              "*" },
    { ' ',              "Space" },
    { SDLK_F1,          "F1" },
    { SDLK_F2,          "F2" },
    { SDLK_F3,          "F3" },
    { SDLK_F4,          "F4" },
    { SDLK_F5,          "F5" },
    { SDLK_F6,          "F6" },
    { SDLK_F7,          "F7" },
    { SDLK_F8,          "F8" },
    { SDLK_F9,          "F9" },
    { SDLK_F10,         "F10" },
    { SDLK_F11,         "F11" },
    { SDLK_F12,         "F12" },
    { SDLK_KP_ENTER,    "KeyPadEnter" },
    { SDLK_KP_MULTIPLY, "KeyPad*" },
    { SDLK_KP_PLUS,     "KeyPad+" },
    { SDLK_NUMLOCK,     "NumLock" },
    { SDLK_KP_MINUS,    "KeyPad-" },
    { SDLK_KP_PERIOD,   "KeyPad." },
    { SDLK_KP_DIVIDE,   "KeyPad/" },
    { SDLK_KP0,         "KeyPad0" },
    { SDLK_KP1,         "KeyPad1" },
    { SDLK_KP2,         "KeyPad2" },
    { SDLK_KP3,         "KeyPad3" },
    { SDLK_KP4,         "KeyPad4" },
    { SDLK_KP5,         "KeyPad5" },
    { SDLK_KP6,         "KeyPad6" },
    { SDLK_KP7,         "KeyPad7" },
    { SDLK_KP8,         "KeyPad8" },
    { SDLK_KP9,         "KeyPad9" },
    { '0',              "0" },
    { '1',              "1" },
    { '2',              "2" },
    { '3',              "3" },
    { '4',              "4" },
    { '5',              "5" },
    { '6',              "6" },
    { '7',              "7" },
    { '8',              "8" },
    { '9',              "9" },
    { 0,                NULL }
};

//
// Key_GetName
//

int Key_GetName(char *buff, int key)
{
    keyinfo_t *pkey;
        
    if(((key >= 'a') && (key <= 'z')) || ((key >= '0') && (key <= '9')))
    {
        buff[0] = (char)toupper(key);
        buff[1] = 0;
        return true;
    }

    for(pkey = Keys; pkey->name; pkey++)
    {
        if(pkey->code == key)
        {
            strcpy(buff, pkey->name);
            return true;
        }
    }

    sprintf(buff, "Key%02x", key);
    return false;
}

//
// Key_BindCmd
//

void Key_BindCmd(char key, const char *string)
{
    keycmd_t *keycmd;
    cmdlist_t *newcmd;

    keycmd = &keycmds[keycode[shiftdown][key]];
    newcmd = (cmdlist_t*)Z_Calloc(sizeof(cmdlist_t), PU_STATIC, 0);
    newcmd->command = Z_Strdup(string, PU_STATIC, 0);
    newcmd->next = keycmd->cmds;
    keycmd->cmds = newcmd;
}

//
// Key_ExecCmd
//

void Key_ExecCmd(char key)
{
    keycmd_t *keycmd;
    cmdlist_t *cmd;

    keycmd = &keycmds[keycode[shiftdown][key]];

    for(cmd = keycmd->cmds; cmd; cmd = cmd->next)
    {
        Cmd_ExecuteCommand(cmd->command);
    }
}

//
// FCmd_Bind
//

static void FCmd_Bind(void)
{
    int argc;
    int key;
    int i;
    char buff[32];
    char cmd[1024];

    argc = Cmd_GetArgc();

    if(argc < 2)
    {
        Com_Printf("bind <key> <command>\n");
        return;
    }

    if(!(key = Key_GetName(buff, atoi(Cmd_GetArgv(1)))))
    {
        Com_Warning("\"%s\" isn't a valid key\n", Cmd_GetArgv(1));
        return;
    }

    cmd[0] = 0;
    for(i = 2; i < argc; i++)
    {
        strcat(cmd, Cmd_GetArgv(i));
        if(i != (argc - 1))
        {
            strcat(cmd, " ");
        }
    }

    Key_BindCmd(key, cmd);
}

//
// FCmd_UnBind
//

static void FCmd_UnBind(void)
{
}

//
// FCmd_ListBinds
//

static void FCmd_ListBinds(void)
{
}

//
// Key_Init
//

void Key_Init(void)
{
    int c;

    for(c = 0; c < MAX_KEYS; c++)
    {
        keycode[0][c] = c;
        keycode[1][c] = c;
        keydown[c] = false;
        keycmds[c].cmds = NULL;
    }

    keycode[1]['1'] = '!';
    keycode[1]['2'] = '@';
    keycode[1]['3'] = '#';
    keycode[1]['4'] = '$';
    keycode[1]['5'] = '%';
    keycode[1]['6'] = '^';
    keycode[1]['7'] = '&';
    keycode[1]['8'] = '*';
    keycode[1]['9'] = '(';
    keycode[1]['0'] = ')';
    keycode[1]['-'] = '_';
    keycode[1]['='] = '+';
    keycode[1]['['] = '{';
    keycode[1][']'] = '}';
    keycode[1]['\\'] = '|';
    keycode[1][';'] = ':';
    keycode[1]['\''] = '"';
    keycode[1][','] = '<';
    keycode[1]['.'] = '>';
    keycode[1]['/'] = '?';
    keycode[1]['`'] = '~';
    
    for(c = 'a'; c <= 'z'; c++)
    {
        keycode[1][c] = toupper(c);
    }

    Cmd_AddCommand("bind", FCmd_Bind);
    Cmd_AddCommand("unbind", FCmd_UnBind);
    Cmd_AddCommand("listbinds", FCmd_ListBinds);
}

