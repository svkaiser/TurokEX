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

#ifndef _AI_H_
#define _AI_H_

#include "common.h"
#include "actor.h"
#include "physics/physics_ai.h"

typedef enum {
    AIF_TURNING         = BIT(0),
    AIF_DORMANT         = BIT(1),
    AIF_SEETARGET       = BIT(2),
    AIF_HASTARGET       = BIT(3),
    AIF_FINDTARGET      = BIT(4),
    AIF_AVOIDWALLS      = BIT(5),
    AIF_AVOIDACTORS     = BIT(6),
    AIF_DISABLED        = BIT(7),
    AIF_LOOKATTARGET    = BIT(8),
    AIF_FACETARGET      = BIT(9)
} aiFlags_t;

#define AIF_DEFAULT     (AIF_FINDTARGET|AIF_AVOIDWALLS|AIF_AVOIDACTORS|AIF_FACETARGET)

typedef enum {
    AIS_NONE            = 0,
    AIS_IDLE,
    AIS_CALM,
    AIS_ALERT,
    AIS_ATTACK_MELEE,
    AIS_ATTACK_RANGE,
    AIS_DEATH,
    AIS_SPAWNING,
    AIS_TELEPORT_OUT,
    AIS_TELEPORT_IN
} aiState_t;

BEGIN_EXTENDED_CLASS(kexAI, kexActor);
public:
                                kexAI(void);
                                ~kexAI(void);

    virtual void                LocalTick(void);

    void                        Spawn(void);
    void                        Save(kexBinFile *saveFile);
    void                        Load(kexBinFile *loadFile);

    float                       GetTargetDistance(void);
    float                       GetYawToTarget(void);
    void                        TracePosition(traceInfo_t *trace, const kexVec3 &position,
                                              const float radius, const float yaw);
    bool                        CheckPosition(const kexVec3 &position,
                                              const float radius, const float yaw);
    float                       GetBestYawToTarget(const float extendedRadius);
    void                        FireProjectile(const char *fxName, const kexVec3 &org,
                                               const float maxAngle, bool bLocalToActor = false);
    void                        FireProjectile(const kexStr &fxName, const kexVec3 &org,
                                               const float maxAngle, bool bLocalToactor = false);
    void                        SetIdealYaw(const float yaw, const float speed);
    void                        Turn(void);
    bool                        CanSeeTarget(kexWorldObject *object);
    void                        FindTargets(void);
    void                        ClearTargets(void);
    void                        ChangeState(const aiState_t aiState);
    void                        SeekTarget(void);
    void                        TurnYaw(const float yaw);
    bool                        TryMelee(void);
    bool                        TryRange(void);
    bool                        TryTeleport(void);
    void                        TeleportToTarget(void);

    static void                 InitObject(void);

    //
    // template for registering default script actor methods and properties
    //
    template<class type>
    static void                 RegisterBaseProperties(const char *scriptClass) {
        #define OBJMETHOD(str, a, b, c)                     \
            scriptManager.Engine()->RegisterObjectMethod(   \
                scriptClass,                                \
                str,                                        \
                asMETHODPR(type, a, b, c),                  \
                asCALL_THISCALL)
    
        kexActor::RegisterBaseProperties<type>(scriptClass);
    
        OBJMETHOD("void SetIdealYaw(const float, const float)", SetIdealYaw,
                  (const float yaw, const float speed), void);
        OBJMETHOD("void FireProjectile(const kStr &in, const kVec3 &in, const float, bool)",
                  FireProjectile, (const kexStr &fxName, const kexVec3 &org, const float maxAngle,
                                   bool bLocalToActor), void);
        OBJMETHOD("float GetTargetDistance(void)", GetTargetDistance, (void), float);
        OBJMETHOD("float GetYawToTarget(void)", GetYawToTarget, (void), float);
        OBJMETHOD("bool CheckPosition(const kVec3 &in, const float, const float)",
                  CheckPosition, (const kexVec3 &position, const float radius, const float yaw), bool);
        OBJMETHOD("float GetBestYawToTarget(const float)",
                  GetBestYawToTarget, (const float extendedRadius), float);
    
        #define OBJPROPERTY(str, p)                         \
            scriptManager.Engine()->RegisterObjectProperty( \
                scriptClass,                                \
                str,                                        \
                asOFFSET(type, p))
    
        OBJPROPERTY("uint aiFlags", aiFlags);
        OBJPROPERTY("float attackThreshold", attackThreshold);
        OBJPROPERTY("float sightThreshold", sightThreshold);
        OBJPROPERTY("float attackThresholdTime", attackThresholdTime);
        OBJPROPERTY("float yawSpeed", yawSpeed);
        OBJPROPERTY("float thinkTime", thinkTime);
        OBJPROPERTY("bool bCanMelee", bCanMelee);
        OBJPROPERTY("bool bCanRangeAttack", bCanRangeAttack);
        OBJPROPERTY("bool bCanTeleport", bCanTeleport);
        OBJPROPERTY("bool bAttacking", bAttacking);
        OBJPROPERTY("bool bAnimTurning", bAnimTurning);
    
    #undef OBJMETHOD
    #undef OBJPROPERTY
    }

protected:
    float                       activeDistance;
    unsigned int                aiFlags;
    aiState_t                   aiState;
    kexVec3                     goalOrigin;
    float                       thinkTime;
    float                       nextThinkTime;
    unsigned int                nodeHead;
    float                       headYaw;
    float                       headPitch;
    float                       headTurnSpeed;
    float                       maxHeadAngle;
    kexVec3                     headYawAxis;
    kexVec3                     headPitchAxis;
    bool                        bCanMelee;
    bool                        bCanRangeAttack;
    bool                        bCanTeleport;
    bool                        bAttacking;
    bool                        bAnimTurning;
    float                       attackThreshold;
    float                       sightThreshold;
    float                       attackThresholdTime;
    float                       sightRange;
    float                       rangeSightDamp;
    float                       checkRadius;
    float                       meleeRange;
    float                       alertRange;
    float                       rangeDistance;
    int                         teleportChance;
    int                         giveUpChance;
    int                         rangeChance;
    float                       rangeAdjustAngle;
    float                       yawSpeed;

private:
    kexAIPhysics                physics;
    float                       idealYaw;
    float                       turningYaw;
    float                       turnSpeed;

END_CLASS();

#endif
