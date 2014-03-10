// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef _FX_H_
#define _FX_H_

#include "actor.h"
#include "textureObject.h"
#include "keymap.h"
#include "physics/physics_fx.h"

typedef struct {
    float value[3];
    float rand[3];
} fxvector_t;

typedef struct {
    float value;
    float rand;
} fxfloat_t;

typedef struct {
    int value;
    float rand;
} fxint_t;

typedef enum {
    VFX_ANIMDEFAULT     = 0,
    VFX_ANIMONETIME,
    VFX_ANIMLOOP,
    VFX_ANIMSINWAVE
} fxAnimType_t;

typedef enum {
    VFX_DRAWDEFAULT     = 0,
    VFX_DRAWFLAT,
    VFX_DRAWDECAL,
    VFX_DRAWBILLBOARD,
    VFX_DRAWSURFACE
} fxDrawType_t;

typedef enum {
    VFX_DEFAULT         = 0,
    VFX_DESTROY,
    VFX_REFLECT,
    VFX_BOUNCE
} fxCollisionType_t;

typedef struct {
    char                        *fx;
    char                        *snd;
    kexKeyMap                   *damageDef;
} fxEvent_t;

typedef struct {
    bool                        bFadeout;
    bool                        bStopAnimOnImpact;
    bool                        bOffsetFromFloor;
    bool                        bTextureWrapMirror;
    bool                        bLensFlares;
    bool                        bBlood;
    bool                        bAddOffset;
    bool                        bDepthBuffer;
    bool                        bScaleLerp;
    bool                        bActorInstance;
    bool                        bNoDirection;
    bool                        bLocalAxis;
    bool                        bClientSpace;
    bool                        bProjectile;
    bool                        bDestroyOnWaterSurface;
    bool                        bLinkArea;
    bool                        bAttachToSource;
    float                       mass;
    float                       translation_randomscale;
    fxvector_t                  translation;
    fxfloat_t                   gravity;
    float                       friction;
    float                       animFriction;
    fxfloat_t                   scale;
    fxfloat_t                   scaledest;
    fxfloat_t                   forward;
    fxvector_t                  offset;
    fxfloat_t                   rotation_offset;
    fxfloat_t                   rotation_speed;
    float                       screen_offset_x;
    float                       screen_offset_y;
    int                         numTextures;
    char                        **textures;
    fxint_t                     instances;
    fxint_t                     lifetime;
    float                       restart;
    int                         animspeed;
    fxCollisionType_t           ontouch;
    fxCollisionType_t           onplane;
    fxDrawType_t                drawtype;
    fxAnimType_t                animtype;
    byte                        color1[4];
    int                         color1_randomscale;
    byte                        color2[4];
    int                         color2_randomscale;
    int                         saturation_randomscale;
    int                         fadein_time;
    int                         fadeout_time;
    fxEvent_t                   onImpact[IT_NUMIMPACTTYPES];
    fxEvent_t                   onTick;
    fxEvent_t                   onExpire;
    fxEvent_t                   onWaterImpact;
    fxEvent_t                   onWaterTick;
    fxEvent_t                   onWaterExpire;
} fxinfo_t;

typedef struct fxfile_s {
    char                        filePath[MAX_FILEPATH];
    unsigned int                numfx;
    fxinfo_t                    *info;
} fxfile_t;

class kexWorld;

class kexFxManager {
public:
                                kexFxManager(void);
                                ~kexFxManager(void);

    void                        Init(void);
    void                        Shutdown(void);
    void                        UpdateWorld(kexWorld *world);
    fxfile_t                    *LoadKFX(const char *file);

private:
    void                        ParseEvent(fxEvent_t *fxEvent, kexLexer *lexer);
    kexHashList<fxfile_t>       kfxList;
};

extern kexFxManager fxManager;

#define FX_RAND_RANGE()     ((float)((kexRand::SysRand() % 20000) - 10000) * 0.0001f)
#define FX_RAND_FLOAT(x)    (kexRand::Float() * x)
#define FX_RAND_VALUE(x)                        \
    ((x > 0) ? (kexRand::SysRand() % (x + 1)) : \
    -(kexRand::SysRand() % (1 - x)))

BEGIN_EXTENDED_CLASS(kexFx, kexWorldObject);
public:
                                kexFx(void);
                                ~kexFx(void);

    virtual void                LocalTick(void);
    virtual void                Tick(void);

    void                        Spawn(void);
    void                        SetViewDistance(void);
    kexFx                       *SpawnChild(const char *name);
    kexFx                       *Event(fxEvent_t *fxEvent, kexWorldObject *target);
    void                        SetParent(kexFx *targ);

    kexVec3                     &GetVelocityOffset(void) { return velOffset; }
    void                        SetVelocityOffset(const kexVec3 &vel) { velOffset = vel; }
    const float                 Distance(void)const { return distance; }
    kexTexture                  *Texture(void) { return textures[frame]; }
    kexFxPhysics                *Physics(void) { return &physics; }
    kexFx                       *GetParent(void) { return parent; }

    kexLinklist<kexFx>          worldLink;
    fxinfo_t                    *fxInfo;
    fxfile_t                    *fxFile;
    bool                        bAnimate;
    bool                        bForcedRestart;
    float                       restart;
    float                       drawScale;
    float                       drawScaleDest;
    float                       rotationOffset;
    float                       rotationSpeed;
    float                       gravity;
    float                       speed;
    byte                        color1[4];
    byte                        color2[4];

private:
    kexFx                       *parent;
    kexFxPhysics                physics;
    kexVec3                     offset;
    kexVec3                     velOffset;
    int                         instances;
    float                       lifeTime;
    kexTexture                  **textures;
    int                         frame;
    int                         frameTime;
    float                       distance;
END_CLASS();

#endif
