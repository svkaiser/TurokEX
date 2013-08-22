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
// DESCRIPTION: Loading of KANIM files
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "render.h"

enum
{
    scanim_anim = 0,
    scanim_numframes,
    scanim_numnodes,
    scanim_numtranslationsets,
    scanim_numrotationsets,
    scanim_nodeframes,
    scanim_initial_t,
    scanim_initial_r,
    scanim_translationsets,
    scanim_rotationsets,
    scanim_numactions,
    scanim_actions,
    scanim_initialtranslation,
    scanim_initialrotation,
    scanim_turninfo,
    scanim_loopframe,
    scanim_end
};

static const sctokens_t animtokens[scanim_end+1] =
{
    { scanim_anim,              "anim"                  },
    { scanim_numframes,         "numframes"             },
    { scanim_numnodes,          "numnodes"              },
    { scanim_numtranslationsets,"numtranslationsets"    },
    { scanim_numrotationsets,   "numrotationsets"       },
    { scanim_nodeframes,        "nodeframes"            },
    { scanim_initial_t,         "initialtranslation"    },
    { scanim_initial_r,         "initialrotation"       },
    { scanim_translationsets,   "translationsets"       },
    { scanim_rotationsets,      "rotationsets"          },
    { scanim_numactions,        "numactions"            },
    { scanim_actions,           "actions"               },
    { scanim_turninfo,          "turninfo"              },
    { scanim_loopframe,         "loopframe"             },
    { -1,                       NULL                    }
};

//
// Kanim_ParseAnimScript
//

static void Kanim_ParseAnimScript(kmodel_t *model, anim_t *anim, kexLexer *lexer)
{
    unsigned int numnodes;
    unsigned int i;
    unsigned int j;

    numnodes = 0;

    if(model->numnodes <= 0)
    {
        parser.Error("numnodes is 0 or has not been set yet for %s",
            model->filePath);
    }

    anim->frameset = (frameset_t*)Z_Calloc(sizeof(frameset_t)
        * model->numnodes, PU_MODEL, 0);

    lexer->ExpectTokenListID(animtokens, scanim_anim);
    lexer->ExpectNextToken(TK_LBRACK);

    while(lexer->CheckState())
    {
        lexer->Find();

        switch(lexer->TokenType())
        {
        case TK_NONE:
            return;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            {
                switch(lexer->GetIDForTokenList(animtokens, lexer->Token()))
                {
                    // frame count
                case scanim_numframes:
                    lexer->AssignFromTokenList(animtokens, &anim->numframes,
                        scanim_numframes, false);
                    break;
                    // loop frame
                case scanim_loopframe:
                    lexer->AssignFromTokenList(animtokens, &anim->loopframe,
                        scanim_loopframe, false);
                    break;
                    // action count
                case scanim_numactions:
                    lexer->AssignFromTokenList(animtokens, &anim->numactions,
                        scanim_numactions, false);
                    break;
                    // number of nodes (must match numnodes in model file)
                case scanim_numnodes:
                    lexer->AssignFromTokenList(animtokens, &numnodes,
                        scanim_numnodes, false);

                    if(numnodes != model->numnodes)
                    {
                        parser.Error("numnodes(%i) for %s doesn't match numnodes in model file(%i)",
                            numnodes, anim->alias, model->numnodes);
                    }
                    break;
                    // translation table count
                case scanim_numtranslationsets:
                    lexer->AssignFromTokenList(animtokens, &anim->numtranslations,
                        scanim_numtranslationsets, false);
                    break;
                    // rotation table count
                case scanim_numrotationsets:
                    lexer->AssignFromTokenList(animtokens, &anim->numrotations,
                        scanim_numrotationsets, false);
                    break;
                    // translation table
                case scanim_translationsets:
                    lexer->ExpectNextToken(TK_EQUAL);
                    if(anim->numtranslations <= 0)
                    {
                        parser.Error("numtranslations is 0 or has not been set yet for %s",
                            anim->alias);
                    }
                    if(anim->numframes <= 0)
                    {
                        parser.Error("numframes is 0 or has not been set yet for %s",
                            anim->alias);
                    }
                    anim->translations = (animtranslation_t**)Z_Calloc(sizeof(animtranslation_t*)
                        * anim->numtranslations, PU_MODEL, 0);

                    lexer->ExpectNextToken(TK_LBRACK);
                    for(i = 0; i < anim->numtranslations; i++)
                    {
                        anim->translations[i] = (animtranslation_t*)Z_Calloc(
                            sizeof(animtranslation_t) * anim->numframes, PU_MODEL, 0);

                        lexer->ExpectNextToken(TK_LBRACK);
                        for(j = 0; j < anim->numframes; j++)
                        {
                            lexer->ExpectNextToken(TK_LBRACK);
                            anim->translations[i][j].vec[0] = (float)lexer->GetFloat();
                            anim->translations[i][j].vec[1] = (float)lexer->GetFloat();
                            anim->translations[i][j].vec[2] = (float)lexer->GetFloat();
                            lexer->ExpectNextToken(TK_RBRACK);
                        }
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                    // rotation table
                case scanim_rotationsets:
                    lexer->ExpectNextToken(TK_EQUAL);
                    if(anim->numrotations <= 0)
                    {
                        parser.Error("numrotations is 0 or has not been set yet for %s",
                            anim->alias);
                    }
                    if(anim->numframes <= 0)
                    {
                        parser.Error("numframes is 0 or has not been set yet for %s",
                            anim->alias);
                    }
                    anim->rotations = (animrotation_t**)Z_Calloc(sizeof(animrotation_t*)
                        * anim->numrotations, PU_MODEL, 0);

                    lexer->ExpectNextToken(TK_LBRACK);
                    for(i = 0; i < anim->numrotations; i++)
                    {
                        anim->rotations[i] = (animrotation_t*)Z_Calloc(
                            sizeof(animrotation_t) * anim->numframes, PU_MODEL, 0);

                        lexer->ExpectNextToken(TK_LBRACK);
                        for(j = 0; j < anim->numframes; j++)
                        {
                            lexer->ExpectNextToken(TK_LBRACK);
                            anim->rotations[i][j].vec[0] = (float)lexer->GetFloat();
                            anim->rotations[i][j].vec[1] = (float)lexer->GetFloat();
                            anim->rotations[i][j].vec[2] = (float)lexer->GetFloat();
                            anim->rotations[i][j].vec[3] = (float)lexer->GetFloat();
                            lexer->ExpectNextToken(TK_RBRACK);
                        }
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                    // lookup table for model nodes
                case scanim_nodeframes:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(i = 0; i < model->numnodes; i++)
                    {
                        int num;

                        lexer->ExpectNextToken(TK_LBRACK);

                        num = lexer->GetNumber();
                        anim->frameset[i].translation = num != -1 ?
                            anim->translations[num] : NULL;

                        num = lexer->GetNumber();
                        anim->frameset[i].rotation = num != -1 ?
                            anim->rotations[num] : NULL;

                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                    // actions
                case scanim_actions:
                    if(anim->numactions <= 0)
                    {
                        parser.Error("numactions is 0 or has not been set yet for %s",
                            anim->alias);
                    }
                    anim->actions = (action_t*)Z_Calloc(sizeof(action_t) *
                        anim->numactions, PU_MODEL, 0);
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(i = 0; i < anim->numactions; i++)
                    {
                        anim->actions[i].frame = lexer->GetNumber();
                        lexer->GetString();
                        anim->actions[i].function = Z_Strdup(lexer->StringToken(), PU_MODEL, 0);

                        lexer->Find();

                        for(j = 0; j < 4; j++)
                        {
                            switch(lexer->TokenType())
                            {
                            case TK_STRING:
                                anim->actions[i].argStrings[j] =
                                Z_Strdup(lexer->Token(), PU_MODEL, 0);
                                break;
                            case TK_NUMBER:
                                anim->actions[i].args[j] = (float)atof(lexer->Token());
                                break;
                            default:
                                common.Warning("Kanim_ParseAnimScript: Action Argument #%i ", j);
                                common.Warning("is not a number nor a string\n");
                                common.Warning("line=%i, pos=%i\n\n", lexer->LinePos(), lexer->RowPos());
                                break;
                            }

                            if(j >= 3)
                                break;

                            lexer->Find();
                        }
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                    // initial translation frame
                case scanim_initial_t:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);

                    anim->initial.translation = (animtranslation_t*)Z_Calloc(sizeof(animtranslation_t)
                        * model->numnodes, PU_MODEL, 0);

                    for(i = 0; i < model->numnodes; i++)
                    {
                        lexer->ExpectNextToken(TK_LBRACK);
                        anim->initial.translation[i].vec[0] = (float)lexer->GetFloat();
                        anim->initial.translation[i].vec[1] = (float)lexer->GetFloat();
                        anim->initial.translation[i].vec[2] = (float)lexer->GetFloat();
                        lexer->ExpectNextToken(TK_RBRACK);
                    }

                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                    // initial rotation frame
                case scanim_initial_r:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);

                    anim->initial.rotation = (animrotation_t*)Z_Calloc(sizeof(animrotation_t)
                        * model->numnodes, PU_MODEL, 0);

                    for(i = 0; i < model->numnodes; i++)
                    {
                        lexer->ExpectNextToken(TK_LBRACK);
                        anim->initial.rotation[i].vec[0] = (float)lexer->GetFloat();
                        anim->initial.rotation[i].vec[1] = (float)lexer->GetFloat();
                        anim->initial.rotation[i].vec[2] = (float)lexer->GetFloat();
                        anim->initial.rotation[i].vec[3] = (float)lexer->GetFloat();
                        lexer->ExpectNextToken(TK_RBRACK);
                    }

                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                case scanim_turninfo:
                    anim->yawOffsets = (float*)Z_Calloc(sizeof(float) *
                        anim->numframes, PU_MODEL, 0);
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(i = 0; i < anim->numframes; i++)
                        anim->yawOffsets[i] = (float)lexer->GetFloat();
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                default:
                    if(lexer->TokenType() == TK_IDENIFIER)
                    {
                        common.DPrintf("Kanim_ParseAnimScript: Unknown token: %s\n",
                            lexer->Token());
                    }
                    break;
                }
            }
            break;
        default:
            break;
        }
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Kanim_Load
//

void Kanim_Load(kmodel_t *model)
{
    unsigned int i;

    if(model->anims == NULL || model->numanimations <= 0 || model->numnodes <= 0)
        return;

    for(i = 0; i < model->numanimations; i++)
    {
        kexLexer *lexer;

        if(!(lexer = parser.Open(model->anims[i].animpath)))
            continue;

        Kanim_ParseAnimScript(model, &model->anims[i], lexer);
        parser.Close();
    }
}
