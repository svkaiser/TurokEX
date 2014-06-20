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

kexLensFlaresManager kexLensFlares::manager;

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
                        lens[i].material = kexMaterial::manager.Load(lexer->StringToken());
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
    float hw, hh;
    float w, h;
    float proj_x, proj_y;
    float fx, fy;
    float div_x, div_y;
    float ndcx, ndcy;
    kexVec3 pos;
    kexVec3 org;
    float len;
    float scale;
    float d;
    byte alpha;

    if(!localWorld.Camera()->Frustum().TestSphere(origin, 0)) {
        return;
    }

    org = origin;
    pos = localWorld.Camera()->ProjectPoint(org, 0, 0);

    w = (float)sysMain.VideoWidth();
    h = (float)sysMain.VideoHeight();
    hw = w * 0.5f;
    hh = h * 0.5f;

    proj_x = hw - pos.x;
    proj_y = hh - pos.y;

    len = kexMath::Sqrt(proj_x * proj_x + proj_y * proj_y) * 0.4f;

    fx = kexMath::Fabs(proj_x);
    fy = kexMath::Fabs(proj_y);

    div_x = 0;
    div_y = 0;

    if(fx <= fy) {
        if(fy > 0) {
            div_x = proj_x / fy;
        }

        if(proj_y > 0) {
            div_y = 1.0f;
        }
        else {
            div_y = -1.0f;
        }
    }
    else {
        if(fx > 0) {
            div_y = proj_y / fx;
        }
        
        if(proj_x > 0) {
            div_x = 1.0f;
        }
        else {
            div_x = -1.0f;
        }
    }

    proj_x = pos.x;
    proj_y = pos.y;
    
    cpuVertList.BindDrawPointers();

    for(int i = 0; i < numlens; i++) {
        scale = lens[i].scale * 2.0f;

        ndcx = kexMath::Fabs((proj_x / w) * 2.0f - 1.0f);
        ndcy = kexMath::Fabs((proj_y / h) * 2.0f - 1.0f);

        d = 1.0f - ((ndcx * ndcx + ndcy * ndcy) / 2.0f);

        alpha = (byte)(kexMath::Pow(d, 3.0f) * 255.0f);

        cpuVertList.AddVertex(proj_x - scale, proj_y - scale, 0, 0, 0, 255, 255, 255, alpha);
        cpuVertList.AddVertex(proj_x + scale, proj_y - scale, 0, 1, 0, 255, 255, 255, alpha);
        cpuVertList.AddVertex(proj_x - scale, proj_y + scale, 0, 0, 1, 255, 255, 255, alpha);
        cpuVertList.AddVertex(proj_x + scale, proj_y + scale, 0, 1, 1, 255, 255, 255, alpha);
        cpuVertList.AddTriangle(0, 1, 2);
        cpuVertList.AddTriangle(2, 1, 3);
        cpuVertList.DrawElements(lens[i].material);

        proj_x += (len * div_x);
        proj_y += (len * div_y);
    }
}

//
// kexLensFlaresManager::OnLoad
//

kexLensFlares *kexLensFlaresManager::OnLoad(const char *file) {
    kexLensFlares *lens = dataList.Add(file);
    lens->LoadKLF(file);

    return lens;
}
