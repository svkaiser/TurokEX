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
// DESCRIPTION: Collision detection / Physics behavior
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "mathlib.h"
#include "game.h"
#include "actor_old.h"
#include "level.h"
#include "world.h"
#include "physics.h"

#define TRYMOVE_COUNT   5

enum {
    scPhysics_mass = 0,
    scPhysics_friction,
    scPhysics_airFriction,
    scPhysics_bounceDamp,
    scPhysics_rotorSpeed,
    scPhysics_rotorFriction,
    scPhysics_bRotor,
    scPhysics_bOrientOnSlope,
    scPhysics_rotorVector,
    scPhysics_end
};

static const sctokens_t physicsTokens[scPhysics_end+1] = {
    { scPhysics_mass,           "mass"                  },
    { scPhysics_friction,       "friction"              },
    { scPhysics_airFriction,    "airFriction"           },
    { scPhysics_bounceDamp,     "bounceDamp"            },
    { scPhysics_rotorSpeed,     "rotorSpeed"            },
    { scPhysics_rotorFriction,  "rotorFriction"         },
    { scPhysics_bRotor,         "bRotor"                },
    { scPhysics_bOrientOnSlope, "bOrientOnSlope"        },
    { scPhysics_rotorVector,    "rotorVector"           },
    { -1,                       NULL                    }
};

//
// kexPhysics::kexPhysics
//

kexPhysics::kexPhysics(void) {
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
// kexPhysics::~kexPhysics
//

kexPhysics::~kexPhysics(void) {
}

//
// kexPhysics::Parse
//

void kexPhysics::Parse(kexLexer *lexer) {
    // read into nested block
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK) {
        switch(lexer->GetIDForTokenList(physicsTokens, lexer->Token())) {
        case scPhysics_mass:
            this->mass = (float)lexer->GetFloat();
            break;
        case scPhysics_friction:
            this->friction = (float)lexer->GetFloat();
            break;
        case scPhysics_airFriction:
            this->airFriction = (float)lexer->GetFloat();
            break;
        case scPhysics_bounceDamp:
            this->bounceDamp = (float)lexer->GetFloat();
            break;
        case scPhysics_rotorSpeed:
            this->rotorSpeed = (float)lexer->GetFloat();
            break;
        case scPhysics_rotorFriction:
            this->rotorFriction = (float)lexer->GetFloat();
            break;
        case scPhysics_bRotor:
            this->bRotor = (lexer->GetNumber() > 0);
            break;
        case scPhysics_bOrientOnSlope:
            this->bRotor = (lexer->GetNumber() > 0);
            break;
        case scPhysics_rotorVector:
            this->rotorVector = lexer->GetVector3();
            break;
        default:
            if(lexer->TokenType() == TK_IDENIFIER) {
                parser.Error("kexPhysics::Parse: unknown token: %s\n",
                    lexer->Token());
            }
            break;
        }
        
        lexer->Find();
    }
}

//
// kexPhysics::GroundDistance
//

float kexPhysics::GroundDistance(void) {
    if(groundGeom == NULL) {
        return 0;
    }

    float radius = owner->Radius();
    float height = owner->BaseHeight();
    kexVec3 org = owner->GetOrigin();

    kexVec3 offset(groundGeom->plane.a < 0 ? radius : -radius,
                   groundGeom->plane.b < 0 ? height : 0,
                   groundGeom->plane.c < 0 ? radius : -radius);

    return org.y - (-offset.Dot(groundGeom->plane.Normal()) +
        groundGeom->GetDistance(org));
}

//
// kexPhysics::OnSteepSlope
//

bool kexPhysics::OnSteepSlope(void) {
    if(groundGeom == NULL) {
        return false;
    }
    return (groundGeom->plane.Normal().Dot(-localWorld.GetGravity()) <= 0.5f);
}

//
// kexPhysics::OnGround
//

bool kexPhysics::OnGround(void) {
    if(groundGeom == NULL) {
        return false;
    }
    if(OnSteepSlope()) {
        return false;
    }
    return GroundDistance() <= ONPLANE_EPSILON;
}

//
// kexPhysics::ImpactVelocity
//

void kexPhysics::ImpactVelocity(kexVec3 &vel, kexVec3 &normal, const float force) {
    kexVec3 dir = vel;
    float d = vel.Unit();
    float bounce = force;

    if(bounceDamp != 0) {
        if(d >= 1.05f) {
            bounce = (force + bounceDamp);

            if(d < 16.8f && force < 1.0f)
                bounce = 0.2f;
        }
    }

    vel = dir - (normal * (dir.Dot(normal) * bounce));

    if(d != 0) {
        vel *= (dir.Unit() / d);
    }
}

//
// kexPhysics::ApplyFriction
//

void kexPhysics::ApplyFriction(void) {
    float speed;

    speed = velocity.Unit();

    if(speed < VELOCITY_EPSILON) {
        velocity.x = 0;
        velocity.z = 0;
    }
    else {
        float clipspeed = speed - (speed * friction);

        if(clipspeed < 0) {
            clipspeed = 0;
        }

        clipspeed /= speed;

        // de-accelerate velocity
        velocity.x = velocity.x * clipspeed;
        velocity.z = velocity.z * clipspeed;
    }

    float yFriction = 0;

    if(airFriction == 0) {
        if(groundGeom == NULL) {
            return;
        }

        if(GroundDistance() > ONPLANE_EPSILON) {
            return;
        }

        if(OnSteepSlope() && velocity.Dot(groundGeom->plane.Normal()) <= 0) {
            return;
        }

        yFriction = friction;
    }
    else {
        yFriction = airFriction;
    }

    speed = velocity.y;

    if(speed < VELOCITY_EPSILON) {
        velocity.y = 0;
    }
    else {
        float clipspeed = speed - (speed * yFriction);

        if(clipspeed < 0) {
            clipspeed = 0;
        }

        clipspeed /= speed;

        // de-accelerate velocity
        velocity.y = velocity.y * clipspeed;
    }
}

//
// kexPhysics::Think
//

void kexPhysics::Think(const float timeDelta) {
    if(owner == NULL) {
        return;
    }
    if(owner->bStatic == true) {
        return;
    }

    traceInfo_t trace;
    kexVec3 oldVelocity = velocity;
    kexVec3 start = owner->GetOrigin();
    kexVec3 end;
    kexVec3 direction;
    kexVec3 vel;
    kexVec3 normals[TRYMOVE_COUNT];
    int moves = 0;
    int hits;
    float time = timeDelta;
    kexVec3 gravity;
    float massAmount = (mass * timeDelta);
    float radius = owner->Radius();
    float height = owner->BaseHeight();

    gravity = localWorld.GetGravity();

    trace.owner = owner;
    trace.bUseBBox = true;
    trace.localBBox.min.Set(-(radius * 0.5f), 0, -(radius * 0.5f));
    trace.localBBox.max.Set(radius * 0.5f, height, radius * 0.5f);
    trace.bbox = trace.localBBox;
    trace.bbox.min += start;
    trace.bbox.max += start;

    trace.fraction = 1.0f;
    trace.hitActor = NULL;
    trace.hitTri = NULL;
    trace.hitMesh = NULL;
    trace.hitVector.Clear();
    trace.hitNormal.Clear();
    trace.start = start;
    trace.end = (gravity * mass) * mass;
    trace.dir = gravity;

    // need to determine if we're standing on the ground or not
    localWorld.Trace(&trace);
    groundGeom = trace.hitTri;

    bOnGround = OnGround();

    // handle freefall if not touching the ground
    if(!bOnGround) {
        velocity += (gravity * massAmount);
    }
    else {
        // project along the ground plane
        if(velocity.Dot(groundGeom->plane.Normal()) <= 1.024f) {
            ImpactVelocity(velocity, groundGeom->plane.Normal(), 1.024f);

            normals[moves++] = trace.hitNormal;
        }
    }

    for(int i = 0; i < TRYMOVE_COUNT; i++) {
        start = owner->GetOrigin();
        end = start + (velocity * time);

        direction = (end - start);
        direction.Normalize();

        trace.fraction = 1.0f;
        trace.hitActor = NULL;
        trace.hitTri = NULL;
        trace.hitMesh = NULL;
        trace.hitVector.Clear();
        trace.hitNormal.Clear();
        trace.start = start;
        trace.end = end;
        trace.dir = direction;

        // trace through world
        localWorld.Trace(&trace);
        time -= (time * trace.fraction);

        if(trace.fraction >= 1) {
            // went the entire distance
            owner->SetOrigin(end);
            break;
        }

        owner->SetOrigin(trace.hitVector);

        if(trace.hitActor == NULL) {
            // nudge origin away from plane
            owner->SetOrigin(owner->GetOrigin() - (direction * 0.125f));

            // don't climb on steep slopes
            if(trace.hitNormal.Dot(gravity) >= -0.5f) {
                trace.hitNormal.y = 0;
            }
        }

        if(moves >= TRYMOVE_COUNT) {
            break;
        }

        normals[moves++] = trace.hitNormal;

        // try all interacted normals
        for(hits = 0; hits < moves; hits++) {
            if(velocity.Dot(normals[hits]) >= 0) {
                continue;
            }

            vel = velocity;
            ImpactVelocity(vel, normals[hits], 1.024f);

            // try bumping against another plane
            for(int j = 0; j < moves; j++) {
                if(j == hits || vel.Dot(normals[j]) >= 0) {
                    continue;
                }

                // bump into second plane
                ImpactVelocity(vel, normals[j], 1.024f);

                if(vel.Dot(normals[hits]) >= 0) {
                    continue;
                }

                // slide along the crease between two planes
                kexVec3 dir = normals[hits].Cross(normals[j]).Normalize();
                float d = dir.Dot(velocity);
                vel = dir * d;

                // see if it bumps into a third plane
                for(int k = 0; k < moves; k++) {
                    if(k != j && k != hits && vel.Dot(normals[k]) < 0) {
                        // force a dead stop
                        velocity.Clear();
                        return;
                    }
                }
            }

            velocity = vel;
            break;
        }
    }

    ApplyFriction();
}

//
// kexPhysics::InitObject
//

void kexPhysics::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kPhysics",
        sizeof(kexPhysics),
        asOBJ_REF | asOBJ_NOCOUNT);

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kPhysics",                                 \
        str,                                        \
        asMETHODPR(kexPhysics, a, b, c),            \
        asCALL_THISCALL)

    OBJMETHOD("kActor @GetOwner(void)", GetOwner, (void), kexActor*);
    OBJMETHOD("void SetOwner(kActor@)", SetOwner, (kexActor *o), void);
    OBJMETHOD("kVec3 &GetVelocity(void)", GetVelocity, (void), kexVec3&);
    OBJMETHOD("void SetVelocity(const kVec3 &in)", SetVelocity, (const kexVec3 &vel), void);
    OBJMETHOD("bool OnGround(void)", OnGround, (void), bool);
    OBJMETHOD("bool OnSteepSlope(void)", OnSteepSlope, (void), bool);
    OBJMETHOD("float GroundDistance(void)", GroundDistance, (void), float);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kPhysics",                                 \
        str,                                        \
        asOFFSET(kexPhysics, p))

    OBJPROPERTY("bool bRotor", bRotor);
    OBJPROPERTY("bool bOrientOnSlope", bOrientOnSlope);
    OBJPROPERTY("float friction", friction);
    OBJPROPERTY("float airFriction", airFriction);
    OBJPROPERTY("float mass", mass);
    OBJPROPERTY("float bounceDamp", bounceDamp);
    OBJPROPERTY("float rotorSpeed", rotorSpeed);
    OBJPROPERTY("float rotorFriction", rotorFriction);
    OBJPROPERTY("kVec3 rotorVector", rotorVector);

#undef OBJMETHOD
#undef OBJPROPERTY

    scriptManager.Engine()->RegisterEnum("EnumWaterLevelType");
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_INVALID", WLT_INVALID);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_OVER", WLT_OVER);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_BETWEEN", WLT_BETWEEN);
    scriptManager.Engine()->RegisterEnumValue("EnumWaterLevelType","WLT_UNDER", WLT_UNDER);

    scriptManager.Engine()->RegisterObjectMethod(
        "kActor",
        "kPhysics @Physics(void)",
        asMETHODPR(kexWorldActor, Physics, (void), kexPhysics*),
        asCALL_THISCALL);
}








/********************* OLD CODE ***************************/








//
// G_ClipVelocity
//

void G_ClipVelocity(vec3_t out, vec3_t velocity, vec3_t normal, float fudge)
{
    float d;
    vec3_t n;

    d = Vec_Dot(velocity, normal) * fudge;
    Vec_Scale(n, normal, d);
    Vec_Sub(out, velocity, n);
    d = Vec_Unit3(out);

    if(d != 0)
        Vec_Scale(out, out, Vec_Unit3(velocity) / d);
}

//
// G_SlideOnCrease
//

static void G_SlideOnCrease(vec3_t out, vec3_t velocity, vec3_t v1, vec3_t v2)
{
    vec3_t dir;

    Vec_Cross(dir, v1, v2);
    Vec_Normalize3(dir);
    Vec_Scale(out, dir, Vec_Dot(velocity, dir));
}

//
// G_ApplyGravity
//

void G_ApplyGravity(vec3_t origin, vec3_t velocity, plane_t *plane,
                    float mass, float timeDelta)
{
    if(plane == NULL)
        return;

    if(origin[1] - Plane_GetDistance(plane, origin) > 0.01f)
        velocity[1] -= (mass * timeDelta);
}

//
// G_ApplyFriction
//

void G_ApplyFriction(vec3_t velocity, float friction, kbool effectY)
{
    float speed;

    speed = Vec_Unit3(velocity);

    if(speed < VELOCITY_EPSILON)
    {
        velocity[0] = 0;
        velocity[2] = 0;
    }
    else
    {
        float clipspeed = speed - (speed * friction);

        if(clipspeed < 0) clipspeed = 0;
        clipspeed /= speed;

        // de-accelerate velocity
        velocity[0] = velocity[0] * clipspeed;
        velocity[2] = velocity[2] * clipspeed;

        if(effectY)
            velocity[1] = velocity[1] * clipspeed;
    }
}

//
// G_ApplyBounceVelocity
//

void G_ApplyBounceVelocity(vec3_t velocity, vec3_t reflection, float amount)
{
    float bounce = 1.0f;

    if(amount != 0)
    {
        float d = Vec_Unit3(velocity);

        if(d >= 1.05f)
        {
            bounce = (1 + amount);

            if(d < 16.8f && amount < 1.0f)
                bounce = 0.2f;
        }
    }

    G_ClipVelocity(velocity, velocity, reflection, bounce);
}

//
// G_TryMove
//

kbool G_TryMove(gActor_t *source, vec3_t origin, vec3_t dest, plane_t **plane)
{
    plane_t *newPlane = NULL;
    trace_t trace;

    trace = Trace(origin, dest, *plane, source, PF_CLIP_ALL | PF_DROPOFF);
    *plane = trace.pl;

    if(trace.type != TRT_NOHIT)
        Vec_Copy3(dest, trace.hitvec);

    return Plane_PointInRange(*plane, dest[0], dest[2]);
}

//
// G_ClimbOnWall
//

static void G_ClimbOnWall(vec3_t origin, vec3_t velocity, plane_t *plane)
{
    vec3_t end;
    vec3_t vr;
    vec3_t ray;
    float lenxz;
    float leny;
    float y1, y2;
    float d;

    Vec_Add(end, origin, velocity);
    Vec_Sub(vr, end, plane->points[0]);

    y1 = Plane_GetDistance(plane, origin);
    y2 = Plane_GetDistance(plane, end);

    d = Vec_Dot(vr, plane->normal);

    if(d > 0)
        return;

    ray[0] = end[0] - origin[0];
    ray[1] = y2 - y1;
    ray[2] = end[2] - origin[2];

    lenxz = Vec_Unit2(ray);
    leny = ray[1]*ray[1]+lenxz;

    if(leny == 0)
        origin[1] = y1;
    else
    {
        float dist;

        dist = (float)sqrt(lenxz / leny);

        origin[0] = (end[0] - origin[0]) * dist + origin[0];
        origin[1] = (y2 - y1) * dist + y1;
        origin[2] = (end[2] - origin[2]) * dist + origin[2];
    }
}

//
// G_ClipMovement
//
// Trace against surrounding planes and slide
// against it if needed, clipping velocity
// along the way
//

kbool G_ClipMovement(vec3_t origin, vec3_t velocity, float time,
                     plane_t **plane, gActor_t *actor)
{
    trace_t trace;
    vec3_t start;
    vec3_t end;
    vec3_t vel;
    vec3_t normals[TRYMOVE_COUNT];
    int moves;
    int i;
    int hits;
    kbool hitOk;
    kbool onSlope;

    if(*plane == NULL)
        return true;

    onSlope = false;
    hitOk = false;

    // handle cases when standing on wall surfaces
    if(actor && Plane_IsAWall(*plane))
    {
        if(!((*plane)->flags & CLF_CLIMB))
        {
            // slide down on steep slopes
            if(Actor_OnGround(actor))
            {
                vec3_t dir;

                Plane_GetInclinationVector(*plane, dir);

                Vec_Scale(dir, dir, actor->mass * time);
                Vec_Sub(velocity, velocity, dir);

                onSlope = true;
            }
        }
        // handle climbing
        else if(actor->physics & PF_CLIMBSURFACES)
        {
            Vec_Scale(vel, velocity, time);
            G_ClimbOnWall(origin, vel, *plane);
        }
    }

    // set start point
    Vec_Copy3(start, origin);
    Vec_Copy3(vel, velocity);
    moves = 0;

    for(i = 0; i < TRYMOVE_COUNT; i++)
    {
        // set end point
        end[0] = start[0] + (vel[0] * time);
        end[1] = start[1] + (vel[1] * time);
        end[2] = start[2] + (vel[2] * time);

        // get trace results
        trace = Trace(start, end, *plane, actor, actor->physics);

        *plane = trace.pl;

        if(trace.type == TRT_NOHIT)
        {
            // went the entire distance
            break;
        }

        actor->bClimbing = (trace.hitpl && trace.hitpl->flags & CLF_CLIMB);

        hitOk = true;

        Vec_Copy3(normals[moves++], trace.normal);

        // try all interacted normals
        for(hits = 0; hits < moves; hits++)
        {
            if(Vec_Dot(vel, normals[hits]) < 0)
            {
                int j;
                int k;
                float b;

                if(trace.type != TRT_OBJECT)
                    b = 1 - (1 + trace.frac) + 0.01f;
                else
                    b = 1;

                // slide along this plane
                G_ClipVelocity(vel, vel, normals[hits], b);

                // try bumping against another plane
                for(j = 0; j < moves; j++)
                {
                    if(j != hits && Vec_Dot(vel, normals[j]) < 0)
                    {
                        // slide along the crease between two planes
                        G_SlideOnCrease(vel, vel,
                            normals[hits], normals[j]);

                        // see if it bumps into a third plane
                        for(k = 0; k < moves; k++)
                        {
                            if(k != j && k != hits &&
                                Vec_Dot(vel, normals[k]) < 0)
                            {
                                // force a dead stop
                                Vec_Set3(velocity, 0, 0, 0);
                                return true;
                            }
                        }
                    }
                }
            }
        }

        // force a deadstop if clipped velocity is against
        // the original velocity or if exceeded max amount of
        // attempted moves (don't count against objects hit)
        if(trace.type != TRT_OBJECT)
        {
            if(Vec_Dot(vel, velocity) <= 0)
            {
                if(!onSlope)
                {
                    velocity[0] = 0;
                    velocity[2] = 0;
                }
                break;
            }
        }

        // update velocity and try another move
        Vec_Copy3(velocity, vel);
    }

    Vec_Scale(vel, velocity, time);

    // stop all movement if not in a valid plane
    Vec_Add(end, origin, vel);
    if(!Plane_PointInRange(*plane, end[0], end[2]))
    {
        hitOk = true;
        velocity[0] = 0;
        velocity[2] = 0;
    }

    // advance position
    Vec_Add(origin, origin, vel);

    // clip origin/velocity for ceiling and floors
    if(actor)
    {
        float dist;

        // test the floor and adjust height
        dist = origin[1] - Plane_GetDistance(*plane, origin);
        if(dist <= ONPLANE_EPSILON)
        {
            if(!((*plane)->flags & CLF_CLIMB))
                origin[1] = origin[1] - dist;
            else
            {
                float d;

                d = Vec_Dot(origin, (*plane)->normal) -
                    Vec_Dot((*plane)->points[0], (*plane)->normal);

                if(d > 0)
                {
                    vec3_t dir;

                    Vec_Copy3(dir, origin);
                    Vec_Normalize3(dir);
                    Vec_Scale(dir, dir, d);
                    Vec_Sub(origin, origin, dir);
                }
            }

            hitOk = true;
            G_ApplyBounceVelocity(velocity, (*plane)->normal,
                actor->bounceDamp);

            if(actor->bounceDamp == 0 && velocity[1] > 0)
                velocity[1] = 0;
        }

        // test the ceiling and adjust height
        if((*plane)->flags & CLF_CHECKHEIGHT)
        {
            float offset = (actor->height + (actor->viewHeight * 0.5f));

            dist = Plane_GetHeight(*plane, origin);
            if((dist - (origin[1] + offset) < 1.024f))
            {
                origin[1] = dist - (1.024f + offset);
                hitOk = true;
                G_ClipVelocity(velocity, velocity, (*plane)->ceilingNormal, 1);
            }
        }
    }

    return hitOk;
}
