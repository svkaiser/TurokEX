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
// DESCRIPTION: Particle effects system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "mathlib.h"
#include "gl.h"
#include "fx.h"
#include "ai.h"
#include "actor.h"
#include "client.h"
#include "level.h"
#include "sound.h"

fx_t fxRoot;
fx_t *fxRover;

#define FX_RANDVAL() (float)((rand() % 20000) - 10000) * 0.0001f

static fxfile_t *fx_hashlist[MAX_HASH];

enum
{
    scvfx_bFadeout = 0,
    scvfx_bStopAnimOnImpact,
    scvfx_bOffsetFromFloor,
    scvfx_bTextureWrapMirror,
    scvfx_bDepthBuffer,
    scvfx_bLensFlares,
    scvfx_bBlood,
    scvfx_bAddOffset,
    scvfx_bScaleLerp,
    scvfx_bNoDirection,
    scvfx_bLocalAxis,
    scvfx_bProjectile,
    scvfx_mass,
    scvfx_translation_global_randomscale,
    scvfx_translation_randomscale,
    scvfx_translation,
    scvfx_gravity,
    scvfx_gravity_randomscale,
    scvfx_friction,
    scvfx_animFriction,
    scvfx_scale,
    scvfx_scale_randomscale,
    scvfx_scaledest,
    scvfx_scaledest_randomscale,
    scvfx_forward,
    scvfx_forward_randomscale,
    scvfx_offset_randomscale,
    scvfx_rotation_offset,
    scvfx_rotation_offset_randomscale,
    scvfx_rotation_speed,
    scvfx_rotation_speed_randomscale,
    scvfx_screen_offset_x,
    scvfx_screen_offset_y,
    scvfx_offset,
    scvfx_texture,
    scvfx_instances,
    scvfx_instances_randomscale,
    scvfx_lifetime,
    scvfx_lifetime_randomscale,
    scvfx_restart,
    scvfx_animspeed,
    scvfx_ontouch,
    scvfx_onplane,
    scvfx_drawtype,
    scvfx_color1,
    scvfx_color2,
    scvfx_color1_randomscale,
    scvfx_color2_randomscale,
    scvfx_saturation_randomscale,
    scvfx_fadein_time,
    scvfx_fadeout_time,
    scvfx_animtype,
    scvfx_onHitSurface,
    scvfx_onExpire,
    scvfx_onTick,
    scvfx_end
};

static const sctokens_t vfxtokens[scvfx_end+1] =
{
    { scvfx_bFadeout,                       "bFadeout"                          },
    { scvfx_bStopAnimOnImpact,              "bStopAnimOnImpact"                 },
    { scvfx_bOffsetFromFloor,               "bOffsetFromFloor"                  },
    { scvfx_bTextureWrapMirror,             "bTextureWrapMirror"                },
    { scvfx_bDepthBuffer,                   "bDepthBuffer"                      },
    { scvfx_bLensFlares,                    "bLensFlares"                       },
    { scvfx_bBlood,                         "bBlood"                            },
    { scvfx_bAddOffset,                     "bAddOffset"                        },
    { scvfx_bScaleLerp,                     "bScaleLerp"                        },
    { scvfx_mass,                           "mass"                              },
    { scvfx_translation_global_randomscale, "translation_global_randomscale"    },
    { scvfx_translation_randomscale,        "translation_randomscale"           },
    { scvfx_translation,                    "translation"                       },
    { scvfx_gravity,                        "gravity"                           },
    { scvfx_gravity_randomscale,            "gravity_randomscale"               },
    { scvfx_friction,                       "friction"                          },
    { scvfx_animFriction,                   "animFriction"                      },
    { scvfx_scale,                          "scale"                             },
    { scvfx_scale_randomscale,              "scale_randomscale"                 },
    { scvfx_scaledest,                      "scale_dest"                        },
    { scvfx_scaledest_randomscale,          "scale_dest_randomscale"            },
    { scvfx_forward,                        "forward_speed"                     },
    { scvfx_forward_randomscale,            "forward_speed_randomscale"         },
    { scvfx_offset_randomscale,             "offset_random"                     },
    { scvfx_rotation_offset,                "rotation_offset"                   },
    { scvfx_rotation_offset_randomscale,    "rotation_offset_randomscale"       },
    { scvfx_rotation_speed,                 "rotation_speed"                    },
    { scvfx_rotation_speed_randomscale,     "rotation_speed_randomscale"        },
    { scvfx_screen_offset_x,                "screen_offset_x"                   },
    { scvfx_screen_offset_y,                "screen_offset_y"                   },
    { scvfx_offset,                         "offset"                            },
    { scvfx_texture,                        "textures"                          },
    { scvfx_instances,                      "instances"                         },
    { scvfx_instances_randomscale,          "instances_randomscale"             },
    { scvfx_lifetime,                       "lifetime"                          },
    { scvfx_lifetime_randomscale,           "lifetime_randomscale"              },
    { scvfx_restart,                        "restart"                           },
    { scvfx_animspeed,                      "animspeed"                         },
    { scvfx_ontouch,                        "ontouch"                           },
    { scvfx_onplane,                        "onplane"                           },
    { scvfx_drawtype,                       "drawtype"                          },
    { scvfx_color1,                         "color1"                            },
    { scvfx_color2,                         "color2"                            },
    { scvfx_color1_randomscale,             "color1_randomscale"                },
    { scvfx_color2_randomscale,             "color2_randomscale"                },
    { scvfx_saturation_randomscale,         "saturation_randomscale"            },
    { scvfx_fadein_time,                    "fadein_time"                       },
    { scvfx_fadeout_time,                   "fadeout_time"                      },
    { scvfx_animtype,                       "animtype"                          },
    { scvfx_onHitSurface,                   "onHitSurface"                      },
    { scvfx_onExpire,                       "onExpire"                          },
    { scvfx_onTick,                         "onTick"                            },
    { scvfx_bNoDirection,                   "bNoDirection"                      },
    { scvfx_bLocalAxis,                     "bLocalAxis"                        },
    { scvfx_bProjectile,                    "bProjectile"                       },
    { -1,                                   NULL                                }
};

//
// Fx_ParseScript
//

static void Fx_ParseScript(fxfile_t *fx, scparser_t *parser)
{
    unsigned int i;

    SC_Find();

    if(strcmp(parser->token, "fx"))
        Com_Error("Fx_ParseScript: Expected 'fx', found %s", parser->token);

    SC_ExpectNextToken(TK_LSQBRACK);

    fx->numfx = SC_GetNumber();
    fx->info = (fxinfo_t*)Z_Calloc(sizeof(fxinfo_t) * fx->numfx, PU_STATIC, 0);

    SC_ExpectNextToken(TK_RSQBRACK);
    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

#define CHECK_INT(name)                         \
    case scvfx_ ##name:                         \
    SC_AssignInteger(vfxtokens, &info-> ##name, \
        scvfx_ ##name, parser, false);          \
    break

#define CHECK_FLOAT(name, prop)                 \
    case scvfx_ ##name:                         \
    SC_AssignFloat(vfxtokens, &info-> ##prop,   \
        scvfx_ ##name, parser, false);          \
    break

#define CHECK_VFXINT(name)                              \
    case scvfx_ ##name:                                 \
    SC_AssignInteger(vfxtokens, &info-> ##name .value,  \
        scvfx_ ##name, parser, false);                  \
    break;                                              \
    case scvfx_ ##name## _randomscale:                  \
    SC_AssignFloat(vfxtokens, &info-> ##name .rand,     \
        scvfx_ ##name## _randomscale, parser, false);   \
    break

#define CHECK_VFXFLOAT(name)                            \
    case scvfx_ ##name:                                 \
    SC_AssignFloat(vfxtokens, &info-> ##name .value,    \
        scvfx_ ##name, parser, false);                  \
    break;                                              \
    case scvfx_ ##name## _randomscale:                  \
    SC_AssignFloat(vfxtokens, &info-> ##name .rand,     \
        scvfx_ ##name## _randomscale, parser, false);   \
    break

#define CHECK_VFXVECTOR(name)                           \
    case scvfx_ ##name:                                 \
    SC_AssignVector(vfxtokens, info-> ##name .value,    \
        scvfx_ ##name, parser, false);                  \
    break;                                              \
    case scvfx_ ##name## _randomscale:                  \
    SC_AssignVector(vfxtokens, info-> ##name .rand,     \
        scvfx_ ##name## _randomscale, parser, false);   \
    break

    for(i = 0; i < fx->numfx; i++)
    {
        fxinfo_t *info = &fx->info[i];
        int j;

        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK)
        {
            switch(SC_GetIDForToken(vfxtokens, parser->token))
            {
            CHECK_INT(bFadeout);
            CHECK_INT(bStopAnimOnImpact);
            CHECK_INT(bOffsetFromFloor);
            CHECK_INT(bTextureWrapMirror);
            CHECK_INT(bDepthBuffer);
            CHECK_INT(bLensFlares);
            CHECK_INT(bBlood);
            CHECK_INT(bAddOffset);
            CHECK_INT(bScaleLerp);
            CHECK_INT(bNoDirection);
            CHECK_INT(bLocalAxis);
            CHECK_INT(bProjectile);
            CHECK_INT(animspeed);
            CHECK_INT(color1_randomscale);
            CHECK_INT(color2_randomscale);
            CHECK_INT(saturation_randomscale);
            CHECK_INT(fadein_time);
            CHECK_INT(fadeout_time);

            CHECK_FLOAT(mass, mass);
            CHECK_FLOAT(translation_global_randomscale, translation_randomscale);
            CHECK_FLOAT(screen_offset_x, screen_offset_x);
            CHECK_FLOAT(screen_offset_y, screen_offset_y);
            CHECK_FLOAT(restart, restart);
            CHECK_FLOAT(friction, friction);
            CHECK_FLOAT(animFriction, animFriction);

            CHECK_VFXINT(instances);
            CHECK_VFXINT(lifetime);

            CHECK_VFXFLOAT(gravity);
            CHECK_VFXFLOAT(scale);
            CHECK_VFXFLOAT(scaledest);
            CHECK_VFXFLOAT(forward);
            CHECK_VFXFLOAT(rotation_offset);
            CHECK_VFXFLOAT(rotation_speed);

            CHECK_VFXVECTOR(translation);
            CHECK_VFXVECTOR(offset);

            case scvfx_texture:
                SC_ExpectNextToken(TK_LSQBRACK);
                info->numTextures = SC_GetNumber();
                info->textures = (char**)Z_Calloc(sizeof(char*) *
                    info->numTextures, PU_STATIC, 0);
                SC_ExpectNextToken(TK_RSQBRACK);
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);
                for(j = 0; j < info->numTextures; j++)
                {
                    SC_GetString();
                    info->textures[j] = Z_Strndup(parser->stringToken,
                        MAX_FILEPATH, PU_STATIC, 0);
                }
                SC_ExpectNextToken(TK_RBRACK);
                break;

            case scvfx_color1:
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LSQBRACK);
                info->color1[0] = SC_GetNumber();
                info->color1[1] = SC_GetNumber();
                info->color1[2] = SC_GetNumber();
                SC_ExpectNextToken(TK_RSQBRACK);
                break;

            case scvfx_color2:
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LSQBRACK);
                info->color2[0] = SC_GetNumber();
                info->color2[1] = SC_GetNumber();
                info->color2[2] = SC_GetNumber();
                SC_ExpectNextToken(TK_RSQBRACK);
                break;

            case scvfx_ontouch:
                SC_ExpectNextToken(TK_EQUAL);
                SC_Find();
                if(!strcmp(sc_parser->token, "destroy"))
                    info->ontouch = VFX_DESTROY;
                else if(!strcmp(sc_parser->token, "reflect"))
                    info->ontouch = VFX_REFLECT;
                else
                    info->ontouch = VFX_DEFAULT;
                break;

            case scvfx_onplane:
                SC_ExpectNextToken(TK_EQUAL);
                SC_Find();
                if(!strcmp(sc_parser->token, "destroy"))
                    info->onplane = VFX_DESTROY;
                else if(!strcmp(sc_parser->token, "reflect"))
                    info->onplane = VFX_REFLECT;
                else if(!strcmp(sc_parser->token, "bounce"))
                    info->onplane = VFX_BOUNCE;
                else
                    info->onplane = VFX_DEFAULT;
                break;

            case scvfx_drawtype:
                SC_ExpectNextToken(TK_EQUAL);
                SC_Find();
                if(!strcmp(sc_parser->token, "flat"))
                    info->drawtype = VFX_DRAWFLAT;
                else if(!strcmp(sc_parser->token, "decal"))
                    info->drawtype = VFX_DRAWDECAL;
                else if(!strcmp(sc_parser->token, "billboard"))
                    info->drawtype = VFX_DRAWSPRITE;
                else
                    info->drawtype = VFX_DRAWDEFAULT;
                break;

            case scvfx_animtype:
                SC_ExpectNextToken(TK_EQUAL);
                SC_Find();
                if(!strcmp(sc_parser->token, "onetime"))
                    info->animtype = VFX_ANIMONETIME;
                else if(!strcmp(sc_parser->token, "loop"))
                    info->animtype = VFX_ANIMLOOP;
                else if(!strcmp(sc_parser->token, "sinwave"))
                    info->animtype = VFX_ANIMSINWAVE;
                else
                    info->animtype = VFX_ANIMDEFAULT;
                break;

            case scvfx_onHitSurface:
                SC_ExpectNextToken(TK_LBRACK);
                while(1)
                {
                    SC_Find();
                    if(!strcmp(sc_parser->token, "fx"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->hitFX = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "sound"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->hitSnd = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "action"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_ExpectNextToken(TK_LBRACK);

                        SC_GetString();
                        info->hitAction.function = Z_Strdup(parser->stringToken,
                            PU_STATIC, 0);
                        info->hitAction.args[0] = (float)SC_GetFloat();

                        SC_ExpectNextToken(TK_RBRACK);
                    }
                    else if(sc_parser->tokentype == TK_RBRACK)
                        break;
                    else
                    {
                        SC_Error("Fx_ParseScript: Unknown token: %s\n",
                            parser->token);
                    }
                }
                break;

            case scvfx_onExpire:
                SC_ExpectNextToken(TK_LBRACK);
                while(1)
                {
                    SC_Find();
                    if(!strcmp(sc_parser->token, "fx"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->expireFX = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "sound"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->expireSnd = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "action"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_ExpectNextToken(TK_LBRACK);

                        SC_GetString();
                        info->expireAction.function = Z_Strdup(parser->stringToken,
                            PU_STATIC, 0);
                        info->expireAction.args[0] = (float)SC_GetFloat();

                        SC_ExpectNextToken(TK_RBRACK);
                    }
                    else if(sc_parser->tokentype == TK_RBRACK)
                        break;
                    else
                    {
                        SC_Error("Fx_ParseScript: Unknown token: %s\n",
                            parser->token);
                    }
                }
                break;

            case scvfx_onTick:
                SC_ExpectNextToken(TK_LBRACK);
                while(1)
                {
                    SC_Find();
                    if(!strcmp(sc_parser->token, "fx"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->tickFX = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "sound"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_GetString();
                        info->tickSnd = Z_Strndup(parser->stringToken,
                            MAX_FILEPATH, PU_STATIC, 0);
                    }
                    else if(!strcmp(sc_parser->token, "action"))
                    {
                        SC_ExpectNextToken(TK_EQUAL);
                        SC_ExpectNextToken(TK_LBRACK);

                        SC_GetString();
                        info->tickAction.function = Z_Strdup(parser->stringToken,
                            PU_STATIC, 0);
                        info->tickAction.args[0] = (float)SC_GetFloat();

                        SC_ExpectNextToken(TK_RBRACK);
                    }
                    else if(sc_parser->tokentype == TK_RBRACK)
                        break;
                    else
                    {
                        SC_Error("Fx_ParseScript: Unknown token: %s\n",
                            parser->token);
                    }
                }
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER)
                {
                    SC_Error("Fx_ParseScript: Unknown token: %s\n",
                        parser->token);
                }
                break;
            }

            if(info->translation.value[0] > 1.0f)  info->translation.value[0] -= 1.0f;
            if(info->translation.value[0] < -1.0f) info->translation.value[0] += 1.0f;
            if(info->translation.value[1] > 1.0f)  info->translation.value[1] -= 1.0f;
            if(info->translation.value[1] < -1.0f) info->translation.value[1] += 1.0f;
            if(info->translation.value[2] > 1.0f)  info->translation.value[2] -= 1.0f;
            if(info->translation.value[2] < -1.0f) info->translation.value[2] += 1.0f;

            SC_Find();
        }
    }

    SC_ExpectNextToken(TK_RBRACK);

#undef CHECK_INT
#undef CHECK_FLOAT
#undef CHECK_VFXINT
#undef CHECK_VFXFLOAT
#undef CHECK_VFXVECTOR
}

//
// FX_Shutdown
//

void FX_Shutdown(void)
{
    Z_FreeTags(PU_FX, PU_FX);
}

//
// FX_Find
//

fxfile_t *FX_Find(const char *name)
{
    fxfile_t *fx;
    unsigned int hash;

    hash = Com_HashFileName(name);

    for(fx = fx_hashlist[hash]; fx; fx = fx->next)
    {
        if(!strcmp(name, fx->name))
            return fx;
    }

    return NULL;
}

//
// FX_Rand
//

float FX_Rand(float val)
{
    return Random_Float() * val;
}

//
// FX_SetTranslationX
//

static void FX_SetTranslationX(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = FX_RANDVAL();
    vec[1] = vector[1];
    vec[2] = vector[2];

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SetTranslationY
//

static void FX_SetTranslationY(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = vector[0];
    vec[1] = FX_RANDVAL();
    vec[2] = vector[2];

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SetTranslationZ
//

static void FX_SetTranslationZ(vec3_t out, vec3_t vector, float r)
{
    vec3_t vec;

    vec[0] = vector[0];
    vec[1] = vector[1];
    vec[2] = FX_RANDVAL();

    Vec_Normalize3(vec);
    Vec_Normalize3(vector);
    Vec_Lerp3(out, r, vector, vec);
}

//
// FX_SpawnChild
//

static fx_t *FX_SpawnChild(fx_t *parent, const char *file)
{
    vec4_t rot;
    vec3_t origin;
    fxinfo_t *info;

    Vec_ToQuaternion(rot, parent->translation);
    Vec_Copy3(origin, parent->origin);

    info = parent->info;

    if(info->screen_offset_x != 0 || info->screen_offset_y != 0)
    {
        mtx_t mtx1;
        mtx_t mtx3;
        mtx_t mtx4;
        vec3_t svec;
        vec3_t nvec;
        vec3_t org;

        Vec_Set3(svec, -info->screen_offset_x, -info->screen_offset_y, 0);
        Mtx_IdentityZ(mtx1, parent->rotation_offset + DEG2RAD(180));
        Vec_TransformToWorld(mtx1, svec, nvec);

        switch(info->drawtype)
        {
        case VFX_DRAWFLAT:
        case VFX_DRAWDECAL:
            Mtx_IdentityY(mtx3, DEG2RAD(90));
            break;
        default:
            Mtx_ApplyRotation(client.playerActor->rotation, mtx3);
            break;
        }

        Mtx_MultiplyRotation(mtx4, mtx1, mtx3);
        Vec_TransformToWorld(mtx4, nvec, org);
        Vec_Add(origin, origin, org);
    }

    return FX_Spawn(file, parent->source, parent->translation,
        origin, rot, parent->plane);
}

//
// FX_DestroyEvent
//

static void FX_DestroyEvent(fx_t *fx, gActor_t *hitActor)
{
    fx_t *nfx = NULL;

    fx->lifetime = 0;

    // TODO
    if(fx->info->hitFX != NULL)
        nfx = FX_SpawnChild(fx, fx->info->hitFX);

    if(fx->info->hitSnd != NULL)
        Snd_PlayShader(fx->info->hitSnd, (gActor_t*)nfx);

    if(fx->info->hitAction.function != NULL && fx->source)
    {
        Actor_FXEvent(fx->source, hitActor, fx->origin,
            fx->translation, Map_PlaneToIndex(fx->plane),
            &fx->info->hitAction);
    }
}

//
// FX_Sort
//

static void FX_Sort()
{
    fx_t *rfx1;
    fx_t *rfx2;

    for(rfx1 = fxRoot.next; rfx1 != &fxRoot; rfx1 = rfx1->next)
    {
        for(rfx2 = fxRoot.next; rfx2 != &fxRoot; rfx2 = rfx2->next)
        {
            if(rfx1 == rfx2)
                continue;

            if(rfx1->dist > rfx2->dist)
            {
                fx_t *prev = rfx1->prev;
                fx_t *next = rfx1->next;

                prev->next = next;
                next->prev = prev;

                prev = rfx2->prev;
                prev->next = rfx1;
                rfx1->prev = prev;
                rfx1->next = rfx2;
                rfx2->prev = rfx1;
                break;
            }
        }
    }
}

//
// FX_Link
//

fx_t *FX_Spawn(const char *name, gActor_t *source, vec3_t origin,
                vec3_t dest, vec4_t rotation, plane_t *plane)
{
    unsigned int i;
    int j;
    fx_t *fx;
    fxfile_t *fxfile;
    fxinfo_t *info;
    vec3_t translation;

    if(!(fxfile = FX_Load(name)))
        return NULL;

    for(i = 0; i < fxfile->numfx; i++)
    {
        info = &fxfile->info[i];

        fx = (fx_t*)Z_Calloc(sizeof(fx_t), PU_FX, 0);

        fxRoot.prev->next = fx;
        fx->next = &fxRoot;
        fx->prev = fxRoot.prev;
        fxRoot.prev = fx;

        if(info->bNoDirection)
            Mtx_Identity(fx->matrix);
        else
        {
            if(info->bProjectile && source && source->ai)
            {
                if(source->ai->target)
                {
                    vec3_t torg;

                    Vec_Copy3(torg, source->ai->target->origin);
                    torg[1] += 30.72f;

                    Vec_PointAt(dest, torg, source->rotation, 0, rotation);
                }
            }
            
            Mtx_ApplyRotation(rotation, fx->matrix);
        }

        fx->file = fxfile;
        fx->info = info;
        fx->plane = plane;
        fx->frame = 0;
        fx->source = source;
        fx->bAnimate = info->numTextures > 1 ? true : false;
        fx->frametime = client.time + info->animspeed;
        fx->textures = (texture_t**)Z_Calloc(sizeof(texture_t*) *
            info->numTextures, PU_FX, 0);

        for(j = 0; j < info->numTextures; j++)
            fx->textures[j] = Tex_CacheTextureFile(info->textures[j], DGL_CLAMP, true);

        fx->instances = info->instances.value;
        if(info->instances.rand > 0)
            fx->instances += (rand() % ((int)info->instances.rand + 1));

        fx->lifetime = (float)info->lifetime.value;
        if(info->lifetime.rand > 0)
            fx->lifetime += (rand() % ((int)info->lifetime.rand + 1));

        fx->scale = info->scale.value;
        if(info->scale.rand != 0)
            fx->scale += FX_Rand(info->scale.rand);

        fx->scale_dest = info->scaledest.value;
        if(info->scaledest.rand != 0)
            fx->scale_dest += FX_Rand(info->scaledest.rand);

        fx->forward = info->forward.value;
        if(info->forward.rand != 0)
            fx->forward += FX_Rand(info->forward.rand);

        fx->rotation_offset = info->rotation_offset.value;
        if(info->rotation_offset.rand != 0)
            fx->rotation_offset += FX_Rand(info->rotation_offset.rand);

        fx->rotation_speed = info->rotation_speed.value;
        if(info->rotation_speed.rand != 0)
            fx->rotation_speed += FX_Rand(info->rotation_speed.rand);

        fx->gravity = info->gravity.value;
        if(info->gravity.rand != 0)
            fx->gravity += FX_Rand(info->gravity.rand);

        //
        // process translation
        //
        Vec_Copy3(translation, info->translation.value);
        // TODO - FIXME
        translation[0] = -translation[0];

        FX_SetTranslationX(translation, translation, info->translation.rand[0]);
        FX_SetTranslationY(translation, translation, info->translation.rand[1]);
        FX_SetTranslationZ(translation, translation, info->translation.rand[2]);

        if(Vec_Unit3(translation) != 0)
        {
            vec3_t worldvec;
            vec3_t tvec;

            Vec_TransformToWorld(fx->matrix, translation, worldvec);

            tvec[0] = FX_RANDVAL();
            tvec[1] = FX_RANDVAL();
            tvec[2] = FX_RANDVAL();

            Vec_Normalize3(tvec);
            Vec_Normalize3(worldvec);
            Vec_Lerp3(translation,
                info->translation_randomscale, worldvec, tvec);
        }

        Vec_Scale(fx->translation, translation, fx->forward);

        if(info->bAddOffset)
            Vec_Add(fx->translation, fx->translation, origin);

        //
        // process offsets
        //
        Vec_Copy3(fx->offset, info->offset.value);
        if(info->offset.rand[0] != 0) fx->offset[0] += FX_Rand(info->offset.rand[0]);
        if(info->offset.rand[1] != 0) fx->offset[1] += FX_Rand(info->offset.rand[1]);
        if(info->offset.rand[2] != 0) fx->offset[2] += FX_Rand(info->offset.rand[2]);

        Vec_Copy3(fx->origin, dest);

        if(Vec_Unit3(fx->offset) != 0)
        {
            vec3_t worldvec;

            // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
            Vec_TransformToWorld(fx->matrix, fx->offset, worldvec);
            Vec_Add(fx->origin, fx->origin, worldvec);
        }

        // TODO
        fx->color1[0] = info->color1[0];
        fx->color1[1] = info->color1[1];
        fx->color1[2] = info->color1[2];
        fx->color1[3] = info->fadein_time == 0 ? 0xff : 0;
        fx->color2[0] = info->color2[0];
        fx->color2[1] = info->color2[1];
        fx->color2[2] = info->color2[2];
        fx->color2[3] = info->fadein_time == 0 ? 0xff : 0;

        fx->dist = Vec_Length3(fx->origin, client.player->actor->origin);
    }

    return fx;
}

//
// FX_Move
// Moves a particle towards its destination.
// Handle any clipping if needed
//

static void FX_Move(fx_t *fx, vec3_t dest)
{
    trace_t trace;
    float dist;
    fxinfo_t *fxinfo;

    Vec_Add(dest, dest, fx->origin);

    if(fx->plane == NULL)
    {
        Vec_Copy3(fx->origin, dest);
        return;
    }
    
    fxinfo = fx->info;

    trace = Trace(fx->origin, dest, fx->plane, NULL, fx->source, true);
    fx->plane = trace.pl;

    switch(trace.type)
    {
    case TRT_WALL:
    case TRT_EDGE:
        switch(fx->info->onplane)
        {
        case VFX_BOUNCE:
            G_ClipVelocity(fx->translation, fx->translation,
                trace.normal, (1 + fxinfo->mass));
            break;
        case VFX_DESTROY:
            Vec_Copy3(fx->origin, trace.hitvec);
            FX_DestroyEvent(fx, NULL);
            break;
        default:
            break;
        }
        break;
    case TRT_OBJECT:
        switch(fx->info->ontouch)
        {
        case VFX_BOUNCE:
            G_ClipVelocity(fx->translation, fx->translation,
                trace.normal, (1 + fxinfo->mass));
            break;
        case VFX_DESTROY:
            FX_DestroyEvent(fx, trace.hitActor);
            break;
        default:
            Vec_Copy3(fx->origin, dest);
            break;
        }
        break;
    case TRT_NOHIT:
    case TRT_SLOPE:
        Vec_Copy3(fx->origin, dest);
        break;
    }

    dist = fx->origin[1] -
        (Plane_GetDistance(fx->plane, fx->origin) +
        (fxinfo->bOffsetFromFloor ? 3.42f : 0));

    if(dist <= ONPLANE_EPSILON)
    {
        if(fxinfo->bStopAnimOnImpact)
            fx->bAnimate = false;

        switch(fxinfo->onplane)
        {
        case VFX_BOUNCE:
            G_ApplyBounceVelocity(fx->translation,
                fx->plane->normal, fxinfo->mass);

            // apply friction when sliding on the floor
            G_ApplyFriction(fx->translation, 1 - fxinfo->friction, false);
            break;
        case VFX_DESTROY:
            fx->origin[1] = fx->origin[1] - dist + 0.01f;
            FX_DestroyEvent(fx, NULL);
            break;
        default:
            break;
        }
    }

    fx->translation[1] += (fx->gravity * client.runtime);

    if(dist < 0.01f)
        fx->origin[1] = fx->origin[1] - dist + 0.01f;

    // clip against ceiling as well
    if(fx->plane->flags & CLF_CHECKHEIGHT)
    {
        dist = Plane_GetHeight(fx->plane, fx->origin);

        if((dist - fx->origin[1]) < 1.024f)
        {
            switch(fxinfo->onplane)
            {
            case VFX_BOUNCE:
                G_ApplyBounceVelocity(fx->translation,
                    fx->plane->ceilingNormal, fxinfo->mass);
                break;
            case VFX_DESTROY:
                fx->origin[1] = dist - 1.024f;
                FX_DestroyEvent(fx, NULL);
                break;
            default:
                break;
            }
        }
    }
}

//
// FX_Ticker
//

void FX_Ticker(void)
{
    float time;

    if(client.runtime <= 0)
        return;

    time = 15 * client.runtime;

    FX_Sort();

    for(fxRover = fxRoot.next; fxRover != &fxRoot; fxRover = fxRover->next)
    {
        fx_t *fx;
        fxinfo_t *fxinfo;
        int lifetime;
        int alpha;
        vec3_t dest;

        if(fxRover == NULL)
            continue;

        fx = fxRover;
        fxinfo = fx->info;

        if(fx->bStale)
        {
            FX_Kill(fx);
            continue;
        }

        if(fxinfo->tickFX != NULL)
            FX_SpawnChild(fx, fxinfo->tickFX);

        if(fxinfo->tickSnd != NULL && fx->refcount <= 0)
            Snd_PlayShader(fxinfo->tickSnd, (gActor_t*)fx);

        if(fx->info->tickAction.function != NULL && fx->source)
        {
            Actor_FXEvent(fx->source, NULL, fx->origin,
                fx->translation, Map_PlaneToIndex(fx->plane),
                &fx->info->tickAction);
        }

        if(fx->bAnimate)
        {
            if(fx->frametime < client.time && fxinfo->numTextures > 1)
            {
                if(fx->frame + 1 == fxinfo->numTextures)
                {
                    switch(fxinfo->animtype)
                    {
                    case VFX_ANIMONETIME:
                        fx->bAnimate = false;
                        break;

                    case VFX_ANIMLOOP:
                        fx->frame = 0;
                        fx->frametime = client.time + fxinfo->animspeed;
                        break;

                    default:
                        fx->bStale = true;
                        continue;
                    }
                }
                else
                {
                    fx->frametime = client.time + fxinfo->animspeed;
                    fx->frame++;
                }
            }
        }

        fx->rotation_offset += (fx->rotation_speed * time);

        if(!fxinfo->bScaleLerp)
        {
            float sdest = (fx->scale * fx->scale_dest);

            if(sdest > 4096)
                sdest = 4096;

            fx->scale = (sdest - fx->scale) * time + fx->scale;
        }
        else
        {
            float sdest = fx->scale - fx->scale_dest * (time * -10.24f);

            if(sdest > 4096)
                fx->scale = 4096;
            else
                fx->scale = sdest;
        }

        lifetime = (fxinfo->lifetime.value - (int)fx->lifetime);

        alpha = fx->color1[3];

        if(lifetime < fxinfo->fadein_time)
        {
            alpha += (255 / (fxinfo->fadein_time + 1)) >> 1;

            if(alpha > 0xff)
                alpha = 0xff;
        }

        if(fx->lifetime < fxinfo->fadeout_time)
        {
            alpha = (int)(255 * fx->lifetime / (fxinfo->fadeout_time + 1));

            if(alpha < 0)
                alpha = 0;
        }

        fx->color1[3] = alpha;
        fx->color2[3] = alpha;

        Vec_Scale(dest, fx->translation, client.runtime);

        if(Vec_Unit3(dest) >= 0.001f)
            FX_Move(fx, dest);

        if(fx->source && fx->bAttachToSource)
        {
            vec3_t worldvec;

            Vec_Copy3(fx->origin, fx->source->origin);

            // TODO - HANDLE CLIPPING IF OUTSIDE OF WORLD
            Vec_ApplyQuaternion(worldvec, fx->offset, fx->source->rotation);
            Vec_Add(fx->origin, fx->origin, worldvec);
        }

        fx->dist = Vec_Length3(fx->origin, client.player->actor->origin);
        fx->lifetime -= time;

        if(fx->lifetime < 0 || (fx->source && fx->source->bStale))
        {
            fx_t *efx = NULL;

            if(fxinfo->expireFX != NULL)
                efx = FX_SpawnChild(fx, fxinfo->expireFX); 

            if(fxinfo->expireSnd != NULL)
                Snd_PlayShader(fxinfo->expireSnd, (gActor_t*)efx);

            if(fx->info->expireAction.function != NULL && fx->source)
            {
                Actor_FXEvent(fx->source, NULL, fx->origin,
                    fx->translation, Map_PlaneToIndex(fx->plane),
                    &fx->info->expireAction);
            }

            fx->bStale = true;
        }
    }
}

//
// FX_Kill
//

void FX_Kill(fx_t *fx)
{
    fx_t* next;

    if(fx->refcount > 0)
        return;

    /* Remove from main fx list */
    next = fxRover->next;

    (next->prev = fxRover = fx->prev)->next = next;
    Z_Free(fx);
}

//
// FX_ClearLinks
//

void FX_ClearLinks(void)
{
    Z_FreeTags(PU_FX, PU_FX);
    fxRoot.next = fxRoot.prev = &fxRoot;
}

//
// FX_Load
//

fxfile_t *FX_Load(const char *name)
{
    fxfile_t *fx;

    if(name == NULL || name[0] == 0)
        return NULL;

    if(!(fx = FX_Find(name)))
    {
        unsigned int hash;
        scparser_t *parser;

        if(strlen(name) >= MAX_FILEPATH)
            Com_Error("FX_Load: \"%s\" is too long", name);

        if(!(parser = SC_Open(name)))
            return NULL;

        fx = (fxfile_t*)Z_Calloc(sizeof(fxfile_t), PU_STATIC, 0);
        strncpy(fx->name, name, MAX_FILEPATH);

        Fx_ParseScript(fx, parser);

        hash = Com_HashFileName(name);
        fx->next = fx_hashlist[hash];
        fx_hashlist[hash] = fx;

        SC_Close();
    }

    return fx;
}

//
// FCmd_SpawnFX
//

static void FCmd_SpawnFX(void)
{
    plane_t *plane;
    vec3_t vec;

    if(Cmd_GetArgc() < 2)
        return;

    if(client.playerActor == NULL)
        return;

    if(!(plane = Map_FindClosestPlane(client.playerActor->origin)))
        return;

    Vec_Set3(vec,
        client.playerActor->rotation[0],
        client.playerActor->rotation[1],
        client.playerActor->rotation[2]);

    FX_Spawn(Cmd_GetArgv(1),
        client.playerActor,
        vec,
        client.playerActor->origin,
        client.playerActor->rotation, plane);
}

//
// FX_Init
//

void FX_Init(void)
{
    Cmd_AddCommand("spawnfx", FCmd_SpawnFX);
    FX_ClearLinks();
}
