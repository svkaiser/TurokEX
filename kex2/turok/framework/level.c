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
#include "js_shared.h"
#include "sound.h"
#include "ai.h"
#include "fx.h"

gLevel_t gLevel;

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
#if 0
        SC_Error("numplanes is 0 or hasn't been set yet");
#endif
        return;
    }

    if(numpoints <= 0)
    {
#if 0
        SC_Error("numpoints is 0 or hasn't been set yet");
#endif
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
        Plane_GetCeilingNormal(pl->ceilingNormal, pl);
        Vec_Normalize3(pl->ceilingNormal);
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
                    if((numpoints * 4) > 0)
                    {
                        SC_AssignArray(navtokens, AT_FLOAT, &points, numpoints * 4,
                            scnav_points, parser, false, PU_LEVEL);
                    }
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
    scactor_bHidden,
    scactor_bStatic,
    scactor_bTouch,
    scactor_bOrientOnSlope,
    scactor_bNoDropOff,
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
    { scactor_bNoDropOff,       "bNoDropOff"        },
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
    scarea_end
};

static const sctokens_t mapareatokens[scarea_end+1] =
{
    { scarea_components,    "components"    },
    { -1,                   NULL            }
};

//
// Map_SpawnActor
// Spawns a actor into the level. If successful, actor is then linked
// into the map's actorlist
//

gActor_t *Map_SpawnActor(const char *classname, float x, float y, float z,
                         float yaw, float pitch, int plane)
{
    gActor_t *actor = NULL;
    
    if((actor = Actor_Spawn(classname, x, y, z, yaw, pitch, plane)))
        Map_AddActor(&gLevel, actor);

    if(plane == -1 && actor != NULL)
    {
        plane_t *p = Map_FindClosestPlane(actor->origin);

        if(p != NULL)
            actor->plane = p - gLevel.planes;
    }

    return actor;
}

//
// Map_AddActor
//

void Map_AddActor(gLevel_t *level, gActor_t *actor)
{
    level->actorRoot.prev->next = actor;
    actor->next = &level->actorRoot;
    actor->prev = level->actorRoot.prev;
    level->actorRoot.prev = actor;
}

//
// Map_RemoveActor
//

void Map_RemoveActor(gLevel_t *level, gActor_t* actor)
{
    gActor_t* next;

    if(actor->refcount > 0)
        return;

    /* Remove from main actor list */
    next = level->actorRover->next;

    /* Note that level->actorRover is guaranteed to point to us,
    * and since we're freeing our memory, we had better change that. So
    * point it to actor->prev, so the iterator will correctly move on to
    * actor->prev->next = actor->next */
    (next->prev = level->actorRover = actor->prev)->next = next;
    Actor_Remove(actor);
}

//
// Map_ParseActor
//

static void Map_ParseActor(scparser_t *parser, gActor_t *actor)
{
    int numComponents = 0;
    int numTexSwaps = 0;
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
            actor->model = Mdl_Load(parser->stringToken);
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
            Vec_Copy3(actor->bbox.omin, actor->bbox.min);
            Vec_Copy3(actor->bbox.omax, actor->bbox.max);
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
                actor->textureSwaps[j] = Z_Strdup(parser->stringToken, PU_ACTOR, NULL);
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

        case scactor_bNoDropOff:
            SC_AssignInteger(mapactortokens, &actor->bNoDropOff,
                scactor_bNoDropOff, parser, false);
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

        default:
            if(parser->tokentype == TK_IDENIFIER)
            {
                SC_Error("Map_ParseActor: Unknown token: %s\n",
                    parser->token);
            }
            break;
        }

        SC_Find();
    }
}

//
// Map_ParseLevelActorBlock
//

static void Map_ParseLevelActorBlock(scparser_t *parser, gLevel_t *level)
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
        Map_ParseActor(parser, actor);

        // TODO - TEMP
        actor->physics      = PT_NONE;
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
// Map_ParseStaticActorBlock
//

static void Map_ParseStaticActorBlock(scparser_t *parser, int count, gActor_t **actorList)
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
        
        Map_ParseActor(parser, actor);
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
                Map_ParseStaticActorBlock(parser, gridbound->numStatics,
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
                    Map_ParseLevelActorBlock(parser, &gLevel);
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

#if 0
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

#endif

//
// Map_GetArea
//

gArea_t *Map_GetArea(plane_t *plane)
{
    return &gLevel.areas[plane->area_id];
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
// TraverseAdjacentPlanesToHeight
//
// TODO: Plane vertex points are stored in a seperate
// array but in kex2, each vertex point is independent.
// In order to properly drag all neighboring points along
// with the moving plane, a more extensive search for
// matching points is needed
//

static void TraverseAdjacentPlanesToHeight(plane_t *plane, plane_t *parent,
                                               float destHeight, float match)
{
    int i;

    for(i = 0; i < 3; i++)
    {
        if(plane->points[i][1] == match)
        {
            int j;

            if(plane->points[i][1] == destHeight)
                continue;

            plane->points[i][1] = destHeight;

            for(j = 0; j < 3; j++)
            {
                if(!plane->link[j])
                    continue;

                if(plane->link[j] == parent)
                    continue;

                TraverseAdjacentPlanesToHeight(plane->link[j],
                    parent, destHeight, match);
            }
        }
    }
}

//
// Map_TraverseChangePlaneHeight
//
// Takes all points of the affected plane and updates
// the vertical height. All planes that matches the area id
// is traversed. Adjacent planes with unmatched area ids are
// checked for matching points with the affected plane
//

void Map_TraverseChangePlaneHeight(plane_t *plane, float destHeight, int area_id)
{
    plane_t * p;

    if(plane == NULL)
        return;

    p = plane;

    while(p->area_id == area_id)
    {
        int i;

        if(p->points[0][1] == destHeight &&
            p->points[1][1] == destHeight &&
            p->points[2][1] == destHeight)
        {
            break;
        }

        for(i = 0; i < 3; i++)
        {
            if(p->link[i] && p->link[i]->area_id != area_id)
            {
                int j;

                for(j = 0; j < 3; j++)
                {
                    TraverseAdjacentPlanesToHeight(p->link[i], p,
                        destHeight, p->points[j][1]);
                }
            }
        }

        for(i = 0; i < 3; i++)
            p->points[i][1] = destHeight;

        if(p->link[0])
            Map_TraverseChangePlaneHeight(p->link[0], destHeight, area_id);

        if(p->link[1])
            Map_TraverseChangePlaneHeight(p->link[1], destHeight, area_id);

        if(!p->link[2])
            break;

        p = p->link[2];
    }
}

//
// Map_TraverseToggleBlockingPlanes
//

static void Map_TraverseToggleBlockingPlanes(plane_t *plane, kbool toggle, int area_id)
{
    plane_t * p;

    if(plane == NULL)
        return;

    p = plane;

    while(p->area_id == area_id)
    {
        if(toggle)
        {
            if(p->flags & CLF_TOGGLE)
                break;

            p->flags |= CLF_TOGGLE;
        }
        else
        {
            if(!(p->flags & CLF_TOGGLE))
                break;

            p->flags &= ~CLF_TOGGLE;
        }

        if(p->link[0])
            Map_TraverseToggleBlockingPlanes(p->link[0], toggle, area_id);

        if(p->link[1])
            Map_TraverseToggleBlockingPlanes(p->link[1], toggle, area_id);

        if(!p->link[2])
            break;

        p = p->link[2];
    }
}

//
// Map_ToggleBlockingPlanes
//

void Map_ToggleBlockingPlanes(plane_t *pStart, kbool toggle)
{
    if(!pStart)
        return;

    if(!(pStart->flags & CLF_BLOCK))
        return;

    Map_TraverseToggleBlockingPlanes(pStart, toggle, pStart->area_id);
}

//
// Map_TriggerActors
//

void Map_TriggerActors(int targetID, int classFilter)
{
    gActor_t *actor;

    for(actor = gLevel.actorRoot.next; actor != &gLevel.actorRoot;
        actor = actor->next)
    {
        if(classFilter != 0 && !(actor->classFlags & classFilter))
            continue;
        if(actor->bStale)
            continue;
        if(actor->targetID != targetID)
            continue;

        Actor_CallEvent(actor, "onTrigger", NULL, 0);
    }
}

//
// Map_GetActorsInRadius
//

gObject_t *Map_GetActorsInRadius(float radius, float x, float y, float z, plane_t *plane,
                                 int classFilter, kbool bTrace)
{
    gActor_t *actor;
    vec3_t org;
    unsigned int count;
    gObject_t *arrObj;

    Vec_Set3(org, x, y, z);
    arrObj = NULL;
    count = 0;

    for(actor = gLevel.actorRoot.next; actor != &gLevel.actorRoot;
        actor = actor->next)
    {
        jsval val;

        if(!(actor->classFlags & classFilter))
            continue;
        if(actor->bHidden || actor->bStale)
            continue;
        if(Vec_Length3(org, actor->origin) >= radius)
            continue;

        if(plane == NULL)
        {
            if(!(plane = Map_FindClosestPlane(org)))
                continue;
        }

        if(bTrace)
        {
            trace_t trace;
            vec3_t dest;

            Vec_Copy3(dest, actor->origin);
            dest[1] += actor->centerHeight;

            trace = Trace(org, dest, plane, NULL, NULL, true);

            if(trace.type != TRT_OBJECT ||
                !trace.hitActor || trace.hitActor != actor)
                continue;
        }

        if(arrObj == NULL)
            arrObj = JS_NewArrayObject(js_context, 0, NULL);

        Actor_ToVal(actor, &val);
        JS_SetElement(js_context, arrObj, count++, &val);
    }

    return arrObj;
}

//
// Map_Tick
//

void Map_Tick(void)
{
    if(gLevel.loaded == false)
    {
        if(gLevel.nextMap >= 0)
            Map_Load(gLevel.nextMap);

        return;
    }

    gLevel.tics++;
    gLevel.time = (float)(gLevel.tics * SERVER_RUNTIME);
    gLevel.deltaTime = (float)server.runtime;

    if(gLevel.bReadyUnload)
        Map_Unload();
}

//
// Map_Load
//

void Map_Load(int map)
{
    scparser_t *parser;

    if(client.state < CL_STATE_READY)
        return;

    // TODO
    if(gLevel.loaded == true)
        return;

    Random_SetSeed(-470403613);

    gLevel.loaded       = false;
    gLevel.numplanes    = 0;
    gLevel.planes       = NULL;
    gLevel.bReadyUnload = false;

    if(parser = SC_Open(kva("maps/map%02d/map%02d.kcm", map, map)))
    {
        Map_ParseNavScript(parser);
        // we're done with the file
        SC_Close();
    }

    if(!(parser = SC_Open(kva("maps/map%02d/map%02d.kmap", map, map))))
        return;

    Map_ParseLevelScript(parser);
    SC_Close();

    gLevel.nextMap = -1;
    gLevel.loaded = true;

    P_SpawnLocalPlayer();
    client.state = CL_STATE_INGAME;
}

//
// Map_Unload
//

void Map_Unload(void)
{
    unsigned int i;

    Snd_StopAll();

    P_SaveLocalComponentData();

    gLevel.loaded = false;
    client.state = CL_STATE_CHANGINGLEVEL;

    // remove all actors
    for(gLevel.actorRover = gLevel.actorRoot.next;
        gLevel.actorRover != &gLevel.actorRoot;
        gLevel.actorRover = gLevel.actorRover->next)
    {
        gLevel.actorRover->refcount = 0;
        Map_RemoveActor(&gLevel, gLevel.actorRover);
    }

    // remove all non-static actors in grid bounds
    for(i = 0; i < gLevel.numGridBounds; i++)
    {
        gridBounds_t *gb = &gLevel.gridBounds[i];
        unsigned int j;

        for(j = 0; j < gb->numStatics; j++)
        {
            gActor_t *actor = &gb->statics[j];

            if(actor->bStatic)
                continue;

            Actor_ClearData(actor);
        }

        Z_Free(gb->statics);
    }

    // unroot all script objects in areas
    for(i = 0; i < gLevel.numAreas; i++)
    {
        gArea_t *area = &gLevel.areas[i];

        if(area->components)
            JS_RemoveRoot(js_context, &area->iterator);

        if(area->components)
            JS_RemoveRoot(js_context, &area->components);
    }

    FX_ClearLinks();

    // purge all level and actor allocations
    Z_FreeTags(PU_LEVEL, PU_LEVEL);
    Z_FreeTags(PU_ACTOR, PU_ACTOR);

    JS_GC(js_context);
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
// FCmd_UnloadMap
//

static void FCmd_UnloadMap(void)
{
    if(Cmd_GetArgc() < 1)
        return;

    gLevel.bReadyUnload = true;
}

//
// FCmd_SpawnActor
//

static void FCmd_SpawnActor(void)
{
    float x;
    float y;
    float z;
    char *name;
    gActor_t *actor;

    if(Cmd_GetArgc() <= 0)
        return;

    name    = Cmd_GetArgv(1);
    x       = (float)atof(Cmd_GetArgv(2));
    y       = (float)atof(Cmd_GetArgv(3));
    z       = (float)atof(Cmd_GetArgv(4));

    actor = Map_SpawnActor(name, x, y, z, client.player->actor->angles[0], 0, -1);
}

//
// Map_Init
//

void Map_Init(void)
{
    memset(&gLevel, 0, sizeof(gLevel_t));

    gLevel.nextMap = -1;

    Cmd_AddCommand("loadmap", FCmd_LoadTestMap);
    Cmd_AddCommand("unloadmap", FCmd_UnloadMap);
    Cmd_AddCommand("spawnactor", FCmd_SpawnActor);
}

