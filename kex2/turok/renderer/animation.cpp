// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: Basic animation system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "script.h"
#include "renderSystem.h"
#include "renderModel.h"
#include "animation.h"

#define ANIM_CLOCK_SPEED    60

enum {
    scanim_anim = 0,
    scanim_numframes,
    scanim_numnodes,
    scanim_numtranslationsets,
    scanim_numrotationsets,
    scanim_nodeframes,
    scanim_initial_t,
    scanim_initial_r,
    scanim_translationsets,
    scanim_rotationsets,
    scanim_numactions,
    scanim_actions,
    scanim_initialtranslation,
    scanim_initialrotation,
    scanim_turninfo,
    scanim_loopframe,
    scanim_end
};

static const sctokens_t animtokens[scanim_end+1] = {
    { scanim_anim,              "anim"                  },
    { scanim_numframes,         "numframes"             },
    { scanim_numnodes,          "numnodes"              },
    { scanim_numtranslationsets,"numtranslationsets"    },
    { scanim_numrotationsets,   "numrotationsets"       },
    { scanim_nodeframes,        "nodeframes"            },
    { scanim_initial_t,         "initialtranslation"    },
    { scanim_initial_r,         "initialrotation"       },
    { scanim_translationsets,   "translationsets"       },
    { scanim_rotationsets,      "rotationsets"          },
    { scanim_numactions,        "numactions"            },
    { scanim_actions,           "actions"               },
    { scanim_turninfo,          "turninfo"              },
    { scanim_loopframe,         "loopframe"             },
    { -1,                       NULL                    }
};

//
// kexAnimState::kexAnimState
//

kexAnimState::kexAnimState(void) {
    Reset();
}

//
// kexAnimState::~kexAnimState
//

kexAnimState::~kexAnimState(void) {
}

//
// kexAnimState::Reset
//

void kexAnimState::Reset(void) {
    track.frame = 0;
    track.nextFrame = 1;
    prevTrack.frame = 0;
    prevTrack.nextFrame = 0;

    time = 0;
    deltaTime = 0;
    playTime = 0;
    frameTime = 0;
    blendTime = 0;
    flags = 0;
    prevFlags = 0;

    rootMotion.Clear();
    baseOffset = 0.0f;
    restartFrame = 0;
}

//
// kexAnimState::GetRotation
//

kexQuat kexAnimState::GetRotation(kexAnim_t *anim, int nodeNum, int frame) {
    if(anim->frameSet[nodeNum].rotations == NULL) {
        return anim->initialFrame.rotations[nodeNum];
    }
    
    return anim->frameSet[nodeNum].rotations[frame];
}

//
// kexAnimState::GetTranslation
//

kexVec3 kexAnimState::GetTranslation(kexAnim_t *anim, int nodeNum, int frame) {
    if(anim->frameSet[nodeNum].translations == NULL) {
        return anim->initialFrame.translations[nodeNum];
    }

    return anim->frameSet[nodeNum].translations[frame];
}

//
// kexAnimState::GetAnim
//

kexAnim_t *kexAnimState::GetAnim(const kexModel_t *model, const char *name) {
    unsigned int i;

    if(model->anims == NULL || model->numAnimations <= 0)
        return NULL;

    for(i = 0; i < model->numAnimations; i++) {
        if(!strcmp(model->anims[i].alias, name))
            return &model->anims[i];
    }

    return NULL;
}

//
// kexAnimState::GetAnim
//

kexAnim_t *kexAnimState::GetAnim(const kexModel_t *model, const int id) {
    unsigned int i;

    if(model->anims == NULL || model->numAnimations <= 0)
        return NULL;

    for(i = 0; i < model->numAnimations; i++) {
        if(model->anims[i].animID == id)
            return &model->anims[i];
    }

    return NULL;
}

//
// kexAnimState::CheckAnimID
//

bool kexAnimState::CheckAnimID(const kexModel_t *model, const int id) {
    unsigned int i;

    if(model->anims == NULL || model->numAnimations <= 0)
        return false;

    for(i = 0; i < model->numAnimations; i++) {
        if(model->anims[i].animID == id)
            return true;
    }

    return false;
}

//
// kexAnimState::Set
//

void kexAnimState::Set(const kexAnim_t *anim, float animTime, int animFlags) {
    time                    = (float)client.GetTicks() + animTime;
    deltaTime               = 0;
    blendTime               = 0;
    playTime                = 0;
    frameTime               = animTime;
    track.frame             = 0;
    track.nextFrame         = 1;
    flags                   = animFlags;
    prevFlags               = 0;
    prevTrack.frame         = 0;
    prevTrack.nextFrame     = 0;
    track.anim              = const_cast<kexAnim_t*>(anim);
    prevTrack.anim          = NULL;
    restartFrame            = anim->loopFrame;
    currentFrame            = 0;
}

//
// kexAnimState::Set
//

void kexAnimState::Set(const kexStr &animName, float animTime, int animFlags) {
    kexAnim_t *anim;

    if(owner == NULL) {
        return;
    }

    if(anim = kexAnimState::GetAnim(owner->Model(), animName.c_str())) {
        Set(anim, animTime, animFlags);
    }
}

//
// kexAnimState::Set
//

void kexAnimState::Set(const int id, float animTime, int animFlags) {
    kexAnim_t *anim;

    if(owner == NULL) {
        return;
    }

    if(anim = kexAnimState::GetAnim(owner->Model(), id)) {
        Set(anim, animTime, animFlags);
    }
}

//
// kexAnimState::Blend
//

void kexAnimState::Blend(const kexAnim_t *anim, float animTime, float animBlendTime, int animFlags) {
    bool bSameAnim = (anim == track.anim);

    if(flags & ANF_NOINTERRUPT && !(flags & ANF_STOPPED)) {
        return;
    }

    if(bSameAnim && !(flags & ANF_STOPPED) && animFlags == flags) {
        return;
    }

    if(bSameAnim) {
        flags &= ~ANF_STOPPED;
        return;
    }

    prevFlags               = flags;
    flags                   = animFlags | ANF_BLEND;
    prevTrack.frame         = track.frame;
    prevTrack.nextFrame     = track.nextFrame;
    track.frame             = bSameAnim ? anim->loopFrame : 0;
    track.nextFrame         = bSameAnim ? (anim->loopFrame+1) : 1;
    time                    = (float)client.GetTicks() + animBlendTime;
    playTime                = 0;
    frameTime               = animTime;
    blendTime               = animBlendTime;
    deltaTime               = 0;
    prevTrack.anim          = track.anim;
    track.anim              = const_cast<kexAnim_t*>(anim);
    restartFrame            = anim->loopFrame;
    currentFrame            = 0;
}

//
// kexAnimState::Blend
//

void kexAnimState::Blend(const kexStr &animName, float animTime, float animBlendTime, int animFlags) {
    kexAnim_t *anim;

    if(owner == NULL) {
        return;
    }

    if(anim = kexAnimState::GetAnim(owner->Model(), animName.c_str())) {
        Blend(anim, animTime, animBlendTime, animFlags);
    }
}

//
// kexAnimState::Blend
//

void kexAnimState::Blend(const int id, float animTime, float animBlendTime, int animFlags) {
    kexAnim_t *anim;

    if(owner == NULL) {
        return;
    }

    if(anim = kexAnimState::GetAnim(owner->Model(), id)) {
        Blend(anim, animTime, animBlendTime, animFlags);
    }
}

//
// kexAnimState::Update
//

void kexAnimState::Update(void) {
    if(flags & ANF_LOOP && flags & ANF_STOPPED) {
        flags &= ~ANF_STOPPED;
    }

    if(flags & (ANF_STOPPED|ANF_PAUSED)) {
        return;
    }
    if(track.anim == NULL) {
        return;
    }

    float blend = (flags & ANF_BLEND) ? blendTime : frameTime;

    deltaTime += ((client.GetRunTime()*ANIM_CLOCK_SPEED)/blend);

    if(deltaTime > 1) {
        time = (float)client.GetTicks() + frameTime;

        while(deltaTime > 1) {
            deltaTime = deltaTime - 1;
            if(flags & ANF_BLEND) {
                prevTrack.anim = NULL;
                flags &= ~ANF_BLEND;
            }

            if(++track.frame >= (int)track.anim->numFrames) {
                track.frame = restartFrame;
            }

            if(++track.nextFrame >= (int)track.anim->numFrames) {
                track.nextFrame = restartFrame;
                deltaTime = 0;

                if(!(flags & ANF_LOOP)) {
                    playTime = 0;
                    flags |= ANF_STOPPED;
                }
            }
        }
    }

    playTime += client.GetRunTime();
}

//
// kexAnimState::ParseKAnim
//

void kexAnimState::ParseKAnim(const kexModel_t *model, kexAnim_t *anim, kexLexer *lexer) {
    unsigned int numnodes;
    unsigned int i;
    unsigned int j;

    numnodes = 0;

    if(model->numNodes <= 0) {
        parser.Error("numnodes is 0 or has not been set yet for %s",
            model->filePath);
    }

    anim->frameSet = (frameSet_t*)Mem_Calloc(sizeof(frameSet_t)
        * model->numNodes, kexRenderSystem::hb_model);

    lexer->ExpectTokenListID(animtokens, scanim_anim);
    lexer->ExpectNextToken(TK_LBRACK);

    while(lexer->CheckState()) {
        lexer->Find();

        switch(lexer->TokenType())
        {
        case TK_NONE:
            return;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            switch(lexer->GetIDForTokenList(animtokens, lexer->Token())) {
                // frame count
            case scanim_numframes:
                lexer->AssignFromTokenList(animtokens, &anim->numFrames,
                    scanim_numframes, false);
                break;
                // loop frame
            case scanim_loopframe:
                lexer->AssignFromTokenList(animtokens, &anim->loopFrame,
                    scanim_loopframe, false);
                break;
                // action count
            case scanim_numactions:
                lexer->AssignFromTokenList(animtokens, &anim->numActions,
                    scanim_numactions, false);
                break;
                // number of nodes (must match numnodes in model file)
            case scanim_numnodes:
                lexer->AssignFromTokenList(animtokens, &numnodes,
                    scanim_numnodes, false);

                if(numnodes != model->numNodes)
                {
                    parser.Error("numnodes(%i) for %s doesn't match numnodes in model file(%i)",
                        numnodes, anim->alias, model->numNodes);
                }
                break;
                // translation table count
            case scanim_numtranslationsets:
                lexer->AssignFromTokenList(animtokens, &anim->numTranslations,
                    scanim_numtranslationsets, false);
                break;
                // rotation table count
            case scanim_numrotationsets:
                lexer->AssignFromTokenList(animtokens, &anim->numRotations,
                    scanim_numrotationsets, false);
                break;
                // translation table
            case scanim_translationsets:
                lexer->ExpectNextToken(TK_EQUAL);
                if(anim->numTranslations <= 0) {
                    parser.Error("numtranslations is 0 or has not been set yet for %s",
                        anim->alias);
                }
                if(anim->numFrames <= 0) {
                    parser.Error("numframes is 0 or has not been set yet for %s",
                        anim->alias);
                }
                anim->translations = (kexVec3**)Mem_Calloc(sizeof(kexVec3*)
                    * anim->numTranslations, kexRenderSystem::hb_model);

                lexer->ExpectNextToken(TK_LBRACK);
                for(i = 0; i < anim->numTranslations; i++) {
                    anim->translations[i] = (kexVec3*)Mem_Calloc(
                        sizeof(kexVec3) * anim->numFrames, kexRenderSystem::hb_model);

                    lexer->ExpectNextToken(TK_LBRACK);
                    for(j = 0; j < anim->numFrames; j++) {
                        lexer->ExpectNextToken(TK_LBRACK);
                        anim->translations[i][j].x = (float)lexer->GetFloat();
                        anim->translations[i][j].y = (float)lexer->GetFloat();
                        anim->translations[i][j].z = (float)lexer->GetFloat();
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // rotation table
            case scanim_rotationsets:
                lexer->ExpectNextToken(TK_EQUAL);
                if(anim->numRotations <= 0) {
                    parser.Error("numrotations is 0 or has not been set yet for %s",
                        anim->alias);
                }
                if(anim->numFrames <= 0) {
                    parser.Error("numframes is 0 or has not been set yet for %s",
                        anim->alias);
                }
                anim->rotations = (kexQuat**)Mem_Calloc(sizeof(kexQuat*)
                    * anim->numRotations, kexRenderSystem::hb_model);

                lexer->ExpectNextToken(TK_LBRACK);
                for(i = 0; i < anim->numRotations; i++) {
                    anim->rotations[i] = (kexQuat*)Mem_Calloc(
                        sizeof(kexQuat) * anim->numFrames, kexRenderSystem::hb_model);

                    lexer->ExpectNextToken(TK_LBRACK);
                    for(j = 0; j < anim->numFrames; j++) {
                        lexer->ExpectNextToken(TK_LBRACK);
                        anim->rotations[i][j].x = (float)lexer->GetFloat();
                        anim->rotations[i][j].y = (float)lexer->GetFloat();
                        anim->rotations[i][j].z = (float)lexer->GetFloat();
                        anim->rotations[i][j].w = (float)lexer->GetFloat();
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    lexer->ExpectNextToken(TK_RBRACK);
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // lookup table for model nodes
            case scanim_nodeframes:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);
                for(i = 0; i < model->numNodes; i++) {
                    int num;

                    lexer->ExpectNextToken(TK_LBRACK);

                    num = lexer->GetNumber();
                    anim->frameSet[i].translations = num != -1 ?
                        anim->translations[num] : NULL;

                    num = lexer->GetNumber();
                    anim->frameSet[i].rotations = num != -1 ?
                        anim->rotations[num] : NULL;

                    lexer->ExpectNextToken(TK_RBRACK);
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // actions
            case scanim_actions:
                if(anim->numActions <= 0) {
                    parser.Error("numactions is 0 or has not been set yet for %s",
                        anim->alias);
                }
                anim->actions = (frameAction_t*)Mem_Calloc(sizeof(frameAction_t) *
                    anim->numActions, kexRenderSystem::hb_model);
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);
                for(i = 0; i < anim->numActions; i++) {
                    anim->actions[i].frame = lexer->GetNumber();
                    lexer->GetString();
                    anim->actions[i].function = Mem_Strdup(lexer->StringToken(),
                        kexRenderSystem::hb_model);

                    lexer->Find();

                    for(j = 0; j < 4; j++) {
                        switch(lexer->TokenType())
                        {
                        case TK_STRING:
                            anim->actions[i].argStrings[j] =
                            Mem_Strdup(lexer->Token(), kexRenderSystem::hb_model);
                            break;
                        case TK_NUMBER:
                            anim->actions[i].args[j] = (float)atof(lexer->Token());
                            break;
                        default:
                            common.Warning("Kanim_ParseAnimScript: Action Argument #%i ", j);
                            common.Warning("is not a number nor a string\n");
                            common.Warning("line=%i, pos=%i\n\n", lexer->LinePos(), lexer->RowPos());
                            break;
                        }

                        if(j >= 3)
                            break;

                        lexer->Find();
                    }
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // initial translation frame
            case scanim_initial_t:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);

                anim->initialFrame.translations = (kexVec3*)Mem_Calloc(sizeof(kexVec3)
                    * model->numNodes, kexRenderSystem::hb_model);

                for(i = 0; i < model->numNodes; i++) {
                    lexer->ExpectNextToken(TK_LBRACK);
                    anim->initialFrame.translations[i].x = (float)lexer->GetFloat();
                    anim->initialFrame.translations[i].y = (float)lexer->GetFloat();
                    anim->initialFrame.translations[i].z = (float)lexer->GetFloat();
                    lexer->ExpectNextToken(TK_RBRACK);
                }

                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // initial rotation frame
            case scanim_initial_r:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);

                anim->initialFrame.rotations = (kexQuat*)Mem_Calloc(sizeof(kexQuat)
                    * model->numNodes, kexRenderSystem::hb_model);

                for(i = 0; i < model->numNodes; i++) {
                    lexer->ExpectNextToken(TK_LBRACK);
                    anim->initialFrame.rotations[i].x = (float)lexer->GetFloat();
                    anim->initialFrame.rotations[i].y = (float)lexer->GetFloat();
                    anim->initialFrame.rotations[i].z = (float)lexer->GetFloat();
                    anim->initialFrame.rotations[i].w = (float)lexer->GetFloat();
                    lexer->ExpectNextToken(TK_RBRACK);
                }

                lexer->ExpectNextToken(TK_RBRACK);
                break;
            case scanim_turninfo:
                anim->yawOffsets = (float*)Mem_Calloc(sizeof(float) *
                    anim->numFrames, kexRenderSystem::hb_model);
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);
                for(i = 0; i < anim->numFrames; i++) {
                    anim->yawOffsets[i] = (float)lexer->GetFloat();
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
            default:
                if(lexer->TokenType() == TK_IDENIFIER) {
                    common.DPrintf("Kanim_ParseAnimScript: Unknown token: %s\n",
                        lexer->Token());
                }
                break;
            }
            break;
        default:
            break;
        }
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// kexAnimState::LoadKAnim
//

void kexAnimState::LoadKAnim(const kexModel_t *model) {
    unsigned int i;

    if(model->anims == NULL || model->numAnimations <= 0 || model->numNodes <= 0) {
        return;
    }

    for(i = 0; i < model->numAnimations; i++) {
        kexLexer *lexer;

        if(!(lexer = parser.Open(model->anims[i].animFile))) {
            continue;
        }

        ParseKAnim(model, &model->anims[i], lexer);
        parser.Close();
    }
}

//
// kexAnimState::InitObject
//

void kexAnimState::InitObject(void) {
    scriptManager.Engine()->RegisterObjectType(
        "kAnimState",
        sizeof(kexAnimState),
        asOBJ_REF | asOBJ_NOCOUNT);

#define OBJMETHOD(str, a, b, c)                     \
    scriptManager.Engine()->RegisterObjectMethod(   \
        "kAnimState",                               \
        str,                                        \
        asMETHODPR(kexAnimState, a, b, c),          \
        asCALL_THISCALL)

    OBJMETHOD("void Reset(void)", Reset, (void), void);
    OBJMETHOD("void Update(void)", Update, (void), void);
    OBJMETHOD("void Set(const kStr &in, float, int)", Set,
        (const kexStr &animName, float animTime, int animFlags), void);
    OBJMETHOD("void Set(const int, float, int)", Set,
        (const int id, float animTime, int animFlags), void);
    OBJMETHOD("void Blend(const kStr &in, float, float, int)", Blend,
        (const kexStr &animName, float animTime, float animBlendTime, int animFlags), void);
    OBJMETHOD("void Blend(const int, float, float, int)", Blend,
        (const int id, float animTime, float animBlendTime, int animFlags), void);
    OBJMETHOD("const int CurrentFrame(void)", CurrentFrame, (void)const, const int);
    OBJMETHOD("const float PlayTime(void)", PlayTime, (void)const, const float);

#define OBJPROPERTY(str, p)                         \
    scriptManager.Engine()->RegisterObjectProperty( \
        "kAnimState",                               \
        str,                                        \
        asOFFSET(kexAnimState, p))

    OBJPROPERTY("int flags", flags);

#undef OBJMETHOD
#undef OBJPROPERTY

    scriptManager.Engine()->RegisterObjectMethod(
        "kActor",
        "kAnimState @AnimState(void)",
        asMETHODPR(kexWorldActor, AnimState, (void), kexAnimState*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterEnum("AnimStateFlags");
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_BLEND", ANF_BLEND);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_LOOP", ANF_LOOP);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_STOPPED", ANF_STOPPED);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_NOINTERRUPT", ANF_NOINTERRUPT);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_ROOTMOTION", ANF_ROOTMOTION);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_PAUSED", ANF_PAUSED);
    scriptManager.Engine()->RegisterEnumValue("AnimStateFlags", "ANF_CROSSFADE", ANF_CROSSFADE);
}
