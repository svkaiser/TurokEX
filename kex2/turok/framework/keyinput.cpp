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

#include <ctype.h>
#include <string.h>

#include "common.h"
#include "keyinput.h"
#include "defs.h"
#include "gameManager.h"
#include "scriptAPI/scriptSystem.h"

kexInputKey inputKey;

typedef struct {
    int         code;
    const char  *name;
} keyinfo_t;

static keyinfo_t Keys[] = {
    { KM_BUTTON_LEFT,       "mouse1" },
    { KM_BUTTON_MIDDLE,     "mouse2" },
    { KM_BUTTON_RIGHT,      "mouse3" },
    { KKEY_RIGHT,           "right" },
    { KKEY_LEFT,            "left" },
    { KKEY_UP,              "up" },
    { KKEY_DOWN,            "down" },
    { KKEY_ESCAPE,          "escape" },
    { KKEY_RETURN,          "enter" },
    { KKEY_TAB,             "tab" },
    { KKEY_BACKSPACE,       "backsp" },
    { KKEY_PAUSE,           "pause" },
    { KKEY_LSHIFT,          "shift" },
    { KKEY_LALT,            "alt" },
    { KKEY_LCTRL,           "ctrl" },
    { KKEY_PLUS,            "+" },
    { KKEY_MINUS,           "-" },
    { KKEY_CAPSLOCK,        "caps" },
    { KKEY_INSERT,          "ins" },
    { KKEY_DELETE,          "del" },
    { KKEY_HOME,            "home" },
    { KKEY_END,             "end" },
    { KKEY_PAGEUP,          "pgup" },
    { KKEY_PAGEDOWN,        "pgdn" },
    { KKEY_SPACE,           "space" },
    { KKEY_F1,              "f1" },
    { KKEY_F2,              "f2" },
    { KKEY_F3,              "f3" },
    { KKEY_F4,              "f4" },
    { KKEY_F5,              "f5" },
    { KKEY_F6,              "f6" },
    { KKEY_F7,              "f7" },
    { KKEY_F8,              "f8" },
    { KKEY_F9,              "f9" },
    { KKEY_F10,             "f10" },
    { KKEY_F11,             "f11" },
    { KKEY_F12,             "f12" },
    { KKEY_KP_ENTER,        "keypadenter" },
    { KKEY_KP_MULTIPLY,     "keypad*" },
    { KKEY_KP_PLUS,         "keypad+" },
    { KKEY_NUMLOCKCLEAR,    "numlock" },
    { KKEY_KP_MINUS,        "keypad-" },
    { KKEY_KP_PERIOD,       "keypad." },
    { KKEY_KP_DIVIDE,       "keypad/" },
    { KKEY_KP_0,            "keypad0" },
    { KKEY_KP_1,            "keypad1" },
    { KKEY_KP_2,            "keypad2" },
    { KKEY_KP_3,            "keypad3" },
    { KKEY_KP_4,            "keypad4" },
    { KKEY_KP_5,            "keypad5" },
    { KKEY_KP_6,            "keypad6" },
    { KKEY_KP_7,            "keypad7" },
    { KKEY_KP_8,            "keypad8" },
    { KKEY_KP_9,            "keypad9" },
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
// bind
//

COMMAND(bind) {
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
// unbind
//

COMMAND(unbind) {
}

//
// listbinds
//

COMMAND(listbinds) {
    inputKey.ListBindings();
}

//
// kexInputKey::GetKeyCode
//

int kexInputKey::GetKeyCode(char *key) {
    keyinfo_t *pkey;
    int len;
    kexStr tmp(key);

    tmp.ToLower();
    len = tmp.Length();

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
        int keycode = pkey->code;
        keycode &= ~KKEY_SCANCODE_MASK;
        
        if(keycode == key) {
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
    newcmd = (cmdlist_t*)Mem_Malloc(sizeof(cmdlist_t), hb_static);
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
    kexStr tmp;

    common.Printf("\n");

    for(int i = 0; i < MAX_KEYS; i++) {
        keycmd = &keycmds[i];

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next) {
            char buff[32];

            GetName(buff, i);
            tmp = buff;
            tmp.ToLower();

            common.CPrintf(COLOR_GREEN, "%s : \"%s\"\n", tmp.c_str(), cmd->command);
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

    hash = kexStr::Hash(name);

    for(action = keyactions[hash]; action; action = action->next) {
        if(!strcmp(name, action->name))
            return action->keyid;
    }

    return -1;
}

//
// kexInputKey::ExecuteCommand
//

void kexInputKey::ExecuteCommand(int key, bool keyup) {
    keycmd_t *keycmd;
    cmdlist_t *cmd;
    
    key &= ~KKEY_SCANCODE_MASK;

    if(key >= MAX_KEYS) {
        return;
    }

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

    if(!command.Verify(name)) {
        return;
    }

    keyaction = (keyaction_t*)Mem_Malloc(sizeof(keyaction_t), hb_static);
    keyaction->keyid = id;
    strcpy(keyaction->name, name);

    command.Add(keyaction->name, FCmd_KeyAction);

    hash = kexStr::Hash(keyaction->name);
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
    kexStr tmp;

    for(int i = 0; i < MAX_KEYS; i++) {
        keycmd = &keycmds[i];

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next) {
            char buff[32];

            GetName(buff, i);
            tmp = buff;
            tmp.ToLower();

            fprintf(file, "bind %s \"%s\"\n", tmp.c_str(), cmd->command);
        }
    }
}

//
// kexInputKey::InitActions
//

void kexInputKey::InitActions(void) {
    kexStr actionDef;
    kexKeyMap *keys;
    kexArray<kexHashKey> *list;
    kexStr name;
    int value;

    if( !gameManager.GameDef()                                      ||
        !gameManager.GameDef()->GetString("actionDef", actionDef)   ||
        !(keys = defManager.FindDefEntry(actionDef))) {
            common.Warning("kexInputKey::InitActions: No input action definition found\n");
            return;
    }

    list = keys->GetHashList();

    for(int i = 0; i < keys->GetHashSize(); i++) {
        for(unsigned int j = 0; j < list[i].Length(); j++) {
            name = list[i][j].GetName();
            if(!keys->GetInt(name, value)) {
                common.Warning("kexInputKey::InitActions: entry %s contains non-integer value (%s)\n",
                    name.c_str(), list[i][j].GetString());
                continue;
            }

            AddAction((byte)value, (kexStr("+") + name).c_str());
            AddAction((byte)value, (kexStr("-") + name).c_str());
        }
    }
}

//
// kexInputKey::InitObject
//

void kexInputKey::InitObject(void) {
    scriptManager.Engine()->RegisterEnum("EnumInputKey");
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNKNOWN", KKEY_UNDEFINED);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSPACE", KKEY_BACKSPACE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_TAB", KKEY_TAB);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RETURN", KKEY_RETURN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAUSE", KKEY_PAUSE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ESCAPE", KKEY_ESCAPE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SPACE", KKEY_SPACE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EXCLAIM", KKEY_EXCLAIM);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTEDBL", KKEY_QUOTEDBL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HASH", KKEY_HASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOLLAR", KKEY_DOLLAR);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AMPERSAND", KKEY_AMPERSAND);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTE", KKEY_QUOTE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTPAREN", KKEY_LEFTPAREN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTPAREN", KKEY_RIGHTPAREN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ASTERISK", KKEY_ASTERISK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PLUS", KKEY_PLUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COMMA", KKEY_COMMA);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MINUS", KKEY_MINUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PERIOD", KKEY_PERIOD);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SLASH", KKEY_SLASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_0", KKEY_0);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_1", KKEY_1);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_2", KKEY_2);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_3", KKEY_3);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_4", KKEY_4);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_5", KKEY_5);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_6", KKEY_6);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_7", KKEY_7);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_8", KKEY_8);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_9", KKEY_9);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COLON", KKEY_COLON);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SEMICOLON", KKEY_SEMICOLON);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LESS", KKEY_LESS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EQUALS", KKEY_EQUALS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_GREATER", KKEY_GREATER);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUESTION", KKEY_QUESTION);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AT", KKEY_AT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTBRACKET", KKEY_LEFTBRACKET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSLASH", KKEY_BACKSLASH);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTBRACKET", KKEY_RIGHTBRACKET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CARET", KKEY_CARET);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNDERSCORE", KKEY_UNDERSCORE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKQUOTE", KKEY_BACKQUOTE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_a", KKEY_a);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_b", KKEY_b);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_c", KKEY_c);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_d", KKEY_d);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_e", KKEY_e);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_f", KKEY_f);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_g", KKEY_g);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_h", KKEY_h);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_i", KKEY_i);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_j", KKEY_j);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_k", KKEY_k);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_l", KKEY_l);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_m", KKEY_m);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_n", KKEY_n);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_o", KKEY_o);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_p", KKEY_p);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_q", KKEY_q);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_r", KKEY_r);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_s", KKEY_s);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_t", KKEY_t);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_u", KKEY_u);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_v", KKEY_v);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_w", KKEY_w);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_x", KKEY_x);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_y", KKEY_y);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_z", KKEY_z);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DELETE", KKEY_DELETE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PERIOD", KKEY_KP_PERIOD);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_DIVIDE", KKEY_KP_DIVIDE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MULTIPLY", KKEY_KP_MULTIPLY);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MINUS", KKEY_KP_MINUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PLUS", KKEY_KP_PLUS);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_ENTER", KKEY_KP_ENTER);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UP", KKEY_UP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOWN", KKEY_DOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHT", KKEY_RIGHT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFT", KKEY_LEFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_INSERT", KKEY_INSERT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HOME", KKEY_HOME);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_END", KKEY_END);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEUP", KKEY_PAGEUP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEDOWN", KKEY_PAGEDOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F1", KKEY_F1);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F2", KKEY_F2);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F3", KKEY_F3);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F4", KKEY_F4);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F5", KKEY_F5);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F6", KKEY_F6);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F7", KKEY_F7);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F8", KKEY_F8);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F9", KKEY_F9);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F10", KKEY_F10);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F11", KKEY_F11);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F12", KKEY_F12);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CAPSLOCK", KKEY_CAPSLOCK);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RSHIFT", KKEY_RSHIFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LSHIFT", KKEY_LSHIFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RCTRL", KKEY_RCTRL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LCTRL", KKEY_LCTRL);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RALT", KKEY_RALT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LALT", KKEY_LALT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_LEFT", KM_BUTTON_LEFT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_MIDDLE", KM_BUTTON_MIDDLE);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_RIGHT", KM_BUTTON_RIGHT);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELUP", KM_BUTTON_SCROLL_UP);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELDOWN", KM_BUTTON_SCROLL_DOWN);
}

//
// kexInputKey::Init
//

void kexInputKey::Init(void) {
    for(int c = 0; c < MAX_KEYS; c++) {
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
    
    for(int c = 'a'; c <= 'z'; c++) {
        keycode[1][c] = toupper(c);
    }

    common.Printf("Key System Initialized\n");
}
