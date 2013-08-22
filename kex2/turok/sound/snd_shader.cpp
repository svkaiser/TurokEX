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
// DESCRIPTION: Sound shader system
//
//-----------------------------------------------------------------------------

#include "al.h"
#include "alc.h"
#include "common.h"
#include "zone.h"
#include "sound.h"
#include "script.h"
#include "client.h"

static sndShader_t *snd_hashlist[MAX_HASH];

enum
{
    scsfx_wavefile = 0,
    scsfx_delay,
    scsfx_dbFreq,
    scsfx_random,
    scsfx_gain,
    scsfx_interpgain,
    scsfx_interpfreq,
    scsfx_gainfactorstart,
    scsfx_gainfactorend,
    scsfx_gainfactortime,
    scsfx_gainfactordelay,
    scsfx_freqfactorstart,
    scsfx_freqfactorend,
    scsfx_freqfactortime,
    scsfx_freqfactordelay,
    scsfx_end
};

static const sctokens_t sfxtokens[scsfx_end+1] =
{
    { scsfx_wavefile,           "wavefile"          },
    { scsfx_delay,              "delay"             },
    { scsfx_dbFreq,             "dbFreq"            },
    { scsfx_random,             "random"            },
    { scsfx_gain,               "gain"              },
    { scsfx_interpgain,         "bInterpGain"       },
    { scsfx_interpfreq,         "bInterpFreq"       },
    { scsfx_gainfactorstart,    "gainFactorStart"   },
    { scsfx_gainfactorend,      "gainFactorEnd"     },
    { scsfx_gainfactortime,     "gainInterpTime"    },
    { scsfx_gainfactordelay,    "gainInterpDelay"   },
    { scsfx_freqfactorstart,    "freqFactorStart"   },
    { scsfx_freqfactorend,      "freqFactorEnd"     },
    { scsfx_freqfactortime,     "freqInterpTime"    },
    { scsfx_freqfactordelay,    "freqInterpDelay"   },
    { -1,                       NULL                }
};

//
// Snd_ParseShaderScript
//

static void Snd_ParseShaderScript(sndShader_t *snd, kexLexer *lexer)
{
    unsigned int i;

    lexer->Find();

    if(strcmp(lexer->Token(), "sounds"))
        common.Error("Snd_ParseShaderScript: Expected 'sound', found %s", lexer->Token());

    lexer->ExpectNextToken(TK_LSQBRACK);

    snd->numsfx = lexer->GetNumber();
    snd->sfx = (sfx_t*)Z_Calloc(sizeof(sfx_t) * snd->numsfx, PU_SOUND, 0);

    snd->sfx->rolloffFactor = 1.0f;

    lexer->ExpectNextToken(TK_RSQBRACK);
    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    for(i = 0; i < snd->numsfx; i++)
    {
        sfx_t *sfx = &snd->sfx[i];

        lexer->ExpectNextToken(TK_LBRACK);
        lexer->Find();

        while(lexer->TokenType() != TK_RBRACK)
        {
            switch(lexer->GetIDForTokenList(sfxtokens, lexer->Token()))
            {
            case scsfx_wavefile:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->GetString();
                sfx->wave = Snd_CacheWaveFile(lexer->StringToken());
                break;

            case scsfx_delay:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->delay,
                    scsfx_delay, false);
                break;

            case scsfx_dbFreq:
                lexer->AssignFromTokenList(sfxtokens, &sfx->dbFreq,
                    scsfx_dbFreq, false);
                break;
                
            case scsfx_gain:
                lexer->AssignFromTokenList(sfxtokens, &sfx->gain,
                    scsfx_gain, false);
                break;

            case scsfx_random:
                lexer->AssignFromTokenList(sfxtokens, &sfx->random,
                    scsfx_random, false);
                break;

            case scsfx_interpgain:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->bLerpVol,
                    scsfx_interpgain, false);
                break;

            case scsfx_interpfreq:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->bLerpFreq,
                    scsfx_interpfreq, false);
                break;

            case scsfx_gainfactorstart:
                lexer->AssignFromTokenList(sfxtokens, &sfx->gainLerpStart,
                    scsfx_gainfactorstart, false);
                break;

            case scsfx_gainfactorend:
                lexer->AssignFromTokenList(sfxtokens, &sfx->gainLerpEnd,
                    scsfx_gainfactorend, false);
                break;

            case scsfx_gainfactortime:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->gainLerpTime,
                    scsfx_gainfactortime, false);
                break;

            case scsfx_gainfactordelay:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->gainLerpDelay,
                    scsfx_gainfactordelay, false);
                break;

            case scsfx_freqfactorstart:
                lexer->AssignFromTokenList(sfxtokens, &sfx->freqLerpStart,
                    scsfx_freqfactorstart, false);
                break;

            case scsfx_freqfactorend:
                lexer->AssignFromTokenList(sfxtokens, &sfx->freqLerpEnd,
                    scsfx_freqfactorend, false);
                break;

            case scsfx_freqfactortime:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->freqLerpTime,
                    scsfx_freqfactortime, false);
                break;

            case scsfx_freqfactordelay:
                lexer->AssignFromTokenList(sfxtokens, (unsigned int*)&sfx->freqLerpDelay,
                    scsfx_freqfactordelay, false);
                break;

            default:
                if(lexer->TokenType() == TK_IDENIFIER)
                {
                    parser.Error("Snd_ParseShaderScript: Unknown token: %s\n",
                        lexer->Token());
                }
                break;
            }

            lexer->Find();
        }
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Snd_FindShader
//

sndShader_t *Snd_FindShader(const char *name)
{
    sndShader_t *snd;
    unsigned int hash;

    hash = common.HashFileName(name);

    for(snd = snd_hashlist[hash]; snd; snd = snd->next)
    {
        if(!strcmp(name, snd->name))
            return snd;
    }

    return NULL;
}

//
// Snd_LoadShader
//

sndShader_t *Snd_LoadShader(const char *name)
{
    sndShader_t *snd;

    if(name == NULL || name[0] == 0)
        return NULL;

    if(!(snd = Snd_FindShader(name)))
    {
        unsigned int hash;
        kexLexer *lexer;

        if(strlen(name) >= MAX_FILEPATH)
            common.Error("Snd_LoadShader: \"%s\" is too long", name);

        if(!(lexer = parser.Open(name)))
            return NULL;

        snd = (sndShader_t*)Z_Calloc(sizeof(sndShader_t), PU_STATIC, 0);
        strncpy(snd->name, name, MAX_FILEPATH);

        Snd_ParseShaderScript(snd, lexer);

        hash = common.HashFileName(name);
        snd->next = snd_hashlist[hash];
        snd_hashlist[hash] = snd;

        parser.Close();
    }

    return snd;
}

//
// Snd_PlayShaderDirect
//

void Snd_PlayShaderDirect(sndShader_t *shader, gActor_t *actor)
{
    sndSource_t *src;
    unsigned int i;

    if(shader == NULL)
        return;

    Snd_EnterCriticalSection();

    for(i = 0; i < shader->numsfx; i++)
    {
        src = Snd_GetAvailableSource();

        if(src == NULL)
        {
            Snd_ExitCriticalSection();
            return;
        }

        src->sfx = &shader->sfx[i];
        Actor_SetTarget(&src->actor, actor);

        if(src->sfx->bLerpVol)
            src->volume = src->sfx->gainLerpStart;

        if(src->sfx->bLerpFreq)
            src->pitch = src->sfx->freqLerpStart;

        if(actor != NULL && client.playerActor != actor)
        {
            alSource3f(src->handle, AL_POSITION,
                actor->origin[0],
                actor->origin[1],
                actor->origin[2]);
        }
    }

    Snd_ExitCriticalSection();
}

//
// Snd_PlayShader
//

void Snd_PlayShader(const char *name, gActor_t *actor)
{
    sndShader_t *shader;

    // TODO
    if(actor && Vec_Length3(client.playerActor->origin,
        actor->origin) >= 4096)
        return;
    
    shader = Snd_LoadShader(name);

    if(shader == NULL)
        return;

    Snd_PlayShaderDirect(shader, actor);
}
