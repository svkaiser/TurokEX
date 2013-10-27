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

#include "al.h"
#include "alc.h"
#include "actor.h"

class kexWavFile;

typedef struct {
    kexWavFile      *wavFile;
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

#define SND_MAX_SOURCES 64

class kexSoundShader {
public:
                                        kexSoundShader(void);
                                        ~kexSoundShader(void);

    void                                Play(kexActor *actor);
    void                                Load(kexLexer *lexer);

    filepath_t                          filePath;
    kexSoundShader                      *next;

private:
    int                                 numsfx;
    sfx_t                               *sfxList;
};

class kexWavFile {
public:
                                        kexWavFile(void);
                                        ~kexWavFile(void);

    int                                 GetFormat(void);
    bool                                CompareTag(const char *tag, int offset);
    void                                Allocate(const char *name, byte *data);
    void                                Delete(void);
    ALuint                              *GetBuffer(void) { return &buffer; }

    filepath_t                          filePath;
    kexWavFile                          *next;

private:
    short                               formatCode;
    short                               channels;
    int                                 samples;
    int                                 bytes;
    short                               blockAlign;
    short                               bits;
    int                                 waveSize;
    byte                                *data;
    byte                                *waveFile;
    ALuint                              buffer;
};

class kexSoundSource {
    public:
                                        kexSoundSource(void);
                                        ~kexSoundSource(void);

    bool                                Generate(void);
    void                                Set(sfx_t *sfxRef, kexActor *actor);
    void                                Play(void);
    void                                Stop(void);
    void                                Reset(void);
    void                                Free(void);
    void                                Delete(void);
    void                                Update(void);

    bool                                InUse(void) const { return bInUse; }

private:
    ALuint                              handle;
    int                                 startTime;
    bool                                bInUse;
    bool                                bPlaying;
    bool                                bLooping;
    float                               volume;
    float                               pitch;
    sfx_t                               *sfx;
    kexActor                            *actor;
};

class kexSoundSystem {
public:
                                        kexSoundSystem(void);
                                        ~kexSoundSystem(void);

    void                                Init(void);
    void                                Shutdown(void);
    char                                *GetDeviceName(void);
    void                                UpdateListener(void);
    void                                StopAll(void);
    kexWavFile                          *CacheWavFile(const char *name);
    kexSoundShader                      *CacheShaderFile(const char *name);
    kexSoundSource                      *GetAvailableSource(void);
    void                                PlaySound(const char *name, kexActor *actor);

    const int                           GetNumActiveSources(void) const { return activeSources; }
    kexSoundSource                      *GetSources(void) { return sources; }

    static void                         EnterCriticalSection(void);
    static void                         ExitCriticalSection(void);
    static int SDLCALL                  Thread(void *param);

    static SDL_mutex                    *mutex;
    static SDL_Thread                   *thread;
    static unsigned long                time;
    static bool                         bKillSoundThread;

private:
    ALCdevice                           *alDevice;
    ALCcontext                          *alContext;
    kexSoundSource                      sources[SND_MAX_SOURCES];
    kexFileCacheList<kexWavFile>        wavList;
    kexFileCacheList<kexSoundShader>    shaderList;
    int                                 activeSources;
};

extern kexSoundSystem soundSystem;

#endif
