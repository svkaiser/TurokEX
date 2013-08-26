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
// DESCRIPTION: Console System
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "system.h"
#include "zone.h"
#include "array.h"
#include "kstring.h"
#include "client.h"
#include "keyInput.h"
#include "render.h"
#include "console.h"

kexCvar cvarDisplayConsole("con_alwaysShowConsole", CVF_BOOL|CVF_CONFIG, "0", "TODO");

kexConsole console;

//
// FCmd_ClearConsole
//

static void FCmd_ClearConsole(void) {
    console.Clear();
}

//
// kexConsole::kexConsole
//

kexConsole::kexConsole(void) {
    this->scrollBackPos     = 0;
    this->historyTop        = 0;
    this->historyCur        = -1;
    this->typeStrPos        = 0;
    this->scrollBackLines   = 0;
    this->bShiftDown        = false;
    this->bCtrlDown         = false;
    this->state             = CON_STATE_UP;
    this->blinkTime         = 0;
    this->bKeyHeld          = false;
    this->lastKeyPressed    = 0;
    this->timePressed       = 0;
    this->bShowPrompt       = true;
    this->outputLength      = 0;

    ClearOutput();
}

//
// kexConsole::~kexConsole
//

kexConsole::~kexConsole(void) {
    ClearOutput();
}

//
// kexConsole::ClearOutput
//

void kexConsole::ClearOutput(void) {
    for(int i = 0; i < CON_BUFFER_SIZE; i++) {
        memset(this->scrollBackStr[i], 0, CON_LINE_LENGTH);
        lineColor[i] = COLOR_WHITE;
    }

    scrollBackLines = 0;
}

//
// kexConsole::Clear
//

void kexConsole::Clear(void) {
    ClearOutput();
    ResetInputText();
}

//
// kexConsole::OutputTextLine
//

void kexConsole::OutputTextLine(rcolor color, const char *text) {
    if(scrollBackLines >= CON_BUFFER_SIZE) {
        for(unsigned int i = 0; i < CON_BUFFER_SIZE-1; i++) {
            memset(scrollBackStr[i], 0, CON_LINE_LENGTH);
            strcpy(scrollBackStr[i], scrollBackStr[i+1]);
            lineColor[i] = lineColor[i+1];
        }

        scrollBackLines = CON_BUFFER_SIZE-1;
    }

    unsigned int len = strlen(text);
    if(len >= CON_LINE_LENGTH)
        len = CON_LINE_LENGTH-1;

    strncpy(scrollBackStr[scrollBackLines], text, len);
    scrollBackStr[scrollBackLines][len] = '\0';
    lineColor[scrollBackLines] = color;

    scrollBackLines++;
}

//
// kexConsole::Print
//

void kexConsole::Print(rcolor color, const char *text) {
    unsigned int strLength = strlen(text);
    char *curText = (char*)text;
    char tmpChar[CON_LINE_LENGTH];

    if(cvarDeveloper.GetBool()) {
        /*memset(con_lastOutputBuffer, 0, 512);
        strcpy(con_lastOutputBuffer, kva("%f : %s",
            (sysMain.GetMS() / 1000.0f), s));*/
    }

    while(strLength > 0) {
        int lineLength = kexStr::IndexOf(curText, "\n");

        if(lineLength == -1)
            lineLength = strLength;

        strncpy(tmpChar, curText, lineLength);
        tmpChar[lineLength] = '\0';
        OutputTextLine(color, tmpChar);

        curText = (char*)&text[lineLength+1];
        strLength -= (lineLength+1);
    }
}

//
// kexConsole::LineScroll
//

void kexConsole::LineScroll(bool dir) {
    if(dir) {
        if(scrollBackPos < scrollBackLines)
            scrollBackPos++;
    }
    else {
        if(scrollBackPos > 0)
            scrollBackPos--;
    }
}

//
// kexConsole::BackSpace
//

void kexConsole::BackSpace(void) {
    if(strlen(typeStr) <= 0)
        return;

    char *trim = typeStr;
    int len = strlen(trim);

    typeStr[typeStrPos-1] = '\0';
    if(typeStrPos < len) {
        strncat(typeStr, &trim[typeStrPos], len-typeStrPos);
    }

    typeStrPos--;

    if(typeStrPos < 0)
        typeStrPos = 0;
}

//
// kexConsole::DeleteChar
//

void kexConsole::DeleteChar(void) {
    int tsLen = strlen(typeStr);

    if(tsLen > 0 && typeStrPos < tsLen) {
        char *trim = typeStr;
        int len = strlen(trim);

        typeStr[typeStrPos] = '\0';
        strncat(typeStr, &trim[typeStrPos+1], len-typeStrPos);
    }
}

//
// kexConsole::ShiftHeld
//

bool kexConsole::ShiftHeld(int c) const {
    return (c == SDLK_RSHIFT || c == SDLK_LSHIFT);
}

//
// kexConsole::MoveTypePos
//

void kexConsole::MoveTypePos(bool dir) {
    if(dir) {
        int len = strlen(typeStr);
        typeStrPos++;
        if(typeStrPos > len)
            typeStrPos = len;
    }
    else {
        typeStrPos--;
        if(typeStrPos < 0)
            typeStrPos = 0;
    }
}

//
// kexConsole::CheckShift
//

void kexConsole::CheckShift(const event_t *ev) {

    if(!ShiftHeld(ev->data1))
        return;

    switch(ev->type) {
    case ev_keydown:
        bShiftDown = true;
        break;
    case ev_keyup:
        bShiftDown = false;
        break;
    default:
        break;
    }
}

//
// kexConsole::CheckStickyKeys
//

void kexConsole::CheckStickyKeys(const event_t *ev) {
    if(ShiftHeld(ev->data1) || ev->data1 == SDLK_RETURN ||
        ev->data1 == SDLK_TAB) {
            return;
    }

    lastKeyPressed = ev->data1;

    switch(ev->type) {
    case ev_keydown:
        bKeyHeld = true;
        timePressed = sysMain.GetMS();
        break;
    case ev_keyup:
        bKeyHeld = false;
        timePressed = 0;
        break;
    default:
        break;
    }
}

//
// kexConsole::ParseKey
//

void kexConsole::ParseKey(int c) {
    switch(c) {
    case SDLK_BACKSPACE:
        BackSpace();
        return;
    case SDLK_DELETE:
        DeleteChar();
        return;
    case SDLK_LEFT:
        MoveTypePos(0);
        return;
    case SDLK_RIGHT:
        MoveTypePos(1);
        return;
    case SDLK_PAGEUP:
        LineScroll(1);
        return;
    case SDLK_PAGEDOWN:
        LineScroll(0);
        return;
    }

    if(c >= 8 && c < 256) {
        if(typeStrPos >= CON_INPUT_LENGTH)
            return;

        typeStr[typeStrPos++] = inputKey.GetAsciiKey((char)c, bShiftDown);
        typeStr[typeStrPos] = '\0';
    }
}

//
// kexConsole::StickyKeyTick
//

void kexConsole::StickyKeyTick(void) {
    if(bKeyHeld && ((sysMain.GetMS() - timePressed) >= CON_STICKY_TIME))
        ParseKey(lastKeyPressed);
}

//
// kexConsole::UpdateBlink
//

void kexConsole::UpdateBlink(void) {
    if(blinkTime >= client.GetTime())
        return;

    bShowPrompt = !bShowPrompt;
    blinkTime = client.GetTime() + CON_BLINK_TIME;
}

//
// kexConsole::ParseInput
//

void kexConsole::ParseInput(void) {
    if(typeStrPos <= 0 || strlen(typeStr) <= 0)
        return;

    OutputTextLine(RGBA(192, 192, 192, 255), typeStr);
    command.Execute(typeStr);
    ResetInputText();

    historyCur = (historyTop - 1);
}

//
// kexConsole::ProcessInput
//

bool kexConsole::ProcessInput(const event_t *ev) {
    if(ev->type == ev_mousedown || ev->type == ev_mouseup ||
        ev->type == ev_mouse) {
            return false;
    }

    if(ev->type == ev_mousewheel && state == CON_STATE_DOWN) {
        switch(ev->data1) {
        case SDL_BUTTON_WHEELUP:
            LineScroll(1);
            break;
        case SDL_BUTTON_WHEELDOWN:
            LineScroll(0);
            break;
        }

        return true;
    }

    CheckShift(ev);
    CheckStickyKeys(ev);

    int c = ev->data1;

    switch(state) {
    case CON_STATE_DOWN:
        if(ev->type == ev_keydown) {
            switch(c) {
            case SDLK_BACKQUOTE:
                state = CON_STATE_UP;
                return true;
            case SDLK_RETURN:
                ParseInput();
                return true;
            case SDLK_UP:
                return true;
            case SDLK_DOWN:
                return true;
            case SDLK_TAB:
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                ParseKey(SDLK_SPACE);
                return true;
            default:
                ParseKey(c);
                return true;
            }

            return false;
        }
        break;
    case CON_STATE_UP:
        if(ev->type == ev_keydown) {
            switch(c) {
            case SDLK_BACKQUOTE:
                state = CON_STATE_DOWN;
                return true;
            default:
                break;
            }

            return false;
        }
        break;
    default:
        return false;
    }

    return false;
}

//
// kexConsole::Tick
//

void kexConsole::Tick(void) {
    if(state == CON_STATE_UP)
        return;

    StickyKeyTick();
    UpdateBlink();
}

//
// kexConsole::Init
//

void kexConsole::Init(void) {
    textFont.texture = "fonts/confont.tga";
    textFont.width = 256;
    textFont.height = 256;

    Font_MapChar(&textFont, 0, 0, 1, 13, 16);
    Font_MapChar(&textFont, 1, 14, 1, 13, 16);
    Font_MapChar(&textFont, 2, 28, 1, 13, 16);
    Font_MapChar(&textFont, 3, 42, 1, 13, 16);
    Font_MapChar(&textFont, 4, 56, 1, 13, 16);
    Font_MapChar(&textFont, 5, 70, 1, 13, 16);
    Font_MapChar(&textFont, 6, 84, 1, 13, 16);
    Font_MapChar(&textFont, 7, 98, 1, 13, 16);
    Font_MapChar(&textFont, 8, 112, 1, 13, 16);
    Font_MapChar(&textFont, 9, 126, 1, 13, 16);
    Font_MapChar(&textFont, 10, 140, 1, 13, 16);
    Font_MapChar(&textFont, 11, 154, 1, 13, 16);
    Font_MapChar(&textFont, 12, 168, 1, 13, 16);
    Font_MapChar(&textFont, 13, 182, 1, 13, 16);
    Font_MapChar(&textFont, 14, 196, 1, 13, 16);
    Font_MapChar(&textFont, 15, 210, 1, 13, 16);
    Font_MapChar(&textFont, 16, 224, 1, 13, 16);
    Font_MapChar(&textFont, 17, 238, 1, 13, 16);
    Font_MapChar(&textFont, 18, 0, 18, 13, 16);
    Font_MapChar(&textFont, 19, 14, 18, 13, 16);
    Font_MapChar(&textFont, 20, 28, 18, 13, 16);
    Font_MapChar(&textFont, 21, 42, 18, 13, 16);
    Font_MapChar(&textFont, 22, 56, 18, 13, 16);
    Font_MapChar(&textFont, 23, 70, 18, 13, 16);
    Font_MapChar(&textFont, 24, 84, 18, 13, 16);
    Font_MapChar(&textFont, 25, 98, 18, 13, 16);
    Font_MapChar(&textFont, 26, 112, 18, 13, 16);
    Font_MapChar(&textFont, 27, 126, 18, 13, 16);
    Font_MapChar(&textFont, 28, 140, 18, 13, 16);
    Font_MapChar(&textFont, 29, 154, 18, 13, 16);
    Font_MapChar(&textFont, 30, 168, 18, 13, 16);
    Font_MapChar(&textFont, 31, 182, 18, 13, 16);
    Font_MapChar(&textFont, 32, 196, 18, 5, 16);
    Font_MapChar(&textFont, 33, 202, 18, 5, 16);
    Font_MapChar(&textFont, 34, 208, 18, 5, 16);
    Font_MapChar(&textFont, 35, 214, 18, 10, 16);
    Font_MapChar(&textFont, 36, 225, 18, 8, 16);
    Font_MapChar(&textFont, 37, 234, 18, 13, 16);
    Font_MapChar(&textFont, 38, 0, 35, 9, 16);
    Font_MapChar(&textFont, 39, 10, 35, 3, 16);
    Font_MapChar(&textFont, 40, 14, 35, 6, 16);
    Font_MapChar(&textFont, 41, 21, 35, 6, 16);
    Font_MapChar(&textFont, 42, 28, 35, 9, 16);
    Font_MapChar(&textFont, 43, 38, 35, 9, 16);
    Font_MapChar(&textFont, 44, 48, 35, 5, 16);
    Font_MapChar(&textFont, 45, 54, 35, 7, 16);
    Font_MapChar(&textFont, 46, 62, 35, 5, 16);
    Font_MapChar(&textFont, 47, 68, 35, 6, 16);
    Font_MapChar(&textFont, 48, 75, 35, 8, 16);
    Font_MapChar(&textFont, 49, 84, 35, 8, 16);
    Font_MapChar(&textFont, 50, 93, 35, 8, 16);
    Font_MapChar(&textFont, 51, 102, 35, 8, 16);
    Font_MapChar(&textFont, 52, 111, 35, 8, 16);
    Font_MapChar(&textFont, 53, 120, 35, 8, 16);
    Font_MapChar(&textFont, 54, 129, 35, 8, 16);
    Font_MapChar(&textFont, 55, 138, 35, 8, 16);
    Font_MapChar(&textFont, 56, 147, 35, 8, 16);
    Font_MapChar(&textFont, 57, 156, 35, 8, 16);
    Font_MapChar(&textFont, 58, 165, 35, 6, 16);
    Font_MapChar(&textFont, 59, 172, 35, 6, 16);
    Font_MapChar(&textFont, 60, 179, 35, 9, 16);
    Font_MapChar(&textFont, 61, 189, 35, 9, 16);
    Font_MapChar(&textFont, 62, 199, 35, 9, 16);
    Font_MapChar(&textFont, 63, 209, 35, 7, 16);
    Font_MapChar(&textFont, 64, 217, 35, 13, 16);
    Font_MapChar(&textFont, 65, 231, 35, 9, 16);
    Font_MapChar(&textFont, 66, 241, 35, 8, 16);
    Font_MapChar(&textFont, 67, 0, 52, 9, 16);
    Font_MapChar(&textFont, 68, 10, 52, 9, 16);
    Font_MapChar(&textFont, 69, 20, 52, 8, 16);
    Font_MapChar(&textFont, 70, 29, 52, 8, 16);
    Font_MapChar(&textFont, 71, 38, 52, 9, 16);
    Font_MapChar(&textFont, 72, 48, 52, 9, 16);
    Font_MapChar(&textFont, 73, 58, 52, 5, 16);
    Font_MapChar(&textFont, 74, 64, 52, 6, 16);
    Font_MapChar(&textFont, 75, 71, 52, 8, 16);
    Font_MapChar(&textFont, 76, 80, 52, 7, 16);
    Font_MapChar(&textFont, 77, 88, 52, 11, 16);
    Font_MapChar(&textFont, 78, 100, 52, 9, 16);
    Font_MapChar(&textFont, 79, 110, 52, 10, 16);
    Font_MapChar(&textFont, 80, 121, 52, 8, 16);
    Font_MapChar(&textFont, 81, 130, 52, 10, 16);
    Font_MapChar(&textFont, 82, 141, 52, 8, 16);
    Font_MapChar(&textFont, 83, 150, 52, 9, 16);
    Font_MapChar(&textFont, 84, 160, 52, 9, 16);
    Font_MapChar(&textFont, 85, 170, 52, 9, 16);
    Font_MapChar(&textFont, 86, 180, 52, 9, 16);
    Font_MapChar(&textFont, 87, 190, 52, 13, 16);
    Font_MapChar(&textFont, 88, 204, 52, 9, 16);
    Font_MapChar(&textFont, 89, 214, 52, 9, 16);
    Font_MapChar(&textFont, 90, 224, 52, 9, 16);
    Font_MapChar(&textFont, 91, 234, 52, 6, 16);
    Font_MapChar(&textFont, 92, 241, 52, 6, 16);
    Font_MapChar(&textFont, 93, 248, 52, 6, 16);
    Font_MapChar(&textFont, 94, 0, 69, 11, 16);
    Font_MapChar(&textFont, 95, 12, 69, 8, 16);
    Font_MapChar(&textFont, 96, 21, 69, 8, 16);
    Font_MapChar(&textFont, 97, 30, 69, 8, 16);
    Font_MapChar(&textFont, 98, 39, 69, 8, 16);
    Font_MapChar(&textFont, 99, 48, 69, 8, 16);
    Font_MapChar(&textFont, 100, 57, 69, 8, 16);
    Font_MapChar(&textFont, 101, 66, 69, 8, 16);
    Font_MapChar(&textFont, 102, 75, 69, 5, 16);
    Font_MapChar(&textFont, 103, 81, 69, 8, 16);
    Font_MapChar(&textFont, 104, 90, 69, 8, 16);
    Font_MapChar(&textFont, 105, 99, 69, 3, 16);
    Font_MapChar(&textFont, 106, 103, 69, 4, 16);
    Font_MapChar(&textFont, 107, 108, 69, 7, 16);
    Font_MapChar(&textFont, 108, 116, 69, 3, 16);
    Font_MapChar(&textFont, 109, 120, 69, 11, 16);
    Font_MapChar(&textFont, 110, 132, 69, 8, 16);
    Font_MapChar(&textFont, 111, 141, 69, 8, 16);
    Font_MapChar(&textFont, 112, 150, 69, 8, 16);
    Font_MapChar(&textFont, 113, 159, 69, 8, 16);
    Font_MapChar(&textFont, 114, 168, 69, 5, 16);
    Font_MapChar(&textFont, 115, 174, 69, 7, 16);
    Font_MapChar(&textFont, 116, 182, 69, 6, 16);
    Font_MapChar(&textFont, 117, 189, 69, 8, 16);
    Font_MapChar(&textFont, 118, 198, 69, 8, 16);
    Font_MapChar(&textFont, 119, 207, 69, 11, 16);
    Font_MapChar(&textFont, 120, 219, 69, 7, 16);
    Font_MapChar(&textFont, 121, 227, 69, 8, 16);
    Font_MapChar(&textFont, 122, 236, 69, 7, 16);
    Font_MapChar(&textFont, 123, 244, 69, 8, 16);
    Font_MapChar(&textFont, 124, 0, 86, 7, 16);
    Font_MapChar(&textFont, 125, 8, 86, 8, 16);
    Font_MapChar(&textFont, 126, 17, 86, 11, 16);
    Font_MapChar(&textFont, 127, 29, 86, 13, 16);
    Font_MapChar(&textFont, 128, 43, 86, 13, 16);
    Font_MapChar(&textFont, 129, 57, 86, 13, 16);
    Font_MapChar(&textFont, 130, 71, 86, 13, 16);
    Font_MapChar(&textFont, 131, 85, 86, 13, 16);
    Font_MapChar(&textFont, 132, 99, 86, 13, 16);
    Font_MapChar(&textFont, 133, 113, 86, 13, 16);
    Font_MapChar(&textFont, 134, 127, 86, 13, 16);
    Font_MapChar(&textFont, 135, 141, 86, 13, 16);
    Font_MapChar(&textFont, 136, 155, 86, 13, 16);
    Font_MapChar(&textFont, 137, 169, 86, 13, 16);
    Font_MapChar(&textFont, 138, 183, 86, 13, 16);
    Font_MapChar(&textFont, 139, 197, 86, 13, 16);
    Font_MapChar(&textFont, 140, 211, 86, 13, 16);
    Font_MapChar(&textFont, 141, 225, 86, 13, 16);
    Font_MapChar(&textFont, 142, 239, 86, 13, 16);
    Font_MapChar(&textFont, 143, 0, 103, 13, 16);
    Font_MapChar(&textFont, 144, 14, 103, 13, 16);
    Font_MapChar(&textFont, 145, 28, 103, 13, 16);
    Font_MapChar(&textFont, 146, 42, 103, 13, 16);
    Font_MapChar(&textFont, 147, 56, 103, 13, 16);
    Font_MapChar(&textFont, 148, 70, 103, 13, 16);
    Font_MapChar(&textFont, 149, 84, 103, 13, 16);
    Font_MapChar(&textFont, 150, 98, 103, 13, 16);
    Font_MapChar(&textFont, 151, 112, 103, 13, 16);
    Font_MapChar(&textFont, 152, 126, 103, 13, 16);
    Font_MapChar(&textFont, 153, 140, 103, 13, 16);
    Font_MapChar(&textFont, 154, 154, 103, 13, 16);
    Font_MapChar(&textFont, 155, 168, 103, 13, 16);
    Font_MapChar(&textFont, 156, 182, 103, 13, 16);
    Font_MapChar(&textFont, 157, 196, 103, 13, 16);
    Font_MapChar(&textFont, 158, 210, 103, 13, 16);
    Font_MapChar(&textFont, 159, 224, 103, 13, 16);
    Font_MapChar(&textFont, 160, 238, 103, 5, 16);
    Font_MapChar(&textFont, 161, 244, 103, 5, 16);
    Font_MapChar(&textFont, 162, 0, 120, 8, 16);
    Font_MapChar(&textFont, 163, 9, 120, 8, 16);
    Font_MapChar(&textFont, 164, 18, 120, 8, 16);
    Font_MapChar(&textFont, 165, 27, 120, 8, 16);
    Font_MapChar(&textFont, 166, 36, 120, 7, 16);
    Font_MapChar(&textFont, 167, 44, 120, 8, 16);
    Font_MapChar(&textFont, 168, 53, 120, 8, 16);
    Font_MapChar(&textFont, 169, 62, 120, 13, 16);
    Font_MapChar(&textFont, 170, 76, 120, 7, 16);
    Font_MapChar(&textFont, 171, 84, 120, 8, 16);
    Font_MapChar(&textFont, 172, 93, 120, 9, 16);
    Font_MapChar(&textFont, 173, 103, 120, 7, 16);
    Font_MapChar(&textFont, 174, 111, 120, 13, 16);
    Font_MapChar(&textFont, 175, 125, 120, 8, 16);
    Font_MapChar(&textFont, 176, 134, 120, 7, 16);
    Font_MapChar(&textFont, 177, 142, 120, 9, 16);
    Font_MapChar(&textFont, 178, 152, 120, 7, 16);
    Font_MapChar(&textFont, 179, 160, 120, 7, 16);
    Font_MapChar(&textFont, 180, 168, 120, 8, 16);
    Font_MapChar(&textFont, 181, 177, 120, 8, 16);
    Font_MapChar(&textFont, 182, 186, 120, 8, 16);
    Font_MapChar(&textFont, 183, 195, 120, 5, 16);
    Font_MapChar(&textFont, 184, 201, 120, 8, 16);
    Font_MapChar(&textFont, 185, 210, 120, 7, 16);
    Font_MapChar(&textFont, 186, 218, 120, 7, 16);
    Font_MapChar(&textFont, 187, 226, 120, 8, 16);
    Font_MapChar(&textFont, 188, 235, 120, 13, 16);
    Font_MapChar(&textFont, 189, 0, 137, 13, 16);
    Font_MapChar(&textFont, 190, 14, 137, 13, 16);
    Font_MapChar(&textFont, 191, 28, 137, 7, 16);
    Font_MapChar(&textFont, 192, 36, 137, 9, 16);
    Font_MapChar(&textFont, 193, 46, 137, 9, 16);
    Font_MapChar(&textFont, 194, 56, 137, 9, 16);
    Font_MapChar(&textFont, 195, 66, 137, 9, 16);
    Font_MapChar(&textFont, 196, 76, 137, 9, 16);
    Font_MapChar(&textFont, 197, 86, 137, 9, 16);
    Font_MapChar(&textFont, 198, 96, 137, 12, 16);
    Font_MapChar(&textFont, 199, 109, 137, 9, 16);
    Font_MapChar(&textFont, 200, 119, 137, 8, 16);
    Font_MapChar(&textFont, 201, 128, 137, 8, 16);
    Font_MapChar(&textFont, 202, 137, 137, 8, 16);
    Font_MapChar(&textFont, 203, 146, 137, 8, 16);
    Font_MapChar(&textFont, 204, 155, 137, 5, 16);
    Font_MapChar(&textFont, 205, 161, 137, 5, 16);
    Font_MapChar(&textFont, 206, 167, 137, 5, 16);
    Font_MapChar(&textFont, 207, 173, 137, 5, 16);
    Font_MapChar(&textFont, 208, 179, 137, 9, 16);
    Font_MapChar(&textFont, 209, 189, 137, 9, 16);
    Font_MapChar(&textFont, 210, 199, 137, 10, 16);
    Font_MapChar(&textFont, 211, 210, 137, 10, 16);
    Font_MapChar(&textFont, 212, 221, 137, 10, 16);
    Font_MapChar(&textFont, 213, 232, 137, 10, 16);
    Font_MapChar(&textFont, 214, 243, 137, 10, 16);
    Font_MapChar(&textFont, 215, 0, 154, 11, 16);
    Font_MapChar(&textFont, 216, 12, 154, 10, 16);
    Font_MapChar(&textFont, 217, 23, 154, 9, 16);
    Font_MapChar(&textFont, 218, 33, 154, 9, 16);
    Font_MapChar(&textFont, 219, 43, 154, 9, 16);
    Font_MapChar(&textFont, 220, 53, 154, 9, 16);
    Font_MapChar(&textFont, 221, 63, 154, 9, 16);
    Font_MapChar(&textFont, 222, 73, 154, 8, 16);
    Font_MapChar(&textFont, 223, 82, 154, 8, 16);
    Font_MapChar(&textFont, 224, 91, 154, 8, 16);
    Font_MapChar(&textFont, 225, 100, 154, 8, 16);
    Font_MapChar(&textFont, 226, 109, 154, 8, 16);
    Font_MapChar(&textFont, 227, 118, 154, 8, 16);
    Font_MapChar(&textFont, 228, 127, 154, 8, 16);
    Font_MapChar(&textFont, 229, 136, 154, 8, 16);
    Font_MapChar(&textFont, 230, 145, 154, 11, 16);
    Font_MapChar(&textFont, 231, 157, 154, 8, 16);
    Font_MapChar(&textFont, 232, 166, 154, 8, 16);
    Font_MapChar(&textFont, 233, 175, 154, 8, 16);
    Font_MapChar(&textFont, 234, 184, 154, 8, 16);
    Font_MapChar(&textFont, 235, 193, 154, 8, 16);
    Font_MapChar(&textFont, 236, 202, 154, 3, 16);
    Font_MapChar(&textFont, 237, 206, 154, 3, 16);
    Font_MapChar(&textFont, 238, 210, 154, 3, 16);
    Font_MapChar(&textFont, 239, 214, 154, 3, 16);
    Font_MapChar(&textFont, 240, 218, 154, 8, 16);
    Font_MapChar(&textFont, 241, 227, 154, 8, 16);
    Font_MapChar(&textFont, 242, 236, 154, 8, 16);
    Font_MapChar(&textFont, 243, 245, 154, 8, 16);
    Font_MapChar(&textFont, 244, 0, 171, 8, 16);
    Font_MapChar(&textFont, 245, 9, 171, 8, 16);
    Font_MapChar(&textFont, 246, 18, 171, 8, 16);
    Font_MapChar(&textFont, 247, 27, 171, 9, 16);
    Font_MapChar(&textFont, 248, 37, 171, 8, 16);
    Font_MapChar(&textFont, 249, 46, 171, 8, 16);
    Font_MapChar(&textFont, 250, 55, 171, 8, 16);
    Font_MapChar(&textFont, 251, 64, 171, 8, 16);
    Font_MapChar(&textFont, 252, 73, 171, 8, 16);
    Font_MapChar(&textFont, 253, 82, 171, 8, 16);
    Font_MapChar(&textFont, 254, 91, 171, 8, 16);
    Font_MapChar(&textFont, 255, 100, 171, 8, 16);

    command.Add("clear", FCmd_ClearConsole);
    common.Printf("Console Initialized\n");
}

//
// kexConsole::Draw
//

// TODO - TEMP/PLACEHOLDER
void kexConsole::Draw(void) {
    if(state == CON_STATE_UP && !cvarDisplayConsole.GetBool())
        return;

    bool bOverlay = (state == CON_STATE_UP && cvarDisplayConsole.GetBool());
    canvas_t canvas;

    float w = (float)sysMain.VideoWidth();
    float h = (float)sysMain.VideoHeight() * 0.6875f;

    GL_SetState(GLSTATE_BLEND, 1);

    texture_t *white = Tex_CacheTextureFile("textures/white.tga", DGL_CLAMP, true);

    GL_BindTextureName(textFont.texture);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    Canvas_SetFont(&canvas, &textFont);
    Canvas_SetDrawAlpha(&canvas, 128);

    if(!bOverlay) {
        Canvas_SetDrawColor(&canvas, 4, 8, 16);
        Canvas_SetDrawAlpha(&canvas, 192);
        Canvas_DrawTile(&canvas, white, 0, 0, w, h);

        Canvas_SetDrawColor(&canvas, 255, 255, 255);
        Canvas_SetDrawAlpha(&canvas, 255);
        Canvas_DrawString(&canvas, "> ", 0, h-15, false);

        if(bShowPrompt) {
            Canvas_DrawString(&canvas, "_", 16 +
                Font_StringWidth(&textFont, typeStr, 1.0f, typeStrPos), h-15, false);
        }

        if(strlen(typeStr) > 0) {
            Canvas_DrawString(&canvas, typeStr, 16, h-15, false);
        }
    }

    if(scrollBackLines > 0) {
        float scy = h-34;

        for(int i = scrollBackLines-(scrollBackPos)-1; i >= 0; i--) {
            if(scy < 0)
                break;

            Canvas_SetDrawColor(&canvas,
                (lineColor[i] >> 0 ) & 0xff,
                (lineColor[i] >> 8 ) & 0xff,
                (lineColor[i] >> 16) & 0xff);
            Canvas_DrawString(&canvas, scrollBackStr[i], 0, scy, false);
            scy -= 16;
        }
    }

    GL_SetState(GLSTATE_BLEND, 0);

    if(!bOverlay) {
        Canvas_SetDrawColor(&canvas, 0, 128, 255);
        Canvas_SetDrawAlpha(&canvas, 255);
        Canvas_DrawTile(&canvas, white, 0, h-17, w, 1);
        Canvas_DrawTile(&canvas, white, 0, h, w, 1);
    }
}
