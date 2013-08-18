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

#ifndef _SND_H_
#define _SND_H_

typedef struct wave_s
{
    char            name[MAX_FILEPATH];
    short           formatCode;
    short           channels;
    int             samples;
    int             bytes;
    short           blockAlign;
    short           bits;
    int             waveSize;
    byte            *data;
    byte            *waveFile;
    unsigned int    buffer;
    struct wave_s   *next;
} wave_t;

typedef struct
{
    wave_t          *wave;
    int             delay;
    float           random;
    float           gain;
    float           dbFreq;
    kbool           bLerpVol;
    kbool           bLerpFreq;
    float           gainLerpStart;
    float           gainLerpEnd;
    int             gainLerpTime;
    int             gainLerpDelay;
    float           freqLerpStart;
    float           freqLerpEnd;
    int             freqLerpTime;
    int             freqLerpDelay;
    float           rolloffFactor;
} sfx_t;

typedef struct sndShader_s
{
    char                name[MAX_FILEPATH];
    unsigned int        numsfx;
    sfx_t               *sfx;
    struct sndShader_s  *next;
} sndShader_t;

typedef struct sndSource_s
{
    unsigned int        handle;
    int                 startTime;
    kbool               inUse;
    kbool               playing;
    kbool               looping;
    float               volume;
    float               pitch;
    sfx_t               *sfx;
    gActor_t            *actor;
} sndSource_t;

#define SND_MAX_SOURCES 64

extern sndSource_t sndSources[SND_MAX_SOURCES];
extern unsigned long sndTime;

sndShader_t *Snd_LoadShader(const char *name);
void Snd_PlayShader(const char *name, gActor_t *actor);
void Snd_PlayShaderDirect(sndShader_t *shader, gActor_t *actor);

void Snd_Shutdown(void);
void Snd_Init(void);
char *Snd_GetDeviceName(void);
void Snd_EnterCriticalSection(void);
void Snd_ExitCriticalSection(void);
wave_t *Snd_CacheWaveFile(const char *name);
sndSource_t *Snd_GetAvailableSource(void);
void Snd_FreeSource(sndSource_t *src);
void Snd_UpdateListener(void);
void Snd_StopAll(void);

#endif
