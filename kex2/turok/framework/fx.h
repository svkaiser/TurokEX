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

typedef struct
{
    vec3_t value;
    vec3_t rand;
} fxvector_t;

typedef struct
{
    float value;
    float rand;
} fxfloat_t;

typedef struct
{
    int value;
    float rand;
} fxint_t;

enum
{
    VFX_ANIMDEFAULT,
    VFX_ANIMONETIME,
    VFX_ANIMLOOP,
    VFX_ANIMSINWAVE
};

enum
{
    VFX_DRAWDEFAULT,
    VFX_DRAWFLAT,
    VFX_DRAWDECAL,
    VFX_DRAWSPRITE
};

enum
{
    VFX_DEFAULT,
    VFX_DESTROY,
    VFX_REFLECT,
    VFX_BOUNCE
};

typedef struct
{
    kbool               bFadeout;
    kbool               bStopAnimOnImpact;
    kbool               bOffsetFromFloor;
    kbool               bTextureWrapMirror;
    kbool               bLensFlares;
    kbool               bBlood;
    kbool               bAddOffset;
    kbool               bDepthBuffer;
    kbool               bScaleLerp;
    kbool               bNoDirection;
    kbool               bLocalAxis;
    kbool               bClientSpace;
    float               mass;
    float               translation_randomscale;
    fxvector_t          translation;
    fxfloat_t           gravity;
    float               friction;
    float               animFriction;
    fxfloat_t           scale;
    fxfloat_t           scaledest;
    fxfloat_t           forward;
    fxvector_t          offset;
    fxfloat_t           rotation_offset;
    fxfloat_t           rotation_speed;
    float               screen_offset_x;
    float               screen_offset_y;
    int                 numTextures;
    char                **textures;
    fxint_t             instances;
    fxint_t             lifetime;
    float               restart;
    int                 animspeed;
    byte                ontouch;
    byte                onplane;
    byte                drawtype;
    byte                animtype;
    byte                color1[4];
    int                 color1_randomscale;
    byte                color2[4];
    int                 color2_randomscale;
    int                 saturation_randomscale;
    int                 fadein_time;
    int                 fadeout_time;
    char                *hitFX;
    char                *hitSnd;
    char                *tickFX;
    char                *tickSnd;
    char                *expireFX;
    char                *expireSnd;
} fxinfo_t;

typedef struct fxfile_s
{
    char                name[MAX_FILEPATH];
    unsigned int        numfx;
    fxinfo_t            *info;
    struct fxfile_s     *next;
} fxfile_t;

typedef struct fx_s
{
    vec3_t              origin;
    vec4_t              rotation;
    vec3_t              translation;
    vec3_t              offset;
    fxinfo_t            *info;
    float               gravity;
    float               scale;
    float               scale_dest;
    float               forward;
    float               rotation_offset;
    float               rotation_speed;
    int                 instances;
    float               lifetime;
    float               restart;
    kbool               bAnimate;
    byte                color1[4];
    byte                color2[4];
    texture_t           **textures;
    int                 frame;
    int                 frametime;
    plane_t             *plane;
    mtx_t               matrix;
    fxfile_t            *file;
    gActor_t            *source;
    float               dist;
    struct fx_s         *prev;
    struct fx_s         *next;
} fx_t;

extern fx_t fxRoot;
extern fx_t *fxRover;

void FX_Init(void);
void FX_Kill(fx_t *fx);
void FX_Ticker(void);
fxfile_t *FX_Load(const char *name);
void FX_ClearLinks(void);
void FX_Shutdown(void);
fx_t *FX_Spawn(const char *name, gActor_t *source, vec3_t axis,
                vec3_t origin, vec4_t rotation, plane_t *plane);

#endif
