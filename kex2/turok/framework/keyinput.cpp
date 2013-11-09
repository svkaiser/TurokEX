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
#include "scriptAPI/scriptSystem.h"

kexInputKey inputKey;

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
        if(key >= SDL_SCANCODE_F1 && key <= SDL_SCANCODE_F12) {
            if(pkey->code == SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_F1 + (key-SDL_SCANCODE_F1))) {
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
    newcmd = (cmdlist_t*)Mem_Calloc(sizeof(cmdlist_t), hb_static);
    newcmd->command = Mem_Strdup(string, hb_static);
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

    if(!command.Verify(name))
        return;

    keyaction = (keyaction_t*)Mem_Calloc(sizeof(keyaction_t), hb_static);
    keyaction->keyid = id;
    strcpy(keyaction->name, name);

    command.Add(keyaction->name, FCmd_KeyAction);

    hash = common.HashFileName(keyaction->name);
    keyaction->next = keyactions[hash];
    keyactions[hash] = keyaction;
}

//
// kexInputKey::AddAction
//

void kexInputKey::AddAction(byte id, const kexStr &str) {
    AddAction(id, str.c_str());
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
// kexInputKey::InitObject
//

void kexInputKey::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kInput",
        sizeof(kexInputKey),
        asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    scriptManager.Engine()->RegisterObjectMethod(
        "kInput",
        "void AddAction(const int, const kStr &in)",
        asMETHODPR(kexInputKey, AddAction, (byte id, const kexStr &str), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterGlobalProperty("kInput Input", &inputKey);

    scriptManager.Engine()->RegisterEnum("EnumInputKey");
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNKNOWN", SDLK_UNKNOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSPACE", SDLK_BACKSPACE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_TAB", SDLK_TAB);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CLEAR", SDLK_CLEAR);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RETURN", SDLK_RETURN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAUSE", SDLK_PAUSE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ESCAPE", SDLK_ESCAPE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SPACE", SDLK_SPACE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EXCLAIM", SDLK_EXCLAIM);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTEDBL", SDLK_QUOTEDBL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HASH", SDLK_HASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOLLAR", SDLK_DOLLAR);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AMPERSAND", SDLK_AMPERSAND);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTE", SDLK_QUOTE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTPAREN", SDLK_LEFTPAREN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTPAREN", SDLK_RIGHTPAREN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ASTERISK", SDLK_ASTERISK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PLUS", SDLK_PLUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COMMA", SDLK_COMMA);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MINUS", SDLK_MINUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PERIOD", SDLK_PERIOD);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SLASH", SDLK_SLASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_0", SDLK_0);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_1", SDLK_1);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_2", SDLK_2);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_3", SDLK_3);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_4", SDLK_4);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_5", SDLK_5);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_6", SDLK_6);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_7", SDLK_7);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_8", SDLK_8);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_9", SDLK_9);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COLON", SDLK_COLON);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SEMICOLON", SDLK_SEMICOLON);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LESS", SDLK_LESS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EQUALS", SDLK_EQUALS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_GREATER", SDLK_GREATER);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUESTION", SDLK_QUESTION);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AT", SDLK_AT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTBRACKET", SDLK_LEFTBRACKET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSLASH", SDLK_BACKSLASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTBRACKET", SDLK_RIGHTBRACKET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CARET", SDLK_CARET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNDERSCORE", SDLK_UNDERSCORE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKQUOTE", SDLK_BACKQUOTE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_a", SDLK_a);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_b", SDLK_b);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_c", SDLK_c);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_d", SDLK_d);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_e", SDLK_e);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_f", SDLK_f);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_g", SDLK_g);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_h", SDLK_h);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_i", SDLK_i);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_j", SDLK_j);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_k", SDLK_k);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_l", SDLK_l);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_m", SDLK_m);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_n", SDLK_n);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_o", SDLK_o);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_p", SDLK_p);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_q", SDLK_q);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_r", SDLK_r);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_s", SDLK_s);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_t", SDLK_t);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_u", SDLK_u);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_v", SDLK_v);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_w", SDLK_w);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_x", SDLK_x);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_y", SDLK_y);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_z", SDLK_z);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DELETE", SDLK_DELETE);
    /*scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP0", SDLK_KP0);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP1", SDLK_KP1);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP2", SDLK_KP2);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP3", SDLK_KP3);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP4", SDLK_KP4);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP5", SDLK_KP5);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP6", SDLK_KP6);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP7", SDLK_KP7);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP8", SDLK_KP8);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP9", SDLK_KP9);*/
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PERIOD", SDLK_KP_PERIOD);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_DIVIDE", SDLK_KP_DIVIDE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MULTIPLY", SDLK_KP_MULTIPLY);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MINUS", SDLK_KP_MINUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PLUS", SDLK_KP_PLUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_ENTER", SDLK_KP_ENTER);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_EQUALS", SDLK_KP_EQUALS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UP", SDLK_UP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOWN", SDLK_DOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHT", SDLK_RIGHT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFT", SDLK_LEFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_INSERT", SDLK_INSERT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HOME", SDLK_HOME);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_END", SDLK_END);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEUP", SDLK_PAGEUP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEDOWN", SDLK_PAGEDOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F1", SDLK_F1);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F2", SDLK_F2);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F3", SDLK_F3);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F4", SDLK_F4);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F5", SDLK_F5);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F6", SDLK_F6);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F7", SDLK_F7);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F8", SDLK_F8);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F9", SDLK_F9);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F10", SDLK_F10);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F11", SDLK_F11);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F12", SDLK_F12);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F13", SDLK_F13);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F14", SDLK_F14);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F15", SDLK_F15);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_NUMLOCK", SDLK_NUMLOCK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CAPSLOCK", SDLK_CAPSLOCK);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SCROLLOCK", SDLK_SCROLLOCK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RSHIFT", SDLK_RSHIFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LSHIFT", SDLK_LSHIFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RCTRL", SDLK_RCTRL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LCTRL", SDLK_LCTRL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RALT", SDLK_RALT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LALT", SDLK_LALT);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RMETA", SDLK_RMETA);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LMETA", SDLK_LMETA);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LSUPER", SDLK_LSUPER);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RSUPER", SDLK_RSUPER);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MODE", SDLK_MODE);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COMPOSE", SDLK_COMPOSE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HELP", SDLK_HELP);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PRINT", SDLK_PRINT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SYSREQ", SDLK_SYSREQ);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BREAK", SDLK_BREAK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MENU", SDLK_MENU);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_POWER", SDLK_POWER);
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EURO", SDLK_EURO);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNDO", SDLK_UNDO);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_LEFT",SDL_BUTTON_LEFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_MIDDLE",SDL_BUTTON_MIDDLE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_RIGHT",SDL_BUTTON_RIGHT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELUP",SDL_BUTTON_WHEELUP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELDOWN",SDL_BUTTON_WHEELDOWN);
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
