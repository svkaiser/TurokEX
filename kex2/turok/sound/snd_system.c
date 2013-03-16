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
// DESCRIPTION: Sound system
//
//-----------------------------------------------------------------------------

#include "al.h"
#include "alc.h"
#include "common.h"
#include "sound.h"

ALCdevice *alDevice = NULL;
ALCcontext *alContext = NULL;

sndSource_t sndSources[SND_MAX_SOURCES];
int nSndSources = 0;

//
// Snd_Shutdown
//

void Snd_Shutdown(void)
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(alContext);
    alcCloseDevice(alDevice);
}

//
// Snd_GetDeviceName
//

char *Snd_GetDeviceName(void)
{
    return (char*)alcGetString(alDevice, ALC_DEVICE_SPECIFIER);
}

//
// FCmd_SoundInfo
//

static void FCmd_SoundInfo(void)
{
    Com_CPrintf(COLOR_CYAN, "------------- Sound Info -------------\n");
    Com_CPrintf(COLOR_GREEN, "Device: %s\n", Snd_GetDeviceName());
    Com_CPrintf(COLOR_GREEN, "Available Sources: %i\n", nSndSources);
}

//
// Snd_Init
//

void Snd_Init(void)
{
    int i;
    unsigned int handle;

    alDevice = alcOpenDevice(NULL);
    if(!alDevice)
        Com_Error("Snd_Init: Failed to create OpenAL device");

    alContext = alcCreateContext(alDevice, NULL);
    if(!alContext)
        Com_Error("Snd_Init: Failed to create OpenAL context");

    if(!alcMakeContextCurrent(alContext))
        Com_Error("Snd_Init: Failed to set current context");

    for(i = 0; i < SND_MAX_SOURCES; i++)
    {
        sndSource_t *sndSrc = &sndSources[nSndSources];

        alGetError();
        alGenSources(1, &handle);

        if(alGetError() != AL_NO_ERROR)
            break;

        sndSrc->handle = handle;
        sndSrc->startTime = 0;
        sndSrc->inUse = false;
        sndSrc->looping = false;

        alSourcei(handle, AL_LOOPING, false);
        alSourcef(handle, AL_GAIN, 1.0f);
        alSourcef(handle, AL_PITCH, 1.0f);
        nSndSources++;
    }

    Cmd_AddCommand("PrintSoundInfo", FCmd_SoundInfo);
}
