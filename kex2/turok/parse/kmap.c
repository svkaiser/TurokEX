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

static void Kmap_ParseActor(scparser_t *parser, gActor_t *actor)
{
    int numComponents = 0;
    int j;

    Vec_Set3(actor->scale, 1, 1, 1);

    // read into nested actor block
    SC_ExpectNextToken(TK_LBRACK);
    SC_Find();

    while(parser->tokentype != TK_RBRACK)
    {
        switch(SC_GetIDForToken(mapactortokens, parser->token))
        {
        case scactor_name:
            SC_AssignString(mapactortokens, actor->name,
                scactor_name, parser, false);
            break;

        case scactor_mesh:
            SC_ExpectNextToken(TK_EQUAL);
            SC_GetString();
            Actor_UpdateModel(actor, parser->stringToken);
            break;

        case scactor_bounds:
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);
            actor->bbox.min[0] = (float)SC_GetFloat();
            actor->bbox.min[1] = (float)SC_GetFloat();
            actor->bbox.min[2] = (float)SC_GetFloat();
            actor->bbox.max[0] = (float)SC_GetFloat();
            actor->bbox.max[1] = (float)SC_GetFloat();
            actor->bbox.max[2] = (float)SC_GetFloat();
            Vec_Copy3(actor->baseBBox.min, actor->bbox.min);
            Vec_Copy3(actor->baseBBox.max, actor->bbox.max);
            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_textureSwaps:
            if(actor->model == NULL)
                SC_Error("Kmap_ParseActor: Attempted to parse \"textureSwaps\" token while model is null\n");

            SC_ExpectNextToken(TK_EQUAL);
            // texture swap block
            SC_ExpectNextToken(TK_LBRACK);
            for(j = 0; j < (int)actor->model->numnodes; j++)
            {
                unsigned int k;
                mdlnode_t *node;

                node = &actor->model->nodes[j];

                // node block
                SC_ExpectNextToken(TK_LBRACK);
                for(k = 0; k < node->nummeshes; k++)
                {
                    unsigned int l;
                    mdlmesh_t *mesh;

                    mesh = &node->meshes[k];

                    // mesh block
                    SC_ExpectNextToken(TK_LBRACK);
                    for(l = 0; l < mesh->numsections; l++)
                    {
                        // parse sections
                        SC_GetString();
                        actor->textureSwaps[j][k][l] = Z_Strdup(parser->stringToken, PU_ACTOR, NULL);
                    }
                    // end mesh block
                    SC_ExpectNextToken(TK_RBRACK);
                }
                // end node block
                SC_ExpectNextToken(TK_RBRACK);
            }
            // end texture swap block
            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_components:
            SC_ExpectNextToken(TK_LSQBRACK);
            numComponents = SC_GetNumber();
            SC_ExpectNextToken(TK_RSQBRACK);
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);
            JParse_Start(parser, &actor->components, numComponents);
            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_bCollision:
            SC_AssignInteger(mapactortokens, &actor->bCollision,
                scactor_bCollision, parser, false);
            break;

        case scactor_bHidden:
            SC_AssignInteger(mapactortokens, &actor->bHidden,
                scactor_bHidden, parser, false);
            break;

        case scactor_bStatic:
            SC_AssignInteger(mapactortokens, &actor->bStatic,
                scactor_bStatic, parser, false);
            break;

        case scactor_bTouch:
            SC_AssignInteger(mapactortokens, &actor->bTouch,
                scactor_bTouch, parser, false);
            break;

        case scactor_bOrientOnSlope:
            SC_AssignInteger(mapactortokens, &actor->bOrientOnSlope,
                scactor_bOrientOnSlope, parser, false);
            break;

        case scactor_bRotor:
            SC_AssignInteger(mapactortokens, &actor->bRotor,
                scactor_bRotor, parser, false);
            break;

        case scactor_plane:
            SC_AssignInteger(mapactortokens, &actor->plane,
                scactor_plane, parser, false);
            break;

        case scactor_origin:
            SC_AssignVector(mapactortokens, actor->origin,
                scactor_origin, parser, false);
            break;

        case scactor_scale:
            SC_AssignVector(mapactortokens, actor->scale,
                scactor_scale, parser, false);
            break;

        case scactor_angles:
            SC_AssignVector(mapactortokens, actor->angles,
                scactor_angles, parser, false);
            break;

        case scactor_rotation:
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);

            actor->rotation[0] = (float)SC_GetFloat();
            actor->rotation[1] = (float)SC_GetFloat();
            actor->rotation[2] = (float)SC_GetFloat();
            actor->rotation[3] = (float)SC_GetFloat();

            SC_ExpectNextToken(TK_RBRACK);
            break;

        case scactor_radius:
            SC_AssignFloat(mapactortokens, &actor->radius,
                scactor_radius, parser, false);
            break;

        case scactor_height:
            SC_AssignFloat(mapactortokens, &actor->baseHeight,
                scactor_height, parser, false);
            break;

        case scactor_centerheight:
            SC_AssignFloat(mapactortokens, &actor->centerHeight,
                scactor_centerheight, parser, false);
            break;

        case scactor_viewheight:
            SC_AssignFloat(mapactortokens, &actor->viewHeight,
                scactor_viewheight, parser, false);
            break;

        case scactor_targetID:
            SC_AssignInteger(mapactortokens, &actor->targetID,
                scactor_targetID, parser, false);
            break;

        case scactor_modelVariant:
            SC_AssignInteger(mapactortokens, &actor->variant,
                scactor_modelVariant, parser, false);
            break;

        case scactor_classFlags:
            SC_AssignInteger(mapactortokens, &actor->classFlags,
                scactor_classFlags, parser, false);

            if(actor->classFlags & AC_AI)
                AI_Spawn(actor);

            break;

        case scactor_cullDistance:
            SC_AssignFloat(mapactortokens, &actor->cullDistance,
                scactor_cullDistance, parser, false);
            break;

        case scactor_friction:
            SC_AssignFloat(mapactortokens, &actor->friction,
                scactor_friction, parser, false);
            break;

        case scactor_mass:
            SC_AssignFloat(mapactortokens, &actor->mass,
                scactor_mass, parser, false);
            break;

        case scactor_bounceDamp:
            SC_AssignFloat(mapactortokens, &actor->bounceDamp,
                scactor_bounceDamp, parser, false);
            break;

        case scactor_physics:
            SC_AssignInteger(mapactortokens, &actor->physics,
                scactor_physics, parser, false);
            break;

        case scactor_rotorSpeed:
            SC_AssignFloat(mapactortokens, &actor->rotorSpeed,
                scactor_rotorSpeed, parser, false);
            break;

        case scactor_rotorVector:
            SC_AssignVector(mapactortokens, actor->rotorVector,
                scactor_rotorVector, parser, false);
            break;

        case scactor_rotorFriction:
            SC_AssignFloat(mapactortokens, &actor->rotorFriction,
                scactor_rotorFriction, parser, false);
            break;

        case scactor_tickDistance:
            SC_AssignFloat(mapactortokens, &actor->tickDistance,
                scactor_tickDistance, parser, false);
            break;

        default:
            if(parser->tokentype == TK_IDENIFIER)
            {
                SC_Error("Kmap_ParseActor: Unknown token: %s\n",
                    parser->token);
            }
            break;
        }

        SC_Find();
    }
}

//
// Kmap_ParseLevelActorBlock
//

static void Kmap_ParseLevelActorBlock(scparser_t *parser, gLevel_t *level)
{
    int i;
    int count;

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    count = level->numActors;

    if(count <= 0)
    {
        SC_ExpectNextToken(TK_RBRACK);
        return;
    }

    level->actorRoot.next = level->actorRoot.prev = &level->actorRoot;

    for(i = 0; i < count; i++)
    {
        gActor_t *actor = (gActor_t*)Z_Calloc(sizeof(gActor_t), PU_ACTOR, NULL);
        Map_AddActor(level, actor);
        Kmap_ParseActor(parser, actor);

        // TODO - TEMP
        actor->mass         = 1200;
        actor->friction     = 1.0f;
        actor->airfriction  = 1.0f;
        actor->bounceDamp   = 0.0f;

        actor->angles[0] = -actor->angles[0];
        Actor_Setup(actor);
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseStaticActorBlock
//

static void Kmap_ParseStaticActorBlock(scparser_t *parser, int count, gActor_t **actorList)
{
    int i;

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    if(count <= 0)
    {
        SC_ExpectNextToken(TK_RBRACK);
        return;
    }

    *actorList = (gActor_t*)Z_Calloc(sizeof(gActor_t) * count, PU_ACTOR, NULL);

    for(i = 0; i < count; i++)
    {
        gActor_t *actor = &(*actorList)[i];
        
        Kmap_ParseActor(parser, actor);
        Actor_Setup(actor);
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseGridSectionBlock
//

static void Kmap_ParseGridSectionBlock(scparser_t *parser)
{
    unsigned int i;

    if(gLevel.numGridBounds == 0)
        return;

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    gLevel.gridBounds = (gridBounds_t*)Z_Calloc(sizeof(gridBounds_t) *
        gLevel.numGridBounds, PU_LEVEL, 0);

    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gridbound = &gLevel.gridBounds[i];

        // read into nested gridBound block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(mapgridtokens, parser->token))
            {
            case scgridbnd_bounds:
                SC_ExpectNextToken(TK_EQUAL);
                gridbound->minx = (float)SC_GetFloat();
                gridbound->minz = (float)SC_GetFloat();
                gridbound->maxx = (float)SC_GetFloat();
                gridbound->maxz = (float)SC_GetFloat();
                break;

            case scgridbnd_statics:
                SC_ExpectNextToken(TK_LSQBRACK);
                gridbound->numStatics = SC_GetNumber();
                SC_ExpectNextToken(TK_RSQBRACK);
                Kmap_ParseStaticActorBlock(parser, gridbound->numStatics,
                    &gridbound->statics);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Kmap_ParseGridSectionBlock: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseAreaBlock
//

static void Kmap_ParseAreaBlock(scparser_t *parser)
{
    unsigned int i;

    if(gLevel.numAreas == 0)
        return;

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    gLevel.areas = (gArea_t*)Z_Calloc(sizeof(gArea_t) *
        gLevel.numAreas, PU_LEVEL, 0);

    for(i = 0; i < gLevel.numAreas; i++)
    {
        int numComponents = 0;
        gArea_t *area = &gLevel.areas[i];

        area->actorRoot.linkNext = area->actorRoot.linkPrev = &area->actorRoot;

        // read into nested gridBound block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(mapareatokens, parser->token))
            {
            case scarea_components:
                SC_ExpectNextToken(TK_LSQBRACK);
                numComponents = SC_GetNumber();
                SC_ExpectNextToken(TK_RSQBRACK);
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);
                JParse_Start(parser, &area->components, numComponents);
                SC_ExpectNextToken(TK_RBRACK);
                break;

            case scarea_waterplane:
                SC_AssignFloat(mapareatokens, &area->waterplane,
                    scarea_waterplane, parser, false);
                break;

            case scarea_flags:
                SC_AssignInteger(mapareatokens, &area->flags,
                    scarea_waterplane, parser, false);
                break;

            case scarea_targetID:
                SC_AssignInteger(mapareatokens, &area->targetID,
                    scarea_targetID, parser, false);
                break;

            case scarea_triggerSound:
                SC_ExpectNextToken(TK_EQUAL);
                SC_GetString();
                area->triggerSound = Z_Strdup(parser->stringToken, PU_LEVEL, NULL);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Kmap_ParseAreaBlock: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }

        if(area->components)
        {
            area->iterator = JS_NewPropertyIterator(js_context, area->components);
            JS_AddRoot(js_context, &area->iterator);
        }
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Kmap_ParseLevelScript
//

static void Kmap_ParseLevelScript(scparser_t *parser)
{
    while(SC_CheckScriptState())
    {
        SC_Find();

        switch(parser->tokentype)
        {
        case TK_NONE:
            break;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            {
                switch(SC_GetIDForToken(maptokens, parser->token))
                {
                case scmap_title:
                    SC_AssignString(maptokens, gLevel.name,
                        scmap_title, parser, false);
                    break;

                case scmap_mapID:
                    SC_AssignInteger(maptokens, &gLevel.mapID,
                        scmap_mapID, parser, false);
                    break;

                case scmap_glight_origin:
                    SC_AssignVector(mapactortokens, gLevel.worldLightOrigin,
                        scmap_glight_origin, parser, false);
                    break;

                case scmap_glight_color:
                    SC_AssignVector(mapactortokens, gLevel.worldLightColor,
                        scmap_glight_color, parser, false);
                    break;

                case scmap_glight_ambience:
                    SC_AssignVector(mapactortokens, gLevel.worldLightAmbience,
                        scmap_glight_ambience, parser, false);
                    break;

                case scmap_glight_modelamb:
                    SC_AssignVector(mapactortokens, gLevel.worldLightModelAmbience,
                        scmap_glight_modelamb, parser, false);
                    break;

                case scmap_actors:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numActors = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseLevelActorBlock(parser, &gLevel);
                    break;

                case scmap_gridbounds:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numGridBounds = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseGridSectionBlock(parser);
                    break;

                case scmap_areas:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numAreas = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Kmap_ParseAreaBlock(parser);
                    break;

                default:
                    if(parser->tokentype == TK_IDENIFIER)
                    {
                        SC_Error("Kmap_ParseLevelScript: Unknown token: %s\n",
                            parser->token);
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
    scparser_t *parser;
    filepath_t file;

    sprintf(file, "maps/map%02d/map%02d.kmap", map, map);
    Com_Printf("Kmap_Load: Loading %s...\n", file);

    if(!(parser = SC_Open(file)))
        return;

    Kmap_ParseLevelScript(parser);
    SC_Close();

    Com_Printf("Level loaded\n");
    Com_Printf("Actors allocated: %ikb\n", Z_TagUsage(PU_ACTOR) >> 10);
    Com_Printf("Level data allocated: %ikb\n", Z_TagUsage(PU_LEVEL) >> 10);
}
