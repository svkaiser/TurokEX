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

extern short texindexes[2000];
extern const char *sndfxnames[];

char *GetActionName(int id, float arg0);

static byte *fxdata;

#define CHUNK_DIRECTORY_VFX     20

#define CHUNK_VFX_CHUNKSIZE     0
#define CHUNK_VFX_DATA          4
#define CHUNK_VFX_INDEXES       8
#define CHUNK_VFX_EVENTS        12

#define CHUNK_VFX_INDEX_COUNT   4

#define CHUNK_VFX_DATA_OFFSETS  4
#define CHUNK_VFX_DATA_START    8

#define CHUNK_EVENTS_COUNT      4
#define CHUNK_EVENTS_START      8

#define FXF_FADEOUT         0x00001
#define FXF_UNKNOWN2        0x00002
#define FXF_STOPANIMONHIT   0x00004
#define FXF_FLOOROFFSET     0x00008
#define FXF_UNKNOWN5        0x00010
#define FXF_ADDOFFSET       0x00020
#define FXF_UNKNOWN7        0x00040
#define FXF_ACTORINSTANCE   0x00080
#define FXF_MIRROR          0x00100
#define FXF_UNKNOWN10       0x00200
#define FXF_UNKNOWN11       0x00400
#define FXF_UNKNOWN12       0x00800
#define FXF_SCALELERP       0x01000
#define FXF_UNKNOWN14       0x02000
#define FXF_DEPTHBUFFER     0x04000
#define FXF_UNKNOWN15       0x08000
#define FXF_UNKNOWN16       0x10000
#define FXF_DESTROYONWATER  0x20000
#define FXF_PROJECTILE      0x40000
#define FXF_LENSFLARES      0x80000
#define FXF_LOCALAXIS       0x100000
#define FXF_UNKNOWN21       0x200000
#define FXF_UNKNOWN22       0x400000
#define FXF_STICKONTARGET   0x800000
#define FXF_NODIRECTION     0x1000000
#define FXF_BLOOD           0x2000000
#define FXF_UNKNOWN26       0x4000000
#define FXF_UNKNOWN27       0x8000000

typedef struct
{
    short hitplane;
    short hitwater;
    short onhitmetal;
    short onhitstone;
    short onhitflesh;
    short onhitmonster;
    short onhitalien;
    short onhitlava;
    short onhitslime;
    short onhitforcefield;
    short onexpire;
    short active;
    short onexpirewater;
    short activewater;
} fxbehavior_t;

typedef struct
{
    float f[14];
    fxbehavior_t b_fx;
    fxbehavior_t b_action;
    fxbehavior_t b_sound;
} fxevent_t;

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

void FX_GetName(int index, char *name)
{
    if(index == -1)
    {
        sprintf(name, "none");
        return;
    }

    switch(index)
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
    case 12:
        sprintf(name, "fx/projectile_grenade_explosion.kfx");
        break;
    case 14:
        sprintf(name, "fx/impact_water_splash01.kfx");
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
    case 21:
        sprintf(name, "fx/projectile_greenblob.kfx");
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
    /*case 30:
        sprintf(name, "fx/muzzle_rifle.kfx");
        break;*/
    case 32:
        sprintf(name, "fx/muzzle_grenade_launcher.kfx");
        break;
    /*case 38:
        sprintf(name, "fx/muzzle_pistol.kfx");
        break;
    case 39:
        sprintf(name, "fx/muzzle_shotgun.kfx");
        break;*/
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
    case 91:
        sprintf(name, "fx/projectile_rocket_explosion.kfx");
        break;
    case 92:
        sprintf(name, "fx/projectile_rocket_smoketrail.kfx");
        break;
    case 101:
        sprintf(name, "fx/projectile_pulseshot_impact.kfx");
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
    case 121:
        sprintf(name, "fx/impact_bubbles.kfx");
        break;
    case 127:
        sprintf(name, "fx/projectile_greenblob_trail.kfx");
        break;
    case 128:
        sprintf(name, "fx/projectile_greenblob_explode.kfx");
        break;
    case 132:
        sprintf(name, "fx/projectile_tekarrow_explode.kfx");
        break;
    case 166:
        sprintf(name, "fx/ambience_bubbles01.kfx");
        break;
    case 186:
        sprintf(name, "fx/projectile_grenade_explosion_water.kfx");
        break;
    case 189:
        sprintf(name, "fx/impact_water_splash02.kfx");
        break;
    case 192:
        sprintf(name, "fx/bubble_trail01.kfx");
        break;
    case 193:
        sprintf(name, "fx/projectile_shellexp.kfx");
        break;
    case 194:
        sprintf(name, "fx/projectile_tekarrow.kfx");
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
    case 229:
        sprintf(name, "fx/projectile_hummer_bullet_1.kfx");
        break;
    case 230:
        sprintf(name, "fx/projectile_trex_rockets.kfx");
        break;
    case 254:
        sprintf(name, "fx/blood_spurt1.kfx");
        break;
    case 255:
        sprintf(name, "fx/projectile_pulseshot_trail.kfx");
        break;
    case 264:
        sprintf(name, "fx/projectile_enemy_pulseshot.kfx");
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
    case 309:
        sprintf(name, "fx/electric_spaz.kfx");
        break;
    case 310:
        sprintf(name, "fx/projectile_chronoblast.kfx");
        break;
    case 333:
        sprintf(name, "fx/ambience_waterfall_bubbles.kfx");
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
        sprintf(name, "fx/fx_%03d.kfx", index);
        break;
    }
}

static char *FX_GetDamageClass(int id, float arg0)
{
    static char damageClass[128];
    int iArg0 = (int)arg0;

    strcpy(damageClass, va("defs/damage.def@damageClass_%03d_%i", id, iArg0));
    return damageClass;
}

static dboolean FX_CanDamage(fxevent_t *fxevent)
{
    return
        (fxevent->b_action.onhitalien       != -1 ||
         fxevent->b_action.onhitflesh       != -1 ||
         fxevent->b_action.onhitforcefield  != -1 ||
         fxevent->b_action.onhitlava        != -1 ||
         fxevent->b_action.onhitmetal       != -1 ||
         fxevent->b_action.onhitmonster     != -1 ||
         fxevent->b_action.onhitslime       != -1 ||
         fxevent->b_action.onhitstone       != -1);
}

static void FX_WriteEvents(fxevent_t *fxevent, const char *eventName,
                      short fxEvent, short soundEvent, short actionEvent, short actionSlot)
{
    char name[256];

    if(fxEvent != -1 ||
        soundEvent != -1 ||
        actionEvent != -1)
    {
        Com_Strcat("        %s\n", eventName);
        Com_Strcat("        {\n");
        if(fxEvent != -1)
        {
            FX_GetName(fxEvent, name);
            Com_Strcat("            fx = \"%s\"\n", name);
        }
        if(soundEvent != -1)
        {
            Com_Strcat("            sound = \"sounds/shaders/%s.ksnd\"\n",
                sndfxnames[soundEvent]);
        }
        if(actionEvent != -1)
        {
            float evf;

            evf = fxevent->f[actionSlot];

            //Com_Strcat("            action = { \"%s\" %f }\n",
                //GetActionName(actionEvent, evf), evf);
            Com_Strcat("            damageDef = \"%s\"\n",
                FX_GetDamageClass(actionEvent, evf));
        }
        Com_Strcat("        }\n");
    }
}

static void FX_PrintFlag(const char *flagName, const int flags, const int bit)
{
    if(flags & bit)
        Com_Strcat("        %s = 1\n", flagName);
}

static void FX_PrintInt(const char *name, short val)
{
    if(val != 0)
        Com_Strcat("        %s = %i\n", name, val);
}

static void FX_PrintFloat(const char *name, short val)
{
    float f = CoerceFloat(val);

    if(f != 0)
        Com_Strcat("        %s = %f\n", name, f);
}

static void FX_PrintVector(const char *name, short *val)
{
    float f1 = CoerceFloat(val[0]);
    float f2 = CoerceFloat(val[1]);
    float f3 = CoerceFloat(val[2]);

    if(f1 != 0 || f2 != 0 || f3 != 0)
        Com_Strcat("        %s = { %f %f %f }\n", name, f1, f2, f3);
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
    fxevent_t *fxevents;

    tmp = Com_GetCartData(cartfile, CHUNK_DIRECTORY_VFX, &size);
    fxdata = RNC_ParseFile(tmp, size, 0);

    indexes = Com_GetCartData(fxdata, CHUNK_VFX_INDEXES, 0);
    count = Com_GetCartOffset(indexes, CHUNK_VFX_INDEX_COUNT, 0);
    data = Com_GetCartData(fxdata, CHUNK_VFX_DATA, 0);
    events = Com_GetCartData(fxdata, CHUNK_VFX_EVENTS, 0);
    fxevents = (fxevent_t*)(events + 8);

    PK_AddFolder("fx/");
    StoreExternalFile("fx/muzzle_pistol.kfx", "fx/muzzle_pistol.kfx");
    StoreExternalFile("fx/muzzle_rifle.kfx", "fx/muzzle_rifle.kfx");
    StoreExternalFile("fx/muzzle_shotgun.kfx", "fx/muzzle_shotgun.kfx");

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
                int k;
                fxevent_t *fxevent;

                fxchunk = Com_GetCartData(fxdata, CHUNK_VFX_DATA_START, 0);
                vfx = (short*)((fxchunk + 8) + 104 * start);
                fxevent = &fxevents[vfx[0]];

                flags = vfx[2] | (vfx[3] << 16);

                Com_Strcat("    {\n");

                FX_PrintFlag("bFadeout", flags, FXF_FADEOUT);
                FX_PrintFlag("bStopAnimOnImpact", flags, FXF_STOPANIMONHIT);
                FX_PrintFlag("bOffsetFromFloor", flags, FXF_FLOOROFFSET);
                FX_PrintFlag("bTextureWrapMirror", flags, FXF_MIRROR);
                FX_PrintFlag("bDepthBuffer", flags, FXF_DEPTHBUFFER);
                FX_PrintFlag("bActorInstance", flags, FXF_ACTORINSTANCE);
                FX_PrintFlag("bScaleLerp", flags, FXF_SCALELERP);
                FX_PrintFlag("bLensFlares", flags, FXF_LENSFLARES);
                FX_PrintFlag("bBlood", flags, FXF_BLOOD);
                FX_PrintFlag("bAddOffset", flags, FXF_ADDOFFSET);
                FX_PrintFlag("bNoDirection", flags, FXF_NODIRECTION);
                FX_PrintFlag("bLocalAxis", flags, FXF_LOCALAXIS);
                FX_PrintFlag("bProjectile", flags, FXF_PROJECTILE);
                FX_PrintFlag("bDestroyOnWaterSurface", flags, FXF_DESTROYONWATER);

                if(FX_CanDamage(fxevent))
                    Com_Strcat("        bLinkArea = 1\n");

                FX_PrintFloat("mass", vfx[4]);
                FX_PrintFloat("translation_global_randomscale", vfx[5]);
                FX_PrintVector("translation_randomscale", &vfx[6]);
                FX_PrintVector("translation", &vfx[9]);
                FX_PrintFloat("gravity", vfx[12]);
                FX_PrintFloat("gravity_randomscale", vfx[13]);
                FX_PrintFloat("friction", vfx[14]);
                FX_PrintFloat("animFriction", vfx[15]);
                FX_PrintFloat("scale", vfx[18]);
                FX_PrintFloat("scale_randomscale", vfx[19]);
                FX_PrintFloat("scale_dest", vfx[20]);
                FX_PrintFloat("scale_dest_randomscale", vfx[21]);
                FX_PrintFloat("forward_speed", vfx[22]);
                FX_PrintFloat("forward_speed_randomscale", vfx[23]);
                FX_PrintVector("offset_random", &vfx[24]);
                FX_PrintFloat("rotation_offset", vfx[27]);
                FX_PrintFloat("rotation_offset_randomscale", vfx[28]);
                FX_PrintFloat("rotation_speed", vfx[29]);
                FX_PrintFloat("rotation_speed_randomscale", vfx[30]);
                FX_PrintFloat("screen_offset_x", vfx[31]);
                FX_PrintFloat("screen_offset_y", vfx[32]);
                FX_PrintVector("offset", &vfx[33]);

                Com_Strcat("        textures[%i] =\n", texindexes[vfx[36]]);
                Com_Strcat("        {\n");
                for(k = 0; k < texindexes[vfx[36]]; k++)
                    Com_Strcat("            \"textures/tex%04d_%02d.tga\"\n", vfx[36], k);
                Com_Strcat("        }\n");

                FX_PrintInt("instances", vfx[37]);
                FX_PrintInt("instances_randomscale", vfx[38]);
                FX_PrintInt("lifetime", vfx[39]);
                FX_PrintInt("lifetime_randomscale", vfx[40]);
                FX_PrintInt("restart", vfx[41]);
                FX_PrintInt("animspeed", vfx[42] & 0xff);

                Com_Strcat("        color1 = [%i %i %i]\n",
                    (vfx[44] >> 8) & 0xff, vfx[45] & 0xff, (vfx[45] >> 8) & 0xff);
                Com_Strcat("        color2 = [%i %i %i]\n",
                    vfx[46] & 0xff, (vfx[46] >> 8) & 0xff, vfx[47] & 0xff);

                FX_PrintInt("color1_randomscale", (vfx[47] >> 8) & 0xff);
                FX_PrintInt("color2_randomscale", (vfx[48] >> 8) & 0xff);
                FX_PrintInt("saturation_randomscale", vfx[48] & 0xff);
                FX_PrintInt("fadein_time", (vfx[50] >> 8) & 0xff);
                FX_PrintInt("fadeout_time", vfx[51]);

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
                case 3:
                    Com_Strcat("        onplane = bounce\n");
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
                case 3:
                    Com_Strcat("        onplane = bounce\n");
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
                case 4:
                    Com_Strcat("        drawtype = surface\n");
                    break;
                case 5:
                    Com_Strcat("        drawtype = billboard\n");
                    break;
                default:
                    Com_Strcat("        // drawtype = unknown_%i\n", vfx[44] & 0xff);
                    break;
                }
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

                // fxevent ## 11 appears to be blank, so we'll just
                // take advantage of that
                if(vfx[0] != 11)
                {
                    Com_Strcat("        onImpact\n");
                    Com_Strcat("        {\n");
                    FX_WriteEvents(fxevent, "[0]",
                        fxevent->b_fx.hitplane,
                        fxevent->b_sound.hitplane,
                        fxevent->b_action.hitplane, 0);
                    FX_WriteEvents(fxevent, "[2]",
                        fxevent->b_fx.onhitmetal,
                        fxevent->b_sound.onhitmetal,
                        fxevent->b_action.onhitmetal, 2);
                    FX_WriteEvents(fxevent, "[3]",
                        fxevent->b_fx.onhitstone,
                        fxevent->b_sound.onhitstone,
                        fxevent->b_action.onhitstone, 3);
                    FX_WriteEvents(fxevent, "[4]",
                        fxevent->b_fx.onhitflesh,
                        fxevent->b_sound.onhitflesh,
                        fxevent->b_action.onhitflesh, 4);
                    FX_WriteEvents(fxevent, "[5]",
                        fxevent->b_fx.onhitmonster,
                        fxevent->b_sound.onhitmonster,
                        fxevent->b_action.onhitmonster, 5);
                    FX_WriteEvents(fxevent, "[6]",
                        fxevent->b_fx.onhitalien,
                        fxevent->b_sound.onhitalien,
                        fxevent->b_action.onhitalien, 6);
                    FX_WriteEvents(fxevent, "[7]",
                        fxevent->b_fx.onhitlava,
                        fxevent->b_sound.onhitlava,
                        fxevent->b_action.onhitlava, 7);
                    FX_WriteEvents(fxevent, "[8]",
                        fxevent->b_fx.onhitslime,
                        fxevent->b_sound.onhitslime,
                        fxevent->b_action.onhitslime, 8);
                    FX_WriteEvents(fxevent, "[9]",
                        fxevent->b_fx.onhitforcefield,
                        fxevent->b_sound.onhitforcefield,
                        fxevent->b_action.onhitforcefield, 9);
                    Com_Strcat("        }\n");
                }

                FX_WriteEvents(fxevent, "onTick",
                    fxevent->b_fx.active,
                    fxevent->b_sound.active,
                    fxevent->b_action.active, 11);

                FX_WriteEvents(fxevent, "onExpire",
                    fxevent->b_fx.onexpire,
                    fxevent->b_sound.onexpire,
                    fxevent->b_action.onexpire, 10);

                FX_WriteEvents(fxevent, "onWaterImpact",
                    fxevent->b_fx.hitwater,
                    fxevent->b_sound.hitwater,
                    fxevent->b_action.hitwater, 1);

                FX_WriteEvents(fxevent, "onWaterTick",
                    fxevent->b_fx.activewater,
                    fxevent->b_sound.activewater,
                    fxevent->b_action.activewater, 13);

                FX_WriteEvents(fxevent, "onWaterExpire",
                    fxevent->b_fx.onexpirewater,
                    fxevent->b_sound.onexpirewater,
                    fxevent->b_action.onexpirewater, 12);

                Com_Strcat("    }\n");
                start++;
            }

            Com_Strcat("}\n");

            FX_GetName(i, name);
            Com_StrcatAddToFile(name);
            j++;
        }
    }

    Com_Free(&fxdata);
}
