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

#include "common.h"
#include "sound.h"
#include "script.h"
#include "client.h"

enum {
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

static const sctokens_t sfxtokens[scsfx_end+1] = {
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
// kexSoundShader::kexSoundShader
//

kexSoundShader::kexSoundShader(void) {
}

//
// kexSoundShader::~kexSoundShader
//

kexSoundShader::~kexSoundShader(void) {
}

//
// kexSoundShader::Load
//

void kexSoundShader::Load(kexLexer *lexer) {
    int i;

    lexer->Find();

    if(!lexer->Matches("sounds")) {
        common.Error("Snd_ParseShaderScript: Expected 'sound', found %s", lexer->Token());
    }

    lexer->ExpectNextToken(TK_LSQBRACK);

    numsfx = lexer->GetNumber();

    if(numsfx == 0) {
        sfxList = NULL;
        lexer->ExpectNextToken(TK_RSQBRACK);
        lexer->ExpectNextToken(TK_EQUAL);
        lexer->ExpectNextToken(TK_LBRACK);
        lexer->ExpectNextToken(TK_RBRACK);
        return;
    }

    sfxList = (sfx_t*)Mem_Calloc(sizeof(sfx_t) * numsfx, kexSoundSystem::hb_sound);

    lexer->ExpectNextToken(TK_RSQBRACK);
    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    for(i = 0; i < numsfx; i++) {
        sfx_t *sfx = &sfxList[i];
        sfx->rolloffFactor = 8.0f;

        lexer->ExpectNextToken(TK_LBRACK);
        lexer->Find();

        while(lexer->TokenType() != TK_RBRACK) {
            switch(lexer->GetIDForTokenList(sfxtokens, lexer->Token())) {
            case scsfx_wavefile:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->GetString();
                sfx->wavFile = soundSystem.CacheWavFile(lexer->StringToken());
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
                if(lexer->TokenType() == TK_IDENIFIER) {
                    parser.Error("kexSoundShader::Load: Unknown token: %s\n",
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
// kexSoundShader::Play
//

void kexSoundShader::Play(kexGameObject *obj) {
    kexSoundSource *src;

    for(int i = 0; i < numsfx; i++) {

        if(!(src = soundSystem.GetAvailableSource())) {
            return;
        }

        src->Set(&sfxList[i], obj);
    }
}
