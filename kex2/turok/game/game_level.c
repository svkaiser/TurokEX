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
// DESCRIPTION: Level system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "actor.h"
#include "level.h"
#include "kernel.h"
#include "zone.h"
#include "script.h"

kmap_t kmaps[MAXMAPS];

enum
{
    scmap_numactors = 0,
    scmap_numgridbounds,
    scmap_numinstancegroups,
    scmap_actors,
    scmap_gridbounds,
    scmap_instancegroups,
    scmap_end
};

static const sctokens_t maptokens[scmap_end] =
{
    { scmap_numactors,          "numactors"             },
    { scmap_numgridbounds,      "numgridbounds"         },
    { scmap_numinstancegroups,  "numinstancegroups"     },
    { scmap_actors,             "actors"                },
    { scmap_gridbounds,         "gridbounds"            },
    { scmap_instancegroups,     "instancegroups"        }
};

enum
{
    scactor_model = 0,
    scactor_texturealt,
    scactor_skin,
    scactor_targetid,
    scactor_target,
    scactor_variant,
    scactor_leaf,
    scactor_angle,
    scactor_position,
    scactor_scale,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end] =
{
    { scactor_model,        "model"         },
    { scactor_texturealt,   "texture_alt"   },
    { scactor_skin,         "skin"          },
    { scactor_targetid,     "target_id"     },
    { scactor_target,       "target"        },
    { scactor_variant,      "variant"       },
    { scactor_leaf,         "leaf"          },
    { scactor_angle,        "angle"         },
    { scactor_position,     "position"      },
    { scactor_scale,        "scale"         }
};

enum
{
    scinst_position = 0,
    scinst_scale,
    scinst_bounds,
    scinst_model,
    scinst_angle,
    scinst_texturealt,
    scinst_skin,
    scinst_targetid,
    scinst_target,
    scinst_variant,
    scinst_boundize,
    scinst_instances,
    scinst_numinstances,
    scinst_staticinstnaces,
    scinst_numstaticinstances,
    scinst_end
};

static const sctokens_t insttokens[scinst_end] =
{
    { scinst_position,              "position"              },
    { scinst_scale,                 "scale"                 },
    { scinst_bounds,                "bounds"                },
    { scinst_model,                 "model"                 },
    { scinst_angle,                 "angle"                 },
    { scinst_texturealt,            "texture_alt"           },
    { scinst_skin,                  "skin"                  },
    { scinst_targetid,              "target_id"             },
    { scinst_target,                "target"                },
    { scinst_variant,               "variant"               },
    { scinst_boundize,              "boundsize"             },
    { scinst_staticinstnaces,       "staticinstances"       },
    { scinst_numstaticinstances,    "numstaticinstances"    },
    { scinst_instances,             "instances"             },
    { scinst_numinstances,          "numinstances"          }
};

//
// Map_ParseActorBlock
//

static void Map_ParseActorBlock(kmap_t *map, scparser_t *parser)
{
    unsigned int i;

    if(map->numactors <= 0)
    {
        return;
    }

    map->actors = (mapactor_t*)Z_Calloc(sizeof(mapactor_t) * map->numactors, PU_LEVEL, 0);

    for(i = 0; i < map->numactors; i++)
    {
        mapactor_t *actor = &map->actors[i];

        // read into nested actor block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(mapactortokens, parser->token))
            {
            case scactor_model:
                SC_AssignString(mapactortokens, actor->mdlpath,
                    scactor_model, parser, false);
                break;

            case scactor_texturealt:
                SC_AssignInteger(mapactortokens, &actor->textureindex,
                    scactor_texturealt, parser, false);
                break;

            case scactor_skin:
                SC_AssignInteger(mapactortokens, &actor->skin,
                    scactor_skin, parser, false);
                break;

            case scactor_targetid:
                SC_AssignInteger(mapactortokens, &actor->tid,
                    scactor_targetid, parser, false);
                break;

            case scactor_target:
                SC_AssignInteger(mapactortokens, &actor->target,
                    scactor_target, parser, false);
                break;

            case scactor_variant:
                SC_AssignInteger(mapactortokens, &actor->variant,
                    scactor_variant, parser, false);
                break;

            case scactor_leaf:
                SC_AssignInteger(mapactortokens, &actor->leafindex,
                    scactor_leaf, parser, false);
                break;

            case scactor_angle:
                SC_AssignFloat(mapactortokens, &actor->yaw,
                    scactor_angle, parser, false);
                break;

            case scactor_position:
                SC_AssignVector(mapactortokens, &actor->origin,
                    scactor_position, parser, false);
                break;

            case scactor_scale:
                SC_AssignVector(mapactortokens, &actor->scale,
                    scactor_scale, parser, false);
                break;

            default:
                break;
            }

            SC_Find();
        }
    }
}

//
// Map_ParseGridSectionBlock
//

static void Map_ParseGridSectionBlock(kmap_t *map, scparser_t *parser)
{
    unsigned int i;

    if(map->numgridbounds <= 0)
    {
        return;
    }

    map->gridbounds = (mapgrid_t*)Z_Calloc(sizeof(mapgrid_t) * map->numgridbounds, PU_LEVEL, 0);

    for(i = 0; i < map->numgridbounds; i++)
    {
        mapgrid_t *gridbound = &map->gridbounds[i];

        gridbound->minx = (float)SC_GetFloat();
        gridbound->minz = (float)SC_GetFloat();
        gridbound->maxx = (float)SC_GetFloat();
        gridbound->maxz = (float)SC_GetFloat();
    }
}

//
// Map_ParseScript
//

static void Map_ParseScript(kmap_t *map, scparser_t *parser)
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
                // there are three main blocks for a kmesh file
                switch(SC_GetIDForToken(maptokens, parser->token))
                {
                case scmap_numactors:
                    SC_AssignInteger(maptokens, &map->numactors,
                        scmap_numactors, parser, false);
                    break;

                case scmap_numgridbounds:
                    SC_AssignInteger(maptokens, &map->numgridbounds,
                        scmap_numgridbounds, parser, false);
                    break;

                case scmap_numinstancegroups:
                    SC_AssignInteger(maptokens, &map->numinstancegroups,
                        scmap_numinstancegroups, parser, false);
                    break;

                case scmap_actors:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    Map_ParseActorBlock(map, parser);
                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                case scmap_gridbounds:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    Map_ParseGridSectionBlock(map, parser);
                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                case scmap_instancegroups:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);

                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                default:
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
// Map_Load
//

kmap_t *Map_Load(int map)
{
    scparser_t *parser;
    kmap_t *kmap;

    if(map >= MAXMAPS)
    {
        return NULL;
    }

    if(!(parser = SC_Open(kva("maps/map%02d/map%02d.kmap", map, map))))
    {
        return NULL;
    }

    kmap = &kmaps[map];
    kmap->actorlist.next = kmap->actorlist.prev = &kmap->actorlist;

    Map_ParseScript(kmap, parser);

    // we're done with the file
    SC_Close();

    return kmap;
}

//
// Map_Init
//

void Map_Init(void)
{
    memset(kmaps, 0, sizeof(kmap_t) * MAXMAPS);
}