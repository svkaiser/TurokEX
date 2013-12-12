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
// DESCRIPTION: Font mapping
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "script.h"
#include "renderSystem.h"
#include "renderFont.h"

//
// kexFont::kexFont
//

kexFont::kexFont(void) {
    this->bLoaded = false;
    this->texture = NULL;
}

//
// kexFont::~kexFont
//

kexFont::~kexFont(void) {
}

//
// kexFont::LoadKFont
//

void kexFont::LoadKFont(const char *file) {
    kexLexer *lexer;
    filepath_t fileName;
    texClampMode_t clamp = TC_CLAMP;
    texFilterMode_t filter = TF_LINEAR;

    if(!(lexer = parser.Open(file))) {
        texture = renderSystem.CacheTexture("textures/default.tga", clamp, filter);
        common.Warning("kexFont::LoadKFont: %s not found\n", file);
        return;
    }

    memset(fileName, 0, sizeof(filepath_t));

    while(lexer->CheckState()) {
        lexer->Find();

        if(lexer->TokenType() != TK_IDENIFIER) {
            continue;
        }

        if(!strcmp(lexer->Token(), "texture")) {
            lexer->ExpectNextToken(TK_LBRACK);
            lexer->Find();

            while(lexer->TokenType() != TK_RBRACK) {
                if(!strcmp(lexer->Token(), "kfont")) {
                    lexer->GetString();
                    strcpy(fileName, lexer->StringToken());
                }

                if(!strcmp(lexer->Token(), "filter")) {
                    lexer->Find();

                    if(!strcmp(lexer->Token(), "nearest")) {
                        filter = TF_NEAREST;
                    }

                    if(!strcmp(lexer->Token(), "linear")) {
                        filter = TF_LINEAR;
                    }
                }

                if(!strcmp(lexer->Token(), "wrap")) {
                    lexer->Find();

                    if(!strcmp(lexer->Token(), "clamp")) {
                        clamp = TC_CLAMP;
                    }

                    if(!strcmp(lexer->Token(), "repeat")) {
                        clamp = TC_REPEAT;
                    }
                }

                lexer->Find();
            }

            if(fileName[0] != 0) {
                texture = renderSystem.CacheTexture(fileName, clamp, filter);
            }
        }

        if(!strcmp(lexer->Token(), "mapchar")) {
            lexer->ExpectNextToken(TK_LBRACK);
            lexer->Find();

            while(lexer->TokenType() != TK_RBRACK) {
                int ch;

                if(lexer->TokenType() == TK_NUMBER) {
                    ch = atoi(lexer->Token());
                }
                else {
                    parser.Error("%s is not a number", lexer->Token());
                    parser.Close();
                    return;
                }

                atlas[ch].x = lexer->GetNumber();
                atlas[ch].y = lexer->GetNumber();
                atlas[ch].w = lexer->GetNumber();
                atlas[ch].h = lexer->GetNumber();

                lexer->Find();
            }
        }
    }

    // we're done with the file
    parser.Close();
    bLoaded = true;
}

//
// kexFont::DrawString
//

void kexFont::DrawString(const char *string, float x, float y, float scale,
                         bool center, byte *rgba1, byte *rgba2) {
    float w;
    float h;
    int tri;
    unsigned int i;
    unsigned int len;
    char ch;
    atlas_t *at;
    float vx1;
    float vy1;
    float vx2;
    float vy2;
    float tx1;
    float tx2;
    float ty1;
    float ty2;
    char *check;

    if(scale <= 0.01f) {
        scale = 1;
    }

    if(center) {
        x -= StringWidth(string, scale, 0) * 0.5f;
    }

    w = (float)texture->OriginalWidth();
    h = (float)texture->OriginalHeight();

    tri = 0;
    len = strlen(string);

    renderSystem.BindDrawPointers();
    texture->Bind();

    for(i = 0; i < len; i++) {
        ch      = string[i];
        at      = &atlas[ch];
        vx1     = x;
        vy1     = y;
        vx2     = vx1 + at->w * scale;
        vy2     = vy1 + at->h * scale;
        tx1     = (at->x / w) + 0.001f;
        tx2     = (tx1 + at->w / w) - 0.002f;
        ty1     = (at->y / h);
        ty2     = (ty1 + at->h / h);
        check   = (char*)string+i;

        renderSystem.AddVertex(vx1, vy1, 0, tx1, ty1, rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
        renderSystem.AddVertex(vx2, vy1, 0, tx2, ty1, rgba1[0], rgba1[1], rgba1[2], rgba1[3]);
        renderSystem.AddVertex(vx1, vy2, 0, tx1, ty2, rgba2[0], rgba2[1], rgba2[2], rgba2[3]);
        renderSystem.AddVertex(vx2, vy2, 0, tx2, ty2, rgba2[0], rgba2[1], rgba2[2], rgba2[3]);

        renderSystem.AddTriangle(0+tri, 1+tri, 2+tri);
        renderSystem.AddTriangle(2+tri, 1+tri, 3+tri);

        x += at->w * scale;
        tri += 4;
    }

    renderSystem.DrawElements();
}

//
// kexFont::StringWidth
//

float kexFont::StringWidth(const char* string, float scale, int fixedLen) {
    float width = 0;
    int len = strlen(string);
    int i;

    if(fixedLen > 0) {
        len = fixedLen;
    }
        
    for(i = 0; i < len; i++) {
        width += (atlas[string[i]].w * scale);
    }
            
    return width;
}
