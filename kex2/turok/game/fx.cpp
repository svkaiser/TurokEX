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
#include "script.h"
#include "mathlib.h"
#include "fx.h"
#include "client.h"
#include "sound.h"
#include "fileSystem.h"
#include "renderSystem.h"
#include "world.h"

//
// FCmd_SpawnFX
//

static void FCmd_SpawnFX(void) {
    kexCamera *camera = localWorld.Camera();
    kexVec3 forward = camera->GetAngles().ToForwardAxis();
    
    localWorld.SpawnFX("fx/default.kfx", NULL, kexVec3(0, 0, 0),
        camera->GetOrigin() + (forward * 16.384f), camera->GetRotation());
}

kexFxManager fxManager;

DECLARE_CLASS(kexFxPhysics, kexPhysics)

//
// kexFxPhysics::kexFxPhysics
//

kexFxPhysics::kexFxPhysics(void) {
    this->mass                  = 1800;
    this->friction              = 1;
    this->airFriction           = 0;
    this->bounceDamp            = 0;
    this->rotorSpeed            = 0;
    this->rotorFriction         = 1;
    this->bRotor                = false;
    this->bOrientOnSlope        = false;
    this->bOnGround             = false;
    this->waterLevel            = WLT_INVALID;
    this->groundGeom            = NULL;

    this->rotorVector.Clear();
    this->velocity.Clear();
}

//
// kexFxPhysics::~kexFxPhysics
//

kexFxPhysics::~kexFxPhysics(void) {
}

//
// kexFxPhysics::Think
//

void kexFxPhysics::Think(const float timeDelta) {
    kexVec3 move;
    kexFx *fx;
    float moveAmount;
    fxinfo_t *fxinfo;
    traceInfo_t trace;
    kexVec3 start;

    if(owner == NULL) {
        return;
    }

    velocity += (localWorld.GetGravity() * (mass * timeDelta));

    move = velocity * timeDelta;
    moveAmount = move.UnitSq();

    if(moveAmount < 0.001f) {
        return;
    }

    fx = static_cast<kexFx*>(owner);
    start = owner->GetOrigin();

    if(owner->bCollision == false) {
        owner->SetOrigin(start + move);
        return;
    }

    fxinfo = fx->fxInfo;

    if(moveAmount < 0.001f || fxinfo->onplane == VFX_DEFAULT) {
        owner->SetOrigin(start + move);
    }
    else {
        trace.owner = owner;
        trace.bUseBBox = false;
        trace.start = start;
        trace.end = start + move;
        trace.dir = (trace.end - start).Normalize();

        localWorld.Trace(&trace);

        if(trace.fraction >= 1) {
            owner->SetOrigin(start + move);
        }
        else {
            owner->SetOrigin(trace.hitVector - (trace.dir * 0.125f));

            if(trace.hitActor != NULL) {
                switch(fxinfo->ontouch) {
                    case VFX_BOUNCE:
                        if(fxinfo->bStopAnimOnImpact) {
                            fx->bAnimate = false;
                        }
                        ImpactVelocity(velocity, trace.hitNormal, 1.05f);
                        break;
                    case VFX_DESTROY:
                        owner->GetOrigin() += (trace.hitNormal * 1.024f);
                        fx->Event(&fxinfo->onImpact, trace.hitActor);
                        fx->Remove();
                        break;
                    default:
                        break;
                }
            }

            if(trace.hitTri != NULL) {
                groundGeom = trace.hitTri;

                switch(fxinfo->onplane) {
                    case VFX_BOUNCE:
                        if(fxinfo->bStopAnimOnImpact) {
                            fx->bAnimate = false;
                        }
                        ImpactVelocity(velocity, trace.hitNormal, 1.05f);
                        ApplyFriction();
                        break;
                    case VFX_DESTROY:
                        owner->GetOrigin() += (trace.hitNormal * 1.024f);
                        fx->Event(&fxinfo->onImpact, NULL);
                        fx->Remove();
                        break;
                }
            }
        }
    }

    /*
    wl2 = Map_GetWaterLevel(fx->origin, 0, fx->plane);

    if((wl1 == WL_UNDER && wl2 == WL_OVER) ||
        (wl1 == WL_OVER && wl2 == WL_UNDER)) {
        vec3_t oTmp;

        Vec_Copy3(oTmp, fx->origin);

        if(fx->plane)
            fx->origin[1] = (Map_GetArea(fx->plane)->waterplane + 4.096f);

        FX_Event(fx, &fx->info->onWaterImpact, NULL);
        if(fx->info->bDestroyOnWaterSurface) {
            fx->lifetime = 0;
            Vec_Set3(fx->translation, 0, 0, 0);
            Actor_SetTarget((gActor_t**)&fx->source, NULL);
        }

        Vec_Copy3(fx->origin, oTmp);
    }
    */
}

DECLARE_CLASS(kexFx, kexActor)

//
// kexFx::kexFx
//

kexFx::kexFx(void) {
    this->bClientOnly   = true;
    this->bCollision    = true;
    this->owner         = NULL;
    this->target        = NULL;
    this->model         = NULL;
    this->gridBound     = NULL;
    
    this->attachment.SetOwner(this);
    this->physics.SetOwner(this);
    this->scale.Set(1, 1, 1);
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
    bool bUnderWater;

    if(client.GetRunTime() <= 0) {
        return;
    }

    time = 15 * client.GetRunTime();
    restart -= time;

    // ready to spawn?
    if(restart > 0) {
        return;
    }

    attachment.Transform();

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

    physics.Think(client.GetRunTime());

    SetViewDistance();

    // update lifetime
    lifeTime -= time;

    //
    // handle expire event
    //
    if(lifeTime < 0 || (owner && owner->IsStale())) {
        if(bForcedRestart && owner && !owner->IsStale()) {
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

        org += (nvec | (mtx1 | mtx3));
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
    kexMatrix mtx = camera->ModelView();
    kexVec3 camOrg = camera->GetOrigin();

    distance = (((origin.x - camOrg.x) *  mtx.vectors[2].x) * mtx.vectors[1].y +
                 (origin.y - camOrg.y) * -mtx.vectors[1].z +
                ((origin.z - camOrg.z) * -mtx.vectors[0].x) * mtx.vectors[1].y) * 0.5f;

    distance = (float)(int)(distance * distance);
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
        if(fxInfo->bProjectile && owner) {
            kexActor *targ;

            if(targ = owner->GetTarget()) {
                kexVec3 torg(targ->GetOrigin());
                torg.y += targ->GetCenterHeight();

                rotation = owner->GetRotation().RotateFrom(origin, torg, 0);
            }
        }
        
        matrix = kexMatrix(rotation);
    }

    bAnimate = fxInfo->numTextures > 1 ? true : false;
    bForcedRestart = false;
    frameTime = client.GetTime() + fxInfo->animspeed;
    textures = (kexTexture**)Mem_Calloc(sizeof(kexTexture*) *
        fxInfo->numTextures, hb_object);

    // setup texture lookup array
    for(i = 0; i < fxInfo->numTextures; i++) {
        textures[i] = renderSystem.CacheTexture(fxInfo->textures[i], TC_CLAMP, TF_LINEAR);
    }

    // instances
    instances = fxInfo->instances.value;

    // spawn delay time
    restart = (float)FX_RAND_VALUE((int)fxInfo->restart);

    // life time
    lifeTime = (float)fxInfo->lifetime.value +
        FX_RAND_VALUE((int)fxInfo->lifetime.rand);

    // scale
    drawScale = fxInfo->scale.value;
    if(fxInfo->scale.rand != 0) {
        drawScale += FX_RAND_FLOAT(fxInfo->scale.rand);
    }

    // scale destination
    drawScaleDest = fxInfo->scaledest.value;
    if(fxInfo->scaledest.rand != 0) {
        drawScaleDest += FX_RAND_FLOAT(fxInfo->scaledest.rand);
    }

    // forward speed
    speed = fxInfo->forward.value;
    if(fxInfo->forward.rand != 0) {
        speed += FX_RAND_FLOAT(fxInfo->forward.rand);
    }

    // rotation offset
    rotationOffset = fxInfo->rotation_offset.value;
    if(fxInfo->rotation_offset.rand != 0) {
        rotationOffset += FX_RAND_FLOAT(fxInfo->rotation_offset.rand);
    }

    // rotation speed
    rotationSpeed = fxInfo->rotation_speed.value;
    if(fxInfo->rotation_speed.rand != 0) {
        rotationSpeed += FX_RAND_FLOAT(fxInfo->rotation_speed.rand);
    }

    // gravity
    gravity = fxInfo->gravity.value;
    if(fxInfo->gravity.rand != 0) {
        gravity += FX_RAND_FLOAT(fxInfo->gravity.rand);
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
    vel.x = FX_RAND_RANGE();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[0]);

    // randomize velocity y
    vel = destVel;
    vel.y = FX_RAND_RANGE();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[1]);

    // randomize velocity z
    vel = destVel;
    vel.z = FX_RAND_RANGE();
    vel.Normalize();
    destVel.Normalize();
    destVel = destVel.Lerp(vel, fxInfo->translation.rand[2]);

    // randomize global velocity
    if(destVel.Unit() != 0) {
        kexVec3 worldVec;
        kexVec3 tVec;

        worldVec = (destVel | matrix);
        tVec.Set(FX_RAND_RANGE(), FX_RAND_RANGE(), FX_RAND_RANGE());

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

    if(fxInfo->offset.rand[0] != 0) offset.x += FX_RAND_FLOAT(fxInfo->offset.rand[0]);
    if(fxInfo->offset.rand[1] != 0) offset.y += FX_RAND_FLOAT(fxInfo->offset.rand[1]);
    if(fxInfo->offset.rand[2] != 0) offset.z += FX_RAND_FLOAT(fxInfo->offset.rand[2]);

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

    physics.friction = fxInfo->friction;
    physics.airFriction = 0;
    physics.mass = -gravity;
    physics.bounceDamp = fxInfo->mass;
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
    scvfx_bAttachToSource,
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
    { scvfx_bAttachToSource,                "bAttachToSource"                   },
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
        // precache all particle textures
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
    //Z_FreeTags(PU_FX, PU_FX);
}

//
// kexFxManager::UpdateWorld
//

void kexFxManager::UpdateWorld(kexWorld *world) {
    kexFx *tmpFx;

    for(world->fxRover = world->fxList.Next(); world->fxRover != NULL;
        world->fxRover = world->fxRover->worldLink.Next()) {
            world->fxRover->LocalTick();

            if(world->fxRover->Removing()) {
                // unlink from world and free fx
                world->fxRover->worldLink.Remove();
                tmpFx = world->fxRover;
                world->fxRover = world->fxRover->worldLink.Prev();
                Mem_Free(tmpFx);

                if(world->fxRover == NULL) {
                    break;
                }
            }
    }
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
            fxEvent->fx = Mem_Strdup(lexer->StringToken(), hb_static);
        }
        else if(!strcmp(lexer->Token(), "sound")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            fxEvent->snd = Mem_Strdup(lexer->StringToken(), hb_static);
        }
        else if(!strcmp(lexer->Token(), "action")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->ExpectNextToken(TK_LBRACK);

            lexer->GetString();
            fxEvent->action.function = Mem_Strdup(lexer->StringToken(), hb_static);
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
        fx->info = (fxinfo_t*)Mem_Calloc(sizeof(fxinfo_t) * fx->numfx, hb_static);

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
                CHECK_BOOL(bAttachToSource);

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
                    info->textures = (char**)Mem_Calloc(sizeof(char*) *
                        info->numTextures, hb_static);
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(j = 0; j < info->numTextures; j++) {
                        lexer->GetString();
                        info->textures[j] = Mem_Strdup(lexer->StringToken(), hb_static);
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
