// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: AI System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "ai.h"
#include "binFile.h"
#include "world.h"
#include "gameManager.h"

#define AI_THRESHOLD_MAX        100
#define AI_THRESHOLD_AMOUNT     15

DECLARE_CLASS(kexAI, kexActor)

//
// kexAI::kexAI
//

kexAI::kexAI(void) {
    this->activeDistance    = 1024.0f;
    this->turnSpeed         = 4.096f;
    this->thinkTime         = 16;
    this->headTurnSpeed     = 4.096f;
    this->maxHeadAngle      = DEG2RAD(70);
    this->aiFlags           = AIF_FINDTARGET|AIF_AVOIDWALLS|AIF_AVOIDACTORS;
    this->aiState           = AIS_NONE;
    this->bCanMelee         = false;
    this->bCanRangeAttack   = false;
    this->bCanTeleport      = false;
    this->bAttacking        = false;
    this->bTurning          = false;
    this->attackThreshold   = 0;
    this->sightThreshold    = 0;
    this->checkRadius       = 1.5f;
    this->meleeRange        = 0;
    this->alertRange        = 184.25f;
    this->rangeDistance     = 1024.0f;
    this->sightRange        = 0.78f;
    
    headYawAxis.Set(0, 1, 0);
    headPitchAxis.Set(1, 0, 0);
}

//
// kexAI::~kexAI
//

kexAI::~kexAI(void) {
}

//
// kexAI::Tick
//

void kexAI::Tick(void) {
    if(aiFlags & AIF_DISABLED) {
        return;
    }
    if(aiFlags & AIF_TURNING) {
        Turn();
    }
    if(aiFlags & AIF_DORMANT) {
        return;
    }
    
    FindTargets();

    if(aiState == AIS_SPAWNING) {
        // don't do nothing until the animation has finished
        if(!(animState.flags & ANF_STOPPED)) {
            sightThreshold = AI_THRESHOLD_MAX;
            aiFlags |= AIF_FINDTARGET;

            ChangeState(AIS_ALERT);
        }
    }

    if(aiFlags & AIF_SEETARGET) {
        if(++sightThreshold > AI_THRESHOLD_MAX) {
            sightThreshold = AI_THRESHOLD_MAX;
        }
    }
    else if(sightThreshold <= (AI_THRESHOLD_MAX / 2) && bAttacking) {
        bAttacking = false;
    }

    if(aiFlags & AIF_HASTARGET) {
        SeekTarget();
    }
    
    // handle any additional custom tick routines
    if(scriptComponent.onThink) {
        scriptComponent.CallFunction(scriptComponent.onThink);
    }
}

//
// kexAI::Spawn
//

void kexAI::Spawn(void) {
}

//
// kexAI::Save
//

void kexAI::Save(kexBinFile *saveFile) {
}

//
// kexAI::Load
//

void kexAI::Load(kexBinFile *loadFile) {
}

//
// kexAI::ChangeState
//

void kexAI::ChangeState(const aiState_t aiState) {
    int state;
    
    this->aiState = aiState;
    state = scriptComponent.PrepareFunction("void OnStateChange(int)");

    if(state == -1) {
        return;
    }

    scriptComponent.SetCallArgument(0, (int)aiState);

    if(!scriptComponent.ExecuteFunction(state)) {
        return;
    }

    scriptComponent.FinishFunction(state);
}

//
// kexAI::SeekTarget
//

void kexAI::SeekTarget(void) {
    if(attackThreshold > 0) {
        attackThreshold -= AI_THRESHOLD_AMOUNT;
    }

    switch(aiState) {
        case AIS_IDLE:
            if(!bTurning && !bAttacking && bCanMelee) {
                float dist = GetTargetDistance();

                if(dist > alertRange) {
                    ChangeState(AIS_ALERT);
                }
                else if(dist > meleeRange) {
                    ChangeState(AIS_CALM);
                }
            }
            if(aiFlags & AIF_SEETARGET) {
                TurnYaw(GetYawToTarget());
            }
            else {
                TurnYaw(GetBestYawToTarget(checkRadius));
            }
            break;

        case AIS_CALM:
            if(!bTurning && !bAttacking && sightThreshold > 0) {
                float dist = GetTargetDistance();

                if(dist > alertRange) {
                    ChangeState(AIS_ALERT);
                }
                else if(dist <= meleeRange) {
                    ChangeState(AIS_IDLE);
                }

                if(!(aiFlags & AIF_SEETARGET) && sightThreshold <= 0) {
                    if(kexRand::Max(1000) >= 995) {
                        // AI has forgotten about it's target. go back to idling
                        ClearTargets();
                        bTurning = false;
                        bAttacking = false;
                        ChangeState(AIS_IDLE);
                        return;
                    }
                }
                TurnYaw(GetBestYawToTarget(checkRadius));
            }
            break;

        case AIS_ALERT:
            if(!(aiFlags & AIF_SEETARGET) && (--sightThreshold <= 0)) {
                // start calming down after giving up target
                bTurning = false;
                ChangeState(AIS_CALM);
            }
            if(!bTurning && !bAttacking) {
                float an = GetYawToTarget();

                if(!(an <= sightRange && an >= -sightRange)) {
                    float dist = GetTargetDistance();

                    if(dist <= meleeRange) {
                        ChangeState(AIS_IDLE);
                    }
                    else if(dist <= alertRange) {
                        ChangeState(AIS_CALM);
                    }
                }
            }
            TurnYaw(GetBestYawToTarget(checkRadius));
            break;
        default:
            break;
    }
}

//
// kexAI::TurnYaw
//

void kexAI::TurnYaw(const float yaw) {
    int state;
    float an;
    bool ok = false;
    
    this->aiState = aiState;
    state = scriptComponent.PrepareFunction("void OnTurn(const float)");

    if(state == -1) {
        return;
    }

    scriptComponent.SetCallArgument(0, yaw);

    if(!scriptComponent.ExecuteFunction(state)) {
        return;
    }

    scriptComponent.FinishFunction(state, &ok);

    if(ok == false) {
        return;
    }

    an = kexMath::Fabs(yaw);
    SetIdealYaw(angles.yaw + an, 45 * an);
}

//
// kexAI::GetTargetDistance
//

float kexAI::GetTargetDistance(void) {
    float x, y, z;
    kexVec3 aOrg;
    kexVec3 tOrg;
    
    if(!target) {
        return 0;
    }
    
    aOrg = origin;
    tOrg = target->GetOrigin();
    
    x = aOrg[0] - tOrg[0];
    y = (aOrg[1] + height) - (tOrg[1] + static_cast<kexWorldObject*>(target)->Height());
    z = aOrg[2] - tOrg[2];
    
    return kexMath::Sqrt(x * x + y * y + z * z);
}

//
// kexAI::GetYawToTarget
//

float kexAI::GetYawToTarget(void) {
    kexVec3 vec;
    float angle;
    
    if(!target) {
        return angles.yaw;
    }
    
    vec = target->GetOrigin().PointAt(origin);
    angle = kexAngle::ClampInvertSums(angles.yaw, vec.ToYaw());
    
    return kexAngle::Round(angle);
}

//
// kexAI::TracePosition
//

void kexAI::TracePosition(traceInfo_t *trace, const kexVec3 &position,
                          const float radius, const float yaw) {
    float s = kexMath::Sin(yaw);
    float c = kexMath::Cos(yaw);
    kexVec3 dest;
    
    dest[0] = position[0] + (this->radius * radius * s);
    dest[1] = position[1];
    dest[2] = position[2] + (this->radius * radius * c);
    
    trace->start     = position;
    trace->end       = dest;
    trace->dir       = (trace->end - trace->start).Normalize();
    trace->fraction  = 1.0f;
    trace->hitActor  = NULL;
    trace->hitTri    = NULL;
    trace->hitMesh   = NULL;
    trace->hitVector = trace->start;
    trace->owner     = this;
    trace->sector    = &physics.sector;
    trace->bUseBBox  = false;
    
    localWorld.Trace(trace);
}

//
// kexAI::CheckPosition
//

bool kexAI::CheckPosition(const kexVec3 &position,
                          const float radius, const float yaw) {
    traceInfo_t trace;
    bool bHitWall = false;
    bool bHitObject = false;
    
    TracePosition(&trace, position, radius, yaw);
    
    if(trace.fraction == 1) {
        return true;
    }
    
    if(aiFlags & AIF_AVOIDWALLS) {
        bHitWall = true;
    }
    
    if(aiFlags & AIF_AVOIDACTORS) {
        bHitObject = (trace.hitActor != NULL);
    }
    
    return !(bHitWall | bHitObject);
}

//
// kexAI::GetBestYawToTarget
//

float kexAI::GetBestYawToTarget(const float extendedRadius) {
    float yaw;
    kexVec3 position;
    
    yaw = GetYawToTarget();
    
    if(!target) {
        return yaw;
    }
    
    position = origin;
    position[1] += (height * 0.8f);
    
    if(!CheckPosition(position, extendedRadius, angles.yaw + yaw)) {
        float an;
        float pAn;
        
        pAn = M_PI / 8;
        
        for(int i = 0; i < 8; i++) {
            float dir;
            
            an = ((i+1) * pAn);
            dir = an + yaw;
            
            if(CheckPosition(position, extendedRadius, angles.yaw + dir)) {
                yaw = DEG2RAD(15) + dir;
                kexAngle::Clamp(&yaw);
                break;
            }
            
            dir = yaw - an;
            
            if(CheckPosition(position, extendedRadius, angles.yaw + dir)) {
                yaw = dir - DEG2RAD(15);
                kexAngle::Clamp(&yaw);
                break;
            }
        }
    }
    
    return yaw;
}

//
// kexAI::FireProjectile
//

void kexAI::FireProjectile(const char *fxName, const kexVec3 &org,
                           const float maxAngle, bool bLocalToActor) {
    kexVec3 tOrg;
    kexVec3 aOrg;
    kexQuat frot;
    
    if(!target) {
        return;
    }
    
    if(bLocalToActor) {
        aOrg = ToLocalOrigin(org);
    }
    else {
        aOrg = org;
    }
    
    tOrg = target->GetOrigin();
    tOrg[1] += static_cast<kexWorldObject*>(target)->GetViewHeight();
    
    frot = rotation.RotateFrom(aOrg, tOrg, maxAngle);
    localWorld.SpawnFX(fxName, this, kexVec3(0, 0, 0), aOrg, frot);
}

//
// kexAI::FireProjectile
//

void kexAI::FireProjectile(const kexStr &fxName, const kexVec3 &org,
                           const float maxAngle, bool bLocalToActor) {
    FireProjectile(fxName.c_str(), org, maxAngle, bLocalToActor);
}

//
// kexAI::SetIdealYaw
//

void kexAI::SetIdealYaw(const float yaw, const float speed) {
    idealYaw = kexAngle::Round(yaw);
    turnSpeed = speed;
    aiFlags |= AIF_TURNING;
}

//
// kexAI::Turn
//

void kexAI::Turn(void) {
    float speed;
    float current;
    float diff;
    
    speed = turnSpeed * client.GetRunTime();
    current = kexAngle::Round(angles.yaw);
    
    if(kexMath::Fabs(current - idealYaw) <= 0.001f) {
        aiFlags &= ~AIF_TURNING;
        return;
    }
    
    diff = idealYaw - current;
    kexAngle::Clamp(&diff);
    
    if(diff > 0) {
        if(diff > speed) {
            diff = speed;
        }
    }
    else if(diff < -speed) {
        diff = -speed;
    }
    
    angles.yaw = kexAngle::Round(current + diff);
}

//
// kexAI::CanSeeTarget
//

bool kexAI::CanSeeTarget(kexWorldObject *object) {
    kexVec3 aOrg;
    kexVec3 tOrg;
    traceInfo_t trace;
    
    if(!object) {
        return false;
    }
    
    aOrg = origin;
    tOrg = object->GetOrigin();
    
    aOrg[1] += (baseHeight * 0.8f);
    tOrg[1] += (object->BaseHeight() * 0.8f);
    
    trace.start     = aOrg;
    trace.end       = tOrg;
    trace.dir       = (trace.end - trace.start).Normalize();
    trace.fraction  = 1.0f;
    trace.hitActor  = NULL;
    trace.hitTri    = NULL;
    trace.hitMesh   = NULL;
    trace.hitVector = trace.start;
    trace.owner     = this;
    trace.sector    = &physics.sector;
    trace.bUseBBox  = false;
    
    localWorld.Trace(&trace);
    
    if(trace.fraction == 1 || trace.hitActor == object) {
        aiFlags |= AIF_SEETARGET;
        return true;
    }
    
    // something is obstructing its line of sight
    aiFlags &= ~AIF_SEETARGET;
    return false;
}

//
// kexAI::ClearTargets
//

void kexAI::ClearTargets(void) {
    SetTarget(NULL);
    
    aiFlags &= ~AIF_HASTARGET;
    aiFlags &= ~AIF_SEETARGET;
    aiFlags &= ~AIF_LOOKATTARGET;
}

//
// kexAI::FindTargets
//

void kexAI::FindTargets(void) {
    if(!(aiFlags & AIF_FINDTARGET)) {
        return;
    }
    
    // TODO - handle network players
    if(CanSeeTarget(gameManager.localPlayer.Puppet())) {
        if(target == NULL && !(aiFlags & AIF_HASTARGET)) {
            SetTarget(gameManager.localPlayer.Puppet());
            aiFlags |= AIF_HASTARGET;
        }
    }
}

//
// kexAI::InitObject
//

void kexAI::InitObject(void) {
}
