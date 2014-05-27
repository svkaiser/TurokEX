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
// DESCRIPTION: Static world models
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "worldModel.h"
#include "world.h"
#include "renderBackend.h"

enum {
    scwmdl_mesh = 0,
    scwmdl_bounds,
    scwmdl_materials,
    scwmdl_bCollision,
    scwmdl_bHidden,
    scwmdl_bStatic,
    scwmdl_origin,
    scwmdl_scale,
    scwmdl_angles,
    scwmdl_rotation,
    scwmdl_radius,
    scwmdl_height,
    scwmdl_centerheight,
    scwmdl_viewheight,
    scwmdl_cullDistance,
    scwmdl_clipmesh,
    scwmdl_impactType,
    scwmdl_end
};

static const sctokens_t mapworldmodeltokens[scwmdl_end+1] = {
    { scwmdl_mesh,              "mesh"              },
    { scwmdl_bounds,            "bounds"            },
    { scwmdl_materials,         "materials"         },
    { scwmdl_bCollision,        "bCollision"        },
    { scwmdl_bHidden,           "bHidden"           },
    { scwmdl_bStatic,           "bStatic"           },
    { scwmdl_origin,            "origin"            },
    { scwmdl_scale,             "scale"             },
    { scwmdl_angles,            "angles"            },
    { scwmdl_rotation,          "rotation"          },
    { scwmdl_radius,            "radius"            },
    { scwmdl_height,            "height"            },
    { scwmdl_centerheight,      "centerheight"      },
    { scwmdl_viewheight,        "viewheight"        },
    { scwmdl_cullDistance,      "cullDistance"      },
    { scwmdl_clipmesh,          "clipMesh"          },
    { scwmdl_impactType,        "impactType"        },
    { -1,                       NULL                }
};

DECLARE_CLASS(kexWorldModel, kexWorldObject)

//
// kexWorldModel::kexWorldModel
//

kexWorldModel::kexWorldModel(void) {
    this->baseBBox.min.Set(-32, -32, -32);
    this->baseBBox.max.Set(32, 32, 32);

    this->worldLink.SetData(this);
    this->renderNode.link.SetData(this);
    this->clipMesh.SetOwner(this);

    this->bbox              = baseBBox;
    this->bTraced           = false;
    this->validcount        = 0;
    this->materials         = NULL;
    this->wireframeColor    = RGBA(0, 224, 224, 255);
}

//
// kexWorldModel::~kexWorldModel
//

kexWorldModel::~kexWorldModel(void) {
    if(model && materials) {
        Mem_Free(materials);
        materials = NULL;
    }
}

//
// kexWorldModel::LocalTick
//

void kexWorldModel::LocalTick(void) {
}

//
// kexWorldModel::Tick
//

void kexWorldModel::Tick(void) {
}

//
// kexWorldModel::Spawn
//

void kexWorldModel::Spawn(void) {
    if(model == NULL) {
        SetModel("models/default.kmesh");
    }
    
    UpdateTransform();

    height = baseHeight;

    clipMesh.CreateShape();
    clipMesh.Transform();
}

//
// kexWorldModel::Parse
//

void kexWorldModel::Parse(kexLexer *lexer) {
    unsigned int i;

    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(mapworldmodeltokens, lexer->Token())) {
            case scwmdl_mesh:
                lexer->GetString();
                SetModel(lexer->StringToken());
                break;
            case scwmdl_origin:
                origin = lexer->GetVector3();
                break;
            case scwmdl_angles:
                angles = lexer->GetVector3();
                rotation = angles.ToQuat();
                break;
            case scwmdl_scale:
                scale = lexer->GetVector3();
                break;
            case scwmdl_rotation:
                rotation = lexer->GetVector4();
                break;
            case scwmdl_bounds:
                baseBBox.min = lexer->GetVector3();
                baseBBox.max = lexer->GetVector3();
                bbox = baseBBox;
                break;
            case scwmdl_materials:
                if(model == NULL) {
                    parser.Error("kexWorldModel::ParseDefault: attempted to parse \"materials\" token while model is null\n");
                }
                
                AllocateMaterials();
                
                // texture swap block
                lexer->ExpectNextToken(TK_LBRACK);
                i = 0;
                
                while(lexer->TokenType() != TK_RBRACK) {
                    if(lexer->TokenType() == TK_STRING) {
                        char *str;

                        if(i >= model->nodes[0].numSurfaces) {
                            continue;
                        }

                        str = (char*)lexer->Token();

                        if(str[0] != '-') {
                            materials[i] = renderBackend.CacheMaterial(str);
                        }
                        else {
                            materials[i] = NULL;
                        }

                        i++;
                    }

                    lexer->Find();
                }
                break;
            case scwmdl_bCollision:
                bCollision = (lexer->GetNumber() > 0);
                break;
            case scwmdl_bHidden:
                bHidden = (lexer->GetNumber() > 0);
                break;
            case scwmdl_bStatic:
                bStatic = (lexer->GetNumber() > 0);
                break;
            case scwmdl_radius:
                radius = (float)lexer->GetFloat();
                break;
            case scwmdl_height:
                baseHeight = (float)lexer->GetFloat();
                break;
            case scwmdl_centerheight:
                centerHeight = (float)lexer->GetFloat();
                break;
            case scwmdl_viewheight:
                viewHeight = (float)lexer->GetFloat();
                break;
            case scwmdl_impactType:
                impactType = (impactType_t)lexer->GetNumber();
                break;
            case scwmdl_cullDistance:
                cullDistance = (float)lexer->GetFloat();
                break;
            case scwmdl_clipmesh:
                clipMesh.Parse(lexer);
                break;
            default:
                if(lexer->TokenType() != TK_IDENIFIER) {
                    parser.Error("kexWorldModel::ParseDefault: unknown token: %s\n",
                                 lexer->Token());
                }
                break;
        }
        
        lexer->Find();
    }
}

//
// kexWorldModel::UpdateTransform
//

void kexWorldModel::UpdateTransform(void) {
    matrix = kexMatrix(rotation);
    matrix.Scale(scale);
    rotMatrix = matrix;

    matrix.AddTranslation(origin);

    bbox.min += origin;
    bbox.max += origin;
}

//
// kexWorldModel::AllocateMaterials
//

void kexWorldModel::AllocateMaterials(void) {
    modelNode_t *node;

    node = &model->nodes[0];

    if(node->numSurfaces == 0) {
        return;
    }

    // allocate data for material swap array
    materials = (kexMaterial**)Mem_Calloc(sizeof(kexMaterial*) *
        node->numSurfaces, hb_static);
}

//
// kexWorldModel::SetModel
//

void kexWorldModel::SetModel(const char* modelFile) {
    if(modelFile) {
        model = modelManager.LoadModel(modelFile);
    }
}

//
// kexWorldModel::SetModel
//

void kexWorldModel::SetModel(const kexStr &modelFile) {
    SetModel(modelFile.c_str());
}
