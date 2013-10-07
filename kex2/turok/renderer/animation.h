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

#ifndef __ANIMATION_H__
#define __ANIMATION_H__

typedef struct {
    kexVec3                 *translations;
    kexQuat                 *rotations;
} frameSet_t;

typedef struct {
    int                     frame;
    char                    *function;
    float                   args[4];
    char                    *argStrings[4];
} frameAction_t;

typedef struct kexAnim_s {
    char                    *alias;
    filepath_t              animFile;
    unsigned int            numFrames;
    unsigned int            numAnimsets;
    unsigned int            numActions;
    unsigned int            numTranslations;
    unsigned int            numRotations;
    kexVec3                 **translations;
    kexQuat                 **rotations;
    frameSet_t              *frameSet;
    frameSet_t              initialFrame;
    unsigned int            loopFrame;
    frameAction_t           *actions;
    float                   *yawOffsets;
    int                     animID;
} kexAnim_t;

typedef struct {
    kexAnim_t               *anim;
    int                     frame;
    int                     nextFrame;
} animTrack_t;

class kexAnimState {
public:
                            kexAnimState(void);
                            ~kexAnimState(void);

    void                    Reset(void);
    void                    Update(void);
    void                    Set(const kexAnim_t *anim, float time, int flags);
    void                    Blend(const kexAnim_t *anim, float time, float blendTime, int flags);

    static kexQuat          GetRotation(kexAnim_t *anim, int nodeNum, int frame);
    static kexVec3          GetTranslation(kexAnim_t *anim, int nodeNum, int frame);
    static kexAnim_t        *GetAnim(const kexModel_t *model, const char *name);
    static kexAnim_t        *GetAnim(const kexModel_t *model, const int id);
    static bool             CheckAnimID(const kexModel_t *model, const int id);
    static void             LoadKAnim(const kexModel_t *model);

    animTrack_t             track;
    animTrack_t             prevTrack;
    float                   deltaTime;
    float                   frameTime;
    int                     flags;
    int                     prevFlags;
    float                   baseOffset;
    kexVec3                 rootMotion;

private:
    static void             ParseKAnim(const kexModel_t *model, kexAnim_t *anim, kexLexer *lexer);

    int                     currentFrame;
    float                   time;
    float                   playTime;
    float                   blendTime;
    unsigned int            restartFrame;
};

#endif
