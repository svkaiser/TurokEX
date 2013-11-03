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
#include "renderSystem.h"
#include "world.h"

fx_t fxRoot;
fx_t *fxRover;

#define FX_RANDVAL() (float)((kexRand::SysRand() % 20000) - 10000) * 0.0001f

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
    return kexRand::Float() * val;
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

    //if(fxEvent->snd != NULL && nfx->refcount <= 0)
        //soundSystem.PlaySound(fxEvent->snd, (gActor_t*)nfx);

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

    //if(fxEvent->snd != NULL && fx->refcount <= 0)
        //soundSystem.PlaySound(fxEvent->snd, (gActor_t*)fx);

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
    float s = kexMath::Sin(client.LocalPlayer().actor->angles[0]);
    float c = kexMath::Cos(client.LocalPlayer().actor->angles[0]);

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
    /*plane_t *plane;
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
        client.LocalPlayer().actor->rotation, plane);*/

    kexCamera *camera = localWorld.Camera();
    kexVec3 forward = camera->GetAngles().ToForwardAxis();

    traceInfo_t trace;

    trace.owner = NULL;
    trace.bUseBBox = false;
    trace.start = camera->GetOrigin();
    trace.end = trace.start + (forward * 1024);
    trace.dir = forward.Normalize();

    localWorld.Trace(&trace);
    if(trace.fraction != 1) {
        localWorld.SpawnFX("fx/default.kfx", NULL, kexVec3(0, 0, 0),
            trace.hitVector - (trace.dir * 8.192f), camera->GetRotation());
    }
}

//
// FX_Init
//

void FX_Init(void)
{
    command.Add("spawnfx", FCmd_SpawnFX);
    FX_ClearLinks();

    if(common.CheckParam("-nofxcache") == 0) {
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
}

kexFxManager fxManager;

DECLARE_CLASS(kexFx, kexActor)

//
// kexFx::kexFx
//

kexFx::kexFx(void) {
    this->worldLink.SetData(this);
}

//
// kexFx::~kexFx
//

kexFx::~kexFx(void) {
}

//
// kexFx::LocalTick
//

void kexFx::LocalTick(void) {
    float time;
    int currentLifeTime;
    int alpha;
    kexVec3 dest;
    bool bUnderWater;

    if(client.GetRunTime() <= 0) {
        return;
    }

    time = 10.24f * client.GetRunTime();

    restart -= time;

    // ready to spawn?
    if(restart > 0) {
        return;
    }

    //bUnderWater = (Map_GetWaterLevel(origin, 0, fx->plane) == WL_UNDER);
    bUnderWater = false;

    //
    // handle 'on tick' events
    //
    if(bUnderWater) {
        Event(&fxInfo->onWaterTick, NULL);
    }
    else {
        Event(&fxInfo->onTick, NULL);
    }

    // animation effects
    if(bAnimate) {
        if(frameTime < client.GetTime() && fxInfo->numTextures > 1) {
            if(frame + 1 == fxInfo->numTextures) {
                switch(fxInfo->animtype) {
                case VFX_ANIMONETIME:
                    bAnimate = false;
                    break;
                case VFX_ANIMLOOP:
                    frame = 0;
                    frameTime = client.GetTime() + fxInfo->animspeed;
                    break;
                default:
                    if(bForcedRestart) {
                        frame = 0;
                        bForcedRestart = false;
                        return;
                    }
                    Remove();
                    return;
                }
            }
            else
            {
                frameTime = client.GetTime() + fxInfo->animspeed;
                frame++;
            }
        }
    }

    // update rotation
    rotationOffset += (rotationSpeed * time);

    //
    // update scaling
    //
    if(!fxInfo->bScaleLerp) {
        float sdest = (drawScale * drawScaleDest);

        if(sdest > 4096) {
            sdest = 4096;
        }

        drawScale = (sdest - drawScale) * time + drawScale;
    }
    else {
        float sdest = drawScale - drawScaleDest * (time * -10.24f);

        if(sdest > 4096) {
            drawScale = 4096;
        }
        else {
            drawScale = sdest;
        }
    }

    currentLifeTime = (fxInfo->lifetime.value - (int)lifeTime);

    //
    // process fade in
    //
    alpha = color1[3];

    if(currentLifeTime < fxInfo->fadein_time) {
        alpha += (255 / (fxInfo->fadein_time + 1)) >> 1;

        if(alpha > 0xff) {
            alpha = 0xff;
        }
    }

    //
    // process fade out
    //
    if(lifeTime < fxInfo->fadeout_time) {
        alpha = (int)(255 * lifeTime / (fxInfo->fadeout_time + 1));

        if(alpha < 0) {
            alpha = 0;
        }
    }

    color1[3] = alpha;
    color2[3] = alpha;

    // update translation/velocity
    dest = physics.velocity * client.GetRunTime();

    // process movement
    if(gravity != 0 && gravity >= 0 || dest.UnitSq() >= 0.001f) {
        //FX_Move(fx, dest);
    }

    if(owner && bAttachToSource) {
        // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
        SetOrigin(owner->GetOrigin() + (offset | owner->GetRotation()));
    }

    SetViewDistance();

    // update lifetime
    lifeTime -= time;

    //
    // handle expire event
    //
    if(lifeTime < 0 || (owner && owner->Removing())) {
        if(bForcedRestart && owner && !owner->Removing()) {
            bForcedRestart = false;
            lifeTime = (float)fxInfo->lifetime.value;
            return;
        }

        if(bUnderWater) {
            Event(&fxInfo->onWaterExpire, NULL);
        }
        else {
            Event(&fxInfo->onExpire, NULL);
        }

        Remove();
    }
}

//
// kexFx::Tick
//

void kexFx::Tick(void) {
}

//
// kexFx::SpawnChild
//

kexFx *kexFx::SpawnChild(const char *name) {
    kexQuat rot = physics.velocity.ToQuat();
    kexVec3 org = origin;

    if(fxInfo->screen_offset_x != 0 || fxInfo->screen_offset_y != 0) {
        kexVec3 svec;
        kexVec3 nvec;

        svec.Set(-fxInfo->screen_offset_x, -fxInfo->screen_offset_y, 0);

        kexMatrix mtx1(rotationOffset + DEG2RAD(180), 2);
        kexMatrix mtx3;

        nvec = (svec | mtx1);

        switch(fxInfo->drawtype) {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            mtx3 = kexMatrix(DEG2RAD(90), 1);
            break;
        default:
            mtx3 = kexMatrix(localWorld.Camera()->GetRotation());
            break;
        }

        origin += (nvec | (mtx1 | mtx3));
    }

    return localWorld.SpawnFX(name, owner, physics.velocity, org, rot);
}

//
// kexFx::Event
//

void kexFx::Event(fxEvent_t *fxEvent, kexActor *target) {
    kexFx *nfx = NULL;

    if(fxEvent->fx != NULL) {
        nfx = SpawnChild(fxEvent->fx);
    }

    if(!nfx) {
        nfx = this;
    }

    //if(fxEvent->snd != NULL && nfx->refcount <= 0)
        //soundSystem.PlaySound(fxEvent->snd, (gActor_t*)nfx);

    if(fxEvent->action.function != NULL && owner) {
        //Actor_FXEvent(fx->source, actor, fx->origin,
            //fx->translation, Map_PlaneToIndex(fx->plane),
            //&fxEvent->action);
    }
}

//
// kexFx::SetViewDistance
//

void kexFx::SetViewDistance(void) {
    kexCamera *camera = localWorld.Camera();

    float s = kexMath::Sin(camera->GetAngles().yaw);
    float c = kexMath::Cos(camera->GetAngles().yaw);

    distance = (int)((origin.x - camera->GetOrigin().x) * s +
        (origin.z - camera->GetOrigin().z) * c) * 0.5f;
}

//
// kexFx::Spawn
//

void kexFx::Spawn(void) {
    int i;
    kexVec3 vel;
    kexVec3 destVel;

    // setup initial matrix
    if(fxInfo->bNoDirection) {
        matrix.Identity();
    }
    else {
        /*if(fxInfo->bProjectile && source && source->ai) {
            if(source->ai->target) {
                vec3_t torg;

                Vec_Copy3(torg, source->ai->target->origin);
                torg[1] += 30.72f;

                Vec_PointAt(dest, torg, source->rotation, 0, rotation);
            }
        }*/
        
        matrix = kexMatrix(rotation);
    }

    bAnimate = fxInfo->numTextures > 1 ? true : false;
    bForcedRestart = false;
    frameTime = client.GetTime() + fxInfo->animspeed;
    textures = (kexTexture**)Z_Calloc(sizeof(kexTexture*) *
        fxInfo->numTextures, PU_FX, 0);

    // setup texture lookup array
    for(i = 0; i < fxInfo->numTextures; i++) {
        textures[i] = renderSystem.CacheTexture(fxInfo->textures[i], TC_CLAMP, TF_LINEAR);
    }

    // instances
    instances = fxInfo->instances.value;

    // spawn delay time
    restart = (float)FX_SetRandValue((int)fxInfo->restart);

    // life time
    lifeTime = (float)fxInfo->lifetime.value +
        FX_SetRandValue((int)fxInfo->lifetime.rand);

    // scale
    drawScale = fxInfo->scale.value;
    if(fxInfo->scale.rand != 0) {
        drawScale += FX_Rand(fxInfo->scale.rand);
    }

    // scale destination
    drawScaleDest = fxInfo->scaledest.value;
    if(fxInfo->scaledest.rand != 0) {
        drawScaleDest += FX_Rand(fxInfo->scaledest.rand);
    }

    // forward speed
    speed = fxInfo->forward.value;
    if(fxInfo->forward.rand != 0) {
        speed += FX_Rand(fxInfo->forward.rand);
    }

    // rotation offset
    rotationOffset = fxInfo->rotation_offset.value;
    if(fxInfo->rotation_offset.rand != 0) {
        rotationOffset += FX_Rand(fxInfo->rotation_offset.rand);
    }

    // rotation speed
    rotationSpeed = fxInfo->rotation_speed.value;
    if(fxInfo->rotation_speed.rand != 0) {
        rotationSpeed += FX_Rand(fxInfo->rotation_speed.rand);
    }

    // gravity
    gravity = fxInfo->gravity.value;
    if(fxInfo->gravity.rand != 0) {
        gravity += FX_Rand(fxInfo->gravity.rand);
    }

    //
    // process translation
    //
    destVel.Set(
        fxInfo->translation.value[0],
        fxInfo->translation.value[1],
        fxInfo->translation.value[2]);

    // TODO - FIXME
    destVel.x = -destVel.x;

    // randomize velocity x
    vel = destVel;
    vel.x = FX_RANDVAL();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[0]);

    // randomize velocity y
    vel = destVel;
    vel.y = FX_RANDVAL();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[1]);

    // randomize velocity z
    vel = destVel;
    vel.z = FX_RANDVAL();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[2]);

    // randomize global velocity
    if(destVel.Unit() != 0) {
        kexVec3 worldVec;
        kexVec3 tVec;

        worldVec = (destVel | matrix);
        tVec.Set(FX_RANDVAL(), FX_RANDVAL(), FX_RANDVAL());

        worldVec.Normalize();
        tVec.Normalize();

        destVel = worldVec.Lerp(tVec, fxInfo->translation_randomscale);
    }

    if(speed != 0) {
        physics.velocity = (destVel * speed);
    }
    else {
        physics.velocity = destVel;
    }

    if(fxInfo->bAddOffset) {
        physics.velocity += velOffset;
    }

    //
    // process offsets
    //
    offset.Set(
        fxInfo->offset.value[0],
        fxInfo->offset.value[1],
        fxInfo->offset.value[2]);

    if(fxInfo->offset.rand[0] != 0) offset.x += FX_Rand(fxInfo->offset.rand[0]);
    if(fxInfo->offset.rand[1] != 0) offset.y += FX_Rand(fxInfo->offset.rand[1]);
    if(fxInfo->offset.rand[2] != 0) offset.z += FX_Rand(fxInfo->offset.rand[2]);

    // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
    if(offset.Unit() != 0) {
        origin += (offset | matrix);
    }

    // TODO
    color1[0] = fxInfo->color1[0];
    color1[1] = fxInfo->color1[1];
    color1[2] = fxInfo->color1[2];
    color1[3] = fxInfo->fadein_time == 0 ? 0xff : 0;
    color2[0] = fxInfo->color2[0];
    color2[1] = fxInfo->color2[1];
    color2[2] = fxInfo->color2[2];
    color2[3] = fxInfo->fadein_time == 0 ? 0xff : 0;

    SetViewDistance();
}

enum {
    scvfx_bFadeout = 0,
    scvfx_bStopAnimOnImpact,
    scvfx_bOffsetFromFloor,
    scvfx_bTextureWrapMirror,
    scvfx_bDepthBuffer,
    scvfx_bLensFlares,
    scvfx_bBlood,
    scvfx_bAddOffset,
    scvfx_bScaleLerp,
    scvfx_bNoDirection,
    scvfx_bLocalAxis,
    scvfx_bProjectile,
    scvfx_bActorInstance,
    scvfx_mass,
    scvfx_translation_global_randomscale,
    scvfx_translation_randomscale,
    scvfx_translation,
    scvfx_gravity,
    scvfx_gravity_randomscale,
    scvfx_friction,
    scvfx_animFriction,
    scvfx_scale,
    scvfx_scale_randomscale,
    scvfx_scaledest,
    scvfx_scaledest_randomscale,
    scvfx_forward,
    scvfx_forward_randomscale,
    scvfx_offset_randomscale,
    scvfx_rotation_offset,
    scvfx_rotation_offset_randomscale,
    scvfx_rotation_speed,
    scvfx_rotation_speed_randomscale,
    scvfx_screen_offset_x,
    scvfx_screen_offset_y,
    scvfx_offset,
    scvfx_texture,
    scvfx_instances,
    scvfx_instances_randomscale,
    scvfx_lifetime,
    scvfx_lifetime_randomscale,
    scvfx_restart,
    scvfx_animspeed,
    scvfx_ontouch,
    scvfx_onplane,
    scvfx_drawtype,
    scvfx_color1,
    scvfx_color2,
    scvfx_color1_randomscale,
    scvfx_color2_randomscale,
    scvfx_saturation_randomscale,
    scvfx_fadein_time,
    scvfx_fadeout_time,
    scvfx_animtype,
    scvfx_onHitSurface,
    scvfx_onExpire,
    scvfx_onTick,
    scvfx_onWaterHit,
    scvfx_onWaterExpire,
    scvfx_onWaterTick,
    scvfx_bDestroyOnWaterSurface,
    scvfx_end
};

static const sctokens_t vfxtokens[scvfx_end+1] = {
    { scvfx_bFadeout,                       "bFadeout"                          },
    { scvfx_bStopAnimOnImpact,              "bStopAnimOnImpact"                 },
    { scvfx_bOffsetFromFloor,               "bOffsetFromFloor"                  },
    { scvfx_bTextureWrapMirror,             "bTextureWrapMirror"                },
    { scvfx_bDepthBuffer,                   "bDepthBuffer"                      },
    { scvfx_bLensFlares,                    "bLensFlares"                       },
    { scvfx_bBlood,                         "bBlood"                            },
    { scvfx_bAddOffset,                     "bAddOffset"                        },
    { scvfx_bScaleLerp,                     "bScaleLerp"                        },
    { scvfx_mass,                           "mass"                              },
    { scvfx_translation_global_randomscale, "translation_global_randomscale"    },
    { scvfx_translation_randomscale,        "translation_randomscale"           },
    { scvfx_translation,                    "translation"                       },
    { scvfx_gravity,                        "gravity"                           },
    { scvfx_gravity_randomscale,            "gravity_randomscale"               },
    { scvfx_friction,                       "friction"                          },
    { scvfx_animFriction,                   "animFriction"                      },
    { scvfx_scale,                          "scale"                             },
    { scvfx_scale_randomscale,              "scale_randomscale"                 },
    { scvfx_scaledest,                      "scale_dest"                        },
    { scvfx_scaledest_randomscale,          "scale_dest_randomscale"            },
    { scvfx_forward,                        "forward_speed"                     },
    { scvfx_forward_randomscale,            "forward_speed_randomscale"         },
    { scvfx_offset_randomscale,             "offset_random"                     },
    { scvfx_rotation_offset,                "rotation_offset"                   },
    { scvfx_rotation_offset_randomscale,    "rotation_offset_randomscale"       },
    { scvfx_rotation_speed,                 "rotation_speed"                    },
    { scvfx_rotation_speed_randomscale,     "rotation_speed_randomscale"        },
    { scvfx_screen_offset_x,                "screen_offset_x"                   },
    { scvfx_screen_offset_y,                "screen_offset_y"                   },
    { scvfx_offset,                         "offset"                            },
    { scvfx_texture,                        "textures"                          },
    { scvfx_instances,                      "instances"                         },
    { scvfx_instances_randomscale,          "instances_randomscale"             },
    { scvfx_lifetime,                       "lifetime"                          },
    { scvfx_lifetime_randomscale,           "lifetime_randomscale"              },
    { scvfx_restart,                        "restart"                           },
    { scvfx_animspeed,                      "animspeed"                         },
    { scvfx_ontouch,                        "ontouch"                           },
    { scvfx_onplane,                        "onplane"                           },
    { scvfx_drawtype,                       "drawtype"                          },
    { scvfx_color1,                         "color1"                            },
    { scvfx_color2,                         "color2"                            },
    { scvfx_color1_randomscale,             "color1_randomscale"                },
    { scvfx_color2_randomscale,             "color2_randomscale"                },
    { scvfx_saturation_randomscale,         "saturation_randomscale"            },
    { scvfx_fadein_time,                    "fadein_time"                       },
    { scvfx_fadeout_time,                   "fadeout_time"                      },
    { scvfx_animtype,                       "animtype"                          },
    { scvfx_onHitSurface,                   "onHitSurface"                      },
    { scvfx_onExpire,                       "onExpire"                          },
    { scvfx_onTick,                         "onTick"                            },
    { scvfx_bNoDirection,                   "bNoDirection"                      },
    { scvfx_bLocalAxis,                     "bLocalAxis"                        },
    { scvfx_bProjectile,                    "bProjectile"                       },
    { scvfx_bActorInstance,                 "bActorInstance"                    },
    { scvfx_onWaterHit,                     "onWaterImpact"                     },
    { scvfx_onWaterExpire,                  "onWaterExpire"                     },
    { scvfx_onWaterTick,                    "onWaterTick"                       },
    { scvfx_bDestroyOnWaterSurface,         "bDestroyOnWaterSurface",           },
    { -1,                                   NULL                                }
};

//
// kexFxManager::kexFxManager
//

kexFxManager::kexFxManager(void) {
}

//
// kexFxManager::~kexFxManager
//

kexFxManager::~kexFxManager(void) {
}

//
// kexFxManaager::Init
//

void kexFxManager::Init(void) {
    command.Add("spawnfx", FCmd_SpawnFX);

    if(common.CheckParam("-nofxcache") == 0) {
        kexStrListMem *fxList = fileSystem.GetMatchingFiles("fx/");
        for(unsigned int i = 0; i < fxList->Length(); i++) {
            fxfile_t *fxfile = LoadKFX(fxList->GetData(i)->c_str());

            if(!fxfile) {
                continue;
            }

            for(unsigned int j = 0; j < fxfile->numfx; j++) {
                fxinfo_t *info;

                info = &fxfile->info[j];

                for(int k = 0; k < info->numTextures; k++) {
                    renderSystem.CacheTexture(info->textures[k], TC_CLAMP, TF_LINEAR);
                }
            }
        }

        delete fxList;
    }
}

//
// kexFxManager::Shutdown
//

void kexFxManager::Shutdown(void) {
    Z_FreeTags(PU_FX, PU_FX);
}

//
// kexFxManager::RandValue
//

int kexFxManager::RandValue(int randValue) {
    return (randValue > 0) ? (kexRand::SysRand() % (randValue + 1)) :
        -(kexRand::SysRand() % (1 - randValue));
}

//
// kexFxManager::ParseEvent
//

void kexFxManager::ParseEvent(fxEvent_t *fxEvent, kexLexer *lexer) {
    lexer->ExpectNextToken(TK_LBRACK);
    while(1) {
        lexer->Find();
        if(!strcmp(lexer->Token(), "fx")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            fxEvent->fx = Z_Strndup(lexer->StringToken(),
                MAX_FILEPATH, PU_STATIC, 0);
        }
        else if(!strcmp(lexer->Token(), "sound")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            fxEvent->snd = Z_Strndup(lexer->StringToken(),
                MAX_FILEPATH, PU_STATIC, 0);
        }
        else if(!strcmp(lexer->Token(), "action")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);

            lexer->GetString();
            fxEvent->action.function = Z_Strdup(lexer->StringToken(),
                PU_STATIC, 0);
            fxEvent->action.args[0] = (float)lexer->GetFloat();

            lexer->ExpectNextToken(TK_RBRACK);
        }
        else if(lexer->TokenType() == TK_RBRACK)
            break;
        else {
            parser.Error("kexFxManager::ParseEvent: Unknown token: %s\n",
                lexer->Token());
        }
    }
}

#define CHECK_BOOL(name)                                                        \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, &tmp, scvfx_ ##name, false);          \
    info-> ##name = (tmp != 0);                                                 \
    break

#define CHECK_INT(name)                                                         \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, (unsigned int*)&info-> ##name,        \
        scvfx_ ##name, false);                                                  \
    break

#define CHECK_FLOAT(name, prop)                                                 \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, &info-> ##prop,                       \
        scvfx_ ##name, false);                                                  \
    break

#define CHECK_VFXINT(name)                                                      \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, (unsigned int*)&info-> ##name .value, \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
    lexer->AssignFromTokenList(vfxtokens, &info-> ##name .rand,                 \
        scvfx_ ##name## _randomscale, false);                                   \
    break

#define CHECK_VFXFLOAT(name)                                                    \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, &info-> ##name .value,                \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
    lexer->AssignFromTokenList(vfxtokens, &info-> ##name .rand,                 \
        scvfx_ ##name## _randomscale, false);                                   \
    break

#define CHECK_VFXVECTOR(name)                                                   \
    case scvfx_ ##name:                                                         \
    lexer->AssignVectorFromTokenList(vfxtokens, info-> ##name .value,           \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
    lexer->AssignVectorFromTokenList(vfxtokens, info-> ##name .rand,            \
        scvfx_ ##name## _randomscale, false);                                   \
    break

//
// kexFxManager::LoadKFX
//

fxfile_t *kexFxManager::LoadKFX(const char *file) {
    fxfile_t *fx = NULL;
    unsigned int tmp = 0;

    if(file == NULL || file[0] == 0)
        return NULL;

    if(!(fx = kfxList.Find(file))) {
        kexLexer *lexer;
        unsigned int i;

        if(strlen(file) >= MAX_FILEPATH) {
            common.Error("kexFxManager::LoadKFX: \"%s\" is too long", file);
        }

        if(!(lexer = parser.Open(file))) {
            return NULL;
        }

        fx = kfxList.Add(file);

        lexer->Find();

        if(strcmp(lexer->Token(), "fx")) {
            common.Error("kexFxManager::LoadKFX: Expected 'fx', found %s", lexer->Token());
        }

        lexer->ExpectNextToken(TK_LSQBRACK);

        fx->numfx = lexer->GetNumber();
        fx->info = (fxinfo_t*)Z_Calloc(sizeof(fxinfo_t) * fx->numfx, PU_STATIC, 0);

        lexer->ExpectNextToken(TK_RSQBRACK);
        lexer->ExpectNextToken(TK_EQUAL);
        lexer->ExpectNextToken(TK_LBRACK);

        for(i = 0; i < fx->numfx; i++) {
            fxinfo_t *info = &fx->info[i];
            int j;

            lexer->ExpectNextToken(TK_LBRACK);
            lexer->Find();

            while(lexer->TokenType() != TK_RBRACK) {
                switch(lexer->GetIDForTokenList(vfxtokens, lexer->Token())) {
                CHECK_BOOL(bFadeout);
                CHECK_BOOL(bStopAnimOnImpact);
                CHECK_BOOL(bOffsetFromFloor);
                CHECK_BOOL(bTextureWrapMirror);
                CHECK_BOOL(bDepthBuffer);
                CHECK_BOOL(bLensFlares);
                CHECK_BOOL(bBlood);
                CHECK_BOOL(bAddOffset);
                CHECK_BOOL(bScaleLerp);
                CHECK_BOOL(bNoDirection);
                CHECK_BOOL(bLocalAxis);
                CHECK_BOOL(bProjectile);
                CHECK_BOOL(bDestroyOnWaterSurface);
                CHECK_BOOL(bActorInstance);

                CHECK_INT(animspeed);
                CHECK_INT(color1_randomscale);
                CHECK_INT(color2_randomscale);
                CHECK_INT(saturation_randomscale);
                CHECK_INT(fadein_time);
                CHECK_INT(fadeout_time);

                CHECK_FLOAT(mass, mass);
                CHECK_FLOAT(translation_global_randomscale, translation_randomscale);
                CHECK_FLOAT(screen_offset_x, screen_offset_x);
                CHECK_FLOAT(screen_offset_y, screen_offset_y);
                CHECK_FLOAT(restart, restart);
                CHECK_FLOAT(friction, friction);
                CHECK_FLOAT(animFriction, animFriction);

                CHECK_VFXINT(instances);
                CHECK_VFXINT(lifetime);

                CHECK_VFXFLOAT(gravity);
                CHECK_VFXFLOAT(scale);
                CHECK_VFXFLOAT(scaledest);
                CHECK_VFXFLOAT(forward);
                CHECK_VFXFLOAT(rotation_offset);
                CHECK_VFXFLOAT(rotation_speed);

                CHECK_VFXVECTOR(translation);
                CHECK_VFXVECTOR(offset);

                case scvfx_texture:
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    info->numTextures = lexer->GetNumber();
                    info->textures = (char**)Z_Calloc(sizeof(char*) *
                        info->numTextures, PU_STATIC, 0);
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(j = 0; j < info->numTextures; j++) {
                        lexer->GetString();
                        info->textures[j] = Z_Strndup(lexer->StringToken(),
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                    break;
                case scvfx_color1:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    info->color1[0] = lexer->GetNumber();
                    info->color1[1] = lexer->GetNumber();
                    info->color1[2] = lexer->GetNumber();
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    break;
                case scvfx_color2:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LSQBRACK);
                    info->color2[0] = lexer->GetNumber();
                    info->color2[1] = lexer->GetNumber();
                    info->color2[2] = lexer->GetNumber();
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    break;
                case scvfx_ontouch:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(!strcmp(lexer->Token(), "destroy"))
                        info->ontouch = VFX_DESTROY;
                    else if(!strcmp(lexer->Token(), "reflect"))
                        info->ontouch = VFX_REFLECT;
                    else
                        info->ontouch = VFX_DEFAULT;
                    break;
                case scvfx_onplane:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(!strcmp(lexer->Token(), "destroy"))
                        info->onplane = VFX_DESTROY;
                    else if(!strcmp(lexer->Token(), "reflect"))
                        info->onplane = VFX_REFLECT;
                    else if(!strcmp(lexer->Token(), "bounce"))
                        info->onplane = VFX_BOUNCE;
                    else
                        info->onplane = VFX_DEFAULT;
                    break;
                case scvfx_drawtype:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(!strcmp(lexer->Token(), "flat"))
                        info->drawtype = VFX_DRAWFLAT;
                    else if(!strcmp(lexer->Token(), "decal"))
                        info->drawtype = VFX_DRAWDECAL;
                    else if(!strcmp(lexer->Token(), "billboard"))
                        info->drawtype = VFX_DRAWBILLBOARD;
                    else
                        info->drawtype = VFX_DRAWDEFAULT;
                    break;
                case scvfx_animtype:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(!strcmp(lexer->Token(), "onetime"))
                        info->animtype = VFX_ANIMONETIME;
                    else if(!strcmp(lexer->Token(), "loop"))
                        info->animtype = VFX_ANIMLOOP;
                    else if(!strcmp(lexer->Token(), "sinwave"))
                        info->animtype = VFX_ANIMSINWAVE;
                    else
                        info->animtype = VFX_ANIMDEFAULT;
                    break;
                case scvfx_onHitSurface:
                    ParseEvent(&info->onImpact, lexer);
                    break;
                case scvfx_onExpire:
                    ParseEvent(&info->onExpire, lexer);
                    break;
                case scvfx_onTick:
                    ParseEvent(&info->onTick, lexer);
                    break;
                case scvfx_onWaterHit:
                    ParseEvent(&info->onWaterImpact, lexer);
                    break;
                case scvfx_onWaterExpire:
                    ParseEvent(&info->onWaterExpire, lexer);
                    break;
                case scvfx_onWaterTick:
                    ParseEvent(&info->onWaterTick, lexer);
                    break;
                default:
                    if(lexer->TokenType() == TK_IDENIFIER) {
                        parser.Error("kexFxManager::LoadKFX: Unknown token: %s\n",
                            lexer->Token());
                    }
                    break;
                }

                if(info->translation.value[0] > 1.0f)  info->translation.value[0] -= 1.0f;
                if(info->translation.value[0] < -1.0f) info->translation.value[0] += 1.0f;
                if(info->translation.value[1] > 1.0f)  info->translation.value[1] -= 1.0f;
                if(info->translation.value[1] < -1.0f) info->translation.value[1] += 1.0f;
                if(info->translation.value[2] > 1.0f)  info->translation.value[2] -= 1.0f;
                if(info->translation.value[2] < -1.0f) info->translation.value[2] += 1.0f;

                lexer->Find();
            }
        }

        lexer->ExpectNextToken(TK_RBRACK);
        parser.Close();
    }

    return fx;
}

#undef CHECK_BOOL
#undef CHECK_INT
#undef CHECK_FLOAT
#undef CHECK_VFXINT
#undef CHECK_VFXFLOAT
#undef CHECK_VFXVECTOR
