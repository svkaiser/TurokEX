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
#include "actor_old.h"
#include "level.h"
#include "script.h"
#include "mathlib.h"
#include "client.h"
#include "server.h"
#include "js.h"
#include "jsobj.h"
#include "js_parse.h"
#include "js_shared.h"
#include "sound.h"
#include "ai.h"
#include "fx.h"
#include "debug.h"

gLevel_t gLevel;

static int numAreaActors = 0;

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
// Map_GetArea
//

gArea_t *Map_GetArea(plane_t *plane)
{
    return plane ? &gLevel.areas[plane->area_id] : NULL;
}

//
// Map_CallAreaEvent
//

kbool Map_CallAreaEvent(gArea_t *area, const char *function, long *args, unsigned int nargs)
{
    jsval val;
    kbool ok;

    if(area->components == NULL)
        return true;

#ifdef JS_LOGNEWOBJECTS
    common.Printf("\nLogging Event (%s)...\n", function);
#endif

    ok = false;

    JS_ITERATOR_START(area, val);
    JS_ITERATOR_LOOP(area, val, function);
    {
        gObject_t *func;
        jsval rval;
        jsval argv = JSVAL_VOID;

        if(!JS_ValueToObject(js_context, vp, &func))
            continue;
        if(!JS_ObjectIsFunction(js_context, func))
            continue;

        if(args == NULL)
            args = &argv;

        JS_CallFunctionName(js_context, component, function, nargs, args, &rval);
        ok = true;
    }
    JS_ITERATOR_END(area, val);

    return ok;
}

//
// Map_LinkActorToWorld
//

void Map_LinkActorToWorld(gActor_t *actor)
{
    gArea_t *area;

    if(actor->plane >= (int)gLevel.numplanes ||
        actor->plane <= -1 || !gLevel.loaded)
        return;

    area = Map_GetArea(Map_IndexToPlane(actor->plane));

    if(area == NULL)
        return;

    area->actorRoot.linkPrev->linkNext = actor;
    actor->linkNext = &area->actorRoot;
    actor->linkPrev = area->actorRoot.linkPrev;
    area->actorRoot.linkPrev = actor;
}

//
// Map_UnlinkActorFromWorld
//

void Map_UnlinkActorFromWorld(gActor_t *actor)
{
    gArea_t *area;
    gActor_t *next;

    if(actor->plane >= (int)gLevel.numplanes ||
        actor->plane <= -1 || !gLevel.loaded)
        return;

    area = Map_GetArea(Map_IndexToPlane(actor->plane));

    if(area == NULL)
        return;

    next = actor->linkNext;
    (next->linkPrev = actor->linkPrev)->linkNext = next;
}

//
// Map_GetWaterLevel
//

int Map_GetWaterLevel(vec3_t origin, float height, plane_t *plane)
{
    gArea_t *area;

    if(plane == NULL)
        return WL_INVALID;

    area = &gLevel.areas[plane->area_id];

    if(plane != NULL && area->flags & AAF_WATER)
    {
        if(height + origin[1] >= area->waterplane)
        {
            if(origin[1] < area->waterplane)
                return WL_BETWEEN;
            else
                return WL_OVER;
        }

        return WL_UNDER;
    }

    return WL_INVALID;
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
                continue;

            if(dist < 0)
                dist = -dist;

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

            Plane_GetNormal(plane->normal, plane);
            Vec_Normalize3(plane->normal);

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

        Plane_GetNormal(p->normal, p);
        Vec_Normalize3(p->normal);

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
        if(Vec_Length3(org, actor->origin) >= (radius + actor->radius) * 0.925f)
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
            dest[1] += actor->height;

            trace = Trace(org, dest, plane, NULL, PF_CLIP_ALL | PF_DROPOFF);

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
// Map_GetAreaLinkCount
//

static void Map_GetAreaLinkCount(void)
{
    gActor_t *rover;
    gArea_t *area;

    numAreaActors = 0;

    if(client.LocalPlayer().actor->plane <= -1)
        return;

    area = Map_GetArea(Map_IndexToPlane(client.LocalPlayer().actor->plane));

    if(area == NULL)
        return;

    for(rover = area->actorRoot.linkNext;
        rover != &area->actorRoot;
        rover = rover->linkNext)
    {
        numAreaActors++;
    }
}

//
// Map_ImportObjToKNav
//

static void Map_ImportObjToKNav(const char *filename) {
    kexLexer *lexer;
    filepath_t file;

    sprintf(file, filename);

    if(lexer = parser.Open(file)) {
        int numpoints = 0;
        float *verts = NULL;
        int pointRover = 0;
        int *faces = NULL;
        int numfaces = 0;
        int faceRover = 0;

        while(lexer->CheckState()) {
            lexer->Find();

            switch(lexer->TokenType()) {
            case TK_NONE:
            case TK_EOF:
                break;
            case TK_IDENIFIER:
                if(!strcmp(lexer->Token(), "o")) {
                    lexer->Find();
                }
                else if(!strcmp(lexer->Token(), "v")) {
                    numpoints++;
                    verts = (float*)Mem_Realloc(verts,
                        sizeof(float) * (numpoints * 3), hb_static);

                    verts[pointRover++] = (float)lexer->GetFloat() * 256.0f;
                    verts[pointRover++] = (float)lexer->GetFloat() * 256.0f;
                    verts[pointRover++] = (float)lexer->GetFloat() * 256.0f;
                }
                else if(!strcmp(lexer->Token(), "f")) {
                    numfaces++;
                    faces = (int*)Mem_Realloc(faces,
                        sizeof(int) * (numfaces * 3), hb_static);

                    faces[faceRover++] = lexer->GetNumber() - 1;
                    faces[faceRover++] = lexer->GetNumber() - 1;
                    faces[faceRover++] = lexer->GetNumber() - 1;
                }
                break;
            }
        }

        parser.Close();

        int *links = (int*)Mem_Calloc(sizeof(int) * (numfaces * 3), hb_static);
        memset(links, -1, sizeof(int) * (numfaces * 3));

        for(int i = 0; i < numfaces * 3; i += 3) {
            int *face = &faces[i];

            for(int j = 0; j < 3; j++) {
                if(links[i + j] != -1)
                    continue;

                int link1 = face[j];
                int link2 = face[(j + 1) % 3];
                bool ok = false;

                for(int k = 0; k < numfaces * 3; k += 3) {
                    if(k == i)
                        continue;

                    int *nface = &faces[k];

                    for(int l = 0; l < 3; l++) {
                        int nlink1 = nface[l];
                        int nlink2 = nface[(l + 1) % 3];

                        if(link2 == nlink1 && link1 == nlink2) {
                            links[i + j] = (k / 3);
                            links[k + l] = (i / 3); 
                            ok = true;
                            break;
                        }
                    }

                    if(ok == true)
                        break;
                }
            }
        }

        FILE *f = fopen("out.kcm", "w");
        fprintf(f, "numpoints = %i\n", numpoints);
        fprintf(f, "numleafs = %i\n", numfaces);
        fprintf(f, "points = {\n");
        for(int i = 0; i < numpoints; i++) {
            fprintf(f, "%f %f %f 0.0\n",
                verts[i * 3 + 0],
                verts[i * 3 + 1],
                verts[i * 3 + 2]);
        }
        fprintf(f, "}\n");
        fprintf(f, "leafs = {\n");
        for(int i = 0; i < numfaces; i++) {
            fprintf(f, "0 0 %i %i %i %i %i %i\n",
                faces[i * 3 + 0],
                faces[i * 3 + 1],
                faces[i * 3 + 2],
                links[i * 3 + 0],
                links[i * 3 + 1],
                links[i * 3 + 2]);
        }
        fprintf(f, "}\n");
        fclose(f);

        Mem_Free(verts);
        Mem_Free(faces);
        Mem_Free(links);
    }
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

    if(cvarDeveloper.GetBool())
        Map_GetAreaLinkCount();

    gLevel.tics++;
    gLevel.time = (float)(gLevel.tics * SERVER_RUNTIME);
    gLevel.deltaTime = (float)server.GetElaspedTime();

    if(gLevel.bReadyUnload)
        Map_Unload();
}

//
// Map_PreLoad
//

static void Map_PreLoad(void)
{
    Random_SetSeed(-470403613);

    gLevel.loaded       = false;
    gLevel.numplanes    = 0;
    gLevel.planes       = NULL;
    gLevel.bReadyUnload = false;

    Vec_Set4(gLevel.worldLightOrigin, 0, 0, 0, 0);
    Vec_Set4(gLevel.worldLightColor, 1, 1, 1, 1);
    Vec_Set4(gLevel.worldLightAmbience, 1, 1, 1, 1);
    Vec_Set4(gLevel.worldLightModelAmbience, 1, 1, 1, 1);
}

//
// Map_PostLoad
//

static void Map_PostLoad(void)
{
    gActor_t *actor;
    jsval val;

    gLevel.nextMap = -1;
    gLevel.loaded = true;

    //P_SpawnLocalPlayer();
    client.SetState(CL_STATE_INGAME);

    JS_CallFunctionName(js_context, js_objGame, "onLevelLoad", 0, NULL, &val);

    // TODO - area data may not be fully initialized or initialized last
    // after actors were spawned. here, actors are unlinked and linked again
    // to insure that all area data has been fully loaded
    for(actor = gLevel.actorRoot.next; actor != &gLevel.actorRoot;
        actor = actor->next)
    {
        Map_LinkActorToWorld(actor);
    }
}

//
// Map_Load
//

void Map_Load(int map)
{
    if(client.GetState() < CL_STATE_READY)
        return;

    // TODO
    if(gLevel.loaded)
        return;

    Map_PreLoad();
    //Knav_Load(map);
    //Kmap_Load(map);
    Map_PostLoad();
}

//
// Map_Unload
//

void Map_Unload(void)
{
    unsigned int i;
    jsval val;

    soundSystem.StopAll();

    JS_CallFunctionName(js_context, js_objGame, "onLevelUnLoad", 0, NULL, &val);

    gLevel.loaded = false;
    client.SetState(CL_STATE_CHANGINGLEVEL);

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

            if(actor->plane >= 0)
            {
                Map_UnlinkActorFromWorld(actor);
                actor->linkNext = actor->linkPrev = NULL;
            }

            if(actor->bStatic)
                continue;

            Actor_ClearData(actor);
        }

        Mem_Free(gb->statics);
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

    //FX_ClearLinks();

    // purge all level and actor allocations
    //Z_FreeTags(PU_LEVEL, PU_LEVEL);
    //Z_FreeTags(PU_ACTOR, PU_ACTOR);

    JS_GC(js_context);
}

//
// FCmd_LoadTestMap
//

static void FCmd_LoadTestMap(void)
{
    int map;

    if(command.GetArgc() < 2)
        return;

    map = atoi(command.GetArgv(1));
    Map_Load(map);
}

//
// FCmd_UnloadMap
//

static void FCmd_UnloadMap(void)
{
    if(command.GetArgc() < 1)
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

    if(command.GetArgc() <= 0)
        return;

    name    = command.GetArgv(1);
    x       = (float)atof(command.GetArgv(2));
    y       = (float)atof(command.GetArgv(3));
    z       = (float)atof(command.GetArgv(4));

    actor = Map_SpawnActor(name, x, y, z, client.LocalPlayer().actor->angles[0], 0, -1);
}

//
// FCmd_ImportObjToKNav
//

static void FCmd_ImportObjToKNav(void) {
    if(command.GetArgc() <= 0)
        return;

    Map_ImportObjToKNav(command.GetArgv(1));
}

//
// Map_Init
//

void Map_Init(void)
{
    memset(&gLevel, 0, sizeof(gLevel_t));

    gLevel.nextMap = -1;

    command.Add("loadmap", FCmd_LoadTestMap);
    command.Add("unloadmap", FCmd_UnloadMap);
    command.Add("spawnactor", FCmd_SpawnActor);
    command.Add("importObjToKNav", FCmd_ImportObjToKNav);

    Debug_RegisterPerfStatVar((float*)&numAreaActors, "Area Link Count", false);
}

