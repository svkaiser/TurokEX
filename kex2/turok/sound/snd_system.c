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

#include "SDL.h"
#include "al.h"
#include "alc.h"
#include "common.h"
#include "zone.h"
#include "sound.h"

SDL_mutex   *snd_mutex  = NULL;
SDL_Thread  *snd_thread = NULL;

ALCdevice   *alDevice   = NULL;
ALCcontext  *alContext  = NULL;

sndSource_t sndSources[SND_MAX_SOURCES];
int nSndSources = 0;

static wave_t *wave_hashlist[MAX_HASH];

//
// Snd_Shutdown
//

void Snd_Shutdown(void)
{
    int i;

    SDL_KillThread(snd_thread);
    SDL_DestroyMutex(snd_mutex);

    for(i = 0; i < nSndSources; i++)
    {
        sndSource_t *sndSrc = &sndSources[i];

        alSourceStop(sndSrc->handle);
        alSourcei(sndSrc->handle, AL_BUFFER, 0);
        alDeleteSources(1, &sndSrc->handle);
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(alContext);
    alcCloseDevice(alDevice);

    Z_FreeTags(PU_SOUND, PU_SOUND);
}

//
// Snd_GetDeviceName
//

char *Snd_GetDeviceName(void)
{
    return (char*)alcGetString(alDevice, ALC_DEVICE_SPECIFIER);
}

//
// Snd_GetWaveFormat
//

int Snd_GetWaveFormat(wave_t *wave)
{
    switch(wave->channels)
    {
    case 1:
        switch(wave->bits)
        {
        case 8:
            return AL_FORMAT_MONO8;
        case 16:
            return AL_FORMAT_MONO16;
        }
        break;
    case 2:
        switch(wave->bits)
        {
        case 8:
            return AL_FORMAT_STEREO8;
        case 16:
            return AL_FORMAT_STEREO16;
        }
        break;
    default:
        Com_Error("Snd_GetWaveFormat: Unsupported number of channels - %i", wave->channels);
        return -1;
    }

    Com_Error("Snd_GetWaveFormat: Unknown bits format - %i", wave->bits);
    return -1;
}

//
// Snd_CompareWaveTag
//

kbool Snd_CompareWaveTag(byte *buf, const char *tag)
{
    return
        (buf[0] == tag[0] &&
         buf[1] == tag[1] &&
         buf[2] == tag[2] &&
         buf[3] == tag[3]);
}

//
// Snd_AllocWave
//

wave_t *Snd_AllocWave(const char *name, byte *data)
{
    wave_t *wave;
    unsigned int hash;

    if(strlen(name) >= MAX_FILEPATH)
        Com_Error("Snd_AllocWave: \"%s\" is too long", name);

    wave = Z_Calloc(sizeof(wave_t), PU_SOUND, 0);
    strcpy(wave->name, name);

    wave->waveFile = data;

    if(!Snd_CompareWaveTag(data, "RIFF"))
        Com_Error("Snd_AllocWave: RIFF header not found");
    if(!Snd_CompareWaveTag(data + 8, "WAVE"))
        Com_Error("Snd_AllocWave: WAVE header not found");
    if(!Snd_CompareWaveTag(data + 12, "fmt "))
        Com_Error("Snd_AllocWave: fmt header not found");
    if(!Snd_CompareWaveTag(data + 36, "data"))
        Com_Error("Snd_AllocWave: data header not found");

    if(*(data + 16) != 16)
        Com_Error("Snd_AllocWave: WAV chunk size must be 16");

    wave->formatCode    = *(short*)(data + 20);
    wave->channels      = *(short*)(data + 22);
    wave->samples       = *(int*)(data + 24);
    wave->bytes         = *(int*)(data + 28);
    wave->blockAlign    = *(short*)(data + 32);
    wave->bits          = *(short*)(data + 34);
    wave->waveSize      = *(int*)(data + 40);
    wave->data          = data + 44;

    hash = Com_HashFileName(name);
    wave->next = wave_hashlist[hash];
    wave_hashlist[hash] = wave;

    return wave;
}

//
// Snd_FindWave
//

wave_t *Snd_FindWave(const char *name)
{
    wave_t *wave;
    unsigned int hash;

    if(name[0] == 0)
        return NULL;

    hash = Com_HashFileName(name);

    for(wave = wave_hashlist[hash]; wave; wave = wave->next)
    {
        if(!strcmp(name, wave->name))
            return wave;
    }

    return NULL;
}

//
// Snd_CacheWaveFile
//

wave_t *Snd_CacheWaveFile(const char *name)
{
    wave_t *wave;

    if(name[0] == 0)
        return NULL;

    wave = Snd_FindWave(name);

    if(wave == NULL)
    {
        byte *data;

        if(KF_OpenFileCache(name, &data, PU_SOUND) == 0)
            return NULL;

        wave = Snd_AllocWave(name, data);
    }

    return wave;
}

//
// Thread_SoundHandler
//

static int SDLCALL Thread_SoundHandler(void *param)
{
    long start = SDL_GetTicks();
    long delay = 0;
    unsigned long count = 0;

    while(1)
    {
        count++;
        // try to avoid incremental time de-syncs
        delay = count - (SDL_GetTicks() - start);

        if(delay > 0)
            Sys_Sleep(delay);
    }

    return 0;
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
// FCmd_LoadTestSound
//

static void FCmd_LoadTestSound(void)
{
    wave_t *wave;
    ALuint buffer;
    ALint state;

    if(Cmd_GetArgc() < 2)
        return;

    wave = Snd_CacheWaveFile(Cmd_GetArgv(1));

    if(wave == NULL)
        return;

    Com_CPrintf(COLOR_GREEN, "bits: %i\n", wave->bits);
    Com_CPrintf(COLOR_GREEN, "block align: %i\n", wave->blockAlign);
    Com_CPrintf(COLOR_GREEN, "bytes: %i\n", wave->bytes);
    Com_CPrintf(COLOR_GREEN, "samples: %i\n", wave->samples);
    Com_CPrintf(COLOR_GREEN, "size: %i\n", wave->waveSize);
    Com_CPrintf(COLOR_GREEN, "channels: %i\n\n", wave->channels);

    alGetError();
    alGenBuffers(1, &buffer);

    if(alGetError() != AL_NO_ERROR)
        return;

    alBufferData(buffer, Snd_GetWaveFormat(wave), wave->data, wave->waveSize, wave->samples);
    alSourceQueueBuffers(sndSources[0].handle, 1, &buffer);
    alSourcePlay(sndSources[0].handle);

    if(alGetError() != AL_NO_ERROR)
    {
        alSourceUnqueueBuffers(sndSources[0].handle, 1, &buffer);
        alDeleteBuffers(1, &buffer);
        return;
    }

    do
    {
        alGetSourcei(sndSources[0].handle, AL_SOURCE_STATE, &state);
    } while(state == AL_PLAYING);

    alSourceUnqueueBuffers(sndSources[0].handle, 1, &buffer);
    alDeleteBuffers(1, &buffer);
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

    snd_mutex = SDL_CreateMutex();
    snd_thread = SDL_CreateThread(Thread_SoundHandler, NULL);

    Cmd_AddCommand("printsoundinfo", FCmd_SoundInfo);
    Cmd_AddCommand("playsound", FCmd_LoadTestSound);
}
