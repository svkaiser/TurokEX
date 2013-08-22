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
// DESCRIPTION: Loading of KMAP files
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "level.h"
#include "ai.h"
#include "js.h"
#include "js_shared.h"
#include "js_parse.h"

enum
{
    scmap_title = 0,
    scmap_mapID,
    scmap_glight_origin,
    scmap_glight_color,
    scmap_glight_ambience,
    scmap_glight_modelamb,
    scmap_actors,
    scmap_gridbounds,
    scmap_areas,
    scmap_end
};

static const sctokens_t maptokens[scmap_end+1] =
{
    { scmap_title,              "title"                 },
    { scmap_mapID,              "mapID"                 },
    { scmap_glight_origin,      "global_light_position" },
    { scmap_glight_color,       "global_light_color"    },
    { scmap_glight_ambience,    "global_light_ambience" },
    { scmap_glight_modelamb,    "global_model_ambience" },
    { scmap_actors,             "actors"                },
    { scmap_gridbounds,         "gridbounds"            },
    { scmap_areas,              "areas"                 },
    { -1,                       NULL                    }
};

enum
{
    scactor_name = 0,
    scactor_mesh,
    scactor_bounds,
    scactor_textureSwaps,
    scactor_components,
    scactor_bCollision,
    scactor_bHidden,
    scactor_bStatic,
    scactor_bTouch,
    scactor_bOrientOnSlope,
    scactor_bRotor,
    scactor_plane,
    scactor_origin,
    scactor_scale,
    scactor_angles,
    scactor_rotation,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_targetID,
    scactor_modelVariant,
    scactor_classFlags,
    scactor_cullDistance,
    scactor_friction,
    scactor_mass,
    scactor_bounceDamp,
    scactor_physics,
    scactor_rotorSpeed,
    scactor_rotorVector,
    scactor_rotorFriction,
    scactor_tickDistance,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] =
{
    { scactor_name,             "name"              },
    { scactor_mesh,             "mesh"              },
    { scactor_bounds,           "bounds"            },
    { scactor_textureSwaps,     "textureSwaps"      },
    { scactor_components,       "components"        },
    { scactor_bCollision,       "bCollision"        },
    { scactor_bHidden,          "bHidden"           },
    { scactor_bStatic,          "bStatic"           },
    { scactor_bTouch,           "bTouch"            },
    { scactor_bOrientOnSlope,   "bOrientOnSlope"    },
    { scactor_bRotor,           "bRotor"            },
    { scactor_plane,            "plane"             },
    { scactor_origin,           "origin"            },
    { scactor_scale,            "scale"             },
    { scactor_angles,           "angles"            },
    { scactor_rotation,         "rotation"          },
    { scactor_radius,           "radius"            },
    { scactor_height,           "height"            },
    { scactor_centerheight,     "centerheight"      },
    { scactor_viewheight,       "viewheight"        },
    { scactor_targetID,         "targetID"          },
    { scactor_modelVariant,     "modelVariant"      },
    { scactor_classFlags,       "classFlags"        },
    { scactor_cullDistance,     "cullDistance"      },
    { scactor_friction,         "friction"          },
    { scactor_mass,             "mass"              },
    { scactor_bounceDamp,       "bounceDamp"        },
    { scactor_physics,          "physics"           },
    { scactor_rotorSpeed,       "rotorSpeed"        },
    { scactor_rotorVector,      "rotorVector"       },
    { scactor_rotorFriction,    "rotorFriction"     },
    { scactor_tickDistance,     "tickDistance"      },
    { -1,                       NULL                }
};

enum
{
    scgridbnd_bounds = 0,
    scgridbnd_statics,
    scgridbnd_end
};

static const sctokens_t mapgridtokens[scgridbnd_end+1] =
{
    { scgridbnd_bounds,     "bounds"    },
    { scgridbnd_statics,    "statics"   },
    { -1,                   NULL        }
};

enum
{
    scarea_components = 0,
    scarea_waterplane,
    scarea_flags,
    scarea_triggerSound,
    scarea_targetID,
    scarea_end
};

static const sctokens_t mapareatokens[scarea_end+1] =
{
    { scarea_components,    "components"    },
    { scarea_waterplane,    "waterplane"    },
    { scarea_flags,         "flags"         },
    { scarea_triggerSound,  "triggerSound"  },
    { scarea_targetID,      "targetID"      },
    { -1,                   NULL            }
};

//
// Kmap_ParseActor
//

static void Kmap_ParseActor(kexLexer *lexer, gActor_t *actor)
{
    int numComponents = 0;
    int j;

    Vec_Set3(actor->scale, 1, 1, 1);

    // read into nested actor block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK)
    {
        switch(lexer->GetIDForTokenList(mapactortokens, lexer->Token()))
        {
        case scactor_name:
            lexer->AssignFromTokenList(mapactortokens, actor->name,
                scactor_name, false);
            break;

        case scactor_mesh:
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            Actor_UpdateModel(actor, lexer->StringToken());
            break;

        case scactor_bounds:
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);
            actor->bbox.min[0] = (float)lexer->GetFloat();
            actor->bbox.min[1] = (float)lexer->GetFloat();
            actor->bbox.min[2] = (float)lexer->GetFloat();
            actor->bbox.max[0] = (float)lexer->GetFloat();
            actor->bbox.max[1] = (float)lexer->GetFloat();
            actor->bbox.max[2] = (float)lexer->GetFloat();
            Vec_Copy3(actor->baseBBox.min, actor->bbox.min);
            Vec_Copy3(actor->baseBBox.max, actor->bbox.max);
            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_textureSwaps:
            if(actor->model == NULL)
                parser.Error("Kmap_ParseActor: Attempted to parse \"textureSwaps\" token while model is null\n");

            lexer->ExpectNextToken(TK_EQUAL);
            // texture swap block
            lexer->ExpectNextToken(TK_LBRACK);
            for(j = 0; j < (int)actor->model->numnodes; j++)
            {
                unsigned int k;
                mdlnode_t *node;

                node = &actor->model->nodes[j];

                // node block
                lexer->ExpectNextToken(TK_LBRACK);
                for(k = 0; k < node->nummeshes; k++)
                {
                    unsigned int l;
                    mdlmesh_t *mesh;

                    mesh = &node->meshes[k];

                    // mesh block
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(l = 0; l < mesh->numsections; l++)
                    {
                        // parse sections
                        lexer->GetString();
                        actor->textureSwaps[j][k][l] = Z_Strdup(lexer->StringToken(), PU_ACTOR, NULL);
                    }
                    // end mesh block
                    lexer->ExpectNextToken(TK_RBRACK);
                }
                // end node block
                lexer->ExpectNextToken(TK_RBRACK);
            }
            // end texture swap block
            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_components:
            lexer->ExpectNextToken(TK_LSQBRACK);
            numComponents = lexer->GetNumber();
            lexer->ExpectNextToken(TK_RSQBRACK);
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);
            JParse_Start(lexer, &actor->components, numComponents);
            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_bCollision:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bCollision,
                scactor_bCollision, false);
            break;

        case scactor_bHidden:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bHidden,
                scactor_bHidden, false);
            break;

        case scactor_bStatic:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bStatic,
                scactor_bStatic, false);
            break;

        case scactor_bTouch:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bTouch,
                scactor_bTouch, false);
            break;

        case scactor_bOrientOnSlope:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bOrientOnSlope,
                scactor_bOrientOnSlope, false);
            break;

        case scactor_bRotor:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->bRotor,
                scactor_bRotor, false);
            break;

        case scactor_plane:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->plane,
                scactor_plane, false);
            break;

        case scactor_origin:
            lexer->AssignVectorFromTokenList(mapactortokens, actor->origin,
                scactor_origin, false);
            break;

        case scactor_scale:
            lexer->AssignVectorFromTokenList(mapactortokens, actor->scale,
                scactor_scale, false);
            break;

        case scactor_angles:
            lexer->AssignVectorFromTokenList(mapactortokens, actor->angles,
                scactor_angles, false);
            break;

        case scactor_rotation:
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);

            actor->rotation[0] = (float)lexer->GetFloat();
            actor->rotation[1] = (float)lexer->GetFloat();
            actor->rotation[2] = (float)lexer->GetFloat();
            actor->rotation[3] = (float)lexer->GetFloat();

            lexer->ExpectNextToken(TK_RBRACK);
            break;

        case scactor_radius:
            lexer->AssignFromTokenList(mapactortokens, &actor->radius,
                scactor_radius, false);
            break;

        case scactor_height:
            lexer->AssignFromTokenList(mapactortokens, &actor->baseHeight,
                scactor_height, false);
            break;

        case scactor_centerheight:
            lexer->AssignFromTokenList(mapactortokens, &actor->centerHeight,
                scactor_centerheight, false);
            break;

        case scactor_viewheight:
            lexer->AssignFromTokenList(mapactortokens, &actor->viewHeight,
                scactor_viewheight, false);
            break;

        case scactor_targetID:
            lexer->AssignFromTokenList(mapactortokens, &actor->targetID,
                scactor_targetID, false);
            break;

        case scactor_modelVariant:
            lexer->AssignFromTokenList(mapactortokens, (unsigned int*)&actor->variant,
                scactor_modelVariant, false);
            break;

        case scactor_classFlags:
            lexer->AssignFromTokenList(mapactortokens, &actor->classFlags,
                scactor_classFlags, false);

            if(actor->classFlags & AC_AI)
                AI_Spawn(actor);

            break;

        case scactor_cullDistance:
            lexer->AssignFromTokenList(mapactortokens, &actor->cullDistance,
                scactor_cullDistance, false);
            break;

        case scactor_friction:
            lexer->AssignFromTokenList(mapactortokens, &actor->friction,
                scactor_friction, false);
            break;

        case scactor_mass:
            lexer->AssignFromTokenList(mapactortokens, &actor->mass,
                scactor_mass, false);
            break;

        case scactor_bounceDamp:
            lexer->AssignFromTokenList(mapactortokens, &actor->bounceDamp,
                scactor_bounceDamp, false);
            break;

        case scactor_physics:
            lexer->AssignFromTokenList(mapactortokens, &actor->physics,
                scactor_physics, false);
            break;

        case scactor_rotorSpeed:
            lexer->AssignFromTokenList(mapactortokens, &actor->rotorSpeed,
                scactor_rotorSpeed, false);
            break;

        case scactor_rotorVector:
            lexer->AssignVectorFromTokenList(mapactortokens, actor->rotorVector,
                scactor_rotorVector, false);
            break;

        case scactor_rotorFriction:
            lexer->AssignFromTokenList(mapactortokens, &actor->rotorFriction,
                scactor_rotorFriction, false);
            break;

        case scactor_tickDistance:
            lexer->AssignFromTokenList(mapactortokens, &actor->tickDistance,
                scactor_tickDistance, false);
            break;

        default:
            if(lexer->TokenType() == TK_IDENIFIER)
            {
                parser.Error("Kmap_ParseActor: Unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }

        lexer->Find();
    }
}

//
// Kmap_ParseLevelActorBlock
//

static void Kmap_ParseLevelActorBlock(kexLexer *lexer, gLevel_t *level)
{
    int i;
    int count;

    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    count = level->numActors;

    if(count <= 0)
    {
        lexer->ExpectNextToken(TK_RBRACK);
        return;
    }

    level->actorRoot.next = level->actorRoot.prev = &level->actorRoot;

    for(i = 0; i < count; i++)
    {
        gActor_t *actor = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_ACTOR, NULL);
        Map_AddActor(level, actor);
        Kmap_ParseActor(lexer, actor);

        // TODO - TEMP
        actor->mass         = 1200;
        actor->friction     = 1.0f;
        actor->airfriction  = 1.0f;
        actor->bounceDamp   = 0.0f;

        actor->angles[0] = -actor->angles[0];
        Actor_Setup(actor);
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseStaticActorBlock
//

static void Kmap_ParseStaticActorBlock(kexLexer *lexer, int count, gActor_t **actorList)
{
    int i;

    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    if(count <= 0)
    {
        lexer->ExpectNextToken(TK_RBRACK);
        return;
    }

    *actorList = (gActor_t*)Z_Calloc(sizeof(gActor_t) * count, PU_ACTOR, NULL);

    for(i = 0; i < count; i++)
    {
        gActor_t *actor = &(*actorList)[i];
        
        Kmap_ParseActor(lexer, actor);
        Actor_Setup(actor);
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseGridSectionBlock
//

static void Kmap_ParseGridSectionBlock(kexLexer *lexer)
{
    unsigned int i;

    if(gLevel.numGridBounds == 0)
        return;

    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    gLevel.gridBounds = (gridBounds_t*)Z_Calloc(sizeof(gridBounds_t) *
        gLevel.numGridBounds, PU_LEVEL, 0);

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gridbound = &gLevel.gridBounds[i];

        // read into nested gridBound block
        lexer->ExpectNextToken(TK_LBRACK);
        lexer->Find();

        while(lexer->TokenType() != TK_RBRACK)
        {
            switch(lexer->GetIDForTokenList(mapgridtokens, lexer->Token()))
            {
            case scgridbnd_bounds:
                lexer->ExpectNextToken(TK_EQUAL);
                gridbound->minx = (float)lexer->GetFloat();
                gridbound->minz = (float)lexer->GetFloat();
                gridbound->maxx = (float)lexer->GetFloat();
                gridbound->maxz = (float)lexer->GetFloat();
                break;

            case scgridbnd_statics:
                lexer->ExpectNextToken(TK_LSQBRACK);
                gridbound->numStatics = lexer->GetNumber();
                lexer->ExpectNextToken(TK_RSQBRACK);
                Kmap_ParseStaticActorBlock(lexer, gridbound->numStatics,
                    &gridbound->statics);
                break;

            default:
                if(lexer->TokenType() == TK_IDENIFIER)
                {
                    parser.Error("Kmap_ParseGridSectionBlock: Unknown token: %s\n",
                        lexer->Token());
                }
                break;
            }

            lexer->Find();
        }
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseAreaBlock
//

static void Kmap_ParseAreaBlock(kexLexer *lexer)
{
    unsigned int i;

    if(gLevel.numAreas == 0)
        return;

    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    gLevel.areas = (gArea_t*)Z_Calloc(sizeof(gArea_t) *
        gLevel.numAreas, PU_LEVEL, 0);

    for(i = 0; i < gLevel.numAreas; i++)
    {
        int numComponents = 0;
        gArea_t *area = &gLevel.areas[i];

        area->actorRoot.linkNext = area->actorRoot.linkPrev = &area->actorRoot;

        // read into nested gridBound block
        lexer->ExpectNextToken(TK_LBRACK);
        lexer->Find();

        while(lexer->TokenType() != TK_RBRACK)
        {
            switch(lexer->GetIDForTokenList(mapareatokens, lexer->Token()))
            {
            case scarea_components:
                lexer->ExpectNextToken(TK_LSQBRACK);
                numComponents = lexer->GetNumber();
                lexer->ExpectNextToken(TK_RSQBRACK);
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);
                JParse_Start(lexer, &area->components, numComponents);
                lexer->ExpectNextToken(TK_RBRACK);
                break;

            case scarea_waterplane:
                lexer->AssignFromTokenList(mapareatokens, &area->waterplane,
                    scarea_waterplane, false);
                break;

            case scarea_flags:
                lexer->AssignFromTokenList(mapareatokens, &area->flags,
                    scarea_waterplane, false);
                break;

            case scarea_targetID:
                lexer->AssignFromTokenList(mapareatokens, &area->targetID,
                    scarea_targetID, false);
                break;

            case scarea_triggerSound:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->GetString();
                area->triggerSound = Z_Strdup(lexer->StringToken(), PU_LEVEL, NULL);
                break;

            default:
                if(lexer->TokenType() == TK_IDENIFIER)
                {
                    parser.Error("Kmap_ParseAreaBlock: Unknown token: %s\n",
                        lexer->Token());
                }
                break;
            }

            lexer->Find();
        }

        if(area->components)
        {
            area->iterator = JS_NewPropertyIterator(js_context, area->components);
            JS_AddRoot(js_context, &area->iterator);
        }
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseLevelScript
//

static void Kmap_ParseLevelScript(kexLexer *lexer)
{
    while(lexer->CheckState())
    {
        lexer->Find();

        switch(lexer->TokenType())
        {
        case TK_NONE:
            break;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            {
                switch(lexer->GetIDForTokenList(maptokens, lexer->Token()))
                {
                case scmap_title:
                    lexer->AssignFromTokenList(maptokens, gLevel.name,
                        scmap_title, false);
                    break;

                case scmap_mapID:
                    lexer->AssignFromTokenList(maptokens, (unsigned int*)&gLevel.mapID,
                        scmap_mapID, false);
                    break;

                case scmap_glight_origin:
                    lexer->AssignVectorFromTokenList(mapactortokens,
                        gLevel.worldLightOrigin, scmap_glight_origin, false);
                    break;

                case scmap_glight_color:
                    lexer->AssignVectorFromTokenList(mapactortokens,
                        gLevel.worldLightColor, scmap_glight_color, false);
                    break;

                case scmap_glight_ambience:
                    lexer->AssignVectorFromTokenList(mapactortokens,
                        gLevel.worldLightAmbience, scmap_glight_ambience, false);
                    break;

                case scmap_glight_modelamb:
                    lexer->AssignVectorFromTokenList(mapactortokens,
                        gLevel.worldLightModelAmbience, scmap_glight_modelamb, false);
                    break;

                case scmap_actors:
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    gLevel.numActors = lexer->GetNumber();
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseLevelActorBlock(lexer, &gLevel);
                    break;

                case scmap_gridbounds:
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    gLevel.numGridBounds = lexer->GetNumber();
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseGridSectionBlock(lexer);
                    break;

                case scmap_areas:
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    gLevel.numAreas = lexer->GetNumber();
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseAreaBlock(lexer);
                    break;

                default:
                    if(lexer->TokenType() == TK_IDENIFIER)
                    {
                        parser.Error("Kmap_ParseLevelScript: Unknown token: %s\n",
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
}

//
// Kmap_Load
//

void Kmap_Load(int map)
{
    kexLexer *lexer;
    filepath_t file;

    sprintf(file, "maps/map%02d/map%02d.kmap", map, map);
    common.Printf("Kmap_Load: Loading %s...\n", file);

    if(!(lexer = parser.Open(file)))
        return;

    Kmap_ParseLevelScript(lexer);
    parser.Close();

    common.Printf("Level loaded\n");
    common.Printf("Actors allocated: %ikb\n", Z_TagUsage(PU_ACTOR) >> 10);
    common.Printf("Level data allocated: %ikb\n", Z_TagUsage(PU_LEVEL) >> 10);
}
