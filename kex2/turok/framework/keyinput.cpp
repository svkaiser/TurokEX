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
#include "keyinput.h"
#include "zone.h"

kexInputKey inputKey;

control_t control;

typedef struct {
    int		code;
    char	*name;
} keyinfo_t;

static keyinfo_t Keys[] = {
    { SDL_BUTTON_LEFT,      "mouse1" },
    { SDL_BUTTON_MIDDLE,    "mouse2" },
    { SDL_BUTTON_RIGHT,     "mouse3" },
    { SDLK_RIGHT,           "right" },
    { SDLK_LEFT,            "left" },
    { SDLK_UP,              "up" },
    { SDLK_DOWN,            "down" },
    { SDLK_ESCAPE,          "escape" },
    { SDLK_RETURN,          "enter" },
    { SDLK_TAB,             "tab" },
    { SDLK_BACKSPACE,       "backsp" },
    { SDLK_PAUSE,           "pause" },
    { SDLK_LSHIFT,          "shift" },
    { SDLK_LALT,            "alt" },
    { SDLK_LCTRL,           "ctrl" },
    { SDLK_PLUS,            "+" },
    { SDLK_MINUS,           "-" },
    { SDLK_CAPSLOCK,        "caps" },
    { SDLK_INSERT,          "ins" },
    { SDLK_DELETE,          "del" },
    { SDLK_HOME,            "home" },
    { SDLK_END,             "end" },
    { SDLK_PAGEUP,          "pgup" },
    { SDLK_PAGEDOWN,        "pgdn" },
    { SDLK_SPACE,           "space" },
    { SDLK_F1,              "f1" },
    { SDLK_F2,              "f2" },
    { SDLK_F3,              "f3" },
    { SDLK_F4,              "f4" },
    { SDLK_F5,              "f5" },
    { SDLK_F6,              "f6" },
    { SDLK_F7,              "f7" },
    { SDLK_F8,              "f8" },
    { SDLK_F9,              "f9" },
    { SDLK_F10,             "f10" },
    { SDLK_F11,             "f11" },
    { SDLK_F12,             "f12" },
    { SDLK_KP_ENTER,        "keypadenter" },
    { SDLK_KP_MULTIPLY,     "keypad*" },
    { SDLK_KP_PLUS,         "keypad+" },
    { SDLK_NUMLOCKCLEAR,    "numlock" },
    { SDLK_KP_MINUS,        "keypad-" },
    { SDLK_KP_PERIOD,       "keypad." },
    { SDLK_KP_DIVIDE,       "keypad/" },
    { SDLK_KP_0,            "keypad0" },
    { SDLK_KP_1,            "keypad1" },
    { SDLK_KP_2,            "keypad2" },
    { SDLK_KP_3,            "keypad3" },
    { SDLK_KP_4,            "keypad4" },
    { SDLK_KP_5,            "keypad5" },
    { SDLK_KP_6,            "keypad6" },
    { SDLK_KP_7,            "keypad7" },
    { SDLK_KP_8,            "keypad8" },
    { SDLK_KP_9,            "keypad9" },
    { 0,                    NULL }
};

//
// FCmd_KeyAction
//

static void FCmd_KeyAction(void) {
    char *argv;
    int action;

    argv = command.GetArgv(0);
    action = inputKey.FindAction(argv);

    if(action == -1)
        return;

    if(argv[0] == '-')
        action |= CKF_UP;

    inputKey.HandleControl(action);
}

//
// FCmd_Bind
//

static void FCmd_Bind(void) {
    int argc;
    int key;
    int i;
    char cmd[1024];

    argc = command.GetArgc();

    if(argc < 3) {
        common.Printf("bind <key> <command>\n");
        return;
    }

    if(!(key = inputKey.GetKeyCode(command.GetArgv(1)))) {
        common.Warning("\"%s\" isn't a valid key\n", command.GetArgv(1));
        return;
    }

    cmd[0] = 0;
    for(i = 2; i < argc; i++) {
        strcat(cmd, command.GetArgv(i));
        if(i != (argc - 1)) {
            strcat(cmd, " ");
        }
    }

    inputKey.BindCommand(key, cmd);
}

//
// FCmd_UnBind
//

static void FCmd_UnBind(void) {
}

//
// FCmd_ListBinds
//

static void FCmd_ListBinds(void) {
    inputKey.ListBindings();
}

//
// kexInputKey::GetKeyCode
//

int kexInputKey::GetKeyCode(char *key) {
    keyinfo_t *pkey;
    int len;

    strlwr(key);
    len = strlen(key);

    if(len == 1) {
        if(((*key >= 'a') && (*key <= 'z')) || ((*key >= '0') && (*key <= '9'))) {
            return *key;
        }
    }

    for(pkey = Keys; pkey->name; pkey++) {
        if(!strcmp(key, pkey->name)) {
            return pkey->code;
        }
    }

    return 0;
}

//
// kexInputKey::GetName
//

bool kexInputKey::GetName(char *buff, int key) {
    keyinfo_t *pkey;
        
    if(((key >= 'a') && (key <= 'z')) || ((key >= '0') && (key <= '9'))) {
        buff[0] = (char)toupper(key);
        buff[1] = 0;

        return true;
    }
    for(pkey = Keys; pkey->name; pkey++) {
        // F1 - F12 keys
        if(key >= 26 && key <= 37) {
            if(pkey->code == (SDLK_F1 + (key-26))) {
                strcpy(buff, pkey->name);
                return true;
            }
        }

        if(pkey->code == key) {
            strcpy(buff, pkey->name);
            return true;
        }
    }
    sprintf(buff, "Key%02x", key);
    return false;
}

//
// kexInputKey::BindCommand
//

void kexInputKey::BindCommand(char key, const char *string) {
    keycmd_t *keycmd;
    cmdlist_t *newcmd;

    keycmd = &keycmds[keycode[bShiftdown][key]];
    newcmd = (cmdlist_t*)Z_Calloc(sizeof(cmdlist_t), PU_STATIC, 0);
    newcmd->command = Z_Strdup(string, PU_STATIC, 0);
    newcmd->next = keycmd->cmds;
    keycmd->cmds = newcmd;
}

//
// kexInputKey::Clear
//

void kexInputKey::Clear(void) {
    memset(&control, 0, sizeof(control_t));
}

//
// kexInputKey::HandleControl
//

void kexInputKey::HandleControl(int ctrl) {
    int ctrlkey;
    
    ctrlkey = (ctrl & CKF_COUNTMASK);
    
    if(ctrl & CKF_UP) {
        if((control.actions[ctrlkey] & CKF_COUNTMASK) > 0)
            control.actions[ctrlkey] = 0;
    }
    else {
        control.actions[ctrlkey] = 1;
    }
}

//
// kexInputKey::ListBindings
//

void kexInputKey::ListBindings(void) {
    keycmd_t *keycmd;
    cmdlist_t *cmd;

    common.Printf("\n");

    for(int i = 0; i < MAX_KEYS; i++) {
        keycmd = &keycmds[i];

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next) {
            char buff[32];

            GetName(buff, i);
            strlwr(buff);

            common.CPrintf(COLOR_GREEN, "%s : \"%s\"\n", buff, cmd->command);
        }
    }
}

//
// kexInputKey::FindAction
//

int kexInputKey::FindAction(const char *name) {
    keyaction_t *action;
    unsigned int hash;

    if(name[0] == 0)
        return -1;

    hash = common.HashFileName(name);

    for(action = keyactions[hash]; action; action = action->next) {
        if(!strcmp(name, action->name))
            return action->keyid;
    }

    return -1;
}

//
// kexInputKey::ExecuteCommand
//

void kexInputKey::ExecuteCommand(char key, bool keyup) {
    keycmd_t *keycmd;
    cmdlist_t *cmd;

    keycmd = &keycmds[keycode[bShiftdown][key]];

    for(cmd = keycmd->cmds; cmd; cmd = cmd->next) {
        if(cmd->command[0] == '+' || cmd->command[0] == '-') {
            if(keyup && cmd->command[0] == '+') {
                cmd->command[0] = '-';
            }

            command.Execute(cmd->command);

            if(keyup && cmd->command[0] == '-') {
                cmd->command[0] = '+';
            }
        }
        else if(!keyup) {
            command.Execute(cmd->command);
        }
    }
}

//
// kexInputKey::AddAction
//

void kexInputKey::AddAction(byte id, const char *name) {
    keyaction_t *keyaction;
    unsigned int hash;
    
    if(strlen(name) >= MAX_FILEPATH)
        common.Error("Key_AddAction: \"%s\" is too long", name);

    keyaction = (keyaction_t*)Z_Calloc(sizeof(keyaction_t), PU_STATIC, 0);
    keyaction->keyid = id;
    strcpy(keyaction->name, name);

    command.Add(keyaction->name, FCmd_KeyAction);

    hash = common.HashFileName(keyaction->name);
    keyaction->next = keyactions[hash];
    keyactions[hash] = keyaction;
}

//
// kexInputKey::WriteBindings
//

void kexInputKey::WriteBindings(FILE *file) {
    keycmd_t *keycmd;
    cmdlist_t *cmd;

    for(int i = 0; i < MAX_KEYS; i++) {
        keycmd = &keycmds[i];

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next) {
            char buff[32];

            GetName(buff, i);
            strlwr(buff);

            fprintf(file, "bind %s \"%s\"\n", buff, cmd->command);
        }
    }
}

//
// kexInputKey::Init
//

void kexInputKey::Init(void) {
    for(int c = 0; c < MAX_KEYS; c++)
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
    
    for(int c = 'a'; c <= 'z'; c++)
        keycode[1][c] = toupper(c);

    command.Add("bind", FCmd_Bind);
    command.Add("unbind", FCmd_UnBind);
    command.Add("listbinds", FCmd_ListBinds);

    common.Printf("Key System Initialized\n");
}
