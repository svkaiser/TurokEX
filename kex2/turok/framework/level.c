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
#include "server.h"
#include "js.h"
#include "js_parse.h"

gLevel_t gLevel;

kmap_t kmaps[MAXMAPS];
kmap_t *g_currentmap = NULL;

enum
{
    scnav_numpoints = 0,
    scnav_numleafs,
    scnav_points,
    scnav_leafs,
    scnav_end
};

static const sctokens_t navtokens[scnav_end+1] =
{
    { scnav_numpoints,      "numpoints"     },
    { scnav_numleafs,       "numleafs"      },
    { scnav_points,         "points"        },
    { scnav_leafs,          "leafs"         },
    { -1,                   NULL            }
};

//
// Map_ParseCollisionPlanes
//

static void Map_ParseCollisionPlanes(scparser_t *parser,
                                     unsigned int numpoints, float *points)
{
    unsigned int i;

    if(gLevel.numplanes <= 0)
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

    gLevel.planes = (plane_t*)Z_Calloc(sizeof(plane_t) *
        gLevel.numplanes, PU_LEVEL, 0);

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < gLevel.numplanes; i++)
    {
        int p;
        plane_t *pl = &gLevel.planes[i];

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
            pl->link[p] = link == -1 ? NULL : &gLevel.planes[link];
        }

        Plane_GetNormal(pl->normal, pl);
        Vec_Normalize3(pl->normal);
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Map_ParseNavScript
//

static void Map_ParseNavScript(scparser_t *parser)
{
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
                case scnav_numpoints:
                    SC_AssignInteger(navtokens, &numpoints,
                        scnav_numpoints, parser, false);
                    break;

                case scnav_numleafs:
                    SC_AssignInteger(navtokens, &gLevel.numplanes,
                        scnav_numleafs, parser, false);
                    break;

                case scnav_points:
                    SC_AssignArray(navtokens, AT_FLOAT, &points, numpoints * 4,
                        scnav_points, parser, false, PU_LEVEL);
                    break;

                case scnav_leafs:
                    Map_ParseCollisionPlanes(parser, numpoints, points);
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
        Z_Free(points);
}

enum
{
    scmap_title = 0,
    scmap_mapID,
    scmap_actors,
    scmap_gridbounds,
    scmap_areas,
    scmap_end
};

static const sctokens_t maptokens[scmap_end+1] =
{
    { scmap_title,              "title"                 },
    { scmap_mapID,              "mapID"                 },
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
    scactor_bStatic,
    scactor_bTouch,
    scactor_plane,
    scactor_origin,
    scactor_scale,
    scactor_angles,
    scactor_rotation,
    scactor_radius,
    scactor_height,
    scactor_centerheight,
    scactor_viewheight,
    scactor_end
};

static const sctokens_t mapactortokens[scactor_end+1] =
{
    { scactor_name,         "name"          },
    { scactor_mesh,         "mesh"          },
    { scactor_bounds,       "bounds"        },
    { scactor_textureSwaps, "textureSwaps"  },
    { scactor_components,   "components"    },
    { scactor_bCollision,   "bCollision"    },
    { scactor_bStatic,      "bStatic"       },
    { scactor_bTouch,       "bTouch"        },
    { scactor_plane,        "plane"         },
    { scactor_origin,       "origin"        },
    { scactor_scale,        "scale"         },
    { scactor_angles,       "angles"        },
    { scactor_rotation,     "rotation"      },
    { scactor_radius,       "radius"        },
    { scactor_height,       "height"        },
    { scactor_centerheight, "centerheight"  },
    { scactor_viewheight,   "viewheight"    },
    { -1,                   NULL            }
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
    scarea_end
};

static const sctokens_t mapareatokens[scarea_end+1] =
{
    { scarea_components,    "components"    },
    { -1,                   NULL            }
};

//
// Map_ParseActorBlock
//

static void Map_ParseActorBlock(scparser_t *parser, int count, gActor_t **actorList)
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
        int numComponents = 0;
        int numTexSwaps = 0;
        int j;
        gActor_t *actor = &(*actorList)[i];
        
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
                actor->model = Mdl_Load(sc_stringbuffer);
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
                SC_ExpectNextToken(TK_RBRACK);
                break;

            case scactor_textureSwaps:
                SC_ExpectNextToken(TK_LSQBRACK);
                numTexSwaps = SC_GetNumber();
                if(numTexSwaps > 0)
                {
                    actor->textureSwaps = (char**)Z_Calloc(sizeof(char*) *
                        numTexSwaps, PU_ACTOR, NULL);
                }
                else
                {
                    actor->textureSwaps = NULL;
                }
                SC_ExpectNextToken(TK_RSQBRACK);
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);
                for(j = 0; j < numTexSwaps; j++)
                {
                    SC_GetString();
                    actor->textureSwaps[j] = Z_Strdup(sc_stringbuffer, PU_ACTOR, NULL);
                }
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

            case scactor_bStatic:
                SC_AssignInteger(mapactortokens, &actor->bStatic,
                    scactor_bStatic, parser, false);
                break;

            case scactor_bTouch:
                SC_AssignInteger(mapactortokens, &actor->bTouch,
                    scactor_bTouch, parser, false);
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
                SC_AssignFloat(mapactortokens, &actor->height,
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

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Map_ParseActorBlock: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }

        Actor_Setup(actor);
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Map_ParseGridSectionBlock
//

static void Map_ParseGridSectionBlock(scparser_t *parser)
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
                Map_ParseActorBlock(parser, gridbound->numStatics,
                    &gridbound->statics);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Map_ParseGridSectionBlock: Unknown token: %s\n",
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
// Map_ParseAreaBlock
//

static void Map_ParseAreaBlock(scparser_t *parser)
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

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Map_ParseAreaBlock: Unknown token: %s\n",
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
// Map_ParseLevelScript
//

static void Map_ParseLevelScript(scparser_t *parser)
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

                case scmap_actors:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numActors = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Map_ParseActorBlock(parser, gLevel.numActors, &gLevel.gActors);
                    break;

                case scmap_gridbounds:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numGridBounds = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Map_ParseGridSectionBlock(parser);
                    break;

                case scmap_areas:
                    SC_ExpectNextToken(TK_LSQBRACK);
                    gLevel.numAreas = SC_GetNumber();
                    SC_ExpectNextToken(TK_RSQBRACK);
                    Map_ParseAreaBlock(parser);
                    break;

                default:
                    if(parser->tokentype == TK_IDENIFIER)
                    {
                        SC_Error("Map_ParseLevelScript: Unknown token: %s\n",
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
// Map_GetArea
//

area_t *Map_GetArea(plane_t *plane)
{
    return &g_currentmap->areas[plane->area_id];
}

//
// Map_FindClosestPlane
//

plane_t *Map_FindClosestPlane(vec3_t coord)
{
    unsigned int i;
    float dist;
    float curdist;
    plane_t *plane;
    kbool ok;

    ok = false;
    curdist = 0;
    plane = NULL;

    for(i = 0; i < gLevel.numplanes; i++)
    {
        plane_t *p;

        p = &gLevel.planes[i];

        if(Plane_PointInRange(p, coord[0], coord[2]))
        {
            dist = coord[1] - Plane_GetDistance(p, coord);

            if(p->flags & CLF_ONESIDED && dist < -16)
            {
                continue;
            }

            if(dist < 0)
            {
                dist = -dist;
            }

            if(ok)
            {
                if(dist < curdist)
                {
                    curdist = dist;
                    plane = p;
                }
            }
            else
            {
                plane = p;
                curdist = dist;
                ok = true;
            }
        }
    }

    return plane;
}

//
// Map_Load
//

void Map_Load(int map)
{
    scparser_t *parser;

    // TODO
    if(gLevel.loaded == true)
        return;

    Random_SetSeed(-470403613);

    gLevel.loaded       = false;
    gLevel.numplanes    = 0;
    gLevel.planes       = NULL;

    if(!(parser = SC_Open(kva("maps/map%02d/map%02d.kmap", map, map))))
        return;

    Map_ParseLevelScript(parser);
    SC_Close();

    if(parser = SC_Open(kva("maps/map%02d/map%02d.kcm", map, map)))
    {
        Map_ParseNavScript(parser);
        // we're done with the file
        SC_Close();
    }

    gLevel.loaded = true;

    J_RunObjectEvent(JS_EV_GAME, "event_BeginLevel");
}

//
// FCmd_LoadTestMap
//

static void FCmd_LoadTestMap(void)
{
     int map;

    if(Cmd_GetArgc() < 2)
        return;

    map = atoi(Cmd_GetArgv(1));
    Map_Load(map);
}

//
// Map_Init
//

void Map_Init(void)
{
    memset(kmaps, 0, sizeof(kmap_t) * MAXMAPS);
    memset(&gLevel, 0, sizeof(gLevel_t));

    Cmd_AddCommand("loadmap", FCmd_LoadTestMap);
}

