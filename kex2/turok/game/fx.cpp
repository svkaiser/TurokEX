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
// DESCRIPTION: Particle effects system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "mathlib.h"
#include "gl.h"
#include "fx.h"
#include "ai.h"
#include "actor_old.h"
#include "client.h"
#include "level.h"
#include "sound.h"
#include "parse.h"
#include "fileSystem.h"

fx_t fxRoot;
fx_t *fxRover;

#define FX_RANDVAL() (float)((rand() % 20000) - 10000) * 0.0001f

//
// FX_Shutdown
//

void FX_Shutdown(void)
{
    Z_FreeTags(PU_FX, PU_FX);
}

//
// FX_Rand
//

float FX_Rand(float val)
{
    return Random_Float() * val;
}

//
// FX_SetTranslationX
//

static void FX_SetTranslationX(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = FX_RANDVAL();
    vec[1] = vector[1];
    vec[2] = vector[2];

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SetTranslationY
//

static void FX_SetTranslationY(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = vector[0];
    vec[1] = FX_RANDVAL();
    vec[2] = vector[2];

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SetTranslationZ
//

static void FX_SetTranslationZ(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = vector[0];
    vec[1] = vector[1];
    vec[2] = FX_RANDVAL();

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SpawnChild
//

static fx_t *FX_SpawnChild(fx_t *parent, const char *file)
{
    vec4_t rot;
    vec3_t origin;
    fxinfo_t *info;

    Vec_ToQuaternion(rot, parent->translation);
    Vec_Copy3(origin, parent->origin);

    info = parent->info;

    if(info->screen_offset_x != 0 || info->screen_offset_y != 0)
    {
        mtx_t mtx1;
        mtx_t mtx3;
        mtx_t mtx4;
        vec3_t svec;
        vec3_t nvec;
        vec3_t org;

        Vec_Set3(svec, -info->screen_offset_x, -info->screen_offset_y, 0);
        Mtx_IdentityZ(mtx1, parent->rotation_offset + DEG2RAD(180));
        Vec_TransformToWorld(mtx1, svec, nvec);

        switch(info->drawtype)
        {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            Mtx_IdentityY(mtx3, DEG2RAD(90));
            break;
        default:
            Mtx_ApplyRotation(client.LocalPlayer().actor->rotation, mtx3);
            break;
        }

        Mtx_MultiplyRotation(mtx4, mtx1, mtx3);
        Vec_TransformToWorld(mtx4, nvec, org);
        Vec_Add(origin, origin, org);
    }

    return FX_Spawn(file, parent->source, parent->translation,
        origin, rot, parent->plane);
}

//
// FX_Event
//

static void FX_Event(fx_t *fx, fxEvent_t *fxEvent, gActor_t *actor)
{
    fx_t *nfx = NULL;

    if(fxEvent->fx != NULL)
        nfx = FX_SpawnChild(fx, fxEvent->fx);

    if(!nfx)
        nfx = fx;

    if(fxEvent->snd != NULL && nfx->refcount <= 0)
        Snd_PlayShader(fxEvent->snd, (gActor_t*)nfx);

    if(fxEvent->action.function != NULL && fx->source)
    {
        Actor_FXEvent(fx->source, actor, fx->origin,
            fx->translation, Map_PlaneToIndex(fx->plane),
            &fxEvent->action);
    }
}

//
// FX_TickEvent
//
// Same as FX_Event except that sounds are played on the parent actor
// and not on child fx thats spawned from it
//

static void FX_TickEvent(fx_t *fx, fxEvent_t *fxEvent, gActor_t *actor)
{
    fx_t *nfx = NULL;

    if(fxEvent->fx != NULL)
        FX_SpawnChild(fx, fxEvent->fx);

    if(fxEvent->snd != NULL && fx->refcount <= 0)
        Snd_PlayShader(fxEvent->snd, (gActor_t*)fx);

    if(fxEvent->action.function != NULL && fx->source)
    {
        Actor_FXEvent(fx->source, actor, fx->origin,
            fx->translation, Map_PlaneToIndex(fx->plane),
            &fxEvent->action);
    }
}

//
// FX_DestroyEvent
//

static void FX_DestroyEvent(fx_t *fx, fxEvent_t *fxEvent, gActor_t *hitActor)
{
    fx_t *nfx = NULL;

    fx->lifetime = 0;

    FX_Event(fx, fxEvent, hitActor);
    Vec_Set3(fx->translation, 0, 0, 0);
    Actor_SetTarget((gActor_t**)&fx->source, NULL);
}

//
// FX_SetDistance
//

static void FX_SetDistance(fx_t *fx)
{
    float s = (float)sin(client.LocalPlayer().actor->angles[0]);
    float c = (float)cos(client.LocalPlayer().actor->angles[0]);

    fx->dist = (int)((fx->origin[0] - client.LocalPlayer().actor->origin[0]) * s +
        (fx->origin[2] - client.LocalPlayer().actor->origin[2]) * c) * 0.5f;
}

//
// FX_SetRandValue
//

static int FX_SetRandValue(int randValue)
{
    return (randValue > 0) ? (rand() % (randValue + 1)) : -(rand() % (1 - randValue));
}

//
// FX_Link
//

fx_t *FX_Spawn(const char *name, gActor_t *source, vec3_t origin,
                vec3_t dest, vec4_t rotation, plane_t *plane)
{
    unsigned int i;
    int j;
    int k;
    fx_t *fx;
    fxfile_t *fxfile;
    fxinfo_t *info;
    vec3_t translation;

    if(!(fxfile = Kfx_Load(name)))
        return NULL;

    fx = NULL;

    for(i = 0; i < fxfile->numfx; i++)
    {
        kbool ok = true;
        int instances;
        int spawnDice;

        info = &fxfile->info[i];

        instances = info->instances.value;
        spawnDice = instances + FX_SetRandValue((int)info->instances.rand);

        if(spawnDice <= 0)
            continue;

        // allow only one FX instance to be spawned per actor
        if(source && info->bActorInstance)
        {
            for(fxRover = fxRoot.next; fxRover != &fxRoot; fxRover = fxRover->next)
            {
                if(!fxRover->source)
                    continue;

                if(fxRover->source == source && fxRover->info == fxfile->info)
                {
                    fxRover->bForcedRestart = true;
                    ok = false;
                    break;
                }
            }
        }

        if(!ok)
            continue;

        if(instances <= 0)
            instances = 1;

        for(k = 0; k < instances; k++)
        {
            fx = (fx_t*)Z_Calloc(sizeof(fx_t), PU_FX, 0);

            fxRoot.prev->next = fx;
            fx->next = &fxRoot;
            fx->prev = fxRoot.prev;
            fxRoot.prev = fx;

            // setup initial matrix
            if(info->bNoDirection)
                Mtx_Identity(fx->matrix);
            else
            {
                if(info->bProjectile && source && source->ai)
                {
                    if(source->ai->target)
                    {
                        vec3_t torg;

                        Vec_Copy3(torg, source->ai->target->origin);
                        torg[1] += 30.72f;

                        Vec_PointAt(dest, torg, source->rotation, 0, rotation);
                    }
                }
                
                Mtx_ApplyRotation(rotation, fx->matrix);
            }

            // bind target
            Actor_SetTarget((gActor_t**)&fx->source, source);

            // set default properties
            fx->file = fxfile;
            fx->info = info;
            fx->plane = plane;
            fx->frame = 0;
            fx->bAnimate = info->numTextures > 1 ? true : false;
            fx->bForcedRestart = false;
            fx->frametime = client.GetTime() + info->animspeed;
            fx->textures = (texture_t**)Z_Calloc(sizeof(texture_t*) *
                info->numTextures, PU_FX, 0);

            // setup texture lookup array
            for(j = 0; j < info->numTextures; j++)
                fx->textures[j] = Tex_CacheTextureFile(info->textures[j], DGL_CLAMP, true);

            // instances
            fx->instances = info->instances.value;

            // spawn delay time
            fx->restart = (float)FX_SetRandValue((int)info->restart);

            // life time
            fx->lifetime = (float)info->lifetime.value +
                FX_SetRandValue((int)info->lifetime.rand);

            // scale
            fx->scale = info->scale.value;
            if(info->scale.rand != 0)
                fx->scale += FX_Rand(info->scale.rand);

            // scale destination
            fx->scale_dest = info->scaledest.value;
            if(info->scaledest.rand != 0)
                fx->scale_dest += FX_Rand(info->scaledest.rand);

            // forward speed
            fx->forward = info->forward.value;
            if(info->forward.rand != 0)
                fx->forward += FX_Rand(info->forward.rand);

            // rotation offset
            fx->rotation_offset = info->rotation_offset.value;
            if(info->rotation_offset.rand != 0)
                fx->rotation_offset += FX_Rand(info->rotation_offset.rand);

            // rotation speed
            fx->rotation_speed = info->rotation_speed.value;
            if(info->rotation_speed.rand != 0)
                fx->rotation_speed += FX_Rand(info->rotation_speed.rand);

            // gravity
            fx->gravity = info->gravity.value;
            if(info->gravity.rand != 0)
                fx->gravity += FX_Rand(info->gravity.rand);

            //
            // process translation
            //
            Vec_Copy3(translation, info->translation.value);
            // TODO - FIXME
            translation[0] = -translation[0];

            FX_SetTranslationX(translation, translation, info->translation.rand[0]);
            FX_SetTranslationY(translation, translation, info->translation.rand[1]);
            FX_SetTranslationZ(translation, translation, info->translation.rand[2]);

            if(Vec_Unit3(translation) != 0)
            {
                vec3_t worldvec;
                vec3_t tvec;

                Vec_TransformToWorld(fx->matrix, translation, worldvec);

                tvec[0] = FX_RANDVAL();
                tvec[1] = FX_RANDVAL();
                tvec[2] = FX_RANDVAL();

                Vec_Normalize3(tvec);
                Vec_Normalize3(worldvec);
                Vec_Lerp3(translation,
                    info->translation_randomscale, worldvec, tvec);
            }

            if(fx->forward != 0)
                Vec_Scale(fx->translation, translation, fx->forward);
            else
                Vec_Copy3(fx->translation, translation);

            if(info->bAddOffset)
                Vec_Add(fx->translation, fx->translation, origin);

            //
            // process offsets
            //
            Vec_Copy3(fx->offset, info->offset.value);
            if(info->offset.rand[0] != 0) fx->offset[0] += FX_Rand(info->offset.rand[0]);
            if(info->offset.rand[1] != 0) fx->offset[1] += FX_Rand(info->offset.rand[1]);
            if(info->offset.rand[2] != 0) fx->offset[2] += FX_Rand(info->offset.rand[2]);

            Vec_Copy3(fx->origin, dest);

            if(Vec_Unit3(fx->offset) != 0)
            {
                vec3_t worldvec;

                // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
                Vec_TransformToWorld(fx->matrix, fx->offset, worldvec);
                Vec_Add(fx->origin, fx->origin, worldvec);
            }

            // TODO
            fx->color1[0] = info->color1[0];
            fx->color1[1] = info->color1[1];
            fx->color1[2] = info->color1[2];
            fx->color1[3] = info->fadein_time == 0 ? 0xff : 0;
            fx->color2[0] = info->color2[0];
            fx->color2[1] = info->color2[1];
            fx->color2[2] = info->color2[2];
            fx->color2[3] = info->fadein_time == 0 ? 0xff : 0;

            FX_SetDistance(fx);
        }
    }

    return fx;
}

//
// FX_Move
// Moves a particle towards its destination.
// Handle any clipping if needed
//

static void FX_Move(fx_t *fx, vec3_t velocity)
{
    vec3_t dest;
    float dist;
    int wl1;
    int wl2;
    fxinfo_t *fxinfo;

    if(fx->bStale)
        return;

    wl1 = Map_GetWaterLevel(fx->origin, 0, fx->plane);
    Vec_Add(dest, velocity, fx->origin);

    if(fx->plane == NULL)
    {
        Vec_Copy3(fx->origin, dest);
        return;
    }
    
    fxinfo = fx->info;

    if(Vec_Magnitude2(velocity) < 0.001f || fx->info->onplane == VFX_DEFAULT)
        Vec_Copy3(fx->origin, dest);
    else
    {
        trace_t trace;
        int flags;

        flags = PF_CLIPGEOMETRY | PF_CLIPEDGES | PF_DROPOFF | PF_SLIDEMOVE;

        if(fx->info->ontouch != VFX_DEFAULT)
            flags |= (PF_CLIPSTATICS | PF_CLIPACTORS);

        trace = Trace(fx->origin, dest, fx->plane, fx->source, flags);
        fx->plane = trace.pl;

        switch(trace.type)
        {
        case TRT_WALL:
        case TRT_EDGE:
            switch(fx->info->onplane)
            {
            case VFX_BOUNCE:
                G_ClipVelocity(fx->translation, fx->translation,
                    trace.normal, (1 + fxinfo->mass));
                break;
            case VFX_DESTROY:
                Vec_Copy3(fx->origin, trace.hitvec);
                FX_DestroyEvent(fx, &fx->info->onImpact, NULL);
                break;
            default:
                break;
            }
            break;
        case TRT_OBJECT:
            switch(fx->info->ontouch)
            {
            case VFX_BOUNCE:
                G_ClipVelocity(fx->translation, fx->translation,
                    trace.normal, (1 + fxinfo->mass));
                break;
            case VFX_DESTROY:
                Vec_Copy3(fx->origin, trace.hitvec);
                FX_DestroyEvent(fx, &fx->info->onImpact, trace.hitActor);
                break;
            default:
                Vec_Copy3(fx->origin, dest);
                break;
            }
            break;
        case TRT_NOHIT:
        case TRT_SLOPE:
            Vec_Copy3(fx->origin, dest);
            break;
        }
    }

    wl2 = Map_GetWaterLevel(fx->origin, 0, fx->plane);

    if((wl1 == WL_UNDER && wl2 == WL_OVER) ||
        (wl1 == WL_OVER && wl2 == WL_UNDER))
    {
        vec3_t oTmp;

        Vec_Copy3(oTmp, fx->origin);

        if(fx->plane)
            fx->origin[1] = (Map_GetArea(fx->plane)->waterplane + 4.096f);

        FX_Event(fx, &fx->info->onWaterImpact, NULL);
        if(fx->info->bDestroyOnWaterSurface)
        {
            fx->lifetime = 0;
            Vec_Set3(fx->translation, 0, 0, 0);
            Actor_SetTarget((gActor_t**)&fx->source, NULL);
        }

        Vec_Copy3(fx->origin, oTmp);
    }

    dist = fx->origin[1] -
        (Plane_GetDistance(fx->plane, fx->origin) +
        (fxinfo->bOffsetFromFloor ? 3.42f : 0));

    if(dist <= ONPLANE_EPSILON)
    {
        if(fxinfo->bStopAnimOnImpact)
            fx->bAnimate = false;

        switch(fxinfo->onplane)
        {
        case VFX_BOUNCE:
            G_ApplyBounceVelocity(fx->translation,
                fx->plane->normal, fxinfo->mass);

            // apply friction when sliding on the floor
            G_ApplyFriction(fx->translation, fxinfo->friction, false);
            break;
        case VFX_DESTROY:
            fx->origin[1] = fx->origin[1] - dist + 0.01f;
            FX_DestroyEvent(fx, &fx->info->onImpact, NULL);
            break;
        default:
            break;
        }
    }

    fx->translation[1] += (fx->gravity * client.GetRunTime());

    if(dist < 0.01f)
        fx->origin[1] = fx->origin[1] - dist + 0.01f;

    // clip against ceiling as well
    if(fx->plane->flags & CLF_CHECKHEIGHT)
    {
        dist = Plane_GetHeight(fx->plane, fx->origin);

        if((dist - fx->origin[1]) < 1.024f)
        {
            switch(fxinfo->onplane)
            {
            case VFX_BOUNCE:
                G_ApplyBounceVelocity(fx->translation,
                    fx->plane->ceilingNormal, fxinfo->mass);
                break;
            case VFX_DESTROY:
                fx->origin[1] = dist - 1.024f;
                FX_DestroyEvent(fx, &fx->info->onImpact, NULL);
                break;
            default:
                break;
            }
        }
    }
}

//
// FX_Ticker
//

void FX_Ticker(void)
{
    float time;

    if(client.GetRunTime() <= 0)
        return;

    time = 15 * client.GetRunTime();

    for(fxRover = fxRoot.next; fxRover != &fxRoot; fxRover = fxRover->next)
    {
        fx_t *fx;
        fxinfo_t *fxinfo;
        int lifetime;
        int alpha;
        vec3_t dest;
        kbool bUnderWater;

        if(fxRover == NULL)
            continue;

        fx = fxRover;
        fxinfo = fx->info;

        if(fx->bStale)
        {
            FX_Kill(fx);
            continue;
        }

        fx->restart -= time;

        // ready to spawn?
        if(fx->restart > 0)
            continue;

        bUnderWater = (Map_GetWaterLevel(fx->origin, 0, fx->plane) == WL_UNDER);

        //
        // handle 'on tick' events
        //
        if(bUnderWater)
            FX_TickEvent(fx, &fxinfo->onWaterTick, NULL);
        else
            FX_TickEvent(fx, &fxinfo->onTick, NULL);

        // animation effects
        if(fx->bAnimate)
        {
            if(fx->frametime < client.GetTime() && fxinfo->numTextures > 1)
            {
                if(fx->frame + 1 == fxinfo->numTextures)
                {
                    switch(fxinfo->animtype)
                    {
                    case VFX_ANIMONETIME:
                        fx->bAnimate = false;
                        break;

                    case VFX_ANIMLOOP:
                        fx->frame = 0;
                        fx->frametime = client.GetTime() + fxinfo->animspeed;
                        break;

                    default:
                        if(fx->bForcedRestart)
                        {
                            fx->frame = 0;
                            fx->bForcedRestart = false;
                            continue;
                        }
                        fx->bStale = true;
                        continue;
                    }
                }
                else
                {
                    fx->frametime = client.GetTime() + fxinfo->animspeed;
                    fx->frame++;
                }
            }
        }

        // update rotation
        fx->rotation_offset += (fx->rotation_speed * time);

        //
        // update scaling
        //
        if(!fxinfo->bScaleLerp)
        {
            float sdest = (fx->scale * fx->scale_dest);

            if(sdest > 4096)
                sdest = 4096;

            fx->scale = (sdest - fx->scale) * time + fx->scale;
        }
        else
        {
            float sdest = fx->scale - fx->scale_dest * (time * -10.24f);

            if(sdest > 4096)
                fx->scale = 4096;
            else
                fx->scale = sdest;
        }

        lifetime = (fxinfo->lifetime.value - (int)fx->lifetime);

        //
        // process fade in
        //
        alpha = fx->color1[3];

        if(lifetime < fxinfo->fadein_time)
        {
            alpha += (255 / (fxinfo->fadein_time + 1)) >> 1;

            if(alpha > 0xff)
                alpha = 0xff;
        }

        //
        // process fade out
        //
        if(fx->lifetime < fxinfo->fadeout_time)
        {
            alpha = (int)(255 * fx->lifetime / (fxinfo->fadeout_time + 1));

            if(alpha < 0)
                alpha = 0;
        }

        fx->color1[3] = alpha;
        fx->color2[3] = alpha;

        // update translation/velocity
        Vec_Scale(dest, fx->translation, client.GetRunTime());

        // process movement
        if(fx->gravity != 0 && (fx->origin[1] != 0 || fx->gravity >= 0) ||
            Vec_Magnitude3(dest) >= 0.001f)
        {
            FX_Move(fx, dest);
        }

        if(fx->source && fx->bAttachToSource)
        {
            vec3_t worldvec;

            Vec_Copy3(fx->origin, fx->source->origin);

            // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
            Vec_ApplyQuaternion(worldvec, fx->offset, fx->source->rotation);
            Vec_Add(fx->origin, fx->origin, worldvec);
        }

        FX_SetDistance(fx);

        // update lifetime
        fx->lifetime -= time;

        //
        // handle expire event
        //
        if(fx->lifetime < 0 || (fx->source && fx->source->bStale))
        {
            if(fx->bForcedRestart && fx->source && !fx->source->bStale)
            {
                fx->bForcedRestart = false;
                fx->lifetime = (float)fx->info->lifetime.value;
                continue;
            }

            if(bUnderWater)
                FX_Event(fx, &fxinfo->onWaterExpire, NULL);
            else
                FX_Event(fx, &fxinfo->onExpire, NULL);

            fx->bStale = true;
        }
    }
}

//
// FX_Kill
//

void FX_Kill(fx_t *fx)
{
    fx_t* next;

    if(fx->refcount > 0)
        return;

    /* Remove from main fx list */
    next = fxRover->next;

    (next->prev = fxRover = fx->prev)->next = next;
    Z_Free(fx);
}

//
// FX_ClearLinks
//

void FX_ClearLinks(void)
{
    Z_FreeTags(PU_FX, PU_FX);
    fxRoot.next = fxRoot.prev = &fxRoot;
}

//
// FCmd_SpawnFX
//

static void FCmd_SpawnFX(void)
{
    plane_t *plane;
    vec3_t vec;

    if(command.GetArgc() < 2)
        return;

    if(client.LocalPlayer().actor == NULL)
        return;

    if(!(plane = Map_FindClosestPlane(client.LocalPlayer().actor->origin)))
        return;

    Vec_Set3(vec,
        client.LocalPlayer().actor->rotation[0],
        client.LocalPlayer().actor->rotation[1],
        client.LocalPlayer().actor->rotation[2]);

    FX_Spawn(command.GetArgv(1),
        client.LocalPlayer().actor,
        vec,
        client.LocalPlayer().actor->origin,
        client.LocalPlayer().actor->rotation, plane);
}

//
// FX_Init
//

void FX_Init(void)
{
    command.Add("spawnfx", FCmd_SpawnFX);
    FX_ClearLinks();

    kexStrListMem *fxList = fileSystem.GetMatchingFiles("fx/");
    for(unsigned int i = 0; i < fxList->Length(); i++) {
        fxfile_t *fxfile = Kfx_Load(fxList->GetData(i)->c_str());

        if(!fxfile)
            continue;

        for(unsigned int j = 0; j < fxfile->numfx; j++) {
            fxinfo_t *info;

            info = &fxfile->info[j];

            for(int k = 0; k < info->numTextures; k++)
                Tex_CacheTextureFile(info->textures[k], DGL_CLAMP, true);
        }
    }

    delete fxList;
}
