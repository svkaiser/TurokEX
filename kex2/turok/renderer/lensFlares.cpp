// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: Lens flares
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "script.h"
#include "renderBackend.h"
#include "lensFlares.h"
#include "world.h"

//
// kexLensFlares::kexLensFlares
//

kexLensFlares::kexLensFlares(void) {
    this->bLoaded = false;
}

//
// kexLensFlares::~kexLensFlares
//

kexLensFlares::~kexLensFlares(void) {
}

//
// kexLensFlares::Delete
//

void kexLensFlares::Delete(void) {
    if(lens == NULL) {
        return;
    }

    for(int i = 0; i < numlens; i++) {
        if(lens[i].material) {
            lens[i].material->Delete();
        }
    }

    delete[] lens;
}

//
// kexLensFlares::LoadKLF
//

void kexLensFlares::LoadKLF(const char *file) {
    kexLexer *lexer;
    filepath_t fileName;

    if(!(lexer = parser.Open(file))) {
        common.Warning("kexLensFlares::LoadKLF: %s not found\n", file);
        return;
    }

    memset(fileName, 0, sizeof(filepath_t));
    numlens = 0;
    lens = NULL;
    bLoaded = false;

    while(lexer->CheckState()) {
        lexer->Find();

        if(lexer->TokenType() != TK_IDENIFIER) {
            continue;
        }

        if(lexer->Matches("numlensflares")) {
            numlens = lexer->GetNumber();

            if(numlens <= 0) {
                parser.Close();
                return;
            }

            lens = new lfData_t[numlens];

            for(int i = 0; i < numlens; i++) {
                lexer->ExpectNextToken(TK_LBRACK);
                lexer->Find();

                while(lexer->TokenType() != TK_RBRACK) {
                    if(lexer->Matches("scale")) {
                        lens[i].scale = (float)lexer->GetFloat();
                    }
                    else if(lexer->Matches("offset")) {
                        lens[i].offset = (float)lexer->GetFloat();
                    }
                    else if(lexer->Matches("material")) {
                        lexer->GetString();
                        lens[i].material = renderBackend.CacheMaterial(lexer->StringToken());
                    }
                    lexer->Find();
                }
            }
        }
    }

    // we're done with the file
    parser.Close();
    bLoaded = true;
}

//
// kexLensFlares::Draw
//

void kexLensFlares::Draw(const kexVec3 &origin) {
    float hw;
    float hh;
    float scale;
    float len;
    float offs;
    kexFrustum frustum;
    kexVec3 org;
    kexVec3 pos;

    frustum = localWorld.Camera()->Frustum();

    if(!frustum.TestSphere(origin, 4.0f)) {
        return;
    }

    hw = (float)sysMain.VideoWidth() * 0.5f;
    hh = (float)sysMain.VideoHeight() * 0.5f;
    org = origin;
    pos = localWorld.Camera()->ProjectPoint(org, 0, 0);
    org = pos;
    len = localWorld.Camera()->GetOrigin().Distance(origin);

    for(int i = 0; i < numlens; i++) {
        scale = lens[i].scale * 2.0f;
        offs = lens[i].offset / len;

        renderBackend.AddVertex(org.x - scale, org.y - scale, 0, 0, 0, 255, 255, 255, 255);
        renderBackend.AddVertex(org.x + scale, org.y - scale, 0, 1, 0, 255, 255, 255, 255);
        renderBackend.AddVertex(org.x - scale, org.y + scale, 0, 0, 1, 255, 255, 255, 255);
        renderBackend.AddVertex(org.x + scale, org.y + scale, 0, 1, 1, 255, 255, 255, 255);
        renderBackend.AddTriangle(0, 1, 2);
        renderBackend.AddTriangle(2, 1, 3);
        renderBackend.DrawElements(lens[i].material);

        org.x = pos.x - ((pos.x - hw) * offs * (i+1));
        org.y = pos.y - ((pos.y - hh) * offs * (i+1));
    }
}
