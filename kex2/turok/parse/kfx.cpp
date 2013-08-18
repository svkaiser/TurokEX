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
// DESCRIPTION: Loading of KFX files
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "fx.h"
#include "parse.h"

static kexFileCacheList<fxfile_t> kfxList;

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
    scvfx_bActorInstance,
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
    scvfx_onWaterHit,
    scvfx_onWaterExpire,
    scvfx_onWaterTick,
    scvfx_bDestroyOnWaterSurface,
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
    { scvfx_bActorInstance,                 "bActorInstance"                    },
    { scvfx_onWaterHit,                     "onWaterImpact"                     },
    { scvfx_onWaterExpire,                  "onWaterExpire"                     },
    { scvfx_onWaterTick,                    "onWaterTick"                       },
    { scvfx_bDestroyOnWaterSurface,         "bDestroyOnWaterSurface",           },
    { -1,                                   NULL                                }
};

//
// Kfx_ParseEvent
//

static void Kfx_ParseEvent(fxEvent_t *fxEvent, scparser_t *parser) {
    SC_ExpectNextToken(TK_LBRACK);
    while(1) {
        SC_Find();
        if(!strcmp(sc_parser->token, "fx")) {
            SC_ExpectNextToken(TK_EQUAL);
            SC_GetString();
            fxEvent->fx = Z_Strndup(parser->stringToken,
                MAX_FILEPATH, PU_STATIC, 0);
        }
        else if(!strcmp(sc_parser->token, "sound")) {
            SC_ExpectNextToken(TK_EQUAL);
            SC_GetString();
            fxEvent->snd = Z_Strndup(parser->stringToken,
                MAX_FILEPATH, PU_STATIC, 0);
        }
        else if(!strcmp(sc_parser->token, "action")) {
            SC_ExpectNextToken(TK_EQUAL);
            SC_ExpectNextToken(TK_LBRACK);

            SC_GetString();
            fxEvent->action.function = Z_Strdup(parser->stringToken,
                PU_STATIC, 0);
            fxEvent->action.args[0] = (float)SC_GetFloat();

            SC_ExpectNextToken(TK_RBRACK);
        }
        else if(sc_parser->tokentype == TK_RBRACK)
            break;
        else {
            SC_Error("Kfx_ParseScript: Unknown token: %s\n",
                parser->token);
        }
    }
}

//
// Kfx_ParseScript
//

static void Kfx_ParseScript(fxfile_t *fx, scparser_t *parser) {
    unsigned int i;

    SC_Find();

    if(strcmp(parser->token, "fx"))
        common.Error("Kfx_ParseScript: Expected 'fx', found %s", parser->token);

    SC_ExpectNextToken(TK_LSQBRACK);

    fx->numfx = SC_GetNumber();
    fx->info = (fxinfo_t*)Z_Calloc(sizeof(fxinfo_t) * fx->numfx, PU_STATIC, 0);

    SC_ExpectNextToken(TK_RSQBRACK);
    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

#define CHECK_INT(name)                         \
    case scvfx_ ##name:                         \
    SC_AssignInteger(vfxtokens, (unsigned int*)&info-> ##name, \
        scvfx_ ##name, parser, false);          \
    break

#define CHECK_FLOAT(name, prop)                 \
    case scvfx_ ##name:                         \
    SC_AssignFloat(vfxtokens, &info-> ##prop,   \
        scvfx_ ##name, parser, false);          \
    break

#define CHECK_VFXINT(name)                              \
    case scvfx_ ##name:                                 \
    SC_AssignInteger(vfxtokens, (unsigned int*)&info-> ##name .value,  \
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

    for(i = 0; i < fx->numfx; i++) {
        fxinfo_t *info = &fx->info[i];
        int j;

        SC_ExpectNextToken(TK_LBRACK);
        SC_Find();

        while(parser->tokentype != TK_RBRACK) {
            switch(SC_GetIDForToken(vfxtokens, parser->token)) {
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
            CHECK_INT(bDestroyOnWaterSurface);
            CHECK_INT(bActorInstance);
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
                for(j = 0; j < info->numTextures; j++) {
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
                    info->drawtype = VFX_DRAWBILLBOARD;
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
                Kfx_ParseEvent(&info->onImpact, parser);
                break;

            case scvfx_onExpire:
                Kfx_ParseEvent(&info->onExpire, parser);
                break;

            case scvfx_onTick:
                Kfx_ParseEvent(&info->onTick, parser);
                break;

            case scvfx_onWaterHit:
                Kfx_ParseEvent(&info->onWaterImpact, parser);
                break;

            case scvfx_onWaterExpire:
                Kfx_ParseEvent(&info->onWaterExpire, parser);
                break;

            case scvfx_onWaterTick:
                Kfx_ParseEvent(&info->onWaterTick, parser);
                break;

            default:
                if(parser->tokentype == TK_IDENIFIER) {
                    SC_Error("Kfx_ParseScript: Unknown token: %s\n",
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
// Kfx_Load
//

fxfile_t *Kfx_Load(const char *name) {
    fxfile_t *fx;

    if(name == NULL || name[0] == 0)
        return NULL;

    if(!(fx = kfxList.Find(name))) {
        scparser_t *parser;

        if(strlen(name) >= MAX_FILEPATH)
            common.Error("FX_Load: \"%s\" is too long", name);

        if(!(parser = SC_Open(name)))
            return NULL;

        fx = kfxList.Create(name);

        Kfx_ParseScript(fx, parser);

        kfxList.Add(fx);

        SC_Close();
    }

    return fx;
}
