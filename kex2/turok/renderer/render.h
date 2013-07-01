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

#ifndef _RENDER_H_
#define _RENDER_H_

//
// MODELS
//
#define MDF_UNKNOWN1                1
#define MDF_FULLBRIGHT              2
#define MDF_NOCULLFACES             4
#define MDF_RENDERSPECULAR          8
#define MDF_SHINYSURFACE            16
#define MDF_UNKNOWN32               32
#define MDF_SOLID                   64
#define MDF_MASKED                  128
#define MDF_TRANSPARENT1            256
#define MDF_TRANSPARENT2            512
#define MDF_COLORIZE                1024
#define MDF_METALSURFACE            2048
#define MDF_UNKNOWN4096             4096
#define MDF_UNKNOWN8192             8192
#define MDF_UNKNOWN16384            16384
#define MDF_UNKNOWN32768            32768
#define MDF_UNKNOWN65536            65536

typedef struct
{
    int                 frame;
    char                *function;
    float               args[4];
    char                *argStrings[4];
} action_t;

typedef struct
{
    unsigned int        flags;
    unsigned int        numverts;
    unsigned int        numtris;
    vec3_t              *xyz;
    float               *coords;
    float               *normals;
    word                *tris;
    char                texpath[MAX_FILEPATH];
    rcolor              color1;
    rcolor              color2;
} mdlsection_t;

typedef struct
{
    unsigned int        numsections;
    mdlsection_t        *sections;
} mdlmesh_t;

typedef struct
{
    vec3_t vec;
} animtranslation_t;

typedef struct
{
    vec4_t vec;
} animrotation_t;

typedef struct
{
    animtranslation_t   *translation;
    animrotation_t      *rotation;
} frameset_t;

typedef struct
{
    unsigned int        numvariants;
    word                *variants;
    unsigned int        nummeshes;
    mdlmesh_t           *meshes;
    unsigned int        numchildren;
    word                *children;
} mdlnode_t;

typedef enum
{
    ANF_BLEND       = 1,
    ANF_LOOP        = 2,
    ANF_STOPPED     = 4,
    ANF_NOINTERRUPT = 8,
    ANF_ROOTMOTION  = 16,
    ANF_PAUSED      = 32,
    ANF_CROSSFADE   = 64
} animflags_t;

typedef struct anim_s
{
    char                *alias;
    char                animpath[MAX_FILEPATH];
    unsigned int        numframes;
    unsigned int        numanimsets;
    unsigned int        numactions;
    unsigned int        numtranslations;
    unsigned int        numrotations;
    animtranslation_t   **translations;
    animrotation_t      **rotations;
    frameset_t          *frameset;
    frameset_t          initial;
    unsigned int        loopframe;
    action_t            *actions;
    float               *yawOffsets;
    int                 animID;
} anim_t;

typedef struct
{
    anim_t              *anim;
    int                 frame;
    int                 nextframe;
} animtrack_t;

typedef struct
{
    anim_t              *anim;
    int                 frame;
    int                 nextframe;
    int                 flags;
} oldtrack_t;

typedef struct
{
    animtrack_t         track;
    animtrack_t         prevtrack;
    oldtrack_t          oldtrack;
    int                 currentFrame;
    float               time;
    float               deltatime;
    float               playtime;
    float               frametime;
    float               blendtime;
    int                 flags;
    vec3_t              rootMotion;
    unsigned int        restartframe;
} animstate_t;

typedef struct kmodel_s
{
    char                mdlpath[MAX_FILEPATH];
    bbox_t              bbox;
    unsigned int        numbehaviors;
    unsigned int        numnodes;
    unsigned int        numanimations;
    mdlnode_t           *nodes;
    anim_t              *anims;
    struct kmodel_s     *next;
} kmodel_t;

void Mdl_Init(void);
void Mdl_GetAnimRotation(vec4_t out, anim_t *anim, int nodenum, int frame);
void Mdl_GetAnimTranslation(vec3_t out, anim_t *anim, int nodenum, int frame);
void Mdl_UpdateAnimState(animstate_t *astate);
void Mdl_BlendAnimStates(animstate_t *astate, anim_t *anim,
                         float time, float blendtime, animflags_t flags);
void Mdl_SetAnimState(animstate_t *astate, anim_t *anim,
                      float time, animflags_t flags);

anim_t *Mdl_GetAnim(kmodel_t *model, const char *name);
anim_t *Mdl_GetAnimFromID(kmodel_t *model, int id);
kbool Mdl_CheckAnimID(kmodel_t *model, int id);

kmodel_t *Mdl_Find(const char *name);
kmodel_t *Mdl_Load(const char *file);

//
// MAIN
//

extern float rRenderTime;
extern kbool showorigin;
extern kbool bWireframe;

void R_DrawFrame(void);
void R_FinishFrame(void);
void R_Shutdown(void);
void R_Init(void);

void R_TraverseDrawNode(gActor_t *actor, mdlnode_t *node, animstate_t *animstate);

//
// FX
//

void R_DrawFX(void);

//
// CAMERA
//

void R_RenderCameraView(void);

//
// DEBUG
//

void R_DrawCollision(void);
void R_DrawPlaneNormals(void);
void R_DrawRadius(float x, float y, float z, float radius, float height,
                  byte r, byte g, byte b);
void R_DrawBoundingBox(bbox_t bbox, byte r, byte g, byte b);
void R_DrawOrigin(vec3_t origin, float size);

//
// FONT
//

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} atlas_t;

typedef struct
{
    char    *texture;
    int     width;
    int     height;
    atlas_t atlas[256];
} font_t;

void Font_MapChar(font_t *font, byte ch, int x, int y, int w, int h);
float Font_StringWidth(font_t *font, const char* string, float scale, int fixedLen);

//
// CANVAS
//

#include "gl.h"

typedef struct
{
    byte    drawColor[4][4];
    float   drawCoord[4];
    float   scale;
    kbool   center;
    font_t  *font;
} canvas_t;

void Canvas_SetDrawColor(canvas_t *canvas, byte r, byte g, byte b);
void Canvas_SetDrawAlpha(canvas_t *canvas, byte a);
void Canvas_SetDrawScale(canvas_t *canvas, float scale);
void Canvas_SetTextureTile(canvas_t *canvas,
                           float u1, float u2, float t1, float t2);
void Canvas_SetFont(canvas_t *canvas, font_t *font);
void Canvas_DrawTile(canvas_t *canvas, texture_t *texture,
                     float x, float y, float w, float h);
void Canvas_DrawFixedTile(canvas_t *canvas, texture_t *texture,
                     float x, float y);
void Canvas_DrawString(canvas_t *canvas, const char *string,
                       float x, float y, kbool center);
void Canvas_DrawFixedString(canvas_t *canvas, const char *string,
                            float x, float y, kbool center);

#endif