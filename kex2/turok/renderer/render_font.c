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
// DESCRIPTION: Font System
//
//-----------------------------------------------------------------------------

#include "SDL_opengl.h"
#include "common.h"
#include "gl.h"
#include "render.h"

//
// Font_MapChar
//

void Font_MapChar(font_t *font, byte ch, int x, int y, int w, int h)
{
    font->atlas[ch].x = x;
    font->atlas[ch].y = y;
    font->atlas[ch].w = w;
    font->atlas[ch].h = h;
}

//
// Font_StringWidth
//

float Font_StringWidth(font_t *font, const char* string, float scale, int fixedLen)
{
    float width = 0;
    int len = strlen(string);
    int i;

    if(fixedLen > 0)
        len = fixedLen;
        
    for(i = 0; i < len; i++)
        width += (font->atlas[string[i]].w * scale);
            
    return width;
}

