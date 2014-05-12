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
#include "renderUtils.h"

DECLARE_CLASS(kexAI, kexActor)

//
// FCmd_DebugAI
//

static void FCmd_DebugAI(void) {
    traceInfo_t trace;
    kexSector *sector;
    kexVec3 start;
    kexVec3 end;

    if(command.GetArgc() < 1) {
        return;
    }

    start = localWorld.Camera()->GetOrigin();
    end = start + (localWorld.Camera()->GetAngles().ToForwardAxis() * 2048.0f);
    sector = NULL;

    kexAI::debugAI = NULL;

    if(localWorld.CollisionMap().IsLoaded()) {
        sector = localWorld.CollisionMap().PointInSector(start);
    }

    trace.start     = start;
    trace.end       = end;
    trace.dir       = (trace.end - trace.start).Normalize();
    trace.fraction  = 1.0f;
    trace.hitActor  = NULL;
    trace.hitTri    = NULL;
    trace.hitMesh   = NULL;
    trace.hitVector = trace.start;
    trace.owner     = NULL;
    trace.sector    = &sector;
    trace.bUseBBox  = true;

    trace.localBBox.min.Set(-200, -200, -200);
    trace.localBBox.max.Set(200, 200, 200);

    trace.bbox = trace.localBBox;

    trace.bbox.min += trace.start;
    trace.bbox.max += trace.start;
    
    localWorld.Trace(&trace);

    if(trace.hitActor) {
        if(trace.hitActor->InstanceOf(&kexAI::info)) {
            kexAI::debugAI = static_cast<kexAI*>(trace.hitActor);
        }
    }
}

kexAI *kexAI::debugAI = NULL;

//
// kexAI::kexAI
//

kexAI::kexAI(void) {
    this->activeDistance        = 2048.0f;
    this->idealYaw              = 0;
    this->turningYaw            = 0;
    this->turnSpeed             = 4.096f;
    this->thinkTime             = 8;
    this->nextThinkTime         = this->timeStamp + this->thinkTime;
    this->headTurnSpeed         = 4.096f;
    this->maxHeadAngle          = DEG2RAD(70);
    this->aiFlags               = AIF_DEFAULT;
    this->aiState               = AIS_NONE;
    this->bCanMelee             = false;
    this->bCanRangeAttack       = false;
    this->bCanTeleport          = false;
    this->bAttacking            = false;
    this->bAnimTurning          = false;
    this->attackThreshold       = 0;
    this->sightThreshold        = 0;
    this->maxThreshold          = 100;
    this->attackThresholdTime   = 15;
    this->checkRadius           = 1.5f;
    this->meleeRange            = 0;
    this->alertRange            = 184.25f;
    this->rangeDistance         = 1024.0f;
    this->sightRange            = DEG2RAD(45);
    this->rangeSightDamp        = 0.675f;
    this->giveUpChance          = 995;
    this->teleportChance        = 985;
    this->rangeChance           = 100;
    this->rangeAdjustAngle      = DEG2RAD(50);
    this->yawSpeed              = 2.0f;
    
    headYawAxis.Set(0, 1, 0);
    headPitchAxis.Set(1, 0, 0);
}

//
// kexAI::~kexAI
//

kexAI::~kexAI(void) {
}

//
// kexAI::Init
//

void kexAI::Init(void) {
    command.Add("aidebug", FCmd_DebugAI);
}

//
// kexAI::LocalTick
//

void kexAI::LocalTick(void) {
    if(IsStale()) {
        return;
    }

    UpdateTransform();
    animState.Update();

    if(animState.frameTime != 0) {
        height = ((baseHeight * 0.72f) - animState.baseOffset) * 0.5f;
    }

    physicsRef->Think(client.GetRunTime());
    
    // TODO - majority of the code below should be
    // handled by the server...

    if(aiFlags & AIF_DISABLED) {
        return;
    }
    if(aiFlags & AIF_TURNING) {
        Turn();
    }

    AnimStopped();

    if(timeStamp < nextThinkTime) {
        // not ready to think yet
        return;
    }

    // TODO - handle multiple players
    if(client.IsLocal()) {
        float dist = activeDistance * activeDistance;
        if(origin.DistanceSq(gameManager.localPlayer.GetOrigin()) >= dist) {
            GoDormant();
            return;
        }
        else {
            WakeUp();
        }
    }

    if(aiFlags & AIF_DORMANT) {
        return;
    }
    
    FindTargets();

    if(aiState != AIS_SPAWNING) {
        if(aiFlags & AIF_SEETARGET) {
            if(++sightThreshold > maxThreshold) {
                sightThreshold = maxThreshold;
            }
        }
        else if(sightThreshold <= (maxThreshold / 2) && bAttacking) {
            bAttacking = false;
        }

        if(aiFlags & AIF_HASTARGET) {
            SeekTarget();
        }
    }
    
    // handle any additional custom tick routines
    if(scriptComponent.onLocalThink) {
        scriptComponent.CallFunction(scriptComponent.onLocalThink);
    }

    nextThinkTime = timeStamp + thinkTime;
}

//
// kexAI::Spawn
//

void kexAI::Spawn(void) {
    physicsRef = &this->physics;
    physicsRef->SetOwner(this);
    physicsRef->bEnabled = true;

    if(definition != NULL) {
        definition->GetBool("bCanMelee", bCanMelee);
        definition->GetBool("bCanRangeAttack", bCanRangeAttack);
        definition->GetBool("bCanTeleport", bCanTeleport);
        definition->GetFloat("meleeRange", meleeRange);
        definition->GetFloat("alertRange", alertRange);
        definition->GetFloat("rangeDistance", rangeDistance, 1024.0f);
        definition->GetFloat("checkRadius", checkRadius, 1.5f);
        definition->GetFloat("sightRange", sightRange, DEG2RAD(45));
        definition->GetFloat("rangeSightDamp", rangeSightDamp, 0.675f);
        definition->GetFloat("rangeAdjustAngle", rangeAdjustAngle, DEG2RAD(50));
        definition->GetFloat("yawSpeed", yawSpeed, 2.0f);
        definition->GetFloat("thinkTime", thinkTime, 8);
        definition->GetFloat("maxHeadAngle", maxHeadAngle, DEG2RAD(70));
        definition->GetInt("maxThreshold", maxThreshold, 100);
        definition->GetInt("giveUpChance", giveUpChance, 995);
        definition->GetInt("teleportChance", teleportChance, 985);
        definition->GetInt("rangeChance", rangeChance, 100);
        definition->GetInt("aiFlags", (int&)aiFlags, AIF_DEFAULT);

        definition->GetInt("clipFlags", (int&)physicsRef->clipFlags, (PF_CLIPEDGES|PF_NOENTERWATER));
    }

    physicsRef->sector = localWorld.CollisionMap().PointInSector(origin);

    UpdateTransform();
    ChangeState(AIS_IDLE);
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
// Sets state and invokes a script callback
// Useful for changing animations
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
// kexAI::GoDormant
//

void kexAI::GoDormant(void) {
    int state;

    if(aiFlags & AIF_DORMANT) {
        return;
    }
    
    aiFlags |= AIF_DORMANT;
    state = scriptComponent.PrepareFunction("void OnDormant(void)");

    if(state == -1) {
        return;
    }
    if(!scriptComponent.ExecuteFunction(state)) {
        return;
    }

    scriptComponent.FinishFunction(state);
}

//
// kexAI::WakeUp
//

void kexAI::WakeUp(void) {
    int state;

    if(!(aiFlags & AIF_DORMANT)) {
        return;
    }
    
    aiFlags &= ~AIF_DORMANT;
    state = scriptComponent.PrepareFunction("void OnWake(void)");

    if(state == -1) {
        return;
    }
    if(!scriptComponent.ExecuteFunction(state)) {
        return;
    }

    scriptComponent.FinishFunction(state);
}

//
// kexAI::FoundTarget
//

void kexAI::FoundTarget(void) {
    int state;
    
    state = scriptComponent.PrepareFunction("void OnTargetFound(void)");

    if(state == -1) {
        return;
    }
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
        attackThreshold -= attackThresholdTime;
    }
    
    // try to attack if it hasn't already
    if(aiState != AIS_TELEPORT_OUT && aiState != AIS_TELEPORT_IN &&
       aiState != AIS_SPAWNING && !bAnimTurning && !bAttacking) {
        if(!TryMelee()) {
            if(bCanRangeAttack && attackThreshold <= 0) {
                TryRange();
            }
        }
    }

    switch(aiState) {
        ////////////////////////////////////////////////////
        // IDLE STATE
        // Stands still, does nothing until it sees a target
        ////////////////////////////////////////////////////
        case AIS_IDLE:
            if(!bAnimTurning && !bAttacking && bCanMelee) {
                float dist = GetTargetDistance();

                if(dist > alertRange) {
                    ChangeState(AIS_ALERT);
                }
                else if(dist > meleeRange) {
                    ChangeState(AIS_CALM);
                }
            }

            if(aiFlags & AIF_FACETARGET) {
                if(aiFlags & AIF_SEETARGET) {
                    TurnYaw(GetYawToTarget());
                }
                else {
                    TurnYaw(GetBestYawToTarget(checkRadius));
                }
            }
            break;

        ////////////////////////////////////////////////////
        // CALM STATE
        // Expects to see its target. Not too aggressive
        ////////////////////////////////////////////////////
        case AIS_CALM:
            if(!bAnimTurning && !bAttacking && sightThreshold > 0) {
                float dist = GetTargetDistance();

                if(dist > alertRange) {
                    ChangeState(AIS_ALERT);
                }
                else if(dist <= meleeRange) {
                    ChangeState(AIS_IDLE);
                }
            }

            if(!(aiFlags & AIF_SEETARGET) && sightThreshold <= 0 && kexRand::Max(1000) >= giveUpChance) {
                // AI has forgotten about it's target. go back to idling
                ClearTargets();
                bAnimTurning = false;
                bAttacking = false;
                ChangeState(AIS_IDLE);
                return;
            }
            else {
                TurnYaw(GetBestYawToTarget(checkRadius));
            }
            break;

        ////////////////////////////////////////////////////
        // ALERT STATE
        // Sees target; aggressive
        ////////////////////////////////////////////////////
        case AIS_ALERT:
            if(!(aiFlags & AIF_SEETARGET) && (--sightThreshold <= 0)) {
                // start calming down after giving up target
                bAnimTurning = false;
                ChangeState(AIS_CALM);
            }
            if(!bAnimTurning && !bAttacking) {
                if(TryTeleport()) {
                    return;
                }

                if(!(kexMath::Fabs(GetYawToTarget()) <= sightRange)) {
                    float dist = GetTargetDistance();

                    if(dist <= meleeRange) {
                        ChangeState(AIS_IDLE);
                    }
                    else if(dist <= alertRange) {
                        ChangeState(AIS_CALM);
                    }
                }
            }

            if(aiFlags & AIF_FACETARGET) {
                TurnYaw(GetBestYawToTarget(checkRadius));
            }
            break;

        ////////////////////////////////////////////////////
        // ATTACK RANGE STATE
        // Will try to keep aim on its target
        ////////////////////////////////////////////////////
        case AIS_ATTACK_RANGE:
            if(!bCanMelee && aiFlags & AIF_FACETARGET) {
                TurnYaw(GetYawToTarget());
            }
        default:
            break;
    }
}

//
// kexAI::AnimStopped
//
// Take appropriate action when a animation has finished playing
//

void kexAI::AnimStopped(void) {
    bool bWasTurning = false;

    if(!(animState.flags & ANF_STOPPED)) {
        return;
    }

    if(bAnimTurning) {
        bAnimTurning = false;
        bWasTurning = true;
    }

    switch(aiState) {
        ////////////////////////////////////////////////////
        // SPAWNING STATE
        // Become alert if finding a target
        ////////////////////////////////////////////////////
        case AIS_SPAWNING:
            sightThreshold = maxThreshold;
            if(aiFlags & AIF_FINDTARGET) {
                ChangeState(AIS_ALERT);
            }
            break;
        ////////////////////////////////////////////////////
        // MELEE STATE
        // Will try to attack again before changing states
        ////////////////////////////////////////////////////
        case AIS_ATTACK_MELEE:
            aiState = AIS_IDLE;
            bAttacking = false;
            bAnimTurning = false;

            if(!TryMelee()) {
                float dist = GetTargetDistance();
                
                if(dist > alertRange) {
                    ChangeState(AIS_ALERT);
                }
                else if(dist > meleeRange) {
                    ChangeState(AIS_IDLE);
                }
                
                if(aiFlags & AIF_FACETARGET) {
                    TurnYaw(GetBestYawToTarget(checkRadius));
                }
            }
            break;

        ////////////////////////////////////////////////////
        // ATTACK RANGE STATE
        // Will try to attack again before changing states
        ////////////////////////////////////////////////////
        case AIS_ATTACK_RANGE:
            aiState = AIS_IDLE;
            bAttacking = false;
            bAnimTurning = false;
            TryRange();
            break;
            
        ////////////////////////////////////////////////////
        // TELEPORT OUT STATE
        // Will try to teleport close to target if available
        ////////////////////////////////////////////////////
        case AIS_TELEPORT_OUT:
            TeleportToTarget();
            SetIdealYaw(GetBestYawToTarget(checkRadius), 2.0f);
            ChangeState(AIS_TELEPORT_IN);
            break;
            
        ////////////////////////////////////////////////////
        // TELEPORT IN STATE
        // Handles a successful teleport move
        ////////////////////////////////////////////////////
        case AIS_TELEPORT_IN:
            aiState = AIS_IDLE;
            bAttacking = false;
            bAnimTurning = false;
            
            if(bCanMelee) {
                TurnYaw(GetYawToTarget());
            }
            else {
                TryRange();
            }
            break;

        default:
            if(bWasTurning) {
                ChangeState(aiState);
            }
            break;
    }
}

//
// kexAI::TurnYaw
//
// Calls a script callback and if it
// returned true, set the ideal yaw
//

void kexAI::TurnYaw(const float yaw) {
    int state;
    float an;
    bool ok = false;
    
    this->aiState = aiState;
    state = scriptComponent.PrepareFunction("bool OnTurn(const float)");

    an = yaw;
    kexAngle::Clamp(&an);

    // at least let it still turn if it can't find the script function
    if(state != -1) {
        scriptComponent.SetCallArgument(0, an);

        if(!scriptComponent.ExecuteFunction(state)) {
            return;
        }

        scriptComponent.FinishFunction(state, &ok);

        if(ok == false) {
            return;
        }
    }

    SetIdealYaw(angles.yaw + an, yawSpeed);
}

//
// kexAI::TryMelee
//

bool kexAI::TryMelee(void) {
    if(!target || !bCanMelee || !(aiFlags & AIF_SEETARGET)) {
        return false;
    }
    
    // still busy performing an attack
    if(bAttacking) {
        return false;
    }
    
    // needs to be close enough to the target
    if(GetTargetDistance() <= meleeRange) {
        float yaw = GetYawToTarget();
        
        if(!(kexMath::Fabs(yaw) <= sightRange)) {
            TurnYaw(yaw);
        }
        else {
            SetIdealYaw(angles.yaw + yaw, 4.096f);
            bAttacking = true;
            ChangeState(AIS_ATTACK_MELEE);
            return true;
        }
    }
    
    return false;
}

//
// kexAI::TryRange
//

bool kexAI::TryRange(void) {
    float dist;
    float an;
    int cr;
    bool bInView;
    
    if(!target) {
        return false;
    }
    
    if(!bCanMelee) {
        cr = rangeChance * 2;
    }
    else {
        cr = rangeChance;
    }
    
    dist = GetTargetDistance();
    
    // attack more aggressively when close to target
    if(kexRand::Max(cr) <= kexMath::Floor(dist * 100.0f / rangeDistance)) {
        attackThreshold = maxThreshold;
        aiState = AIS_IDLE;
        bAttacking = false;
        return false;
    }
    
    an = GetYawToTarget();
    bInView = (kexMath::Fabs(an) <= (sightRange * rangeSightDamp));
    
    if(bInView && aiFlags & AIF_SEETARGET) {
        if(dist <= rangeDistance && !bAttacking) {
            bAttacking = true;
            ChangeState(AIS_ATTACK_RANGE);
        }
    }
    else if(bAttacking) {
        aiState = AIS_IDLE;
        bAttacking = false;
    }
    
    if(!bInView && rangeAdjustAngle != 0) {
        if(an > 0) {
            if(an < rangeAdjustAngle) {
                an = rangeAdjustAngle;
            }
        }
        else {
            if(an > -rangeAdjustAngle) {
                an = -rangeAdjustAngle;
            }
        }
        
        TurnYaw(an);
    }
    
    return true;
}

//
// kexAI::TryTeleport
//

bool kexAI::TryTeleport(void) {
    if(!target) {
        return false;
    }
    
    if(!bAttacking && !bAnimTurning && bCanTeleport) {
        if(kexRand::Max(1000) >= teleportChance) {
            ChangeState(AIS_TELEPORT_OUT);
            return true;
        }
    }
    return false;
}

//
// kexAI::TeleportToTarget
//
// Will try to teleport around the target's
// position. The distance will be based on
// either melee or range values.
//

void kexAI::TeleportToTarget(void) {
    kexVec3 pos;
    float range;
    float an;
    
    if(!target) {
        return;
    }
    
    pos = target->GetOrigin();
    range = (kexRand::Max(100) >= 50) ? meleeRange : rangeDistance;
    an = DEG2RAD((kexRand::Max(10) * 36.0f));
    
    pos[0] += range * kexMath::Sin(an);
    pos[2] += range * kexMath::Cos(an);
    
    TryMove(origin, pos, &physicsRef->sector);
    localWorld.TeleportActor(this, pos, angles, physicsRef->sector->GetID());
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
// Determines the yaw in radians in which the AI needs
// to turn in order to face it's target
//

float kexAI::GetYawToTarget(void) {
    kexVec2 vec1, vec2;
    kexVec2 diff;
    kexVec3 sincos;
    float tan2;
    float an;
    
    if(!target) {
        return angles.yaw;
    }
    
    vec1 = origin;
    vec2 = target->GetOrigin();
    diff = (vec1 - vec2);
    tan2 = kexMath::ATan2(diff[0], diff[1]);
    
    sincos.Set(
        kexMath::Sin(tan2),
        0,
        kexMath::Cos(tan2));

    an = kexAngle::ClampInvertSums(angles.yaw, sincos.ToYaw());
    kexAngle::Clamp(&an);

    return an;
}

//
// kexAI::TracePosition
//

void kexAI::TracePosition(traceInfo_t *trace, const kexVec3 &position,
                          const float radius, const float yaw) {
    float s = kexMath::Sin(yaw);
    float c = kexMath::Cos(yaw);
    kexVec3 dest;
    kexSector *sector;
    
    dest[0] = position[0] + (this->radius * radius * s);
    dest[1] = position[1];
    dest[2] = position[2] + (this->radius * radius * c);

    sector = physicsRef->sector;
    
    trace->start        = position;
    trace->end          = dest;
    trace->dir          = (trace->end - trace->start).Normalize();
    trace->fraction     = 1.0f;
    trace->hitActor     = NULL;
    trace->hitTri       = NULL;
    trace->hitMesh      = NULL;
    trace->hitVector    = trace->start;
    trace->owner        = this;
    trace->sector       = &physicsRef->sector;
    trace->bUseBBox      = true;

    trace->localBBox.min.Set(-2, -2, -2);
    trace->localBBox.max.Set(2, 2, 2);

    trace->bbox = trace->localBBox;

    trace->bbox.min += trace->start;
    trace->bbox.max += trace->start;
    
    localWorld.Trace(trace, physicsRef->clipFlags);
    physicsRef->sector = sector;
}

//
// kexAI::CheckPosition
//

bool kexAI::CheckPosition(const kexVec3 &position,
                          const float radius, const float yaw) {
    traceInfo_t trace;
    bool bHitWall = false;
    bool bHitObject = false;
    float an = yaw;
    
    kexAngle::Clamp(&an);
    TracePosition(&trace, position, radius, an);
    
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
                yaw = dir + DEG2RAD(15);
                break;
            }
            
            dir = yaw - an;
            
            if(CheckPosition(position, extendedRadius, angles.yaw + dir)) {
                yaw = dir - DEG2RAD(15);
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
    localWorld.SpawnFX(fxName, this, kexVec3::vecZero, aOrg, frot);
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

    turningYaw = angles.yaw;
    kexAngle::Clamp(&turningYaw);
    
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
    
    current = kexAngle::Round(turningYaw);
    
    if(kexMath::Fabs(current - idealYaw) <= 0.001f) {
        aiFlags &= ~AIF_TURNING;
        return;
    }
    
    diff = idealYaw - current;
    kexAngle::Clamp(&diff);

    speed = turnSpeed * client.GetRunTime();
    
    if(diff > 0) {
        if(diff > speed) {
            diff = speed;
        }
    }
    else if(diff < -speed) {
        diff = -speed;
    }
    
    turningYaw = kexAngle::Round(current + diff);

    angles.yaw = turningYaw;
    kexAngle::Clamp(&angles.yaw);
}

//
// kexAI::CanSeeTarget
//

bool kexAI::CanSeeTarget(kexWorldObject *object) {
    kexVec3 aOrg;
    kexVec3 tOrg;
    traceInfo_t trace;
    kexSector *sector;
    
    if(!object) {
        return false;
    }
    
    sector = physicsRef->sector;
    aOrg = origin;
    tOrg = object->GetOrigin();
    
    aOrg[1] += (baseHeight * 0.8f);
    tOrg[1] += (object->BaseHeight() * 0.8f);

    if(aOrg.DistanceSq(tOrg) >= (activeDistance * activeDistance)) {
        return false;
    }
    
    trace.start     = aOrg;
    trace.end       = tOrg;
    trace.dir       = (trace.end - trace.start).Normalize();
    trace.fraction  = 1.0f;
    trace.hitActor  = NULL;
    trace.hitTri    = NULL;
    trace.hitMesh   = NULL;
    trace.hitVector = trace.start;
    trace.owner     = this;
    trace.sector    = &physicsRef->sector;
    trace.bUseBBox  = true;

    trace.localBBox.min.Set(-2, -2, -2);
    trace.localBBox.max.Set(2, 2, 2);

    trace.bbox = trace.localBBox;

    trace.bbox.min += trace.start;
    trace.bbox.max += trace.start;
    
    localWorld.Trace(&trace);
    physicsRef->sector = sector;
    
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

            FoundTarget();
        }
    }
}

//
// kexAI::InitObject
//

void kexAI::InitObject(void) {
    kexScriptManager::RegisterRefObjectNoCount<kexAI>("kAI");
    kexAI::RegisterBaseProperties<kexAI>("kAI");

     scriptManager.Engine()->RegisterObjectMethod(
        "kActor",
        "kAI @ToAI(void)",
        asMETHODPR(kexActor, ToAI, (void), kexAI*),
        asCALL_THISCALL);
    
    scriptManager.Engine()->RegisterEnum("EnumAIFlags");
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_TURNING", AIF_TURNING);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_DORMANT", AIF_DORMANT);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_SEETARGET", AIF_SEETARGET);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_HASTARGET", AIF_HASTARGET);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_FINDTARGET", AIF_FINDTARGET);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_AVOIDWALLS", AIF_AVOIDWALLS);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_AVOIDACTORS", AIF_AVOIDACTORS);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_DISABLED", AIF_DISABLED);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_LOOKATTARGET", AIF_LOOKATTARGET);
    scriptManager.Engine()->RegisterEnumValue("EnumAIFlags", "AIF_FACETARGET", AIF_FACETARGET);
    
    scriptManager.Engine()->RegisterEnum("EnumAIState");
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_NONE", AIS_NONE);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_IDLE", AIS_IDLE);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_CALM", AIS_CALM);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_ALERT", AIS_ALERT);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_ATTACK_MELEE", AIS_ATTACK_MELEE);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_ATTACK_RANGE", AIS_ATTACK_RANGE);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_DEATH", AIS_DEATH);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_SPAWNING", AIS_SPAWNING);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_TELEPORT_OUT", AIS_TELEPORT_OUT);
    scriptManager.Engine()->RegisterEnumValue("EnumAIState", "AIS_TELEPORT_IN", AIS_TELEPORT_IN);
}

//
// kexAI::PrintDebugInfo
//

void kexAI::PrintDebugInfo(void) {
    if(kexAI::debugAI == NULL) {
        return;
    }

    kexAI *ai = kexAI::debugAI;

    if(ai->IsStale() || ai->Removing()) {
        kexAI::debugAI = NULL;
        return;
    }

    kexRenderUtils::PrintStatsText("ai debug info", "");
    kexRenderUtils::AddDebugLineSpacing();
    switch(ai->GetAIState()) {
        case AIS_NONE:
            kexRenderUtils::PrintStatsText("ai state", ": none");
            break;
        case AIS_IDLE:
            kexRenderUtils::PrintStatsText("ai state", ": idle");
            break;
        case AIS_CALM:
            kexRenderUtils::PrintStatsText("ai state", ": calm");
            break;
        case AIS_ALERT:
            kexRenderUtils::PrintStatsText("ai state", ": alert");
            break;
        case AIS_ATTACK_MELEE:
            kexRenderUtils::PrintStatsText("ai state", ": attack melee");
            break;
        case AIS_ATTACK_RANGE:
            kexRenderUtils::PrintStatsText("ai state", ": attack range");
            break;
        case AIS_DEATH:
            kexRenderUtils::PrintStatsText("ai state", ": death");
            break;
        case AIS_SPAWNING:
            kexRenderUtils::PrintStatsText("ai state", ": spawning");
            break;
        case AIS_TELEPORT_OUT:
            kexRenderUtils::PrintStatsText("ai state", ": teleport in");
            break;
        case AIS_TELEPORT_IN:
            kexRenderUtils::PrintStatsText("ai state", ": teleport out");
            break;
    }
    kexRenderUtils::PrintStatsText("turning", ": %i", (ai->GetAIFlags() & AIF_TURNING) != 0);
    kexRenderUtils::PrintStatsText("dormant", ": %i", (ai->GetAIFlags() & AIF_DORMANT) != 0);
    kexRenderUtils::PrintStatsText("see target", ": %i", (ai->GetAIFlags() & AIF_SEETARGET) != 0);
    kexRenderUtils::PrintStatsText("has target", ": %i", (ai->GetAIFlags() & AIF_HASTARGET) != 0);
    kexRenderUtils::PrintStatsText("find target", ": %i", (ai->GetAIFlags() & AIF_FINDTARGET) != 0);
    kexRenderUtils::PrintStatsText("avoid walls", ": %i", (ai->GetAIFlags() & AIF_AVOIDWALLS) != 0);
    kexRenderUtils::PrintStatsText("avoid actors", ": %i", (ai->GetAIFlags() & AIF_AVOIDACTORS) != 0);
    kexRenderUtils::PrintStatsText("disabled", ": %i", (ai->GetAIFlags() & AIF_DISABLED) != 0);
    kexRenderUtils::PrintStatsText("look at target", ": %i", (ai->GetAIFlags() & AIF_LOOKATTARGET) != 0);
    kexRenderUtils::PrintStatsText("face target", ": %i", (ai->GetAIFlags() & AIF_FACETARGET) != 0);
    kexRenderUtils::PrintStatsText("think time", ": %f", ai->ThinkTime());
    kexRenderUtils::PrintStatsText("next think time", ": %f", ai->NextThinkTime());
    kexRenderUtils::PrintStatsText("is attacking", ": %i", ai->IsAttacking());
    kexRenderUtils::PrintStatsText("is anim turning", ": %i", ai->IsAnimTurning());
    kexRenderUtils::PrintStatsText("attack threshold", ": %i", ai->AttackThreshold());
    kexRenderUtils::PrintStatsText("sight threshold", ": %i", ai->SightThreshold());
    kexRenderUtils::PrintStatsText("attack threshold time", ": %i", ai->AttackThresholdTime());
    kexRenderUtils::PrintStatsText("activeDistance", ": %f", ai->ActiveDistance());
    kexRenderUtils::AddDebugLineSpacing();
}
