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
// DESCRIPTION: Model System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kernel.h"
#include "zone.h"
#include "gl.h"
#include "client.h"
#include "script.h"
#include "render.h"
#include "mathlib.h"
#include "parse.h"

//
// Mdl_GetAnim
//

anim_t *Mdl_GetAnim(kmodel_t *model, const char *name)
{
    unsigned int i;

    if(model->anims == NULL || model->numanimations <= 0)
        return NULL;

    for(i = 0; i < model->numanimations; i++)
    {
        if(!strcmp(model->anims[i].alias, name))
            return &model->anims[i];
    }

    return NULL;
}

//
// Mdl_GetAnimFromID
//

anim_t *Mdl_GetAnimFromID(kmodel_t *model, int id)
{
    unsigned int i;

    if(model->anims == NULL || model->numanimations <= 0)
        return NULL;

    for(i = 0; i < model->numanimations; i++)
    {
        if(model->anims[i].animID == id)
            return &model->anims[i];
    }

    return NULL;
}

//
// Mdl_CheckAnimID
//

kbool Mdl_CheckAnimID(kmodel_t *model, int id)
{
    unsigned int i;

    if(model->anims == NULL || model->numanimations <= 0)
        return false;

    for(i = 0; i < model->numanimations; i++)
    {
        if(model->anims[i].animID == id)
            return true;
    }

    return false;
}

//
// Mdl_SetAnimState
//

void Mdl_SetAnimState(animstate_t *astate, anim_t *anim,
                      float time, int flags)
{
    astate->time                    = (float)client.GetTicks() + time;
    astate->deltatime               = 0;
    astate->blendtime               = 0;
    astate->playtime                = 0;
    astate->frametime               = time;
    astate->track.frame             = 0;
    astate->track.nextframe         = 1;
    astate->flags                   = flags;
    astate->prevflags               = 0;
    astate->prevtrack.frame         = 0;
    astate->prevtrack.nextframe     = 0;
    astate->track.anim              = anim;
    astate->prevtrack.anim          = NULL;
    astate->restartframe            = anim->loopframe;
    astate->currentFrame            = 0;
}


//
// Mdl_BlendAnimStates
//

void Mdl_BlendAnimStates(animstate_t *astate, anim_t *anim,
                         float time, float blendtime, int flags)
{
    kbool bSameAnim = (anim == astate->track.anim);

    if(astate->flags & ANF_NOINTERRUPT && !(astate->flags & ANF_STOPPED))
        return;

    if(bSameAnim && !(astate->flags & ANF_STOPPED) &&
        flags == astate->flags)
        return;

    if(bSameAnim)
    {
        astate->flags &= ~ANF_STOPPED;
        return;
    }

    astate->prevflags               = astate->flags;
    astate->flags                   = flags | ANF_BLEND;
    astate->prevtrack.frame         = astate->track.frame;
    astate->prevtrack.nextframe     = astate->track.nextframe;
    astate->track.frame             = bSameAnim ? anim->loopframe : 0;
    astate->track.nextframe         = bSameAnim ? (anim->loopframe+1) : 1;
    astate->time                    = (float)client.GetTicks() + blendtime;
    astate->playtime                = 0;
    astate->frametime               = time;
    astate->blendtime               = blendtime;
    astate->deltatime               = 0;
    astate->prevtrack.anim          = astate->track.anim;
    astate->track.anim              = anim;
    astate->restartframe            = anim->loopframe;
    astate->currentFrame            = 0;
}

//
// Mdl_NextAnimFrame
//

static void Mdl_NextAnimFrame(animstate_t *astate)
{
    if(astate->flags & ANF_BLEND)
    {
        astate->prevtrack.anim = NULL;
        astate->flags &= ~ANF_BLEND;
    }

    if(++astate->track.frame >=
        (int)astate->track.anim->numframes)
    {
        astate->track.frame = astate->restartframe;
    }

    if(++astate->track.nextframe >=
        (int)astate->track.anim->numframes)
    {
        astate->track.nextframe = astate->restartframe;
        astate->deltatime = 0;

        if(!(astate->flags & ANF_LOOP))
        {
            astate->playtime = 0;
            astate->flags |= ANF_STOPPED;
        }
    }
}

//
// Mdl_UpdateAnimState
//

void Mdl_UpdateAnimState(animstate_t *astate)
{
    float blend;

    if(astate->flags & ANF_LOOP && astate->flags & ANF_STOPPED)
        astate->flags &= ~ANF_STOPPED;

    if(astate->flags & (ANF_STOPPED|ANF_PAUSED))
        return;

    if(astate->track.anim == NULL)
        return;

    blend = (astate->flags & ANF_BLEND) ?
            astate->blendtime : astate->frametime;

    astate->deltatime += ((client.GetRunTime()*60)/blend);

    if(astate->deltatime > 1)
    {
        astate->time = (float)client.GetTicks() + astate->frametime;

        while(astate->deltatime > 1)
        {
            astate->deltatime = astate->deltatime - 1;
            Mdl_NextAnimFrame(astate);
        }
    }

    astate->playtime += client.GetRunTime();
}

//
// Mdl_GetAnimRotation
//

void Mdl_GetAnimRotation(vec4_t out, anim_t *anim, int nodenum, int frame)
{
    if(anim->frameset[nodenum].rotation == NULL)
    {
        Vec_Copy4(out, anim->initial.rotation[nodenum].vec);
        return;
    }
    
    Vec_Copy4(out, anim->frameset[nodenum].rotation[frame].vec);
}

//
// Mdl_GetAnimTranslation
//

void Mdl_GetAnimTranslation(vec3_t out, anim_t *anim, int nodenum, int frame)
{
    if(anim->frameset[nodenum].translation == NULL)
    {
        Vec_Copy3(out, anim->initial.translation[nodenum].vec);
        return;
    }
    
    Vec_Copy3(out, anim->frameset[nodenum].translation[frame].vec);
}

//
// FCmd_LoadTestModel
//

static void FCmd_LoadTestModel(void)
{
    kmodel_t *model;
    int time;

    if(command.GetArgc() < 2)
    {
        return;
    }

    time = Sys_GetMilliseconds();

    model = Kmesh_Load(command.GetArgv(1));
    if(model == NULL)
    {
        return;
    }

    common.DPrintf("\nloadtime: %f seconds\n\n",
        (float)(Sys_GetMilliseconds() - time) / 1000.0f);
}

//
// Mdl_Init
//

void Mdl_Init(void)
{
    command.Add("loadmodel", FCmd_LoadTestModel);
}
