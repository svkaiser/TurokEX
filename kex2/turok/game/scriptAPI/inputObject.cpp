// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: Input Namespace Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "scriptAPI/scriptSystem.h"

//
// kexScriptObjInput::Init
//

void kexScriptObjInput::Init(void) {
    scriptManager.Engine()->SetDefaultNamespace("Input");

    scriptManager.Engine()->RegisterGlobalFunction(
        "void AddInputKey(const int, const kStr &in)",
        asFUNCTION(AddInputKey),
        asCALL_CDECL);

    scriptManager.Engine()->RegisterEnum("EnumInputKey");
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNKNOWN", SDLK_UNKNOWN);
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSPACE", SDLK_BACKSPACE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_TAB", SDLK_TAB),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CLEAR", SDLK_CLEAR),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RETURN", SDLK_RETURN),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAUSE", SDLK_PAUSE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ESCAPE", SDLK_ESCAPE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SPACE", SDLK_SPACE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EXCLAIM", SDLK_EXCLAIM),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTEDBL", SDLK_QUOTEDBL),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HASH", SDLK_HASH),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOLLAR", SDLK_DOLLAR),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AMPERSAND", SDLK_AMPERSAND),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUOTE", SDLK_QUOTE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTPAREN", SDLK_LEFTPAREN),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTPAREN", SDLK_RIGHTPAREN),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_ASTERISK", SDLK_ASTERISK),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PLUS", SDLK_PLUS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COMMA", SDLK_COMMA),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MINUS", SDLK_MINUS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PERIOD", SDLK_PERIOD),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SLASH", SDLK_SLASH),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_0", SDLK_0),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_1", SDLK_1),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_2", SDLK_2),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_3", SDLK_3),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_4", SDLK_4),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_5", SDLK_5),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_6", SDLK_6),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_7", SDLK_7),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_8", SDLK_8),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_9", SDLK_9),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COLON", SDLK_COLON),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SEMICOLON", SDLK_SEMICOLON),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LESS", SDLK_LESS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EQUALS", SDLK_EQUALS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_GREATER", SDLK_GREATER),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_QUESTION", SDLK_QUESTION),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_AT", SDLK_AT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFTBRACKET", SDLK_LEFTBRACKET),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKSLASH", SDLK_BACKSLASH),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHTBRACKET", SDLK_RIGHTBRACKET),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CARET", SDLK_CARET),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNDERSCORE", SDLK_UNDERSCORE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BACKQUOTE", SDLK_BACKQUOTE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_a", SDLK_a),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_b", SDLK_b),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_c", SDLK_c),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_d", SDLK_d),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_e", SDLK_e),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_f", SDLK_f),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_g", SDLK_g),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_h", SDLK_h),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_i", SDLK_i),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_j", SDLK_j),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_k", SDLK_k),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_l", SDLK_l),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_m", SDLK_m),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_n", SDLK_n),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_o", SDLK_o),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_p", SDLK_p),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_q", SDLK_q),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_r", SDLK_r),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_s", SDLK_s),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_t", SDLK_t),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_u", SDLK_u),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_v", SDLK_v),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_w", SDLK_w),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_x", SDLK_x),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_y", SDLK_y),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_z", SDLK_z),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DELETE", SDLK_DELETE),
    /*scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP0", SDLK_KP0),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP1", SDLK_KP1),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP2", SDLK_KP2),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP3", SDLK_KP3),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP4", SDLK_KP4),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP5", SDLK_KP5),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP6", SDLK_KP6),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP7", SDLK_KP7),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP8", SDLK_KP8),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP9", SDLK_KP9),*/
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PERIOD", SDLK_KP_PERIOD),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_DIVIDE", SDLK_KP_DIVIDE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MULTIPLY", SDLK_KP_MULTIPLY),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_MINUS", SDLK_KP_MINUS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_PLUS", SDLK_KP_PLUS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_ENTER", SDLK_KP_ENTER),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_KP_EQUALS", SDLK_KP_EQUALS),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UP", SDLK_UP),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_DOWN", SDLK_DOWN),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RIGHT", SDLK_RIGHT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LEFT", SDLK_LEFT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_INSERT", SDLK_INSERT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HOME", SDLK_HOME),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_END", SDLK_END),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEUP", SDLK_PAGEUP),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PAGEDOWN", SDLK_PAGEDOWN),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F1", SDLK_F1),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F2", SDLK_F2),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F3", SDLK_F3),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F4", SDLK_F4),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F5", SDLK_F5),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F6", SDLK_F6),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F7", SDLK_F7),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F8", SDLK_F8),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F9", SDLK_F9),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F10", SDLK_F10),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F11", SDLK_F11),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F12", SDLK_F12),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F13", SDLK_F13),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F14", SDLK_F14),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_F15", SDLK_F15),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_NUMLOCK", SDLK_NUMLOCK),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_CAPSLOCK", SDLK_CAPSLOCK),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SCROLLOCK", SDLK_SCROLLOCK),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RSHIFT", SDLK_RSHIFT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LSHIFT", SDLK_LSHIFT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RCTRL", SDLK_RCTRL),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LCTRL", SDLK_LCTRL),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RALT", SDLK_RALT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LALT", SDLK_LALT),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RMETA", SDLK_RMETA),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LMETA", SDLK_LMETA),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_LSUPER", SDLK_LSUPER),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_RSUPER", SDLK_RSUPER),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MODE", SDLK_MODE),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_COMPOSE", SDLK_COMPOSE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_HELP", SDLK_HELP),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_PRINT", SDLK_PRINT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_SYSREQ", SDLK_SYSREQ),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_BREAK", SDLK_BREAK),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_MENU", SDLK_MENU),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_POWER", SDLK_POWER),
    //scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_EURO", SDLK_EURO),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","K_UNDO", SDLK_UNDO),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_LEFT",SDL_BUTTON_LEFT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_MIDDLE",SDL_BUTTON_MIDDLE),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_RIGHT",SDL_BUTTON_RIGHT),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELUP",SDL_BUTTON_WHEELUP),
    scriptManager.Engine()->RegisterEnumValue("EnumInputKey","BUTTON_WHEELDOWN",SDL_BUTTON_WHEELDOWN),

    scriptManager.Engine()->SetDefaultNamespace("");
}

//
// kexScriptObjInput::AddInputKey
//

void kexScriptObjInput::AddInputKey(const int id, const char *key) {
    inputKey.AddAction((byte)id, key);
}
