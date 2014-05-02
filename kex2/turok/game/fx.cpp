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
#include "renderBackend.h"
#include "world.h"
#include "defs.h"

//
// FCmd_SpawnFX
//

static void FCmd_SpawnFX(void) {
    kexCamera *camera = localWorld.Camera();
    kexVec3 forward = camera->GetAngles().ToForwardAxis();
    kexVec3 org = camera->GetOrigin() + (forward * 16.384f);
    
    localWorld.SpawnFX("fx/default.kfx",
                       NULL,
                       kexVec3::vecZero,
                       org,
                       camera->GetRotation());
}

kexFxManager fxManager;

DECLARE_CLASS(kexFx, kexWorldObject)

//
// kexFx::kexFx
//

kexFx::kexFx(void) {
    this->bClientOnly   = true;
    this->bCollision    = true;
    this->owner         = NULL;
    this->target        = NULL;
    this->parent        = NULL;
    this->radius        = 4.0f;
    this->height        = 4.0f;
    this->baseHeight    = 4.0f;
    
    this->attachment.SetOwner(this);
    this->physics.SetOwner(this);
    this->scale.Set(1, 1, 1);
    this->worldLink.SetData(this);
}

//
// kexFx::~kexFx
//

kexFx::~kexFx(void) {
    SetParent(NULL);
}

//
// kexFx::SetParent
//

void kexFx::SetParent(kexFx *targ) {
    // If there was a parent already, decrease its refcount
    if(parent) {
        parent->RemoveRef();
    }

    // Set new target and if non-NULL, increase its counter
    if((parent = targ)) {
        parent->AddRef();
    }
}

//
// kexFx::LocalTick
//

void kexFx::LocalTick(void) {
    float time;
    int currentLifeTime;
    int alpha;
    bool bUnderWater;

    if(IsStale()) {
        return;
    }

    time = 15 * client.GetRunTime();

    if(time < 0) time = 0;
    if(time > 1) time = 1;

    restart -= time;

    // ready to spawn?
    if(restart > 0) {
        return;
    }

    attachment.Transform();

    bUnderWater = physics.bInWater;

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
    if(bAnimate && fxInfo->animtype != VFX_ANIMDRAWSINGLEFRAME) {
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
    if((drawScale <= 0) || lifeTime < 0 || (owner && owner->IsStale())) {
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
        SetParent(NULL);
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

        nvec = (svec * mtx1);

        switch(fxInfo->drawtype) {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            mtx3 = kexMatrix(DEG2RAD(90), 1);
            break;
        default:
            mtx3 = kexMatrix(localWorld.Camera()->GetRotation());
            break;
        }

        org += (nvec * (mtx1 * mtx3));
    }

    return localWorld.SpawnFX(name, owner, physics.velocity, org, rot, this);
}

//
// kexFx::Event
//

kexFx *kexFx::Event(fxEvent_t *fxEvent, kexWorldObject *target) {
    kexFx *nfx = NULL;

    if(fxEvent->fx != NULL) {
        if((nfx = SpawnChild(fxEvent->fx)) != NULL) {
            nfx->physicsRef->sector = this->physicsRef->sector;
        }
    }

    if(!nfx) {
        nfx = this;
    }

    if(fxEvent->snd != NULL) {
        soundSystem.StartSound(fxEvent->snd, nfx);
    }

    if(fxEvent->damageDef != NULL && owner && target) {
        nfx->InflictDamage(target, fxEvent->damageDef);
    }

    return nfx;
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
    kexVec3 vel;
    kexVec3 destVel;

    physicsRef = &this->physics;
    physics.SetOwner(this);

    // setup initial matrix
    if(fxInfo->bNoDirection) {
        matrix.Identity();
    }
    else {
        if(fxInfo->bProjectile && owner) {
            kexWorldObject *targ;

            if((targ = static_cast<kexWorldObject*>(owner->GetTarget()))) {
                kexVec3 torg(targ->GetOrigin());
                torg.y += targ->GetCenterHeight();

                rotation = targ->GetRotation().RotateFrom(origin, torg, 0);
            }
        }
        
        matrix = kexMatrix(rotation);
    }

    bAnimate = fxInfo->numTextures > 1 ? true : false;
    bForcedRestart = false;
    frameTime = client.GetTime() + fxInfo->animspeed;

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

        worldVec = (destVel * matrix);
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

    if(offset.Unit() != 0) {
        kexVec3 newDest = origin + (offset * matrix);

        // clip position if outside of collision map. non-moving particles are ignored
        if(localWorld.CollisionMap().IsLoaded() && owner && fxInfo->bLinkArea) {
            physics.sector = static_cast<kexWorldObject*>(owner)->Physics()->sector;
            TryMove(owner->GetOrigin(), newDest, &physics.sector);
        }

        origin = newDest;
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

    if(fxInfo->bLinkArea) {
        LinkArea();
    }
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
    scvfx_onImpact,
    scvfx_onExpire,
    scvfx_onTick,
    scvfx_onWaterHit,
    scvfx_onWaterExpire,
    scvfx_onWaterTick,
    scvfx_bDestroyOnWaterSurface,
    scvfx_bLinkArea,
    scvfx_shader,
    scvfx_lensflares,
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
    { scvfx_onImpact,                       "onImpact"                          },
    { scvfx_onExpire,                       "onExpire"                          },
    { scvfx_onTick,                         "onTick"                            },
    { scvfx_bNoDirection,                   "bNoDirection"                      },
    { scvfx_bLocalAxis,                     "bLocalAxis"                        },
    { scvfx_bProjectile,                    "bProjectile"                       },
    { scvfx_bActorInstance,                 "bActorInstance"                    },
    { scvfx_onWaterHit,                     "onWaterImpact"                     },
    { scvfx_onWaterExpire,                  "onWaterExpire"                     },
    { scvfx_onWaterTick,                    "onWaterTick"                       },
    { scvfx_bDestroyOnWaterSurface,         "bDestroyOnWaterSurface"            },
    { scvfx_bLinkArea,                      "bLinkArea"                         },
    { scvfx_shader,                         "shader"                            },
    { scvfx_lensflares,                     "lensFlares"                        },
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
        kexStrList fxList;
        
        fileSystem.GetMatchingFiles(fxList, "fx/");

        for(unsigned int i = 0; i < fxList.Length(); i++) {
            LoadKFX(fxList[i].c_str());
        }
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

    for(world->fxRover = world->fxList.Next(); world->fxRover != NULL; world->fxRover = tmpFx) {
        tmpFx = world->fxRover->worldLink.Next();

        world->fxRover->LocalTick();

        if(world->fxRover->Removing()) {
            // unlink from world and free fx
            world->fxRover->worldLink.Remove();
            world->fxRover->SetParent(NULL);
            world->fxRover->UnlinkArea();
            delete world->fxRover;
        }
    }
}

//
// kexFxManager::ParseEvent
//

void kexFxManager::ParseEvent(fxEvent_t *fxEvent, kexLexer *lexer) {
    fxEvent_t *currentEvent = fxEvent;
    bool bImpactBlock = false;

    lexer->ExpectNextToken(TK_LBRACK);
    while(1) {
        lexer->Find();
        if(lexer->Matches("fx")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            currentEvent->fx = Mem_Strdup(lexer->StringToken(), hb_static);
        }
        else if(lexer->Matches("sound")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            currentEvent->snd = Mem_Strdup(lexer->StringToken(), hb_static);
        }
        else if(lexer->Matches("damageDef")) {
            lexer->ExpectNextToken(TK_EQUAL);
            lexer->GetString();
            currentEvent->damageDef = defManager.FindDefEntry(lexer->StringToken());
        }
        else if(lexer->TokenType() == TK_LSQBRACK) {
            int iType = lexer->GetNumber();

            currentEvent = &fxEvent[iType];
            bImpactBlock = true;

            lexer->ExpectNextToken(TK_RSQBRACK);
            lexer->ExpectNextToken(TK_LBRACK);
        }
        else if(lexer->TokenType() == TK_RBRACK) {
            if(bImpactBlock == true) {
                bImpactBlock = false;
            }
            else {
                break;
            }
        }
        else {
            parser.Error("kexFxManager::ParseEvent: Unknown token: %s\n",
                lexer->Token());
        }
    }
}

#define CHECK_BOOL(name)                                                        \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, &tmp, scvfx_ ##name, false);          \
       info->name = (tmp != 0);                                                 \
    break

#define CHECK_INT(name)                                                         \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, (unsigned int*)&info->name,           \
        scvfx_ ##name, false);                                                  \
    break

#define CHECK_FLOAT(name, prop)                                                 \
    case scvfx_ ##name:                                                         \
       lexer->AssignFromTokenList(vfxtokens, &info->prop,                       \
        scvfx_ ##name, false);                                                  \
    break

#define CHECK_VFXINT(name)                                                      \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, (unsigned int*)&info->name .value,    \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
       lexer->AssignFromTokenList(vfxtokens, &info->name .rand,                 \
        scvfx_ ##name## _randomscale, false);                                   \
    break

#define CHECK_VFXFLOAT(name)                                                    \
    case scvfx_ ##name:                                                         \
    lexer->AssignFromTokenList(vfxtokens, &info->name .value,                   \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
       lexer->AssignFromTokenList(vfxtokens, &info->name .rand,                 \
        scvfx_ ##name## _randomscale, false);                                   \
    break

#define CHECK_VFXVECTOR(name)                                                   \
    case scvfx_ ##name:                                                         \
    lexer->AssignVectorFromTokenList(vfxtokens, info->name .value,              \
        scvfx_ ##name, false);                                                  \
    break;                                                                      \
    case scvfx_ ##name## _randomscale:                                          \
    lexer->AssignVectorFromTokenList(vfxtokens, info->name .rand,               \
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
                CHECK_BOOL(bLinkArea);
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
                    info->textures = (kexTexture**)Mem_Malloc(sizeof(kexTexture*) *
                        info->numTextures, kexTexture::hb_texture);
                    lexer->ExpectNextToken(TK_RSQBRACK);
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->ExpectNextToken(TK_LBRACK);
                    for(j = 0; j < info->numTextures; j++) {
                        lexer->GetString();
                        info->textures[j] = renderBackend.CacheTexture(lexer->StringToken(),
                                                                      TC_CLAMP,
                                                                      TF_LINEAR);
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
                    if(lexer->Matches("destroy")) {
                        info->ontouch = VFX_DESTROY;
                    }
                    else if(lexer->Matches("reflect")) {
                        info->ontouch = VFX_REFLECT;
                    }
                    else {
                        info->ontouch = VFX_DEFAULT;
                    }
                    break;
                case scvfx_onplane:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(lexer->Matches("destroy")) {
                        info->onplane = VFX_DESTROY;
                    }
                    else if(lexer->Matches("reflect")) {
                        info->onplane = VFX_REFLECT;
                    }
                    else if(lexer->Matches("bounce")) {
                        info->onplane = VFX_BOUNCE;
                    }
                    else {
                        info->onplane = VFX_DEFAULT;
                    }
                    break;
                case scvfx_drawtype:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(lexer->Matches("flat")) {
                        info->drawtype = VFX_DRAWFLAT;
                    }
                    else if(lexer->Matches("decal")) {
                        info->drawtype = VFX_DRAWDECAL;
                    }
                    else if(lexer->Matches("billboard")) {
                        info->drawtype = VFX_DRAWBILLBOARD;
                    }
                    else if(lexer->Matches("surface")) {
                        info->drawtype = VFX_DRAWSURFACE;
                    }
                    else if(lexer->Matches("hidden")) {
                        info->drawtype = VFX_DRAWHIDDEN;
                    }
                    else {
                        info->drawtype = VFX_DRAWDEFAULT;
                    }
                    break;
                case scvfx_animtype:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->Find();
                    if(lexer->Matches("onetime")) {
                        info->animtype = VFX_ANIMONETIME;
                    }
                    else if(lexer->Matches("loop")) {
                        info->animtype = VFX_ANIMLOOP;
                    }
                    else if(lexer->Matches("sinwave")) {
                        info->animtype = VFX_ANIMSINWAVE;
                    }
                    else if(lexer->Matches("drawsingleframe")) {
                        info->animtype = VFX_ANIMDRAWSINGLEFRAME;
                    }
                    else {
                        info->animtype = VFX_ANIMDEFAULT;
                    }
                    break;
                case scvfx_shader:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->GetString();
                    info->shaderObj = renderBackend.CacheShader(lexer->StringToken());
                    break;
                case scvfx_lensflares:
                    lexer->ExpectNextToken(TK_EQUAL);
                    lexer->GetString();
                    info->lensFlares = renderBackend.CacheLensFlares(lexer->StringToken());
                    break;
                case scvfx_onImpact:
                    ParseEvent(info->onImpact, lexer);
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
