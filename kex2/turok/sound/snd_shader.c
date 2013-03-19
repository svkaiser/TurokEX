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

static sndShader_t *snd_hashlist[MAX_HASH];

enum
{
    scsfx_wavefile = 0,
    scsfx_delay,
    scsfx_dbFreq,
    scsfx_random,
    scsfx_gain,
    scsfx_end
};

static const sctokens_t sfxtokens[scsfx_end+1] =
{
    { scsfx_wavefile,   "wavefile"  },
    { scsfx_delay,      "delay"     },
    { scsfx_dbFreq,     "dbFreq"    },
    { scsfx_random,     "random"    },
    { scsfx_gain,       "gain"      },
    { -1,               NULL        }
};

//
// Snd_ParseShaderScript
//

static void Snd_ParseShaderScript(sndShader_t *snd, scparser_t *parser)
{
    unsigned int i;

    SC_Find();

    if(strcmp(parser->token, "sounds"))
        Com_Error("Snd_ParseShaderScript: Expected 'sound', found %s", parser->token);

    SC_ExpectNextToken(TK_LSQBRACK);

    snd->numsfx = SC_GetNumber();
    snd->sfx = (sfx_t*)Z_Calloc(sizeof(sfx_t) * snd->numsfx, PU_SOUND, 0);

    SC_ExpectNextToken(TK_RSQBRACK);
    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < snd->numsfx; i++)
    {
        sfx_t *sfx = &snd->sfx[i];

        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(sfxtokens, parser->token))
            {
            case scsfx_wavefile:
                SC_ExpectNextToken(TK_EQUAL);
                SC_GetString();
                sfx->wave = Snd_CacheWaveFile(parser->stringToken);
                break;

            case scsfx_delay:
                SC_AssignInteger(sfxtokens, &sfx->delay,
                    scsfx_delay, parser, false);
                break;

            case scsfx_dbFreq:
                SC_AssignFloat(sfxtokens, &sfx->dbFreq,
                    scsfx_dbFreq, parser, false);
                break;
                
            case scsfx_gain:
                SC_AssignFloat(sfxtokens, &sfx->gain,
                    scsfx_gain, parser, false);
                break;

            case scsfx_random:
                SC_AssignFloat(sfxtokens, &sfx->random,
                    scsfx_random, parser, false);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Snd_ParseShaderScript: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            SC_Find();
        }
    }

    SC_ExpectNextToken(TK_RBRACK);
}

//
// Snd_FindShader
//

sndShader_t *Snd_FindShader(const char *name)
{
    sndShader_t *snd;
    unsigned int hash;

    hash = Com_HashFileName(name);

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
        scparser_t *parser;

        if(strlen(name) >= MAX_FILEPATH)
            Com_Error("Snd_LoadShader: \"%s\" is too long", name);

        if(!(parser = SC_Open(name)))
            return NULL;

        snd = (sndShader_t*)Z_Calloc(sizeof(sndShader_t), PU_SOUND, 0);
        strncpy(snd->name, name, MAX_FILEPATH);

        Snd_ParseShaderScript(snd, parser);

        hash = Com_HashFileName(name);
        snd->next = snd_hashlist[hash];
        snd_hashlist[hash] = snd;

        SC_Close();
    }

    return snd;
}

//
// Snd_PlayShader
//

void Snd_PlayShader(const char *name)
{
    sndShader_t *shader = Snd_LoadShader(name);
    sndSource_t *src;
    unsigned int i;

    if(shader == NULL)
        return;

    Snd_EnterCriticalSection();

    src = Snd_GetAvailableSource();

    if(src == NULL)
    {
        Snd_ExitCriticalSection();
        return;
    }

    src->sfx = &shader->sfx[0];
    src->next = NULL;

    for(i = 1; i < shader->numsfx; i++)
    {
        sndSource_t *nextSrc = Snd_GetAvailableSource();

        if(nextSrc == NULL)
            return;

        nextSrc->sfx = &shader->sfx[i];
        nextSrc->next = NULL;
    }

    Snd_ExitCriticalSection();
}
