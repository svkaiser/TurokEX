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
#include "kernel.h"
#include "zone.h"
#include "array.h"
#include "kstring.h"
#include "client.h"
#include "keyInput.h"
#include "render.h"
#include "console.h"

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
            (Sys_GetMilliseconds() / 1000.0f), s));*/
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
        timePressed = Sys_GetMilliseconds();
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
    if(bKeyHeld && ((Sys_GetMilliseconds() - timePressed) >= CON_STICKY_TIME))
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
    command.Add("clear", FCmd_ClearConsole);
}

//
// kexConsole::Draw
//

// TODO - TEMP/PLACEHOLDER
void kexConsole::Draw(void) {
    if(state == CON_STATE_UP)
        return;

    canvas_t canvas;

    float w = (float)video_width;
    float h = (float)video_height * 0.6875f;

    GL_SetState(GLSTATE_BLEND, 1);

    texture_t *white = Tex_CacheTextureFile("textures/white.tga", DGL_CLAMP, true);

    Canvas_SetDrawColor(&canvas, 4, 8, 16);
    Canvas_SetDrawAlpha(&canvas, 192);
    Canvas_DrawTile(&canvas, white, 0, 0, w, h);

    Draw_Text(0, h, 0xFFFFFFFF, 1.0f, "> ");

    if(bShowPrompt) {
        Draw_Text(0, h, 0xFFFFFFFF, 1.0f, "_");
    }

    if(strlen(typeStr) > 0) {
        Draw_Text(16, h, 0xFFFFFFFF, 1.0f, typeStr);
    }

    if(scrollBackLines > 0) {
        float scy = h-19;

        for(int i = scrollBackLines-(scrollBackPos)-1; i >= 0; i--) {
            if(scy < 0)
                break;

            Draw_Text(0, scy, lineColor[i], 1.0f, scrollBackStr[i]);
            scy -= 16;
        }
    }

    GL_SetState(GLSTATE_BLEND, 0);

    Canvas_SetDrawColor(&canvas, 0, 128, 255);
    Canvas_SetDrawAlpha(&canvas, 255);
    Canvas_DrawTile(&canvas, white, 0, h-17, w, 1);
    Canvas_DrawTile(&canvas, white, 0, h, w, 1);
}
