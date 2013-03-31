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

#include "types.h"
#include "common.h"
#include "rnc.h"
#include "pak.h"
#include "decoders.h"

static byte *fxdata;

#define CHUNK_DIRECTORY_VFX     20

#define CHUNK_VFX_CHUNKSIZE     0
#define CHUNK_VFX_DATA          4
#define CHUNK_VFX_INDEXES       8
#define CHUNK_VFX_EVENTS        12

#define CHUNK_VFX_INDEX_COUNT   4

#define CHUNK_VFX_DATA_OFFSETS  4
#define CHUNK_VFX_DATA_START    8

#define FXF_FADEOUT         0x00001
#define FXF_UNKNOWN2        0x00002
#define FXF_STOPANIMONHIT   0x00004
#define FXF_FLOOROFFSET     0x00008
#define FXF_UNKNOWN5        0x00010
#define FXF_UNKNOWN6        0x00020
#define FXF_UNKNOWN7        0x00040
#define FXF_PERFRAME        0x00080
#define FXF_MIRROR          0x00100
#define FXF_UNKNOWN10       0x00200
#define FXF_UNKNOWN11       0x00400
#define FXF_UNKNOWN12       0x00800
#define FXF_UNKNOWN13       0x01000
#define FXF_UNKNOWN14       0x02000
#define FXF_ANIMATE         0x04000
#define FXF_UNKNOWN15       0x08000
#define FXF_UNKNOWN16       0x10000
#define FXF_UNKNOWN17       0x20000
#define FXF_UNKNOWN18       0x40000
#define FXF_LENSFLARES      0x80000
#define FXF_UNKNOWN20       0x100000
#define FXF_UNKNOWN21       0x200000
#define FXF_UNKNOWN22       0x400000
#define FXF_UNKNOWN23       0x800000
#define FXF_UNKNOWN24       0x1000000
#define FXF_BLOOD           0x2000000
#define FXF_UNKNOWN26       0x4000000
#define FXF_UNKNOWN27       0x8000000

static float CoerceFloat(__int16 val)
{
    union
    {
       int i;
       float f;
    } coerce_u;

    coerce_u.i = (val << 16);
    return coerce_u.f;
}

void FX_StoreParticleEffects(void)
{
    byte *tmp;
    byte *indexes;
    byte *data;
    byte *events;
    int count;
    int size;
    int i;
    int j;
    char name[256];

    tmp = Com_GetCartData(cartfile, CHUNK_DIRECTORY_VFX, &size);
    fxdata = RNC_ParseFile(tmp, size, 0);

    indexes = Com_GetCartData(fxdata, CHUNK_VFX_INDEXES, 0);
    count = Com_GetCartOffset(indexes, CHUNK_VFX_INDEX_COUNT, 0);
    data = Com_GetCartData(fxdata, CHUNK_VFX_DATA, 0);
    events = Com_GetCartData(fxdata, CHUNK_VFX_EVENTS, 0);

    PK_AddFolder("fx/");

    for(i = 0, j = 0; i < count; i++)
    {
        int start;
        int end;

        if(DC_LookupParticleFX(data + CHUNK_VFX_DATA_START,
            *((int*)data + 1), i, &start, &end))
        {
            Com_StrcatClear();
            Com_SetDataProgress(1);

            Com_Strcat("fx[%i] =\n{\n", (end-start)+1);

            while(start <= end)
            {
                byte *fxchunk;
                short *vfx;
                int flags;

                fxchunk = Com_GetCartData(fxdata, CHUNK_VFX_DATA_START, 0);
                vfx = (short*)((fxchunk + 8) + 104 * start);

                flags = vfx[2] | (vfx[3] << 16);

                Com_Strcat("    {\n");
                Com_Strcat("        bFadeout = %i\n", (flags & FXF_FADEOUT) ? 1 : 0);
                Com_Strcat("        bStopAnimOnImpact = %i\n", (flags & FXF_STOPANIMONHIT) ? 1 : 0);
                Com_Strcat("        bOffsetFromFloor = %i\n", (flags & FXF_FLOOROFFSET) ? 1 : 0);
                Com_Strcat("        bTextureWrapMirror = %i\n", (flags & FXF_MIRROR) ? 1 : 0);
                Com_Strcat("        bAnimate = %i\n", (flags & FXF_ANIMATE) ? 1 : 0);
                Com_Strcat("        bLensFlares = %i\n", (flags & FXF_LENSFLARES) ? 1 : 0);
                Com_Strcat("        bBlood = %i\n", (flags & FXF_BLOOD) ? 1 : 0);
                Com_Strcat("        mass = %f\n", CoerceFloat(vfx[4]));
                Com_Strcat("        translation_global_randomscale = %f\n", CoerceFloat(vfx[5]));
                Com_Strcat("        translation_randomscale = { %f %f %f }\n",
                    CoerceFloat(vfx[6]), CoerceFloat(vfx[7]), CoerceFloat(vfx[8]));
                Com_Strcat("        translation = { %f %f %f }\n",
                    CoerceFloat(vfx[9]), CoerceFloat(vfx[10]), CoerceFloat(vfx[11]));
                Com_Strcat("        gravity = %f\n", CoerceFloat(vfx[12]));
                Com_Strcat("        gravity_randomscale = %f\n", CoerceFloat(vfx[13]));
                Com_Strcat("        friction = %f\n", CoerceFloat(vfx[14]));
                Com_Strcat("        friction_randomscale = %f\n", CoerceFloat(vfx[15]));
                Com_Strcat("        scale = %f\n", CoerceFloat(vfx[18]));
                Com_Strcat("        scale_randomscale = %f\n", CoerceFloat(vfx[19]));
                Com_Strcat("        scale_dest = %f\n", CoerceFloat(vfx[20]));
                Com_Strcat("        scale_dest_randomscale = %f\n", CoerceFloat(vfx[21]));
                Com_Strcat("        forward_speed = %f\n", CoerceFloat(vfx[22]));
                Com_Strcat("        forward_speed_randomscale = %f\n", CoerceFloat(vfx[23]));
                Com_Strcat("        offset_random = { %f %f %f }\n",
                    CoerceFloat(vfx[24]), CoerceFloat(vfx[25]), CoerceFloat(vfx[26]));
                Com_Strcat("        rotation_offset = %f\n", CoerceFloat(vfx[27]));
                Com_Strcat("        rotation_offset_randomscale = %f\n", CoerceFloat(vfx[28]));
                Com_Strcat("        rotation_speed = %f\n", CoerceFloat(vfx[29]));
                Com_Strcat("        rotation_speed_randomscale = %f\n", CoerceFloat(vfx[30]));
                Com_Strcat("        screen_offset_x = %f\n", CoerceFloat(vfx[31]));
                Com_Strcat("        screen_offset_y = %f\n", CoerceFloat(vfx[32]));
                Com_Strcat("        offset = { %f %f %f }\n",
                    CoerceFloat(vfx[33]), CoerceFloat(vfx[34]), CoerceFloat(vfx[35]));
                Com_Strcat("        texture = \"textures/tex%04d_00.tga\"\n", vfx[36]);
                Com_Strcat("        instances = %i\n", vfx[37]);
                Com_Strcat("        instances_randomscale = %i\n", vfx[38]);
                Com_Strcat("        lifetime = %i\n", vfx[39]);
                Com_Strcat("        lifetime_randomscale = %i\n", vfx[40]);
                Com_Strcat("        restart = %i\n", vfx[41]);
                Com_Strcat("        animspeed = %i\n", vfx[42] & 0xff);
                switch((vfx[42] >> 8) & 0xff)
                {
                case 0:
                    Com_Strcat("        ontouch = default\n");
                    break;
                case 1:
                    Com_Strcat("        ontouch = destroy\n");
                    break;
                case 2:
                    Com_Strcat("        ontouch = reflect\n");
                    break;
                default:
                    Com_Strcat("        // ontouch = unknown_%i\n", (vfx[42] >> 8) & 0xff);
                    break;
                }
                switch((vfx[43] >> 8) & 0xff)
                {
                case 0:
                    Com_Strcat("        onplane = default\n");
                    break;
                case 1:
                    Com_Strcat("        onplane = destroy\n");
                    break;
                case 2:
                    Com_Strcat("        onplane = reflect\n");
                    break;
                default:
                    Com_Strcat("        // onplane = unknown_%i\n", (vfx[43] >> 8) & 0xff);
                    break;
                }
                switch(vfx[44] & 0xff)
                {
                case 0:
                    Com_Strcat("        drawtype = default\n");
                    break;
                case 1:
                    Com_Strcat("        drawtype = flat\n");
                    break;
                case 2:
                    Com_Strcat("        drawtype = decal\n");
                    break;
                case 5:
                    Com_Strcat("        drawtype = billboard\n");
                    break;
                default:
                    Com_Strcat("        // drawtype = unknown_%i\n", vfx[44] & 0xff);
                    break;
                }
                Com_Strcat("        color = [%i %i %i]\n",
                    vfx[46] & 0xff, (vfx[46] >> 8) & 0xff, vfx[47] & 0xff);
                switch((vfx[49] >> 8) & 0xff)
                {
                case 0:
                    Com_Strcat("        animtype = default\n");
                    break;
                case 1:
                    Com_Strcat("        animtype = onetime\n");
                    break;
                case 2:
                    Com_Strcat("        animtype = loop\n");
                    break;
                case 3:
                    Com_Strcat("        animtype = sinwave\n");
                    break;
                default:
                    Com_Strcat("        // animtype = unknown_%i\n", (vfx[49] >> 8) & 0xff);
                    break;
                }

                Com_Strcat("    }\n");
                start++;
            }

            Com_Strcat("}\n");

            switch(i)
            {
            case 1:
                sprintf(name, "fx/projectile_arrow.kfx");
                break;
            case 2:
                sprintf(name, "fx/projectile_bullet.kfx");
                break;
            case 3:
                sprintf(name, "fx/projectile_shell.kfx");
                break;
            case 4:
                sprintf(name, "fx/projectile_grenade.kfx");
                break;
            case 5:
                sprintf(name, "fx/projectile_tekshot.kfx");
                break;
            case 6:
                sprintf(name, "fx/projectile_fusionshot.kfx");
                break;
            case 7:
                sprintf(name, "fx/bulletshell.kfx");
                break;
            case 8:
                sprintf(name, "fx/shotshell.kfx");
                break;
            case 9:
                sprintf(name, "fx/blood_gush1.kfx");
                break;
            case 16:
                sprintf(name, "fx/projectile_acid.kfx");
                break;
            case 17:
                sprintf(name, "fx/projectile_trex_fire.kfx");
                break;
            case 18:
                sprintf(name, "fx/projectile_redbubbles.kfx");
                break;
            case 22:
                sprintf(name, "fx/projectile_shockwave.kfx");
                break;
            case 23:
                sprintf(name, "fx/projectile_shaman.kfx");
                break;
            case 26:
                sprintf(name, "fx/projectile_enemy_bullet.kfx");
                break;
            case 27:
                sprintf(name, "fx/projectile_enemy_tekshot.kfx");
                break;
            case 28:
                sprintf(name, "fx/steppuff1.kfx");
                break;
            case 29:
                sprintf(name, "fx/steppuff2.kfx");
                break;
            case 43:
                sprintf(name, "fx/projectile_flame.kfx");
                break;
            case 44:
                sprintf(name, "fx/projectile_trex_laser_fire.kfx");
                break;
            case 49:
                sprintf(name, "fx/projectile_enemy_shell.kfx");
                break;
            case 50:
                sprintf(name, "fx/projectile_scatter_blast.kfx");
                break;
            case 52:
                sprintf(name, "fx/blood_death.kfx");
                break;
            case 54:
                sprintf(name, "fx/sparks_damage.kfx");
                break;
            case 57:
                sprintf(name, "fx/blood_splatter1.kfx");
                break;
            case 81:
                sprintf(name, "fx/ambience_green_fountain.kfx");
                break;
            case 84:
                sprintf(name, "fx/ambience_waterfall_steam.kfx");
                break;
            case 108:
                sprintf(name, "fx/ambience_underwater_bubbles.kfx");
                break;
            case 116:
                sprintf(name, "fx/projectile_rocket.kfx");
                break;
            case 117:
                sprintf(name, "fx/projectile_particleshot_1.kfx");
                break;
            case 120:
                sprintf(name, "fx/projectile_pulseshot.kfx");
                break;
            case 166:
                sprintf(name, "fx/ambience_rain.kfx");
                break;
            case 193:
                sprintf(name, "fx/projectile_shellexp.kfx");
                break;
            case 196:
                sprintf(name, "fx/shotshellexp.kfx");
                break;
            case 198:
                sprintf(name, "fx/projectile_particleshot_2.kfx");
                break;
            case 199:
                sprintf(name, "fx/projectile_particleshot_3.kfx");
                break;
            case 200:
                sprintf(name, "fx/projectile_particleshot_4.kfx");
                break;
            case 201:
                sprintf(name, "fx/projectile_particleshot_5.kfx");
                break;
            case 230:
                sprintf(name, "fx/projectile_trex_rockets.kfx");
                break;
            case 254:
                sprintf(name, "fx/blood_spurt1.kfx");
                break;
            case 285:
                sprintf(name, "fx/ambience_tall_fire1.kfx");
                break;
            case 286:
                sprintf(name, "fx/ambience_tall_fire2.kfx");
                break;
            case 303:
                sprintf(name, "fx/projectile_minibullet.kfx");
                break;
            case 306:
                sprintf(name, "fx/pickup_powerup.kfx");
                break;
            case 310:
                sprintf(name, "fx/projectile_chronoblast.kfx");
                break;
            case 344:
                sprintf(name, "fx/pickup_key_sparkles.kfx");
                break;
            case 359:
                sprintf(name, "fx/ambience_thunderstorm.kfx");
                break;
            case 362:
                sprintf(name, "fx/hummer_explosion.kfx");
                break;
            case 374:
                sprintf(name, "fx/animal_death_flash.kfx");
                break;
            default:
                sprintf(name, "fx/fx_%03d.kfx", i);
                break;
            }
            Com_StrcatAddToFile(name);
            j++;
        }
    }

    Com_Free(&fxdata);
}
