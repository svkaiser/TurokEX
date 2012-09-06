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

#include "SDL.h"

#include "common.h"
#include "kernel.h"
#include "gl.h"
#include "zone.h"
#include "client.h"

static kbool shiftdown = false;

#define CONSOLE_PROMPTCHAR      '>'
#define MAX_CONSOLE_LINES       256//must be power of 2
#define CONSOLETEXT_MASK        (MAX_CONSOLE_LINES-1)
#define CMD_HISTORY_SIZE        64
#define CONSOLE_Y               ((float)(video_width / 2))

typedef struct
{
    int     len;
    rcolor  color;
    char    line[1];
} conline_t;

enum
{
    CST_UP,
    CST_RAISE,
    CST_LOWER,
    CST_DOWN
};

#define CONCLEARINPUT() (memset(console_inputbuffer+1, 0, MAX_CONSOLE_INPUT_LEN-1))

#define CON_BUFFERSIZE 100
#define MAX_CONSOLE_INPUT_LEN 80

static conline_t    **console_buffer;
static int          console_head;
static int          console_lineoffset;
static int          console_minline;
static kbool        console_enabled = false;
static int          console_pos = 0;//bottom of console, in pixels
static char         console_linebuffer[CON_BUFFERSIZE];
static int          console_linelength;
static kbool        console_state = CST_UP;
static int          console_prevcmds[CMD_HISTORY_SIZE];
static int          console_cmdhead;
static int          console_nextcmd;

static kbool        keyheld = false;
static kbool        lastevent = 0;
static int          lastkey = 0;
static int          timepressed = 0;

char    console_inputbuffer[MAX_CONSOLE_INPUT_LEN];
int     console_inputlength;
kbool   console_initialized = false;

//
// Con_AddLine
//

void Con_AddLine(char *line, int len, rcolor color)
{
    conline_t   *cline;
    int         i;
    kbool       recursed = false;
    
    if(!console_linebuffer)
    {
        //not initialised yet
        return;
    }

    if(recursed)
    {
        //later call to Z_Malloc can fail and call I_Error/I_Printf...
        return;
    }
    
    recursed = true;
    
    if(!line)
        return;

    if(len == -1)
    {
        len = strlen(line);
    }

    cline = (conline_t*)Z_Malloc(sizeof(conline_t)+len, PU_STATIC, NULL);
    cline->len = len;

    if(len)
    {
        memcpy(cline->line, line, len);
    }

    cline->line[len] = 0;
    console_head = (console_lineoffset + CONSOLETEXT_MASK) & CONSOLETEXT_MASK;
    console_minline = console_head;
    console_lineoffset = console_head;
    
    console_buffer[console_head] = cline;
    console_buffer[console_head]->color = color;
    
    i = (console_head + CONSOLETEXT_MASK) & CONSOLETEXT_MASK;
    if(console_buffer[i])
    {
        Z_Free(console_buffer[i]);
        console_buffer[i] = NULL;
    }
    
    recursed = false;
}

//
// Con_AddText
//

void Con_AddText(char *text, rcolor color)
{
    char *src;
    char c;
    
    if(!console_linebuffer)
        return;
    
    src = text;
    c = *(src++);
    while(c)
    {
        if((c == '\n') || (console_linelength >= CON_BUFFERSIZE))
        {
            Con_AddLine(console_linebuffer, console_linelength, color);
            console_linelength = 0;
        }
        if(c != '\n')
            console_linebuffer[console_linelength++] = c;
        
        c = *(src++);
    }
}

//
// Con_Printf
//

void Con_Printf(rcolor clr, const char *s, ...)
{
    static char msg[1024];
    va_list	va;
    
    va_start(va, s);
    vsprintf(msg, s, va);
    va_end(va);
    
    Con_AddText(msg, clr);
}

//
// Con_ParseKey
//

void Con_ParseKey(int c)
{
    if(c == SDLK_BACKSPACE)
    {
        if(console_inputlength > 1)
            console_inputbuffer[--console_inputlength] = 0;

        return;
    }
    
    c = keycode[shiftdown][c];

    if(console_inputlength >= MAX_CONSOLE_INPUT_LEN - 2)
        console_inputlength = MAX_CONSOLE_INPUT_LEN - 2;

    console_inputbuffer[console_inputlength++] = c;
}

//
// Con_Ticker
//

void Con_Ticker(void)
{
    if(!console_enabled)
        return;

    if(keyheld && ((Sys_GetMilliseconds() - timepressed) >= 500))
    {
        Con_ParseKey(lastkey);
    }
}

//
// Con_Responder
//

kbool Con_Responder(event_t* ev)
{
    int c;
    kbool clearheld = true;

    
    if(ev->type != ev_keyup && ev->type != ev_keydown &&
        ev->type != ev_mousewheel)
    {
        return false;
    }
    
    if(ev->type == ev_mousewheel && console_enabled)
    {
        switch(ev->data1)
        {
        case SDL_BUTTON_WHEELUP:
            if(console_head < MAX_CONSOLE_LINES)
            {
                console_head++;
            }
            break;
        case SDL_BUTTON_WHEELDOWN:
            if(console_head > console_minline)
            {
                console_head--;
            }
            break;
        }

        return true;
    }

    c = ev->data1;
    lastkey = c;
    lastevent = ev->type;

    if(ev->type == ev_keydown && !keyheld)
    {
        keyheld = true;
        timepressed = Sys_GetMilliseconds();
    }
    else
    {
        keyheld = false;
        timepressed = 0;
    }
    
    if(IN_Shiftdown(c))
    {
        if(ev->type == ev_keydown)
            shiftdown = true;
        else if(ev->type == ev_keyup)
            shiftdown = false;
    }
    
    switch(console_state)
    {
    case CST_DOWN:
    case CST_LOWER:
        if(ev->type == ev_keydown)
        {
            switch(c)
            {
            case SDLK_BACKQUOTE:
                console_state = CST_UP;
                console_enabled = false;
                break;
                
            case SDLK_ESCAPE:
                console_inputlength = 1;
                break;
                
            case SDLK_TAB:

                if(!Cmd_CompleteCommand(&console_inputbuffer[1]))
                {
                    Cvar_AutoComplete(&console_inputbuffer[1]);
                }
                break;
                
            case SDLK_RETURN:
                if(console_inputlength <= 1)
                {
                    break;
                }
                
                console_inputbuffer[console_inputlength+1] = 0;
                Con_AddLine(console_inputbuffer, console_inputlength, 0xFFC0C0C0);

                console_prevcmds[console_cmdhead] = console_head;
                console_cmdhead++;
                console_nextcmd = console_cmdhead;

                if(console_cmdhead >= CMD_HISTORY_SIZE)
                    console_cmdhead = 0;

                console_prevcmds[console_cmdhead] = -1;
                Cmd_ExecuteCommand(&console_inputbuffer[1]);
                console_inputlength = 1;
                CONCLEARINPUT();
                break;
                
            case SDLK_UP:
                c = console_nextcmd - 1;
                if(c < 0)
                {
                    c = CMD_HISTORY_SIZE - 1;
                }
                
                if(console_prevcmds[c] == -1)
                {
                    break;
                }
                
                console_nextcmd = c;
                c = console_prevcmds[console_nextcmd];
                if(console_buffer[c])
                {
                    CONCLEARINPUT();

                    console_inputlength = console_buffer[c]->len;
                    memcpy(console_inputbuffer,
                        console_buffer[console_prevcmds[console_nextcmd]]->line,
                        console_inputlength);
                }
                break;
                
            case SDLK_DOWN:
                if(console_prevcmds[console_nextcmd] == -1)
                {
                    break;
                }
                
                c = console_nextcmd + 1;
                if(c >= CMD_HISTORY_SIZE)
                {
                    c = 0;
                }
                
                if(console_prevcmds[c] == -1)
                {
                    break;
                }
                
                console_nextcmd = c;
                CONCLEARINPUT();
                console_inputlength = console_buffer[console_prevcmds[console_nextcmd]]->len;
                memcpy(console_inputbuffer,
                    console_buffer[console_prevcmds[console_nextcmd]]->line,
                    console_inputlength);
                break;

            case SDLK_PAGEUP:
                if(console_head < MAX_CONSOLE_LINES)
                {
                    console_head++;
                }
                break;

            case SDLK_PAGEDOWN:
                if(console_head > console_minline)
                {
                    console_head--;
                }
                break;
                
            default:
                if(IN_Shiftdown(c) || IN_Ctrldown(c) || IN_Altdown(c) || c > 255)
                {
                    break;
                }

                clearheld = false;
                Con_ParseKey(c);
                break;
            }

            if(clearheld)
            {
                keyheld = false;
                timepressed = 0;
            }
        }
        return true;
        
    case CST_UP:
    case CST_RAISE:
        if(c == SDLK_BACKQUOTE)
        {
            if(ev->type == ev_keydown)
            {
                console_state = CST_DOWN;
                console_enabled = true;
                console_inputlength = 1;
                CONCLEARINPUT();
            }
            return false;
        }
        break;
    }
    
    return false;
}

//
// Con_Draw
//

#define CONFONT_YPAD    16

void Con_Draw(void)
{
    int     line;
    float   y = 0;
    float   x = 0;
    float   inputlen;
    
    if(!console_linebuffer)
        return;

    if(!console_enabled)
        return;
    
    GL_SetState(GLSTATE_BLEND, 1);

    dglDisable(GL_TEXTURE_2D);
    dglColor4ub(0, 0, 0, 128);
    dglRectf((float)video_width, CONSOLE_Y + CONFONT_YPAD, 0, 0);

    GL_SetState(GLSTATE_BLEND, 0);
    
    dglColor4f(0, 1, 0, 1);
    dglBegin(GL_LINES);
    dglVertex2f(0, CONSOLE_Y - 1);
    dglVertex2f((float)video_width, CONSOLE_Y - 1);
    dglVertex2f(0, CONSOLE_Y + CONFONT_YPAD);
    dglVertex2f((float)video_width, CONSOLE_Y + CONFONT_YPAD);
    dglEnd();
    dglEnable(GL_TEXTURE_2D);
    
    line = console_head;
    
    y = CONSOLE_Y - 2;

    if(line < MAX_CONSOLE_LINES)
    {
        while(console_buffer[line] && y > 0)
        {
            Draw_Text(0, y, console_buffer[line]->color,
                1, "%s", console_buffer[line]->line);

            line = (line + 1) & CONSOLETEXT_MASK;
            y -= CONFONT_YPAD;
        }
    }
    
    y = CONSOLE_Y + CONFONT_YPAD;

    inputlen = Draw_Text(x, y, COLOR_WHITE, 1, "%s", console_inputbuffer);
    Draw_Text(x + inputlen, y, COLOR_WHITE, 1, "_");
}

//
// FCmd_ClearConsole
//

static void FCmd_ClearConsole(void)
{
    int i;

    for(i = 0; i < MAX_CONSOLE_LINES; i++)
    {
        if(console_buffer[i] != NULL)
        {
            Z_Free(console_buffer[i]);
            console_buffer[i] = NULL;
        }
    }

    for(i = 0; i < MAX_CONSOLE_INPUT_LEN; i++)
    {
        console_inputbuffer[i] = 0;
    }

    for(i = 0; i < CMD_HISTORY_SIZE; i++)
    {
        console_prevcmds[i] = -1;
    }

    console_cmdhead = 0;
    console_nextcmd = 0;
    console_head = 0;
    console_minline = 0;
    console_lineoffset = 0;
    console_linelength = 0;
    console_inputlength = 1;
    console_inputbuffer[0] = CONSOLE_PROMPTCHAR;
}

//
// Con_Init
//

void Con_Init(void)
{
    int i;
    
    console_buffer = (conline_t**)Z_Malloc(sizeof(conline_t *) * MAX_CONSOLE_LINES, PU_STATIC, NULL);
    console_head = 0;
    console_minline = 0;
    console_lineoffset = 0;
    
    for(i = 0; i < MAX_CONSOLE_LINES; i++)
    {
        console_buffer[i] = NULL;
    }
    
    for(i = 0; i < MAX_CONSOLE_INPUT_LEN; i++)
    {
        console_inputbuffer[i] = 0;
    }

    console_linelength = 0;
    console_inputlength = 1;
    console_inputbuffer[0] = CONSOLE_PROMPTCHAR;
    
    for(i = 0; i < CMD_HISTORY_SIZE; i++)
    {
        console_prevcmds[i] = -1;
    }
    
    console_cmdhead = 0;
    console_nextcmd = 0;

    console_initialized = true;

    Cmd_AddCommand("clear", FCmd_ClearConsole);
}
