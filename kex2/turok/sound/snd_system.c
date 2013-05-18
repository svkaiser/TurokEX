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
#include "client.h"

CVAR(s_pitchshift, 1);
CVAR_CMD(s_sndvolume, 0.5)
{
    if(cvar->value <= 0)
        cvar->value = 1;
    if(cvar->value > 1)
        cvar->value = 1;
}

SDL_mutex   *snd_mutex  = NULL;
SDL_Thread  *snd_thread = NULL;

ALCdevice   *alDevice   = NULL;
ALCcontext  *alContext  = NULL;

sndSource_t sndSources[SND_MAX_SOURCES];
int nSndSources = 0;
unsigned long sndTime = 0;

static wave_t *wave_hashlist[MAX_HASH];

#define SND_METRICS 0.0075f
#define SND_VECTOR2METRICS(vec) \
    vec[0] * SND_METRICS,       \
    vec[1] * SND_METRICS,       \
    vec[2] * SND_METRICS

#define SND_INT2TIME(t) ((float)t * ((1.0f / 60.0f) * 1000.0f))

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

    for(i = 0; i < MAX_HASH; i++)
    {
        wave_t *wave = wave_hashlist[i];

        if(wave == NULL)
            continue;

        alDeleteBuffers(1, &wave->buffer);
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(alContext);
    alcCloseDevice(alDevice);

    Z_FreeTags(PU_SOUND, PU_SOUND);
}

//
// Snd_EnterCriticalSection
//

void Snd_EnterCriticalSection(void)
{
    SDL_LockMutex(snd_mutex);
}

//
// Snd_ExitCriticalSection
//

void Snd_ExitCriticalSection(void)
{
    SDL_UnlockMutex(snd_mutex);
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

    wave = (wave_t*)Z_Calloc(sizeof(wave_t), PU_SOUND, 0);
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

    Snd_EnterCriticalSection();

    alGetError();
    alGenBuffers(1, &wave->buffer);

    if(alGetError() != AL_NO_ERROR)
    {
        Snd_ExitCriticalSection();
        Com_Error("Snd_AllocWave: failed to create buffer for %s", name);
    }

    alBufferData(wave->buffer, Snd_GetWaveFormat(wave),
        wave->data, wave->waveSize, wave->samples);

    Snd_ExitCriticalSection();

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
// Snd_GetAvailableSource
//

sndSource_t *Snd_GetAvailableSource(void)
{
    int i;
    sndSource_t *src = NULL;

    for(i = 0; i < nSndSources; i++)
    {
        sndSource_t *sndSrc = &sndSources[i];

        if(sndSrc->inUse)
            continue;

        src = sndSrc;

        src->startTime  = sndTime;
        src->inUse      = true;
        src->playing    = false;
        src->volume     = 1.0f;
        src->pitch      = 1.0f;
        break;
    }

    return src;
}

//
// Snd_FreeSource
//

void Snd_FreeSource(sndSource_t *src)
{
    src->inUse      = false;
    src->playing    = false;
    src->sfx        = NULL;
    src->volume     = 1.0f;
    src->pitch      = 1.0f;
    src->startTime  = 0;

    Actor_SetTarget(&src->actor, NULL);
    alSource3f(src->handle, AL_POSITION, 0, 0, 0);
}

//
// Snd_UpdateListener
//

void Snd_UpdateListener(void)
{
    if(client.playerActor)
    {
        ALfloat orientation[6];
        vec3_t axis;
        float angle;

        Vec_QuaternionToAxis(&angle, axis, client.playerActor->rotation);
        
        orientation[0] = (float)sin(angle);
        orientation[1] = 0;
        orientation[2] = (float)cos(angle);
        orientation[3] = 0;
        orientation[4] = 1;
        orientation[5] = 0;

        Snd_EnterCriticalSection();

        alListenerfv(AL_ORIENTATION, orientation);
        alListener3f(AL_POSITION,
            SND_VECTOR2METRICS(client.playerActor->origin));

        Snd_ExitCriticalSection();
    }
}

//
// Snd_UpdateSources
//

static void Snd_UpdateSources(void)
{
    int i;

    Snd_EnterCriticalSection();

    for(i = 0; i < nSndSources; i++)
    {
        sndSource_t *sndSrc = &sndSources[i];

        if(!sndSrc->inUse)
            continue;

        if(sndSrc->sfx != NULL)
        {
            wave_t *wave = sndSrc->sfx->wave;

            if(!sndSrc->playing)
            {
                float time = (float)sndSrc->startTime +
                    SND_INT2TIME(sndSrc->sfx->delay);

                if(time > sndTime)
                    continue;

                alSourceQueueBuffers(sndSrc->handle, 1, &wave->buffer);

                if(s_pitchshift.value)
                    alSourcef(sndSrc->handle, AL_PITCH, sndSrc->sfx->dbFreq);

                if(sndSrc->actor && sndSrc->actor != client.playerActor)
                {
                    alSourcei(sndSrc->handle, AL_SOURCE_RELATIVE, AL_FALSE);
                    alSource3f(sndSrc->handle, AL_POSITION,
                        SND_VECTOR2METRICS(sndSrc->actor->origin));
                }
                else
                    alSourcei(sndSrc->handle, AL_SOURCE_RELATIVE, AL_TRUE);

                alSourcef(sndSrc->handle, AL_GAIN,
                    sndSrc->sfx->gain *
                    sndSrc->volume *
                    s_sndvolume.value);

                alSourcePlay(sndSrc->handle);

                sndSrc->playing = true;
                sndSrc->startTime = sndTime;
            }
            else
            {
                ALint state;

                alGetSourcei(sndSrc->handle, AL_SOURCE_STATE, &state);
                if(state != AL_PLAYING)
                {
                    alSourceStop(sndSrc->handle);
                    alSourceUnqueueBuffers(sndSrc->handle, 1, &wave->buffer);
                    Snd_FreeSource(sndSrc);
                }
                else
                {
                    if(sndSrc->sfx->bLerpVol && sndSrc->playing)
                    {
                        float time = (float)sndSrc->startTime +
                            SND_INT2TIME(sndSrc->sfx->gainLerpDelay);

                        if(time <= sndTime)
                        {
                            float volLerp = (1.0f / (float)sndSrc->sfx->gainLerpTime);
                            sndSrc->volume = (sndSrc->sfx->gainLerpEnd - sndSrc->volume) *
                                volLerp + sndSrc->volume;

                            if(sndSrc->volume > 1)
                                sndSrc->volume = 1;
                            if(sndSrc->volume < 0.01f)
                            {
                                alSourceStop(sndSrc->handle);
                                alSourceUnqueueBuffers(sndSrc->handle, 1, &wave->buffer);
                                Snd_FreeSource(sndSrc);
                                continue;
                            }

                            alSourcef(sndSrc->handle, AL_GAIN,
                                sndSrc->sfx->gain *
                                sndSrc->volume *
                                s_sndvolume.value);
                        }
                    }

                    if(sndSrc->sfx->bLerpFreq && sndSrc->playing && s_pitchshift.value)
                    {
                        float time = (float)sndSrc->startTime +
                            SND_INT2TIME(sndSrc->sfx->freqLerpDelay);

                        if(time <= sndTime)
                        {
                            float freqLerp = (1.0f / (float)sndSrc->sfx->freqLerpTime);
                            sndSrc->pitch = (sndSrc->sfx->freqLerpEnd - sndSrc->pitch) *
                                freqLerp + sndSrc->pitch;

                            alSourcef(sndSrc->handle, AL_PITCH, sndSrc->pitch);
                        }
                    }
                }
            }
        }
    }

    Snd_ExitCriticalSection();
}

//
// Thread_SoundHandler
//

static int SDLCALL Thread_SoundHandler(void *param)
{
    long start = SDL_GetTicks();
    long delay = 0;

    while(1)
    {
        Snd_UpdateSources();
        sndTime++;
        // try to avoid incremental time de-syncs
        delay = sndTime - (SDL_GetTicks() - start);

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
// FCmd_LoadShader
//

static void FCmd_LoadShader(void)
{
    if(Cmd_GetArgc() < 2)
        return;

    Snd_PlayShader(Cmd_GetArgv(1), NULL);
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

        sndSrc->handle      = handle;
        sndSrc->startTime   = 0;
        sndSrc->inUse       = false;
        sndSrc->looping     = false;
        sndSrc->playing     = false;
        sndSrc->sfx         = NULL;
        sndSrc->volume      = 1.0f;

        alSourcei(handle, AL_LOOPING, AL_FALSE);
        alSourcei(sndSrc->handle, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(handle, AL_GAIN, 1.0f);
        alSourcef(handle, AL_PITCH, 1.0f);
        nSndSources++;
    }

    alListener3f(AL_POSITION, 0, 0, 0);

    snd_mutex = SDL_CreateMutex();
    snd_thread = SDL_CreateThread(Thread_SoundHandler, NULL);

    Cmd_AddCommand("printsoundinfo", FCmd_SoundInfo);
    Cmd_AddCommand("playsound", FCmd_LoadTestSound);
    Cmd_AddCommand("playsoundshader", FCmd_LoadShader);

    Cvar_Register(&s_pitchshift);
    Cvar_Register(&s_sndvolume);
}
