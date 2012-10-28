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
#include "zone.h"
#include "script.h"
#include "mathlib.h"
#include "client.h"

kmap_t kmaps[MAXMAPS];
kmap_t *g_currentmap = NULL;

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

static const sctokens_t maptokens[scmap_end+1] =
{
    { scmap_numactors,          "numactors"             },
    { scmap_numgridbounds,      "numgridbounds"         },
    { scmap_numinstancegroups,  "numinstancegroups"     },
    { scmap_actors,             "actors"                },
    { scmap_gridbounds,         "gridbounds"            },
    { scmap_instancegroups,     "instancegroups"        },
    { -1,                       NULL                    }
};

enum
{
    scactor_model = 0,
    scactor_skin,
    scactor_targetid,
    scactor_target,
    scactor_variant,
    scactor_leaf,
    scactor_angle,
    scactor_position,
    scactor_scale,
    scactor_type,
    scactor_flags,
    scactor_meleerange,
    scactor_health,
    scactor_width,
    scactor_height,
    scactor_viewheight,
    scactor_blockflag,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] =
{
    { scactor_model,        "model"         },
    { scactor_skin,         "skin"          },
    { scactor_targetid,     "target_id"     },
    { scactor_target,       "target"        },
    { scactor_variant,      "variant"       },
    { scactor_leaf,         "leaf"          },
    { scactor_angle,        "angle"         },
    { scactor_position,     "position"      },
    { scactor_scale,        "scale"         },
    { scactor_type,         "type"          },
    { scactor_flags,        "flags"         },
    { scactor_meleerange,   "meleerange"    },
    { scactor_health,       "health"        },
    { scactor_width,        "width"         },
    { scactor_height,       "height"        },
    { scactor_viewheight,   "viewheight"    },
    { scactor_blockflag,    "blockflag"     },
    { -1,                   NULL            }
};

enum
{
    scinst_position = 0,
    scinst_scale,
    scinst_bounds,
    scinst_model,
    scinst_angle,
    scinst_skin,
    scinst_targetid,
    scinst_target,
    scinst_variant,
    scinst_boundize,
    scinst_radius,
    scinst_flags,
    scinst_plane,
    scinst_blockflag,
    scinst_height,
    scinst_viewheight,
    scinst_type,
    scinst_overrides,
    scinst_instances,
    scinst_numinstances,
    scinst_staticinstnaces,
    scinst_numstaticinstances,
    scinst_end
};

static const sctokens_t insttokens[scinst_end+1] =
{
    { scinst_position,              "position"              },
    { scinst_scale,                 "scale"                 },
    { scinst_bounds,                "bounds"                },
    { scinst_model,                 "model"                 },
    { scinst_angle,                 "angle"                 },
    { scinst_skin,                  "skin"                  },
    { scinst_targetid,              "target_id"             },
    { scinst_target,                "target"                },
    { scinst_variant,               "variant"               },
    { scinst_boundize,              "boundsize"             },
    { scinst_radius,                "radius"                },
    { scinst_flags,                 "flags"                 },
    { scinst_plane,                 "leaf"                  },
    { scinst_blockflag,             "blockflag"             },
    { scinst_height,                "height"                },
    { scinst_viewheight,            "viewheight"            },
    { scinst_type,                  "type"                  },
    { scinst_overrides,             "overrides"             },
    { scinst_staticinstnaces,       "staticinstances"       },
    { scinst_numstaticinstances,    "numstaticinstances"    },
    { scinst_instances,             "instances"             },
    { scinst_numinstances,          "numinstances"          },
    { -1,                           NULL                    }
};

enum
{
    scnav_numareas = 0,
    scnav_numpoints,
    scnav_numleafs,
    scnav_numzonebounds,
    scnav_areas,
    scnav_points,
    scnav_leafs,
    scnav_zonebounds,
    scnav_end
};

static const sctokens_t navtokens[scnav_end+1] =
{
    { scnav_numareas,       "numareas"      },
    { scnav_numpoints,      "numpoints"     },
    { scnav_numleafs,       "numleafs"      },
    { scnav_numzonebounds,  "numzonebounds" },
    { scnav_areas,          "areas"         },
    { scnav_points,         "points"        },
    { scnav_leafs,          "leafs"         },
    { scnav_zonebounds,     "zonebounds"    },
    { -1,                   NULL            }
};

enum
{
    scarea_fogcolor = 0,
    scarea_waterheight,
    scarea_flags,
    scarea_args,
    scarea_fogz_far,
    scarea_fogz_near,
    scarea_end
};

static const sctokens_t areatokens[scarea_end+1] =
{
    { scarea_fogcolor,      "fogcolor"      },
    { scarea_waterheight,   "waterheight"   },
    { scarea_flags,         "flags"         },
    { scarea_args,          "args"          },
    { scarea_fogz_far,      "fogz_far"      },
    { scarea_fogz_near,     "fogz_near"     },
    { -1,                   NULL            }
};

//
// Map_ParseActorBlock
//

static void Map_ParseActorBlock(kmap_t *map, scparser_t *parser)
{
    unsigned int i;

    if(map->nummapactors <= 0)
    {
        return;
    }

    for(i = 0; i < map->nummapactors; i++)
    {
        actor_t *actor = G_SpawnActor(); // TODO handle test case if returned NULL

        // read into nested actor block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(mapactortokens, parser->token))
            {
            case scactor_model:
                SC_AssignString(mapactortokens, actor->object.mdlpath,
                    scactor_model, parser, false);
                break;

            case scactor_skin:
                SC_AssignWord(mapactortokens, &actor->skin,
                    scactor_skin, parser, false);
                break;

            case scactor_targetid:
                SC_AssignWord(mapactortokens, &actor->object.tid,
                    scactor_targetid, parser, false);
                break;

            case scactor_target:
                SC_AssignWord(mapactortokens, &actor->object.target,
                    scactor_target, parser, false);
                break;

            case scactor_variant:
                SC_AssignWord(mapactortokens, &actor->object.variant,
                    scactor_variant, parser, false);
                break;

            case scactor_leaf:
                SC_AssignWord(mapactortokens, &actor->object.plane_id,
                    scactor_leaf, parser, false);
                break;

            case scactor_angle:
                SC_AssignFloat(mapactortokens, &actor->yaw,
                    scactor_angle, parser, false);

                actor->yaw = actor->yaw * M_RAD;
                break;

            case scactor_position:
                SC_AssignVector(mapactortokens, actor->origin,
                    scactor_position, parser, false);
                break;

            case scactor_scale:
                SC_AssignVector(mapactortokens, actor->object.scale,
                    scactor_scale, parser, false);
                break;

            case scactor_type:
                SC_AssignWord(mapactortokens, &actor->object.type,
                    scactor_type, parser, false);
                break;

            case scactor_flags:
                SC_AssignInteger(mapactortokens, (int*)&actor->object.flags,
                    scactor_flags, parser, false);
                break;

            case scactor_meleerange:
                SC_AssignFloat(mapactortokens, &actor->object.centerheight,
                    scactor_meleerange, parser, false);
                break;

            case scactor_health:
                SC_AssignInteger(mapactortokens, &actor->health,
                    scactor_health, parser, false);
                break;

            case scactor_width:
                SC_AssignFloat(mapactortokens, &actor->object.width,
                    scactor_width, parser, false);
                break;

            case scactor_height:
                SC_AssignFloat(mapactortokens, &actor->object.height,
                    scactor_height, parser, false);
                break;

            case scactor_viewheight:
                SC_AssignFloat(mapactortokens, &actor->object.viewheight,
                    scactor_viewheight, parser, false);
                break;

            case scactor_blockflag:
                SC_AssignInteger(insttokens, (int*)&actor->object.blockflag,
                    scactor_blockflag, parser, false);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    Com_DPrintf("Map_ParseActorBlock: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }

        Vec_SetQuaternion(actor->object.rotation, actor->yaw, 1, 0, 0);
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
// Map_ParseObjectBlock
//

static void Map_ParseObjectBlock(instance_t *instances, scparser_t *parser, kbool nonstatic)
{
    unsigned int i;
    unsigned int count;

    if(nonstatic)
    {
        if(instances->numspecials <= 0)
        {
            return;
        }

        instances->specials = (object_t*)Z_Calloc(sizeof(object_t) *
            instances->numspecials, PU_LEVEL, 0);

        count = instances->numspecials;
    }
    else
    {
        if(instances->numstatics <= 0)
        {
            return;
        }

        instances->statics = (object_t*)Z_Calloc(sizeof(object_t) *
            instances->numstatics, PU_LEVEL, 0);

        count = instances->numstatics;
    }

    for(i = 0; i < count; i++)
    {
        object_t *obj;

        if(nonstatic)
        {
            obj = &instances->specials[i];
        }
        else
        {
            obj = &instances->statics[i];
        }

        // read into nested static instance block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(insttokens, parser->token))
            {
            case scinst_position:
                SC_AssignVector(insttokens, obj->origin,
                    scinst_position, parser, false);
                break;

            case scinst_scale:
                SC_AssignVector(insttokens, obj->scale,
                    scinst_scale, parser, false);
                break;

            case scinst_bounds:
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);

                obj->box.min[0] = (float)SC_GetFloat();
                obj->box.min[1] = (float)SC_GetFloat();
                obj->box.min[2] = (float)SC_GetFloat();
                obj->box.max[0] = (float)SC_GetFloat();
                obj->box.max[1] = (float)SC_GetFloat();
                obj->box.max[2] = (float)SC_GetFloat();

                SC_ExpectNextToken(TK_RBRACK);
                break;

            case scinst_model:
                SC_AssignString(insttokens, obj->mdlpath,
                    scinst_model, parser, false);
                break;

            case scinst_angle:
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);

                obj->rotation[0] = (float)SC_GetFloat();
                obj->rotation[1] = (float)SC_GetFloat();
                obj->rotation[2] = (float)SC_GetFloat();
                obj->rotation[3] = (float)SC_GetFloat();

                Vec_Normalize4(obj->rotation);

                SC_ExpectNextToken(TK_RBRACK);
                break;

            case scinst_overrides:
                {
                    int count = 0;

                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);

                    while(1)
                    {
                        obj->textureswaps = (char**)Z_Realloc(obj->textureswaps,
                            sizeof(char*) * (count+1), PU_LEVEL, 0);

                        SC_ExpectNextToken(TK_LBRACK);
                        SC_GetString();
                        obj->textureswaps[count] = Z_Strdup(sc_stringbuffer, PU_LEVEL, 0);
                        SC_ExpectNextToken(TK_RBRACK);

                        SC_Find();

                        if(sc_parser->tokentype != TK_COMMA)
                        {
                            SC_Rewind();
                            break;
                        }

                        count++;
                    }

                    SC_ExpectNextToken(TK_RBRACK);
                }
                break;

            case scinst_boundize:
                {
                    float size;

                    SC_AssignFloat(insttokens, &size,
                        scinst_boundize, parser, false);

                    obj->box.min[0] = -size;
                    obj->box.min[1] = -size;
                    obj->box.min[2] = -size;
                    obj->box.max[0] = size;
                    obj->box.max[1] = size;
                    obj->box.max[2] = size;
                }
                break;

            case scinst_radius:
                SC_AssignFloat(insttokens, &obj->width,
                    scinst_radius, parser, false);
                break;

            case scinst_height:
                SC_AssignFloat(insttokens, &obj->height,
                    scinst_height, parser, false);
                break;

            case scinst_viewheight:
                SC_AssignFloat(insttokens, &obj->viewheight,
                    scinst_viewheight, parser, false);
                break;

            case scinst_flags:
                SC_AssignInteger(insttokens, (int*)&obj->flags,
                    scinst_flags, parser, false);
                break;

            case scinst_plane:
                SC_AssignWord(insttokens, &obj->plane_id,
                    scinst_plane, parser, false);
                break;

            case scinst_blockflag:
                SC_AssignInteger(insttokens, (int*)&obj->blockflag,
                    scinst_blockflag, parser, false);
                break;

            case scinst_type:
                SC_AssignWord(insttokens, &obj->type,
                    scinst_type, parser, false);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    Com_DPrintf("Map_ParseObjectBlock: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }

        obj->box.min[0] += obj->origin[0];
        obj->box.min[1] += obj->origin[1];
        obj->box.min[2] += obj->origin[2];
        obj->box.max[0] += obj->origin[0];
        obj->box.max[1] += obj->origin[1];
        obj->box.max[2] += obj->origin[2];

        if(nonstatic)
        {
            Vec_Set3(obj->scale, 0.35f, 0.35f, 0.35f);
            Vec_Set4(obj->rotation, 0, 0, 0, 1);
        }

        Mtx_ApplyRotation(obj->rotation, obj->matrix);
        Mtx_Scale(obj->matrix, obj->scale[0], obj->scale[1], obj->scale[2]);
        Mtx_AddTranslation(obj->matrix, obj->origin[0], obj->origin[1], obj->origin[2]);
    }
}

//
// Map_ParseInstanceBlock
//

static void Map_ParseInstanceBlock(kmap_t *map, scparser_t *parser)
{
    unsigned int i;

    if(map->numinstances <= 0)
    {
        return;
    }

    map->instances = (instance_t*)Z_Calloc(sizeof(instance_t) *
        map->numinstances, PU_LEVEL, 0);

    for(i = 0; i < map->numinstances; i++)
    {
        instance_t *instances = &map->instances[i];

        SC_ExpectNextToken(TK_LBRACK);

        SC_ExpectTokenID(insttokens, scinst_staticinstnaces, parser);
        SC_ExpectNextToken(TK_EQUAL);
        SC_ExpectNextToken(TK_LBRACK);
        SC_AssignInteger(insttokens, &instances->numstatics,
            scinst_numstaticinstances, parser, true);
        Map_ParseObjectBlock(instances, parser, false);
        SC_ExpectNextToken(TK_RBRACK);

        SC_ExpectTokenID(insttokens, scinst_instances, parser);
        SC_ExpectNextToken(TK_EQUAL);
        SC_ExpectNextToken(TK_LBRACK);
        SC_AssignInteger(insttokens, &instances->numspecials,
            scinst_numinstances, parser, true);
        Map_ParseObjectBlock(instances, parser, true);
        SC_ExpectNextToken(TK_RBRACK);

        SC_ExpectNextToken(TK_RBRACK);
    }
}

//
// Map_ParseLevelScript
//

static void Map_ParseLevelScript(kmap_t *map, scparser_t *parser)
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
                case scmap_numactors:
                    SC_AssignInteger(maptokens, &map->nummapactors,
                        scmap_numactors, parser, false);
                    break;

                case scmap_numgridbounds:
                    SC_AssignInteger(maptokens, &map->numgridbounds,
                        scmap_numgridbounds, parser, false);
                    break;

                case scmap_numinstancegroups:
                    SC_AssignInteger(maptokens, &map->numinstances,
                        scmap_numinstancegroups, parser, false);
                    break;

                case scmap_actors:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    Map_ParseActorBlock(map, parser);
                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                /*case scmap_gridbounds:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    Map_ParseGridSectionBlock(map, parser);
                    SC_ExpectNextToken(TK_RBRACK);
                    break;*/

                case scmap_instancegroups:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    Map_ParseInstanceBlock(map, parser);
                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                default:
                    if(parser->tokentype == TK_IDENIFIER)
                    {
                        Com_DPrintf("Map_ParseLevelScript: Unknown token: %s\n",
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
// Map_ParseAreaBlock
//

static void Map_ParseAreaBlock(kmap_t *map, scparser_t *parser)
{
    unsigned int i;

    if(map->numareas <= 0)
    {
        SC_Error("numareas is 0 or hasn't been set yet");
        return;
    }

    map->areas = (area_t*)Z_Calloc(sizeof(area_t) *
        map->numareas, PU_LEVEL, 0);

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < map->numareas; i++)
    {
        // read into nested area block
        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(areatokens, parser->token))
            {
            case scarea_fogcolor:
                {
                    byte r, g, b, a;

                    SC_ExpectNextToken(TK_EQUAL);
                    r = SC_GetNumber();
                    g = SC_GetNumber();
                    b = SC_GetNumber();
                    a = SC_GetNumber();

                    map->areas[i].fog_color = RGBA(r, g, b, a);
                }
                break;

            case scarea_waterheight:
                SC_AssignFloat(areatokens, &map->areas[i].waterplane,
                    scarea_waterheight, parser, false);
                break;

            case scarea_flags:
                SC_AssignInteger(areatokens, &map->areas[i].flags,
                    scarea_flags, parser, false);
                break;

            case scarea_args:
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);

                map->areas[i].args[0] = SC_GetNumber();
                map->areas[i].args[1] = SC_GetNumber();
                map->areas[i].args[2] = SC_GetNumber();
                map->areas[i].args[3] = SC_GetNumber();
                map->areas[i].args[4] = SC_GetNumber();
                map->areas[i].args[5] = SC_GetNumber();

                SC_ExpectNextToken(TK_RBRACK);
                break;
                
            case scarea_fogz_far:
                SC_AssignFloat(areatokens, &map->areas[i].fog_far,
                    scarea_fogz_far, parser, false);
                break;

            case scarea_fogz_near:
                SC_AssignFloat(areatokens, &map->areas[i].fog_near,
                    scarea_fogz_near, parser, false);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    Com_DPrintf("Map_ParseAreaBlock: Unknown token: %s\n",
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
// Map_ParseCollisionPlanes
//

static void Map_ParseCollisionPlanes(kmap_t *map, scparser_t *parser,
                                     unsigned int numpoints, float *points)
{
    unsigned int i;

    if(map->numplanes <= 0)
    {
        SC_Error("numplanes is 0 or hasn't been set yet");
        return;
    }

    if(numpoints <= 0)
    {
        SC_Error("numpoints is 0 or hasn't been set yet");
        return;
    }

    if(points == NULL)
    {
        SC_Error("points hasn't been allocated yet");
        return;
    }

    map->planes = (plane_t*)Z_Calloc(sizeof(plane_t) *
        map->numplanes, PU_LEVEL, 0);

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < map->numplanes; i++)
    {
        int p;
        plane_t *pl = &map->planes[i];

        pl->area_id = SC_GetNumber();
        pl->flags = SC_GetNumber();

        pl->blocklist.next =
        pl->blocklist.prev = &pl->blocklist;

        for(p = 0; p < 3; p++)
        {
            int index = SC_GetNumber();

            pl->points[p][0] = points[index * 4 + 0];
            pl->points[p][1] = points[index * 4 + 1];
            pl->points[p][2] = points[index * 4 + 2];
            pl->height[p]    = points[index * 4 + 3];
        }

        for(p = 0; p < 3; p++)
        {
            int link = SC_GetNumber();
            pl->link[p] = link == -1 ? NULL : &map->planes[link];
        }

        Plane_GetNormal(pl->normal, pl);
        Vec_Normalize3(pl->normal);
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Map_ParseNavScript
//

static void Map_ParseNavScript(kmap_t *map, scparser_t *parser)
{
    unsigned int i;
    unsigned int numpoints = 0;
    float *points = NULL;

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
                switch(SC_GetIDForToken(navtokens, parser->token))
                {
                case scnav_numareas:
                    SC_AssignInteger(navtokens, &map->numareas,
                        scnav_numareas, parser, false);
                    break;

                case scnav_numpoints:
                    SC_AssignInteger(navtokens, &numpoints,
                        scnav_numpoints, parser, false);
                    break;

                case scnav_numleafs:
                    SC_AssignInteger(navtokens, &map->numplanes,
                        scnav_numleafs, parser, false);
                    break;

                case scnav_numzonebounds:
                    SC_AssignInteger(navtokens, &map->numzonebounds,
                        scnav_numzonebounds, parser, false);
                    break;

                case scnav_areas:
                    Map_ParseAreaBlock(map, parser);
                    break;

                case scnav_points:
                    SC_AssignArray(navtokens, AT_FLOAT, &points, numpoints * 4,
                        scnav_points, parser, false, PU_LEVEL);
                    break;

                case scnav_leafs:
                    Map_ParseCollisionPlanes(map, parser, numpoints, points);
                    break;

                case scnav_zonebounds:

                    if(map->numzonebounds <= 0)
                    {
                        SC_Error("numzonebounds is 0 or hasn't been set yet");
                        return;
                    }

                    map->zones = (mapgrid_t*)Z_Calloc(sizeof(mapgrid_t) *
                        map->numzonebounds, PU_LEVEL, 0);

                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);

                    for(i = 0; i < map->numzonebounds; i++)
                    {
                        map->zones[i].minx = (float)SC_GetFloat();
                        map->zones[i].minz = (float)SC_GetFloat();
                        map->zones[i].maxx = (float)SC_GetFloat();
                        map->zones[i].maxz = (float)SC_GetFloat();
                        
                        SC_GetNumber();
                        SC_GetNumber();
                        SC_GetNumber();
                    }

                    SC_ExpectNextToken(TK_RBRACK);
                    break;

                default:
                    if(parser->tokentype == TK_IDENIFIER)
                    {
                        Com_DPrintf("Map_ParseNavScript: Unknown token: %s\n",
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

    if(points != NULL)
    {
        Z_Free(points);
    }
}

//
// Obj_GetClassType
//

int Obj_GetClassType(object_t *obj)
{
    if(obj->type >= OT_AI_RAPTOR &&
        obj->type <= OT_AI_INSECT ||
        obj->type == OT_AI_TURRET)
    {
        return OC_AI;
    }

    if(obj->type >= OT_AIBOSS_MANTIS &&
        obj->type <= OT_AIBOSS_HUMMER)
    {
        return OC_AIBOSS;
    }

    if(obj->type >= OT_DYNAMIC_DOOR &&
        obj->type <= OT_DYNAMIC_TECHDOOR)
    {
        return OC_DYNAMIC;
    }

    if(obj->type >= OT_GIB_ALIEN3 &&
        obj->type <= OT_GIB_STALKER1)
    {
        return OC_GIB;
    }

    if(obj->type >= OT_WEAPON_KNIFE &&
        obj->type <= OT_WEAPON_CHRONO)
    {
        return OC_WEAPON;
    }

    if(obj->type >= OT_PICKUP_SMALLHEALTH &&
        obj->type <= OT_PICKUP_CHRONOPIECE8)
    {
        return OC_PICKUP;
    }

    return OC_GENERIC;
}

//
// Map_LinkObjToBlocklist
//

void Map_LinkObjToBlocklist(object_t *obj, plane_t *plane)
{
    blockobj_t *blockobj = (blockobj_t*)Z_Calloc(sizeof(blockobj_t), PU_LEVEL, 0);

    blockobj->object = obj;
    plane->blocklist.prev->next = blockobj;
    blockobj->next = &plane->blocklist;
    blockobj->prev = plane->blocklist.prev;
    plane->blocklist.prev = blockobj;
}

//
// Map_ObjectInBlocklist
//

static kbool Map_ObjectInBlocklist(object_t *obj, plane_t *plane)
{
    blockobj_t *blockobj;

    for(blockobj = plane->blocklist.next;
        blockobj != &plane->blocklist; blockobj = blockobj->next)
    {
        if(blockobj->object == obj)
        {
            return true;
        }
    }

    return false;
}

//
// Map_CheckObjectPlaneRange
//
// Returns true if the object's radius crosses
// within the plane's boundary
//

static kbool Map_CheckObjectPlaneRange(object_t *obj, plane_t *plane)
{
    int i;

    for(i = 0; i < 3; i++)
    {
        vec3_t vp1;
        vec3_t vp2;
        vec3_t n;
        vec3_t v;
        float x;
        float z;
        float d;

        // test against each edge
        Vec_Copy3(vp1, plane->points[i]);
        Vec_Copy3(vp2, plane->points[(i + 1) % 3]);

        x = vp1[0] - vp2[0];
        z = vp2[2] - vp1[2];

        // setup normal
        Vec_Set3(n, z, 0, x);
        Vec_Normalize3(n);

        // test distance
        Vec_Sub(v, obj->origin, vp1);
        d = Vec_Dot(n, v) + obj->width;

        if(d < 0)
        {
            // completely behind it 
            break;
        }
    }

    return (i == 3);
}

//
// Map_TraverseLinkObjects
//

static void Map_TraverseLinkObjects(object_t *obj, plane_t *plane, kmap_t *map)
{
    int i;

    if(plane == NULL || Map_ObjectInBlocklist(obj, plane))
    {
        return;
    }

    Map_LinkObjToBlocklist(obj, plane);

    for(i = 0; i < 3; i++)
    {
        plane_t *pl = plane->link[i];

        if(pl == NULL)
        {
            continue;
        }

        if(Map_CheckObjectPlaneRange(obj, pl))
        {
            Map_TraverseLinkObjects(obj, pl, map);
        }
    }
}

//
// Map_InitBlocklist
//

static void Map_InitBlocklist(kmap_t *map)
{
    unsigned int i;
    unsigned int j;

    for(i = 0; i < map->numinstances; i++)
    {
        instance_t *instances = &map->instances[i];

        for(j = 0; j < instances->numstatics; j++)
        {
            object_t *obj = &instances->statics[j];

            if(obj == NULL)
            {
                continue;
            }

            if(obj->plane_id != -1 && obj->blockflag & BLF_SECTORLINK)
            {
                plane_t *plane = &map->planes[obj->plane_id];

                Map_TraverseLinkObjects(obj, plane, map);
            }
        }

        for(j = 0; j < instances->numspecials; j++)
        {
            object_t *obj = &instances->specials[j];
            plane_t *plane;

            if(obj == NULL)
            {
                continue;
            }

            plane = &map->planes[obj->plane_id];
            
            Map_TraverseLinkObjects(obj, plane, map);
        }
    }
}

//
// Map_SetupActors
//

static void Map_SetupActors(kmap_t *map)
{
    actor_t *actor;

    for(actor = g_actorlist->next; actor != g_actorlist; actor = actor->next)
    {
        if(actor->object.plane_id != -1)
        {
            actor->plane = &map->planes[actor->object.plane_id];

            if(!(actor->flags & AF_NOALIGNPITCH))
            {
                float dist;

                dist = Plane_GetDistance(actor->plane, actor->origin);

                if(actor->origin[1] - dist <
                    (actor->object.centerheight + actor->object.viewheight))
                {
                    actor->origin[1] = dist;
                }
            }
        }

        if(actor->object.type == 0)
        {
            memcpy(&client.localactor, actor, sizeof(actor_t));
        }
    }
}

//
// Map_GetArea
//

area_t *Map_GetArea(plane_t *plane)
{
    return &g_currentmap->areas[plane->area_id];
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

    kmap = &kmaps[map];
    g_currentmap = kmap;

    G_SetActorLinkList(map);

    if(g_currentmap->tics > 0 || g_currentmap->time > 0)
    {
        return kmap;
    }

    g_currentmap->tics = 0;
    g_currentmap->time = 0;

    if(!(parser = SC_Open(kva("maps/map%02d/map%02d.kmap", map, map))))
    {
        return NULL;
    }

    Map_ParseLevelScript(kmap, parser);
    // we're done with the file
    SC_Close();

    if(parser = SC_Open(kva("maps/map%02d/map%02d.knav", map, map)))
    {
        Map_ParseNavScript(kmap, parser);
        // we're done with the file
        SC_Close();
    }

    Map_InitBlocklist(kmap);
    Map_SetupActors(kmap);

    return kmap;
}

//
// FCmd_LoadTestMap
//

static void FCmd_LoadTestMap(void)
{
     kmap_t *kmap;
     int map;

    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    map = atoi(Cmd_GetArgv(1));

    kmap = &kmaps[map];
    Map_Load(map);

    Com_DPrintf("\nplanes: %i\nareas: %i\n\n",
        kmap->numplanes, kmap->numareas);
}

//
// Map_Init
//

void Map_Init(void)
{
    int i;

    memset(kmaps, 0, sizeof(kmap_t) * MAXMAPS);

    for(i = 0; i < MAXMAPS; i++)
    {
        actorlist[i].next = actorlist[i].prev = &actorlist[i];
    }

    Cmd_AddCommand("loadmap", FCmd_LoadTestMap);
}

