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

#include <math.h>

#include "types.h"
#include "common.h"
#include "pak.h"
#include "rnc.h"
#include "decoders.h"
#include "zone.h"

//#define FORMAT_BINARY
//#define OLD_FORMAT

extern const char *sndfxnames[];
void AddTexture(byte *data, int size, const char *path);

#define M_PI                        3.1415926535897932384626433832795
#define M_RAD                       (M_PI / 180.0)
#define M_DEG                       (180.0 / M_PI)

#define ANGLE_LEVELOBJECT           (255.0f / 180.0f)
#define ANGLE_INSTANCE              (ANGLE_LEVELOBJECT / 180.0f)

#define CHUNK_DIRECTORY_ATTRIBUTES  8
#define CHUNK_DIRECTORY_TYPES       12
#define CHUNK_DIRECTORY_LEVEL       28
#define CHUNK_DIRECTORY_WARPS       36

#define CHUNK_ATTRIB_SIZE           0
#define CHUNK_ATTRIB_COUNT          4

#define CHUNK_LEVEL_COUNT           0
#define CHUNK_LEVEL_OFFSET(x)       (4 + (x * 4))

#define CHUNK_LVROOT_ENTIRES        0
#define CHUNK_LVROOT_HEADER         4
#define CHUNK_LVROOT_INFO           8
#define CHUNK_LVROOT_ACTORS         12
#define CHUNK_LVROOT_GRIDBOUNDS     16
#define CHUNK_LVROOT_INSTANCES      20
#define CHUNK_LVROOT_SKYTEXTURE     24

#define CHUNK_LEVELINFO_ENTIRES     0
#define CHUNK_LEVELINFO_AREAS       4
#define CHUNK_LEVELINFO_POINTS      8
#define CHUNK_LEVELIFNO_LEAFS       12
#define CHUNK_LEVELINFO_ZONEBOUNDS  16

#define CHUNK_AREAS_SIZE            0
#define CHUNK_AREAS_COUNT           4

#define CHUNK_POINTS_SIZE           0
#define CHUNK_POINTS_COUNT          4

#define CHUNK_LEAFS_SIZE            0
#define CHUNK_LEAFS_COUNT           4

#define CHUNK_ZONE_SIZE             0
#define CHUNK_ZONE_COUNT            4

#define CHUNK_ACTORS_SIZE           0
#define CHUNK_ACTORS_COUNT          4


#define CHUNK_GRIDBOUNDS_SIZE       0
#define CHUNK_GRIDBOUNDS_COUNT      4

#define CHUNK_LEVELGRID_ENTRIES     0
#define CHUNK_LEVELGRID_XMIN        4
#define CHUNK_LEVELGRID_XMAX        8
#define CHUNK_LEVELGRID_ZMIN        12
#define CHUNK_LEVELGRID_ZMAX        16
#define CHUNK_LEVELGRID_BOUNDS      20

#define CHUNK_GRIDBOUND_XMIN_SIZE   0
#define CHUNK_GRIDBOUND_XMIN_COUNT  4

#define CHUNK_GRIDBOUND_XMAX_SIZE   0
#define CHUNK_GRIDBOUND_XMAX_COUNT  4

#define CHUNK_GRIDBOUND_ZMIN_SIZE   0
#define CHUNK_GRIDBOUND_ZMIN_COUNT  4

#define CHUNK_GRIDBOUND_ZMAX_SIZE   0
#define CHUNK_GRIDBOUND_ZMAX_COUNT  4

#define CHUNK_STATICINST_ENTRIES    0
#define CHUNK_STATICINST_GROUP1     4
#define CHUNK_STATICINST_GROUP2     8
#define CHUNK_STATICINST_GROUP3     12

#define CHUNK_INSTANCE_SIZE         0
#define CHUNK_INSTANCE_COUNT        4

#define CHUNK_INSTGROUP_COUNT       0
#define CHUNK_INSTGROUP_OFFSET(x)   (4 + (x * 4))

#define CHUNK_WARP_SIZE             0
#define CHUNK_WARP_TAGS             4
#define CHUNK_WARP_INFO             8
#define CHUNK_WARPTAGS_SIZE         0
#define CHUNK_WARPTAGS_COUNT        4
#define CHUNK_WARPINFO_SIZE         0
#define CHUNK_WARPINFO_COUNT        4

typedef enum
{
    OT_TUROK = 0,

    OT_AI_RAPTOR = 1,
    OT_AI_GRUNT = 2,
    OT_AI_DINOSAUR1 = 4,
    OT_AI_RIDER = 5,
    OT_AI_SANDWORM = 8,
    OT_AI_STALKER = 9,
    OT_AI_ALIEN = 10,
    OT_AI_PURLIN = 11,
    OT_AI_MECH = 12,
    OT_AI_FISH = 14,
    OT_AI_SEWERCRAB = 15,
    OT_AI_KILLERPLANT = 16,
    OT_AI_UNKNOWN1 = 19,
    OT_AI_ANIMAL = 20,
    OT_AI_INSECT = 21,
    OT_AI_TURRET = 2001,

    OT_AIBOSS_MANTIS = 1000,
    OT_AIBOSS_TREX = 1001,
    OT_AIBOSS_CAMPAINGER = 1002,
    OT_AIBOSS_HUNTER = 1003,
    OT_AIBOSS_HUMMER = 1004,

    OT_DYNAMIC_DOOR = 300,
    OT_DYNAMIC_GENERIC = 301,
    OT_DYNAMIC_MANTISSTATUE = 302,
    OT_DYNAMIC_LIFT = 303,
    OT_DYNAMIC_UNKNOWN1 = 304,
    OT_DYNAMIC_MANTISWALL = 305,
    OT_DYNAMIC_LASERWALL = 308,
    OT_DYNAMIC_DESTROYABLE = 309,
    OT_DYNAMIC_MOVER = 310,
    OT_DYNAMIC_UNKNOWN2 = 311,
    OT_DYNAMIC_LOGO = 313,
    OT_DYNAMIC_TECHLIGHT = 314,
    OT_DYNAMIC_UNKNOWN3 = 316,
    OT_DYNAMIC_PORTAL = 317,
    OT_DYNAMIC_GATEKEYPANEL = 318,
    OT_DYNAMIC_WATER1 = 319,
    OT_DYNAMIC_WATER2 = 320,
    OT_DYNAMIC_PORTALGATE = 321,
    OT_DYNAMIC_ARROW = 322,
    OT_DYNAMIC_MANTISKEY = 323,
    OT_DYNAMIC_UNKNOWN4 = 324,
    OT_DYNAMIC_KEYPLAQUE = 325,
    OT_DYNAMIC_TEXTUREANIMATE = 327,
    OT_DYNAMIC_UNKNOWN5 = 328,
    OT_DYNAMIC_CUSTOM = 329,
    OT_DYNAMIC_UNKNOWN6 = 330,
    OT_DYNAMIC_TECHDOOR = 331,

    OT_UNKNOWN = 200,
    
    OT_GIB_ALIEN3 = 701,
    OT_GIB_STALKER4 = 702,
    OT_GIB_ALIEN2 = 703,
    OT_GIB_ALIEN1 = 704,
    OT_GIB_STALKER3 = 705,
    OT_GIB_STALKER5 = 706,
    OT_GIB_STALKER2 = 707,
    OT_GIB_STALKER1 = 708,

    OT_WEAPON_KNIFE = 100,
    OT_WEAPON_BOW = 102,
    OT_WEAPON_PISTOL = 103,
    OT_WEAPON_RIFLE = 104,
    OT_WEAPON_PULSERIFLE = 105,
    OT_WEAPON_SHOTGUN = 106,
    OT_WEAPON_ASHOTGUN = 107,
    OT_WEAPON_MINIGUN = 108,
    OT_WEAPON_GRENADE = 109,
    OT_WEAPON_ALIENGUN = 110,
    OT_WEAPON_CANNON = 111,
    OT_WEAPON_MISSILE = 112,
    OT_WEAPON_ACCELERATOR = 113,
    OT_WEAPON_CHRONO = 114,
    
    OT_PICKUP_SMALLHEALTH = 400,
    OT_PICKUP_HEALTH = 402,
    OT_PICKUP_FULLHEALTH = 403,
    OT_PICKUP_ULTRAHEALTH = 404,
    OT_PICKUP_MASK = 405,
    OT_PICKUP_BACKPACK = 406,
    OT_PICKUP_SPIRIT = 407,
    OT_PICKUP_PISTOL = 409,
    OT_PICKUP_ASSAULTRIFLE = 410,
    OT_PICKUP_SHOTGUN = 411,
    OT_PICKUP_ASHOTGUN = 412,
    OT_PICKUP_MINIGUN = 413,
    OT_PICKUP_GRENADELAUNCHER = 414,
    OT_PICKUP_PULSERIFLE = 415,
    OT_PICKUP_ALIENWEAPON = 416,
    OT_PICKUP_ROCKETLAUNCHER = 417,
    OT_PICKUP_ACCELERATOR = 418,
    OT_PICKUP_CANNON = 419,
    OT_PICKUP_QUIVER2 = 420,
    OT_PICKUP_ARROWS = 421,
    OT_PICKUP_QUIVER1 = 422,
    OT_PICKUP_CLIP = 423,
    OT_PICKUP_CLIPBOX = 424,
    OT_PICKUP_SHELLS = 425,
    OT_PICKUP_SHELLBOX = 426,
    OT_PICKUP_EXPSHELLS = 427,
    OT_PICKUP_EXPSHELLBOX = 428,
    OT_PICKUP_MINIGUNAMMO = 429,
    OT_PICKUP_GRENADE = 430,
    OT_PICKUP_GRENADEBOX = 431,
    OT_PICKUP_SMALLCELL = 432,
    OT_PICKUP_CELL = 433,
    OT_PICKUP_ROCKET = 434,
    OT_PICKUP_FUSIONCELL = 436,
    OT_PICKUP_ARMOR = 437,
    OT_PICKUP_COIN1 = 438,
    OT_PICKUP_COIN10 = 439,
    OT_PICKUP_KEY1 = 440,
    OT_PICKUP_KEY2 = 441,
    OT_PICKUP_KEY3 = 442,
    OT_PICKUP_KEY4 = 443,
    OT_PICKUP_KEY5 = 444,
    OT_PICKUP_KEY6 = 445,
    OT_PICKUP_FINALKEY1 = 446,
    OT_PICKUP_CHRONOPIECE1 = 447,
    OT_PICKUP_FINALKEY2 = 449,
    OT_PICKUP_CHRONOPIECE2 = 450,
    OT_PICKUP_CHRONOPIECE3 = 451,
    OT_PICKUP_CHRONOPIECE4 = 452,
    OT_PICKUP_CHRONOPIECE5 = 453,
    OT_PICKUP_CHRONOPIECE6 = 454,
    OT_PICKUP_CHRONOPIECE7 = 455,
    OT_PICKUP_CHRONOPIECE8 = 456,

    OT_MINIPORTAL = 600,
    OT_WATER = 800,

} objecttype_t;

typedef enum
{
    AAF_WATER           = 0x1,
    AAF_UNKNOWN2        = 0x2,
    AAF_UNKNOWN4        = 0x4,
    AAF_UNKNOWN8        = 0x8,
    AAF_CLIMB           = 0x10,
    AAF_ONESIDED        = 0x20,
    AAF_REVERB          = 0x40,
    AAF_CRAWL           = 0x80,
    AAF_UNKNOWN100      = 0x100,
    AAF_TOUCH           = 0x200,
    AAF_UNKNOWN400      = 0x400,
    AAF_UNKNOWN800      = 0x800,
    AAF_UNKNOWN1000     = 0x1000,
    AAF_UNKNOWN2000     = 0x2000,
    AAF_DEATHPIT        = 0x4000,
    AAF_UNKNOWN8000     = 0x8000,
    AAF_EVENT           = 0x10000,
    AAF_REPEATABLE      = 0x20000,
    AAF_TELEPORT        = 0x40000,
    AAF_DAMAGE          = 0x80000,
    AAF_DRAWSKY         = 0x100000,
    AAF_WARP            = 0x200000,
    AAF_UNKNOWN400000   = 0x400000,
    AAF_UNKNOWN800000   = 0x800000,
    AAF_UNKNOWN1000000  = 0x1000000,
    AAF_UNKNOWN2000000  = 0x2000000,
    AAF_CHECKPOINT      = 0x4000000,
    AAF_SAVEGAME        = 0x8000000
} areaflags_t;

static byte *leveldata;
static byte *attribdata;
static byte *warpdata;
static byte *areadata;
static short *typedata;
static int numlevels;
static int numattributes;
static byte decode_buffer[0x40000];
static int actorTally = 0;
static int levelID = 0;

static byte lightheader_table1[0x40];
static byte lightheader_table2[0x40];
static byte lightheader_table3[0x40];
static byte lvcolortable1[3];
static byte lvcolortable2[3];
static byte lvcolortable3[3];
static byte lvcolortable4[3];
static byte lvcolortable5[3];

typedef struct
{
    byte *buffer;
    int *size;
    int *actorLookup;
    int **actorStride;
    int *infoLookup;
    int *gridLookup;
    int **gridStride;
    int **staticStride;
} kmapInfo_t;

static kmapInfo_t kmapInfo;

extern short texindexes[2000];
extern short model_nodeCount[800];
extern short model_meshCount[800][100];
extern short section_count[800][100][100];
extern short section_textures[800][100][100][100];
extern bbox mdlboxes[1000];

typedef struct
{
    float u1;
    float uf2;
    float u3;
    float u4;
    float u5;
    float uf6;
    float width;
    float centerHeight;
    float height;
    float viewHeight;
    float u11;
    float u12;
    float triggerDelay;
    int u14;
    int behavior1;
    int behavior2;
    byte behavior3;
    byte behavior4;
    short health;
    short tid;
    short target;
    short skin;
    byte u22;
    byte triggerAnim;
    char variant1;
    char texture;
    char variant2;
    byte u25a;
    short u25b;
    short u26;
    short u27;
    int flags;
} attribute_t;

typedef struct
{
    float dest[3];
    float angle;
    short level;
    short u1;
} warp_t;

typedef struct
{
    short area_id;
    short flags;
    short pt1;
    short pt2;
    short pt3;
    short link1;
    short link2;
    short link3;
} mapleaf_t;

typedef struct
{
    float left;
    float bottom;
    float right;
    float top;
    int unknown1;
    short unknown2;
    short unknown3;
} mapzone_t;

typedef struct
{
    byte    fogrgba[4];
    short   u1;
    byte    u2;
    byte    u3;
    int     u4;
    int     u5;
    float   fogzfar;
    float   u7;
    float   u8;
    float   u9;
    float   waterheight;
    float   skyheight;
    float   u12;
    float   u13;
    float   u14;
    uint    flags;
    short   args1;
    short   args2;
    short   args3;
    short   args4;
    short   args5;
    short   args6;
    byte    u20;
    byte    ambience;
    byte    u22;
    byte    u23;
} maparea_t;

typedef struct
{
    short model;
    short type;
    short attribute;
    short leaf;
    short angle;
    byte flags;
    byte u3;
    float xyz[3];
    float scale[3];
} mapactor_t;

typedef struct
{
    float scale;
    short model;
    short plane;
    short attribute;
    short unknown1;
    short y;
    byte drawflags;
    byte flags;
    byte x;
    byte z;
    byte bbox[6];
    short unknown[24];
} mapinsttype1_t;

typedef struct
{
    float xyz[3];
    float scale[3];
    short bbox[6];
    short model;
    short plane;
    short attribute;
    byte u1;
    byte flags;
    char angle[4];
    short unknown[24];
} mapinsttype2_t;

typedef struct
{
    float xyz[3];
    float bboxsize;
    short model;
    short plane;
    short attribute;
    short u2;
    short serialID;
    byte angle;
    byte flags;
    byte u6;
} mapinsttype3_t;

//
// CoerceFloat
//

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

//
// ClampAngle
//

static void ClampAngle(float *angle)
{
    float an = *angle;
    float pi = (float)M_PI;

    if(an < -pi) for(; an < -pi; an = an + (pi*2));
    if(an >  pi) for(; an >  pi; an = an - (pi*2));

    *angle = an;
}

//
// GetAttribute
//

static attribute_t *GetAttribute(int index)
{
    return (attribute_t*)(attribdata + 8 +
        (index * Com_GetCartOffset(attribdata, CHUNK_ATTRIB_SIZE, 0)));
}

//
// GetArea
//

static maparea_t *GetArea(int index)
{
    if(index <= -1)
        index = 0;

    return (maparea_t*)(areadata + 8 + (index *
        Com_GetCartOffset(areadata, CHUNK_AREAS_SIZE, 0)));
}

//
// GetWarpPoint
//

static warp_t* GetWarpPoint(int tag)
{
    int count;
    int i;
    int *warptags;
    warp_t* warp;

    warptags    = (int*)Com_GetCartData(warpdata, CHUNK_WARP_TAGS, 0);
    warp        = (warp_t*)(Com_GetCartData(warpdata, CHUNK_WARP_INFO, 0) + 8);
    count       = Com_GetCartOffset((byte*)warptags, CHUNK_WARPTAGS_COUNT, 0);

    for(i = 0; i < count; i++)
    {
        if(warptags[2 + i] == tag)
            return warp + i;
    }
    
    return NULL;
}

//
// GetObjectType
//

static short GetObjectType(int index)
{
    return typedata[index + 4];
}

//
// IsAPickup
//

static dboolean IsAPickup(int model)
{
    return (GetObjectType(model) >= OT_PICKUP_SMALLHEALTH &&
            GetObjectType(model) <= OT_PICKUP_CHRONOPIECE8);
}

//
// ProcessTextureOverrides
//

static void ProcessTextureOverrides(short model, int textureid)
{
    int ncount;
    int i;
    int j;
    int k;

    if(textureid == -1 || textureid == 0)
        return;

    ncount = model_nodeCount[model];

    if(ncount <= 0)
        return;

#ifdef OLD_FORMAT
    Com_Strcat("textureSwaps =\n");
#else
    Com_Strcat("textureSwaps\n");
#endif
    Com_Strcat("{\n");

    for(j = 0; j < ncount; j++)
    {
        // node block start
        Com_Strcat("{\n");

        for(k = 0; k < model_meshCount[model][j]; k++)
        {
            // mesh block start
            Com_Strcat("{ ");

            for(i = 0; i < section_count[model][j][k]; i++)
            {
                int texid = section_textures[model][j][k][i];

                if(textureid >= texindexes[texid])
                {
                    Com_Strcat("\"-\" ");
                }
                else
                {
                    Com_Strcat("\"textures/tex%04d_%02d.tga\" ",
                        texid, textureid);
                }
            }

            // mesh block end
            Com_Strcat("}\n");
        }

        // node block end
        Com_Strcat("}\n");
    }

    Com_Strcat("}\n");
}

//
// ProcessHeader
//

static void ProcessHeader(byte *data, int index)
{
    byte *header = Com_GetCartData(data, CHUNK_LVROOT_HEADER, 0);
    int i;
    float d;
    float ambient[3];
    float diffuse[3];
    int ci;
    float x;
    float y;
    float z;

    ci = 0;

    for(i = 0; i < 64; i++)
    {
        float c;
        float cf;
        byte cv;
        short val;

        c = ci * 0.015873f * 255.0f;

        if(c <= 255.0f)
            cf = c;
        else
            cf = 255.0f;

        if(cf >= 0.0f)
        {
            if(c > 255.0f)
                c = 255.0f;
        }
        else
            c = 0.0f;

         val = *(header + 10) * (int)c + (*(header + 13) << 8);
         if(val <= 0xFF00)
             cv = (val >> 8);
         else
             cv = 0xff;

         lightheader_table1[i] = cv;

         val = *(header + 11) * (int)c + (*(header + 14) << 8);
         if(val <= 0xFF00)
             cv = (val >> 8);
         else
             cv = 0xff;

         lightheader_table2[i] = cv;

         val = *(header + 12) * (int)c + (*(header + 15) << 8);
         if(val <= 0xFF00)
             cv = (val >> 8);
         else
             cv = 0xff;

         lightheader_table3[i] = cv;

         ci = i;
    }

    for(i = 0; i < 3; i++)
    {
        byte c1;
        byte c2;
        byte c3;

        c1 = *(header + 13 + i);
        c2 = *(header + 10 + i);
        c3 = *(header + 17 + i);

        lvcolortable1[i] = c1;
        lvcolortable2[i] = c1;
        lvcolortable3[i] = c2;
        lvcolortable4[i] = c2;
        lvcolortable5[i] = c3;
    }

    x = *(char*)(header + 17);
    y = *(char*)(header + 18);
    z = *(char*)(header + 19);

    d = (float)sqrt(x*x+y*y+z*z);

    if(d != 0.0)
    {
        float factor = 1.0f / d;
        x = x * factor;
        y = y * factor;
        z = z * factor;
    }

    diffuse[0] = (float)lvcolortable3[0] / 255.0f;
    diffuse[1] = (float)lvcolortable3[1] / 255.0f;
    diffuse[2] = (float)lvcolortable3[2] / 255.0f;

    ambient[0] = (float)lvcolortable1[0] / 255.0f;
    ambient[1] = (float)lvcolortable1[1] / 255.0f;
    ambient[2] = (float)lvcolortable1[2] / 255.0f;

    Com_Strcat("global_light_position = { %f %f %f }\n", x, y, z);
    Com_Strcat("global_light_color = { %f %f %f }\n",
        diffuse[0], diffuse[1], diffuse[2]);
    Com_Strcat("global_light_ambience = { %f %f %f }\n",
        ambient[0], ambient[1], ambient[2]);
    Com_Strcat("global_model_ambience = { %f %f %f }\n",
        1.5f+ambient[0], 1.5f+ambient[1], 1.5f+ambient[2]);
}

//
// ProcessPoints
//

static void ProcessPoints(byte *buffer, byte *data)
{
    int* rover = (int*)(data + 8);
    int count = Com_GetCartOffset(data, CHUNK_POINTS_COUNT, 0);
    int i;
#ifndef FORMAT_BINARY
    Com_Strcat("points =\n");
    Com_Strcat("{\n");
#endif

    for(i = 0; i < count; i++)
    {
#ifndef FORMAT_BINARY
        Com_Strcat("%f %f %f %f\n",
            (float)rover[0] * 0.029296875f,
            (float)rover[1] * 0.029296875f,
            (float)rover[2] * 0.029296875f,
            (float)rover[3] * 0.029296875f);
#else
        Com_WriteBufferFloat(buffer, (float)rover[0] * 0.029296875f);
        Com_WriteBufferFloat(buffer, (float)rover[1] * 0.029296875f);
        Com_WriteBufferFloat(buffer, (float)rover[2] * 0.029296875f);
        Com_WriteBufferFloat(buffer, (float)rover[3] * 0.029296875f);
#endif

        rover += 4;
        Com_UpdateDataProgress();
    }

#ifndef FORMAT_BINARY
    Com_Strcat("}\n\n");
#endif
}

//
// ProcessLeafs
//

static void ProcessLeafs(byte *buffer, byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_LEAFS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_LEAFS_COUNT, 0);
    int i;

#ifndef FORMAT_BINARY
    Com_Strcat("leafs = // [area, flags, pt1, pt2, pt3, link1, link2, link3]\n");
    Com_Strcat("{\n");
#endif

    for(i = 0; i < count; i++)
    {
        mapleaf_t *leaf = (mapleaf_t*)(data + 8 + (i * size));

#ifdef FORMAT_BINARY
        Com_WriteBuffer16(buffer, leaf->area_id);
        Com_WriteBuffer16(buffer, leaf->flags);
        Com_WriteBuffer16(buffer, leaf->pt1);
        Com_WriteBuffer16(buffer, leaf->pt2);
        Com_WriteBuffer16(buffer, leaf->pt3);
        Com_WriteBuffer16(buffer, leaf->link1);
        Com_WriteBuffer16(buffer, leaf->link2);
        Com_WriteBuffer16(buffer, leaf->link3);
#else
        Com_Strcat("%i %i %i %i %i %i %i %i\n",
            leaf->area_id, leaf->flags, leaf->pt1, leaf->pt2, leaf->pt3,
            leaf->link1, leaf->link2, leaf->link3);
#endif
        Com_UpdateDataProgress();
    }

#ifndef FORMAT_BINARY
    Com_Strcat("}\n\n");
#endif
}

//
// ProcessZones
//

static void ProcessZones(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_ZONE_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_ZONE_COUNT, 0);
    int i;

    Com_Strcat("zonebounds =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        mapzone_t *zone = (mapzone_t*)(data + 8 + (i * size));

        Com_Strcat("%f %f %f %f %i %i %i\n",
            zone->left, zone->bottom, zone->right, zone->top,
            zone->unknown1, zone->unknown2, zone->unknown3);
    }

    Com_Strcat("}\n\n");
}

//
// ProcessAreas
//

static void ProcessAreas(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_AREAS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_AREAS_COUNT, 0);
    int i;
    int total;

    Com_Strcat("areas[%i] =\n{\n", count);
    total = 1;

    for(i = 0; i < count; i++)
    {
        maparea_t *area = (maparea_t*)(data + 8 + (i * size));

        total = 1;

        if(area->flags & AAF_CRAWL)     total++;
        if(area->flags & AAF_EVENT)     total++;
        if(area->flags & AAF_DAMAGE)    total++;
        if(area->flags & AAF_DRAWSKY)   total++;
        if(area->flags & AAF_TELEPORT)  total++;
        if(area->ambience > 0 && area->ambience <= 10) total++;

        Com_Strcat("{\n");
        Com_Strcat("flags = %i\n", area->flags);
        Com_Strcat("waterplane = %f\n", area->waterheight);
        if(area->flags & AAF_EVENT)
        {
            if(area->args2 > 0)
            {
                Com_Strcat("triggerSound = \"sounds/shaders/%s.ksnd\"\n",
                    sndfxnames[area->args2]);
            }
            Com_Strcat("targetID = %i\n", area->args4);
        }

        Com_Strcat("components[%i] =\n", total);
        Com_Strcat("{\n");

        if(area->flags & AAF_CRAWL)
        {
            Com_Strcat("BeginObject = \"ComponentAreaCrawl\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
        }

        if(area->flags & AAF_EVENT)
        {
            if(area->args4 <= 8)
            {
                Com_Strcat("BeginObject = \"ComponentAreaMessage\"\n");
                Com_Strcat("{ \"active\" : true,\n");
                switch(area->args4)
                {
                case 2:
                    Com_Strcat("\"message\" : [\"access to level 2\", \"the jungle\"]");
                    break;
                case 3:
                    Com_Strcat("\"message\" : [\"access to level 3\", \"the ancient city\"]");
                    break;
                case 4:
                    Com_Strcat("\"message\" : [\"access to level 4\", \"the ruins\"]");
                    break;
                case 5:
                    Com_Strcat("\"message\" : [\"access to level 5\", \"the catacombs\"]");
                    break;
                case 6:
                    Com_Strcat("\"message\" : [\"access to level 6\", \"the treetop village\"]");
                    break;
                case 7:
                    Com_Strcat("\"message\" : [\"access to level 7\", \"the lost land\"]");
                    break;
                case 8:
                    Com_Strcat("\"message\" : [\"access to level 8\", \"the final confrontation\"]");
                    break;
                }
                Com_Strcat(" }\nEndObject\n");
            }
            else
            {
                Com_Strcat("BeginObject = \"ComponentAreaTrigger\"\n");
                Com_Strcat("{ \"active\" : true,\n");
                if(area->args2 > 0)
                {
                    Com_Strcat("\"triggerSnd\" : \"%s\",\n",
                        sndfxnames[area->args2]);
                }
                Com_Strcat("\"targetID\" : %i }\n", area->args4);
                Com_Strcat("EndObject\n");
            }
        }

        if(area->flags & AAF_DAMAGE)
        {
            Com_Strcat("BeginObject = \"ComponentAreaDamage\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
        }

        if(area->flags & AAF_DRAWSKY)
        {
            Com_Strcat("BeginObject = \"ComponentAreaSky\"\n");
            Com_Strcat("{ \"active\" : true,\n");
            Com_Strcat("\"skyYPlane\" : %f }\n", area->skyheight);
            Com_Strcat("EndObject\n");
        }

        if(area->flags & AAF_TELEPORT)
        {
            warp_t *warp = GetWarpPoint(area->args1);
            float yaw = warp->angle + (float)M_PI;

            ClampAngle(&yaw);

            Com_Strcat("BeginObject = \"ComponentAreaTeleport\"\n");
            Com_Strcat("{ \"active\" : true,\n");
            Com_Strcat("\"x\" : %f,\n", warp->dest[0]);
            Com_Strcat("\"y\" : %f,\n", warp->dest[1]);
            Com_Strcat("\"z\" : %f,\n", warp->dest[2]);
            Com_Strcat("\"yaw\" : %f,\n", yaw);
            Com_Strcat("\"plane\" : %i,\n", warp->u1);
            Com_Strcat("\"level\" : %i }\n", warp->level);
            Com_Strcat("EndObject\n");
        }

        switch(area->ambience)
        {
        case 1:
            Com_Strcat("BeginObject = \"AmbienceJungle\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 2:
            Com_Strcat("BeginObject = \"AmbienceSwamp\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 3:
            Com_Strcat("BeginObject = \"AmbienceCave\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 4:
            Com_Strcat("BeginObject = \"AmbienceCatcomb01\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 5:
            Com_Strcat("BeginObject = \"AmbienceLostLand\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 6:
            Com_Strcat("BeginObject = \"AmbienceCatcomb02\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 7:
            Com_Strcat("BeginObject = \"AmbienceCatcomb01\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 8:
            Com_Strcat("BeginObject = \"AmbienceRuins\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 9:
            Com_Strcat("BeginObject = \"AmbienceValley\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        case 10:
            Com_Strcat("BeginObject = \"AmbienceVillage\"\n");
            Com_Strcat("{ \"active\" : true }\n");
            Com_Strcat("EndObject\n");
            break;
        default:
            break;
        }

        Com_Strcat("BeginObject = \"ComponentAreaFog\"\n");
        Com_Strcat("{ \"fog_Color_r\" : %i,\n", area->fogrgba[0]);
        Com_Strcat("\"fog_Color_g\" : %i,\n", area->fogrgba[1]);
        Com_Strcat("\"fog_Color_b\" : %i,\n", area->fogrgba[2]);
        Com_Strcat("\"fog_Far\" : %f }\n", area->fogzfar);
        Com_Strcat("EndObject\n");
        Com_Strcat("}\n");
        Com_Strcat("}\n");

        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n");
}

//
// ProcessNavigation
//

#ifdef OLD_FORMAT
static void ProcessNavigation(byte *data, int index)
{
    byte *rncdata;
    byte *info;
    byte *points;
    byte *leafs;
    //byte *zones;
    int size;
    int outsize;
    char name[256];
    byte *buffer = NULL;
#ifdef FORMAT_BINARY
    int *dataSize;
    int *lookup1;
    int *lookup2;
#endif

#ifndef FORMAT_BINARY
    Com_StrcatClear();

#else
    buffer = Com_Alloc(0x850000);
    com_fileoffset = 0;
    Com_WriteBuffer32(buffer, 1296843595);

    dataSize = (int*)&buffer[com_fileoffset];
    Com_WriteBuffer32(buffer, 0);
    lookup1 = (int*)&buffer[com_fileoffset];
    Com_WriteBuffer32(buffer, 0);
    lookup2 = (int*)&buffer[com_fileoffset];
    Com_WriteBuffer32(buffer, 0);

    *lookup1 = com_fileoffset;
#endif

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_INFO, &size);
    info = RNC_ParseFile(rncdata, size, &outsize);

    points = Com_GetCartData(info, CHUNK_LEVELINFO_POINTS, &size);
    DC_DecodeData(points, decode_buffer, 0);
    memcpy(points, decode_buffer, size);

    leafs = Com_GetCartData(info, CHUNK_LEVELIFNO_LEAFS, &size);
    DC_DecodeData(leafs, decode_buffer, 0);
    memcpy(leafs, decode_buffer, size);

    //zones = Com_GetCartData(info, CHUNK_LEVELINFO_ZONEBOUNDS, &size);
    //DC_DecodeData(zones, decode_buffer, 0);
    //memcpy(zones, decode_buffer, size);

#ifndef FORMAT_BINARY
    Com_Strcat("numpoints = %i\n", Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0));
    Com_Strcat("numleafs = %i\n\n", Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));
    //Com_Strcat("numzonebounds = %i\n\n", Com_GetCartOffset(zones, CHUNK_ZONE_COUNT, 0));
#endif

    Com_SetDataProgress(Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0) +
        Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));

#ifdef FORMAT_BINARY
    Com_WriteBuffer32(buffer, Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0));
#endif
    ProcessPoints(buffer, points);

#ifdef FORMAT_BINARY
    *lookup2 = com_fileoffset;
    Com_WriteBuffer32(buffer, Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));
#endif

    ProcessLeafs(buffer, leafs);
    //ProcessZones(zones);

#ifndef FORMAT_BINARY
    sprintf(name, "maps/map%02d/map%02d.kcm", index, index);
    Com_StrcatAddToFile(name);
#endif

    Com_Free(&info);
    
#ifdef FORMAT_BINARY
    *dataSize = (com_fileoffset - 8);
    
    sprintf(name, "maps/map%02d/map%02d.kclm", index, index);
    PK_AddFile(name, buffer, com_fileoffset, true);

    Com_Free(&buffer);
#endif
}
#else
static void ProcessNavigation(byte *data, int index)
{
    byte *rncdata;
    byte *info;
    byte *points;
    byte *leafs;
    int size;
    int outsize;
    char name[256];
    byte *buffer = NULL;
    int leafSize;
    int leafCount;
    int i;
    int *ptRover;
    int numPoints;

    Com_StrcatClear();

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_INFO, &size);
    info = RNC_ParseFile(rncdata, size, &outsize);

    points = Com_GetCartData(info, CHUNK_LEVELINFO_POINTS, &size);
    DC_DecodeData(points, decode_buffer, 0);
    memcpy(points, decode_buffer, size);

    leafs = Com_GetCartData(info, CHUNK_LEVELIFNO_LEAFS, &size);
    DC_DecodeData(leafs, decode_buffer, 0);
    memcpy(leafs, decode_buffer, size);

    Com_Strcat("model { numnodes = 1 nodes {{ numchildren = 0 numvariants = 1 variants = { 0 } numgroups = 1\n");
    Com_Strcat("groups {{ numsections = 1 sections {{ texture = \"textures/default.tga\" rgba = 0 0 0 255\n");

    Com_SetDataProgress(Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0) +
        Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));

    leafSize = Com_GetCartOffset(leafs, CHUNK_LEAFS_SIZE, 0);
    leafCount = Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0);

    Com_Strcat("numtriangles = %i\n", leafCount);
    Com_Strcat("triangles = {\n");

    for(i = 0; i < leafCount; i++)
    {
        mapleaf_t *leaf = (mapleaf_t*)(leafs + 8 + (i * leafSize));

        Com_Strcat("%i %i %i\n", leaf->pt1, leaf->pt2, leaf->pt3);
        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n");

    ptRover = (int*)(points + 8);
    numPoints = Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0);

    Com_Strcat("numvertices = %i\n", numPoints);
    Com_Strcat("vertices = {\n");

    Com_Strcat("xyz = {\n");
    for(i = 0; i < numPoints; i++)
    {
        Com_Strcat("%f %f %f\n",
            (float)ptRover[0] * 0.029296875f,
            (float)ptRover[1] * 0.029296875f,
            (float)ptRover[2] * 0.029296875f);

        ptRover += 4;
        Com_UpdateDataProgress();
    }
    Com_Strcat("}\n");

    Com_Strcat("coords = {\n");
    for(i = 0; i < numPoints; i++)
        Com_Strcat("0 0\n");
    Com_Strcat("}\n");

    Com_Strcat("normals = {\n");
    for(i = 0; i < numPoints; i++)
        Com_Strcat("0 0 0\n");
    Com_Strcat("}}}}}}}}}\n");

    sprintf(name, "maps/map%02d/mapCollision%02d.kmesh", index, index);
    Com_StrcatAddToFile(name);

    Com_Free(&info);
}
#endif

//
// ProcessAIBehavior
//

static void ProcessAIBehavior(mapactor_t *actor, attribute_t *attr)
{
    if(attr->target > 0)
        Com_Strcat("\"targetID\" : %i,\n", attr->target);

    Com_Strcat("\"health\" : %i,\n", attr->health);
    Com_Strcat("\"meleeRange\" : %f,\n", (float)sqrt(attr->u11));
    Com_Strcat("\"rangeDistance\" : %f,\n", (float)sqrt(attr->u5));
    Com_Strcat("\"dropItemFlags\" : %i,\n", attr->behavior2 & 0x3FFF);
    Com_Strcat("\"alligenceID\" : %i,\n", actor->type);

    if(attr->behavior1 & 0x200000)
        Com_Strcat("\"bCanMelee\" : false,\n");

    if(attr->behavior1 & 0x10000)
        Com_Strcat("\"bCanTeleport\" : true,\n");

    if(attr->behavior1 & 0x42 || attr->behavior2 & 0xC3060000 ||
        attr->behavior4 & 0x3)
    {
        int attackFlags = 0;

        Com_Strcat("\"bCanRangeAttack\" : true,\n");

        if(attr->behavior1 & 0x2)
            attackFlags |= 0x1;
        if(attr->behavior1 & 0x40)
            attackFlags |= 0x2;

        if(attr->behavior2 & 0x40000000)
            attackFlags |= 0x4;
        if(attr->behavior2 & 0x80000000)
            attackFlags |= 0x8;
        if(attr->behavior2 & 0x1000000)
            attackFlags |= 0x10;
        if(attr->behavior2 & 0x2000000)
            attackFlags |= 0x20;
        if(attr->behavior2 & 0x20000)
            attackFlags |= 0x40;
        if(attr->behavior2 & 0x40000)
            attackFlags |= 0x80;

        if(attr->behavior4 & 0x1)
            attackFlags |= 0x100;
        if(attr->behavior4 & 0x2)
            attackFlags |= 0x200;

        Com_Strcat("\"rangedAttacks\" : %i,\n", attackFlags);
    }
}

//
// ProcessScriptedActorProperties
//

static void ProcessScriptedActorProperties(mapactor_t *actor, attribute_t *attr)
{
    if(attr->target > 0)
        Com_Strcat("\"targetID\" : %i,\n", attr->target);

    Com_Strcat("\"triggerAnimation\" : %i,\n", attr->triggerAnim + 199);
    Com_Strcat("\"bRemoveOnCompletion\" : %i,\n", (attr->behavior3 & 0x2) != 0);
    Com_Strcat("\"triggerDelay\" : %f,\n", attr->triggerDelay);
    Com_Strcat("\"bSleepUntilTriggered\" : %i,\n",
        (attr->triggerAnim && !(attr->behavior4 & 0x4)) != 0);
}

//
// ProcessActors
//

#ifdef OLD_FORMAT
static void ProcessActors(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_ACTORS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_ACTORS_COUNT, 0);
    int i;

#ifdef FORMAT_BINARY
    Com_WriteBuffer32(kmapInfo.buffer, count);

    for(i = 0; i < count; i++)
    {
        kmapInfo.actorStride[i] = (int*)&kmapInfo.buffer[com_fileoffset];
        Com_WriteBuffer32(kmapInfo.buffer, 0);
    }
#else

    Com_Strcat("actors[%i] =\n{\n", count);
#endif

    for(i = 0; i < count; i++)
    {
        mapactor_t *actor = (mapactor_t*)(data + 8 + (i * size));
        attribute_t *attr = GetAttribute(actor->attribute);
        int variant = abs(attr->variant1);
        float bboxUnit;

#ifdef FORMAT_BINARY
        *kmapInfo.actorStride[i] = com_fileoffset;
        Com_WriteBufferString(kmapInfo.buffer, va("Actor_%i", actorTally++));
        Com_WriteBuffer8(kmapInfo.buffer, 1);
#else
        Com_Strcat("{\n");
        Com_Strcat("name = \"Actor_%i\"\n", actorTally++);
#endif
        
        switch(actor->type)
        {
        case OT_TUROK:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 10\n");
            Com_Strcat("physics = 367\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"ComponentTurokPlayer\"\n");
            Com_Strcat("{\n");
			Com_Strcat("\"playerID\" : 0\n");
			Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#else
            Com_WriteBufferString(kmapInfo.buffer, "ComponentTurokPlayer");
            Com_WriteBufferString(kmapInfo.buffer, "{\"playerID\":0}");
#endif
            break;

        case OT_AI_GRUNT:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"TurokAIGrunt\"\n");
            Com_Strcat("{\n");

            if(attr->behavior2 & 0x200000)
                Com_Strcat("\"bTwoHanded\" : true,\n");

            ProcessAIBehavior(actor, attr);

            Com_Strcat("\"active\" : 1\n");
			Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#else
            Com_WriteBufferString(kmapInfo.buffer, "TurokAIGrunt");
            Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
            break;

        case OT_AI_RAPTOR:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIRaptor\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;

        case OT_AI_DINOSAUR1:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIDimetrodon\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;

        case OT_AI_STALKER:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIStalker\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;
        case OT_AI_PURLIN:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIPurlin\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;

        case OT_AI_ALIEN:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIAlien\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;

            case OT_AI_MECH:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 591\n");
            Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIMech\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#endif
            break;

        case OT_DYNAMIC_GENERIC:
            Com_Strcat("classFlags = 2\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"ComponentScriptedTrap\"\n");
            Com_Strcat("{\n");
            Com_Strcat("\"active\" : 1\n");
		    Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
            break;

        case OT_AI_ANIMAL:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
#endif
            switch(actor->model)
            {
            case 325:
#ifndef FORMAT_BINARY
                Com_Strcat("physics = 591\n");
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIAnimal\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "AnimalDeer");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
                break;
            case 442:
#ifndef FORMAT_BINARY
                Com_Strcat("physics = 77\n");
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"ScriptedMonkey\"\n");
                Com_Strcat("{\n");
                ProcessScriptedActorProperties(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "ScriptedMonkey");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
                break;
            case 471:
#ifndef FORMAT_BINARY
                Com_Strcat("physics = 16\n");
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"ScriptedBird\"\n");
                Com_Strcat("{\n");
                ProcessScriptedActorProperties(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "ScriptedBird");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
                break;
            case 474:
#ifndef FORMAT_BINARY
                Com_Strcat("physics = 591\n");
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIAnimal\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "AnimalBoar");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
                break;
            }
            break;

        case OT_AI_FISH:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 3\n");
            Com_Strcat("physics = 1357\n");
            Com_Strcat("mass = 0.0\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"TurokAIFish\"\n");
            Com_Strcat("{\n");
            ProcessAIBehavior(actor, attr);
            Com_Strcat("\"active\" : 1\n");
		    Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#else
            Com_WriteBufferString(kmapInfo.buffer, "TurokAIFish");
            Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
            break;

        case OT_AI_INSECT:
            switch(actor->model)
            {
            case 330:
#ifndef FORMAT_BINARY
                Com_Strcat("classFlags = 3\n");
                Com_Strcat("physics = 591\n");
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"TurokAIInsect\"\n");
                Com_Strcat("{\n");
                ProcessAIBehavior(actor, attr);
                Com_Strcat("\"active\" : 1\n");
		        Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
                break;
#endif
            }
            break;

        case OT_DYNAMIC_MOVER:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 2\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"ComponentPillarMover\"\n");
            Com_Strcat("{\n");
            Com_Strcat("\"moveSpeed\" : %i,\n", attr->u22);
            Com_Strcat("\"moveAmount\" : %i,\n", attr->variant2);
            if(attr->target >= 0)
            {
                Com_Strcat("\"moveSound\" : \"%s\"\n",
                    sndfxnames[attr->target]);
            }
			Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#else
            Com_WriteBufferString(kmapInfo.buffer, "ComponentPillarMover");
            if(attr->target >= 0)
            {
                Com_WriteBufferString(kmapInfo.buffer,
                    va("{\"moveSpeed\":%i,\"moveAmount\":%i,\"moveSound\":\"%s\"}",
                    attr->u22, attr->variant2, sndfxnames[attr->target]));
            }
            else
            {
                Com_WriteBufferString(kmapInfo.buffer,
                    va("{\"moveSpeed\":%i,\"moveAmount\":%i}", attr->u22, attr->variant2));
            }
#endif
            break;

        case OT_DYNAMIC_DOOR:
#ifndef FORMAT_BINARY
            Com_Strcat("classFlags = 2\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
            Com_Strcat("BeginObject = \"ComponentDoor\"\n");
            Com_Strcat("{\n");
            Com_Strcat("\"delayTime\" : %i,\n", 15*attr->u22);
            Com_Strcat("\"active\" : 1\n");
			Com_Strcat("}\n");
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#else
            Com_WriteBufferString(kmapInfo.buffer, "ComponentDoor");
            Com_WriteBufferString(kmapInfo.buffer, "{\"active\":1}");
#endif
            break;

        default:
            switch(actor->model)
            {
            case 26:
                Com_Strcat("components[1] =\n");
                Com_Strcat("{\n");
                Com_Strcat("BeginObject = \"ComponentEmitter\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"fx\" : \"fx/ambience_tall_fire2.kfx\",\n");
                Com_Strcat("\"active\" : 1\n");
			    Com_Strcat("}\n");
                Com_Strcat("EndObject\n");
                Com_Strcat("}\n");
                break;
            }
            break;
        }

#ifndef FORMAT_BINARY
        Com_Strcat("mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            actor->model, actor->model);

        Com_Strcat("bounds = { %f %f %f %f %f %f }\n",
            mdlboxes[actor->model][0],
            mdlboxes[actor->model][1],
            mdlboxes[actor->model][2],
            mdlboxes[actor->model][3],
            mdlboxes[actor->model][4],
            mdlboxes[actor->model][5]);

        bboxUnit = (float)sqrt(
            mdlboxes[actor->model][0]*mdlboxes[actor->model][0]+
            mdlboxes[actor->model][2]*mdlboxes[actor->model][2]+
            mdlboxes[actor->model][3]*mdlboxes[actor->model][3]+
            mdlboxes[actor->model][5]*mdlboxes[actor->model][5]);

        if(GetAttribute(actor->attribute)->texture != -1)
            ProcessTextureOverrides(actor->model, GetAttribute(actor->attribute)->texture);

        Com_Strcat("bCollision = %i\n",
            attr->behavior1 & 1);
        Com_Strcat("bStatic = 0\n");

        if(actor->flags & 1)
            Com_Strcat("bOrientOnSlope = 0\n");
        else
            Com_Strcat("bOrientOnSlope = 1\n");

        Com_Strcat("plane = %i\n", actor->leaf);
        Com_Strcat("origin = { %f %f %f }\n",
            actor->xyz[0], actor->xyz[1], actor->xyz[2]);
        Com_Strcat("scale = { %f %f %f }\n",
            actor->scale[0], actor->scale[1], actor->scale[2]);
        Com_Strcat("angles = { %f 0.0 0.0 }\n",
            ((((-(float)actor->angle / 180.0f) * M_RAD) + M_PI) / M_RAD) * M_RAD);
        Com_Strcat("radius = %f\n", attr->width);
        Com_Strcat("height = %f\n", attr->height);
        Com_Strcat("targetID = %i\n", attr->tid);
        Com_Strcat("centerheight = %f\n", attr->centerHeight);
        Com_Strcat("viewheight = %f\n", attr->viewHeight);
        Com_Strcat("modelVariant = %i\n", variant);
        Com_Strcat("cullDistance = %f\n", bboxUnit + 4096);
        Com_Strcat("tickDistance = %f\n", attr->u1);

        Com_Strcat("}\n");
#else
        Com_WriteBufferString(kmapInfo.buffer,
            va("models/mdl%03d/mdl%03d.kmesh", actor->model, actor->model));

        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][0]);
        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][1]);
        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][2]);
        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][3]);
        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][4]);
        Com_WriteBufferFloat(kmapInfo.buffer, mdlboxes[actor->model][5]);

        Com_WriteBuffer8(kmapInfo.buffer, attr->behavior1  & 1);
        Com_WriteBuffer8(kmapInfo.buffer, 0);

        Com_WriteBuffer8(kmapInfo.buffer, actor->flags  & 1);

        Com_WriteBuffer16(kmapInfo.buffer, actor->leaf);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->xyz[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->xyz[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->xyz[2]);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->scale[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->scale[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, actor->scale[2]);
        Com_WriteBufferFloat(kmapInfo.buffer,
            (float)(((((-(float)actor->angle / 180.0f) * M_RAD) + M_PI) / M_RAD) * M_RAD));
        Com_WriteBufferFloat(kmapInfo.buffer, 0);
        Com_WriteBufferFloat(kmapInfo.buffer, 0);
        Com_WriteBufferFloat(kmapInfo.buffer, attr->width);
        Com_WriteBufferFloat(kmapInfo.buffer, attr->height);
        Com_WriteBuffer32(kmapInfo.buffer, attr->tid);
        Com_WriteBufferFloat(kmapInfo.buffer, attr->centerHeight);
        Com_WriteBufferFloat(kmapInfo.buffer, attr->viewHeight);

        Com_WriteBufferPad4(kmapInfo.buffer);
#endif

        Com_UpdateDataProgress();
    }

#ifndef FORMAT_BINARY
    Com_Strcat("}\n");
#endif
}
#else
static void ProcessActors(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_ACTORS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_ACTORS_COUNT, 0);
    int i;

    for(i = 0; i < count; i++)
    {
        mapactor_t *actor = (mapactor_t*)(data + 8 + (i * size));
        attribute_t *attr = GetAttribute(actor->attribute);
        int variant = abs(attr->variant1);
        
        switch(actor->type)
        {
        case OT_TUROK:
            Com_Strcat("actor \"kexPlayerPuppet\"\n");
            Com_Strcat("{\n");
            Com_Strcat("origin { %f %f %f }\n",
                actor->xyz[0], actor->xyz[1], actor->xyz[2]);
            Com_Strcat("scale { %f %f %f }\n",
                actor->scale[0], actor->scale[1], actor->scale[2]);
            Com_Strcat("angles { %f 0.0 0.0 }\n",
                ((((-(float)actor->angle / 180.0f) * M_RAD) + M_PI) / M_RAD) * M_RAD);
            Com_Strcat("bStatic 0\n");
            Com_Strcat("mesh \"models/mdl%03d/mdl%03d.kmesh\"\n",
                actor->model, actor->model);
            Com_Strcat("bounds { %f %f %f } { %f %f %f }\n",
                mdlboxes[actor->model][0],
                mdlboxes[actor->model][1],
                mdlboxes[actor->model][2],
                mdlboxes[actor->model][3],
                mdlboxes[actor->model][4],
                mdlboxes[actor->model][5]);
            Com_Strcat("radius %f\n", attr->width);
            Com_Strcat("height %f\n", attr->height);
            Com_Strcat("centerheight %f\n", attr->centerHeight);
            Com_Strcat("viewheight %f\n", attr->viewHeight);
            Com_Strcat("id 0\n");
            Com_Strcat("component \"TurokPlayer\"\n");
            Com_Strcat("}\n");
        }

        Com_UpdateDataProgress();
    }
}
#endif

//
// ProcessLevelKey
//

static void ProcessLevelKey(mapinsttype3_t *mapinst, int id)
{
#ifndef FORMAT_BINARY
    Com_Strcat("BeginObject = \"TurokPickupKey\"\n");
    Com_Strcat("{\n");
    Com_Strcat("\"active\" : true,\n");
    Com_Strcat("\"levelID\" : %i,\n", id);
    Com_Strcat("\"bits\" : %i\n",
        (GetAttribute(mapinst->attribute)->tid & 3));
    Com_Strcat("}\n"); 
#else
    Com_WriteBufferString(kmapInfo.buffer, "TurokPickupKey");
    Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true,\"levelID\":%i,\"bits\":%i}",
        id, (GetAttribute(mapinst->attribute)->tid & 3));
#endif
}

//
// ProcessGridBounds
//

static void ProcessStaticInstances2(byte *data, byte *data2);
static void ProcessInstances(byte *data, int offs);

#ifdef OLD_FORMAT
static void ProcessGridBounds(byte *data, byte *inst)
{
    byte *grid = Com_GetCartData(data, CHUNK_LEVELGRID_BOUNDS, 0);
    int size = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_SIZE, 0);
    int count = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_COUNT, 0);
    int total = count;
    int stride;
    int i;

    // some gridbounds can contain bad or empty data....UGH
    if(count > 1)
    {
        total = 0;

        for(i = 0; i < count; i++)
        {
            float tmp;
            char tmp2[32];

            int gsize;
            byte *rncdata = Com_GetCartData(inst, CHUNK_INSTGROUP_OFFSET(i), &gsize);
            byte *group = RNC_ParseFile(rncdata, gsize, 0);

            tmp = *(float*)((int*)(grid + 8 + 0  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 4  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 8  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 12 + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            if(Com_GetCartOffset(group, CHUNK_INSTANCE_SIZE, 0) &&
                Com_GetCartOffset(group, CHUNK_INSTANCE_COUNT, 0))
            {
                total++;
            }

            Com_Free(&group);
        }
    }

#ifndef FORMAT_BINARY
    Com_Strcat("gridbounds[%i] =\n{\n", total);
#else
    Com_WriteBuffer32(kmapInfo.buffer, total);
    kmapInfo.gridStride = (int**)Com_Alloc(sizeof(int*) * total);

    for(i = 0; i < total; i++)
    {
        kmapInfo.gridStride[i] = (int*)&kmapInfo.buffer[com_fileoffset];
        Com_WriteBuffer32(kmapInfo.buffer, 0);
    }
#endif

    stride = 0;

    for(i = 0; i < count; i++)
    {
        int gsize;
        byte *rncdata = Com_GetCartData(inst, CHUNK_INSTGROUP_OFFSET(i), &gsize);
        byte *group = RNC_ParseFile(rncdata, gsize, 0);

        if(Com_GetCartOffset(group, CHUNK_INSTANCE_SIZE, 0) &&
            Com_GetCartOffset(group, CHUNK_INSTANCE_COUNT, 0))
        {
            float min_x = *(float*)((int*)(grid + 8 + 0  + (i * size)));
            float min_z = *(float*)((int*)(grid + 8 + 4  + (i * size)));
            float max_x = *(float*)((int*)(grid + 8 + 8  + (i * size)));
            float max_z = *(float*)((int*)(grid + 8 + 12 + (i * size)));

#ifdef FORMAT_BINARY
            *kmapInfo.gridStride[stride++] = com_fileoffset;

            Com_WriteBufferFloat(kmapInfo.buffer, min_x);
            Com_WriteBufferFloat(kmapInfo.buffer, min_z);
            Com_WriteBufferFloat(kmapInfo.buffer, max_x);
            Com_WriteBufferFloat(kmapInfo.buffer, max_z);
#else
            Com_Strcat("{\n");
            Com_Strcat("bounds = %f %f %f %f\n", min_x, min_z, max_x, max_z);
#endif
            ProcessStaticInstances2(
                Com_GetCartData(group, CHUNK_STATICINST_GROUP2, 0),
                Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));
            //ProcessInstances(Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));
#ifndef FORMAT_BINARY
            Com_Strcat("}\n");
#endif
        }

#ifdef FORMAT_BINARY
        Com_WriteBufferPad4(kmapInfo.buffer);
#endif

        Com_Free(&group);
        Com_UpdateDataProgress();
    }

#ifndef FORMAT_BINARY
    Com_Strcat("}\n");
#endif
}
#else
static void ProcessGridBounds(byte *data, byte *inst)
{
    byte *grid = Com_GetCartData(data, CHUNK_LEVELGRID_BOUNDS, 0);
    int size = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_SIZE, 0);
    int count = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_COUNT, 0);
    int total = count;
    int stride;
    int i;

    // world collision
    Com_Strcat("gridbound { -32768 -32768 -32768 } { 32768 32768 32768 }\n");
    Com_Strcat("{\n");
    Com_Strcat("actor \"kexWorldActor\"\n");
    Com_Strcat("{\n");
    Com_Strcat("mesh \"maps/map%02d/mapCollision%02d.kmesh\"\n", levelID, levelID);
    Com_Strcat("bounds { -16384 -32768 -16384 } { 16384 32768 16384 }\n");
    Com_Strcat("bCollision 1\n");
    Com_Strcat("bHidden 1\n");
    Com_Strcat("origin { 0 0 0 }\n");
    Com_Strcat("scale { 1 1 1 }\n");
    Com_Strcat("bStatic 1\n");
    Com_Strcat("clipMesh\n");
    Com_Strcat("{\n");
    Com_Strcat("type 6\n");
    Com_Strcat("}\n");
    Com_Strcat("}\n");
    Com_Strcat("}\n");

    // some gridbounds can contain bad or empty data....UGH
    if(count > 1)
    {
        total = 0;

        for(i = 0; i < count; i++)
        {
            float tmp;
            char tmp2[32];

            int gsize;
            byte *rncdata = Com_GetCartData(inst, CHUNK_INSTGROUP_OFFSET(i), &gsize);
            byte *group = RNC_ParseFile(rncdata, gsize, 0);

            tmp = *(float*)((int*)(grid + 8 + 0  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 4  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 8  + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            tmp = *(float*)((int*)(grid + 8 + 12 + (i * size)));
            sprintf(tmp2, "%f", tmp);
            if(strstr(tmp2, "#INF"))
            {
                Com_Free(&group);
                continue;
            }

            if(Com_GetCartOffset(group, CHUNK_INSTANCE_SIZE, 0) &&
                Com_GetCartOffset(group, CHUNK_INSTANCE_COUNT, 0))
            {
                total++;
            }

            Com_Free(&group);
        }
    }

    stride = 0;

    for(i = 0; i < count; i++)
    {
        int gsize;
        byte *rncdata = Com_GetCartData(inst, CHUNK_INSTGROUP_OFFSET(i), &gsize);
        byte *group = RNC_ParseFile(rncdata, gsize, 0);

        if(Com_GetCartOffset(group, CHUNK_INSTANCE_SIZE, 0) &&
            Com_GetCartOffset(group, CHUNK_INSTANCE_COUNT, 0))
        {
            float min_x = *(float*)((int*)(grid + 8 + 0  + (i * size)));
            float min_z = *(float*)((int*)(grid + 8 + 4  + (i * size)));
            float max_x = *(float*)((int*)(grid + 8 + 8  + (i * size)));
            float max_z = *(float*)((int*)(grid + 8 + 12 + (i * size)));

            Com_Strcat("gridbound { %f -32768 %f } { %f 32768 %f }\n", min_x, min_z, max_x, max_z);
            Com_Strcat("{\n");

            ProcessStaticInstances2(
                Com_GetCartData(group, CHUNK_STATICINST_GROUP2, 0),
                Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));

            Com_Strcat("}\n");
        }

        Com_Free(&group);
        Com_UpdateDataProgress();
    }
}
#endif

//
// ProcessInstances
//

static void ProcessInstances(byte *data, int offs)
{
    int size;
    int count;
    int i;

    size = Com_GetCartOffset(data, CHUNK_INSTANCE_SIZE, 0);
    count = Com_GetCartOffset(data, CHUNK_INSTANCE_COUNT, 0);

    DC_DecodeData(data, decode_buffer, 0);
    memcpy(data, decode_buffer, (size * count) + 8);

    for(i = 0; i < count; i++)
    {
        mapinsttype3_t *mapinst = (mapinsttype3_t*)(data + 8 + (i * size));
        float bboxUnit;

#ifdef FORMAT_BINARY
        *kmapInfo.staticStride[offs + i] = com_fileoffset;

        Com_WriteBufferString(kmapInfo.buffer, va("Actor_%i", actorTally++));
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[2]);
        Com_WriteBufferFloat(kmapInfo.buffer, 0.35f);
        Com_WriteBufferFloat(kmapInfo.buffer, 0.35f);
        Com_WriteBufferFloat(kmapInfo.buffer, 0.35f);
        Com_WriteBuffer8(kmapInfo.buffer, 0);
        Com_WriteBuffer8(kmapInfo.buffer, 0);
        Com_WriteBufferFloat(kmapInfo.buffer, (mapinst->angle * ANGLE_LEVELOBJECT) * (float)M_RAD);
        Com_WriteBufferFloat(kmapInfo.buffer, 0);
        Com_WriteBufferFloat(kmapInfo.buffer, 0);
        Com_WriteBuffer32(kmapInfo.buffer, mapinst->plane);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->bboxsize);
        Com_WriteBufferString(kmapInfo.buffer,
            va("models/mdl%03d/mdl%03d.kmesh", mapinst->model, mapinst->model));
        Com_WriteBufferFloat(kmapInfo.buffer, -mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, -mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, -mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->bboxsize);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->bboxsize);
        Com_WriteBuffer8(kmapInfo.buffer, IsAPickup(mapinst->model));
        Com_WriteBuffer8(kmapInfo.buffer, IsAPickup(mapinst->model));
#else

        bboxUnit = (float)sqrt(mapinst->bboxsize*mapinst->bboxsize);

        Com_Strcat("{\n");
        Com_Strcat("name = \"Actor_%i\"\n", actorTally++);
        Com_Strcat("origin = { %f %f %f }\n",
            mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
        Com_Strcat("scale = { 0.35 0.35 0.35 }\n");
        Com_Strcat("bStatic = 0\n");
        Com_Strcat("bCollision = 0\n");
        Com_Strcat("angles = { %f 0 0 }\n",
            (mapinst->angle * ANGLE_LEVELOBJECT) * M_RAD);
        Com_Strcat("plane = %i\n", mapinst->plane);
        Com_Strcat("radius = %f\n", mapinst->bboxsize);
        Com_Strcat("height = %f\n", mapinst->bboxsize);

        if(IsAPickup(mapinst->model))
        {
            Com_Strcat("bRotor = 1\n");
            Com_Strcat("rotorSpeed = 1\n");
            Com_Strcat("rotorVector = { 0 1 0 }\n");
        }

        Com_Strcat("mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            mapinst->model, mapinst->model);
        Com_Strcat("bounds = { %f %f %f %f %f %f }\n",
            -mapinst->bboxsize, -mapinst->bboxsize, -mapinst->bboxsize,
            mapinst->bboxsize, mapinst->bboxsize, mapinst->bboxsize);
        Com_Strcat("cullDistance = %f\n", bboxUnit + 4096.0f);
#endif

        if(IsAPickup(mapinst->model))
        {
            int keyLevelID = 0;
#ifndef FORMAT_BINARY
            Com_Strcat("bTouch = 1\n");
            Com_Strcat("components[1] =\n");
            Com_Strcat("{\n");
#endif
            switch(GetObjectType(mapinst->model))
            {
            case OT_PICKUP_SMALLHEALTH:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupHealthSmall\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupHealthSmall");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif
                break;
            case OT_PICKUP_SMALLHEALTH+1:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupHealthMedium\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupHealthMedium");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_HEALTH:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupHealthLarge\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupHealthLarge");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_FULLHEALTH:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupFullHealth\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupFullHealth");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ULTRAHEALTH:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupHealthUltra\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupHealthUltra");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");     
#endif 
                break;
            case OT_PICKUP_MASK:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupMortalWound\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupMortalWound");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");       
#endif 
                break;
            case OT_PICKUP_BACKPACK:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupBackpack\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupBackpack");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_SPIRIT:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupSpiritual\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n");
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupSpiritual");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_PISTOL:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponPistol\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponPistol");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ASSAULTRIFLE:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponRifle\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponRifle");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_PULSERIFLE:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponPulseRifle\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponPulseRifle");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_SHOTGUN:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponShotgun\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponShotgun");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ASHOTGUN:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponRiotgun\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponRiotgun");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_MINIGUN:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponMinigun\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponMinigun");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_GRENADELAUNCHER:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponGrenadeLauncher\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponGrenadeLauncher");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ALIENWEAPON:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponAlienRifle\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponAlienRifle");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ROCKETLAUNCHER:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponRocketLauncher\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponRocketLauncher");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ACCELERATOR:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponAccelerator\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponAccelerator");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_CANNON:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupWeaponFusion\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupWeaponFusion");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_QUIVER2:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoArrows\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoArrows");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ARROWS:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoTekArrows\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoTekArrows");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_QUIVER1:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoTekArrowsPack\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoTekArrowsPack");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_CLIP:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoClip\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoClip");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_CLIPBOX:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoClipBox\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoClipBox");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_SHELLS:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoShells\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoShells");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_SHELLBOX:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoShellsBox\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoShellsBox");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_EXPSHELLS:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoExpShells\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoExpShells");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_EXPSHELLBOX:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoExpShellsBox\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoExpShellsBox");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_MINIGUNAMMO:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoMiniGun\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoMiniGun");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_GRENADE:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoGrenades\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoGrenades");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_GRENADEBOX:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoGrenadesBox\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoGrenadesBox");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_SMALLCELL:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoCell\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoCell");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_CELL:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoCellLarge\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoCellLarge");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ROCKET:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoRockets\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoRockets");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_FUSIONCELL:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupAmmoCharges\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupAmmoCharges");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_ARMOR:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupArmor\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupArmor");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            case OT_PICKUP_COIN1:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupLifeForce\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true,\n");
                Com_Strcat("\"amount\" : 1\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupLifeForce");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true,\"amount\":1}");
#endif 
                break;
            case OT_PICKUP_COIN10:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickupLifeForce\"\n");
                Com_Strcat("{\n");
                Com_Strcat("\"active\" : true,\n");
                Com_Strcat("\"amount\" : 10\n");
                Com_Strcat("}\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickupLifeForce");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true,\"amount\":10}");
#endif 
                break;
            case OT_PICKUP_KEY1:
                ProcessLevelKey(mapinst, 2);
                break;
            case OT_PICKUP_KEY2:
                ProcessLevelKey(mapinst, 3);
                break;
            case OT_PICKUP_KEY3:
                ProcessLevelKey(mapinst, 4);
                break;
            case OT_PICKUP_KEY4:
                ProcessLevelKey(mapinst, 5);
                break;
            case OT_PICKUP_KEY5:
                ProcessLevelKey(mapinst, 6);
                break;
            case OT_PICKUP_KEY6:
                ProcessLevelKey(mapinst, 7);
                break;
            default:
#ifndef FORMAT_BINARY
                Com_Strcat("BeginObject = \"TurokPickup\"\n");
                Com_Strcat("{ \"active\" : true }\n"); 
#else
                Com_WriteBufferString(kmapInfo.buffer, "TurokPickup");
                Com_WriteBufferString(kmapInfo.buffer, "{\"active\":true}");
#endif 
                break;
            }
#ifndef FORMAT_BINARY
            Com_Strcat("EndObject\n");
            Com_Strcat("}\n");
#endif
        }

#ifndef FORMAT_BINARY
        Com_Strcat("}\n");
#endif
    }
}

//
// ProcessStaticInstances1
//

static void ProcessStaticInstances1(byte *data)
{
    int size;
    int count;
    int i;

    size = Com_GetCartOffset(data, CHUNK_INSTANCE_SIZE, 0);
    count = Com_GetCartOffset(data, CHUNK_INSTANCE_COUNT, 0);

    if(!size || !count)
        return;

    DC_DecodeData(data, decode_buffer, 0);
    memcpy(data, decode_buffer, (size * count) + 8);

    for(i = 0; i < count; i++)
    {
        mapinsttype1_t *mapinst = (mapinsttype1_t*)(data + 8 + (i * size));
    }
}

//
// ProcessEmitterActor
//

#ifdef OLD_FORMAT
static void ProcessEmitterActor(const char *name)
{
    Com_Strcat("components[1] =\n");
    Com_Strcat("{\n");
    Com_Strcat("BeginObject = \"ComponentEmitter\"\n");
    Com_Strcat("{\n");
    Com_Strcat("\"fx\" : \"%s\",\n", name);
    Com_Strcat("\"active\" : 1\n");
    Com_Strcat("}\n");
    Com_Strcat("EndObject\n");
    Com_Strcat("}\n");
}
#else
static void ProcessEmitterActor(mapinsttype2_t *mapinst, const char *name)
{
    float rotvec[4];

    rotvec[0] = (float)mapinst->angle[0] * ANGLE_INSTANCE;
    rotvec[1] = (float)mapinst->angle[1] * ANGLE_INSTANCE;
    rotvec[2] = (float)mapinst->angle[2] * ANGLE_INSTANCE;
    rotvec[3] = (float)mapinst->angle[3] * ANGLE_INSTANCE;

    Com_Strcat("actor \"kexEmitter\"\n");
    Com_Strcat("{\n");
    Com_Strcat("fx \"%s\"\n", name);
    Com_Strcat("bAutoStart 1\n");
    Com_Strcat("bStatic 0\n");
    Com_Strcat("origin { %f %f %f }\n",
        mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
    Com_Strcat("scale { %f %f %f }\n",
        mapinst->scale[0], mapinst->scale[1], mapinst->scale[2]);
    Com_Strcat("rotation { %f %f %f %f }\n",
        rotvec[0], rotvec[1], rotvec[2], rotvec[3]);
    Com_Strcat("bounds { %f %f %f %f %f %f }\n",
        CoerceFloat(mapinst->bbox[0]),
        CoerceFloat(mapinst->bbox[1]),
        CoerceFloat(mapinst->bbox[2]),
        CoerceFloat(mapinst->bbox[3]),
        CoerceFloat(mapinst->bbox[4]),
        CoerceFloat(mapinst->bbox[5]));
    Com_Strcat("}\n");
}
#endif

//
// ProcessStaticInstances2
//

#ifdef OLD_FORMAT
static void ProcessStaticInstances2(byte *data, byte *data2)
{
    int size;
    int count;
    int total;
    int i;

    size = Com_GetCartOffset(data, CHUNK_INSTANCE_SIZE, 0);
    count = Com_GetCartOffset(data, CHUNK_INSTANCE_COUNT, 0);
    total = (count+Com_GetCartOffset(data2, CHUNK_INSTANCE_COUNT, 0));

#ifdef FORMAT_BINARY
    kmapInfo.staticStride = (int**)Com_Alloc(sizeof(int*) * total);

    Com_WriteBuffer32(kmapInfo.buffer, total);

    for(i = 0; i < total; i++)
    {
        kmapInfo.staticStride[i] = (int*)&kmapInfo.buffer[com_fileoffset];
        Com_WriteBuffer32(kmapInfo.buffer, 0);
    }
#else
    Com_Strcat("statics[%i] =\n{\n",
        count + Com_GetCartOffset(data2, CHUNK_INSTANCE_COUNT, 0));
#endif

    DC_DecodeData(data, decode_buffer, 0);
    memcpy(data, decode_buffer, (size * count) + 8);

    for(i = 0; i < count; i++)
    {
        mapinsttype2_t *mapinst = (mapinsttype2_t*)(data + 8 + (i * size));
        float rotvec[4];
        float bboxUnit;
        dboolean bStatic = true;

#ifdef FORMAT_BINARY
        *kmapInfo.staticStride[i] = com_fileoffset;
        Com_WriteBufferString(kmapInfo.buffer, va("Actor_%i", actorTally++));
#else
        Com_Strcat("{\n");
        Com_Strcat("name = \"Actor_%i\"\n", actorTally++);
#endif

        rotvec[0] = (float)mapinst->angle[0] * ANGLE_INSTANCE;
        rotvec[1] = (float)mapinst->angle[1] * ANGLE_INSTANCE;
        rotvec[2] = (float)mapinst->angle[2] * ANGLE_INSTANCE;
        rotvec[3] = (float)mapinst->angle[3] * ANGLE_INSTANCE;

#ifdef FORMAT_BINARY
        Com_WriteBufferString(kmapInfo.buffer,
            va("models/mdl%03d/mdl%03d.kmesh", mapinst->model, mapinst->model));

        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[0]));
        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[1]));
        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[2]));
        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[3]));
        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[4]));
        Com_WriteBufferFloat(kmapInfo.buffer, CoerceFloat(mapinst->bbox[5]));
        Com_WriteBuffer8(kmapInfo.buffer, GetAttribute(mapinst->attribute)->behavior1 & 1);
        Com_WriteBuffer8(kmapInfo.buffer, 1);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->xyz[2]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->scale[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->scale[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, mapinst->scale[2]);
        Com_WriteBufferFloat(kmapInfo.buffer, rotvec[0]);
        Com_WriteBufferFloat(kmapInfo.buffer, rotvec[1]);
        Com_WriteBufferFloat(kmapInfo.buffer, rotvec[2]);
        Com_WriteBufferFloat(kmapInfo.buffer, rotvec[3]);
        Com_WriteBuffer32(kmapInfo.buffer, mapinst->plane);
        Com_WriteBufferFloat(kmapInfo.buffer, GetAttribute(mapinst->attribute)->width);
        Com_WriteBufferFloat(kmapInfo.buffer, GetAttribute(mapinst->attribute)->height);
        Com_WriteBufferPad4(kmapInfo.buffer);
#else
        Com_Strcat("mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            mapinst->model, mapinst->model);
        Com_Strcat("bounds = { %f %f %f %f %f %f }\n",
            CoerceFloat(mapinst->bbox[0]),
            CoerceFloat(mapinst->bbox[1]),
            CoerceFloat(mapinst->bbox[2]),
            CoerceFloat(mapinst->bbox[3]),
            CoerceFloat(mapinst->bbox[4]),
            CoerceFloat(mapinst->bbox[5]));

        bboxUnit = (float)sqrt(
            CoerceFloat(mapinst->bbox[0])*CoerceFloat(mapinst->bbox[0])+
            CoerceFloat(mapinst->bbox[2])*CoerceFloat(mapinst->bbox[2])+
            CoerceFloat(mapinst->bbox[3])*CoerceFloat(mapinst->bbox[3])+
            CoerceFloat(mapinst->bbox[5])*CoerceFloat(mapinst->bbox[5]));

        ProcessTextureOverrides(mapinst->model, GetAttribute(mapinst->attribute)->texture);

        switch(mapinst->model)
        {
        case 9:
            bStatic = false;
            ProcessEmitterActor("fx/fx_263.kfx");
            break;

        case 10:
            bStatic = false;
            ProcessEmitterActor("fx/fx_347.kfx");
            break;

        case 11:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_bubbles01.kfx");
            break;

        case 12:
            bStatic = false;
            ProcessEmitterActor("fx/fx_171.kfx");
            break;

        case 13:
            bStatic = false;
            ProcessEmitterActor("fx/fx_262.kfx");
            break;

        case 14:
            bStatic = false;
            ProcessEmitterActor("fx/fx_353.kfx");
            break;

        case 25:
            bStatic = false;
            ProcessEmitterActor("fx/fx_169.kfx");
            break;

        case 26:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_tall_fire2.kfx");
            break;

        case 28:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_thunderstorm.kfx");
            break;

        case 29:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_tall_fire1.kfx");
            break;

        case 32:
            bStatic = false;
            ProcessEmitterActor("fx/fx_342.kfx");
            break;

        case 33:
            bStatic = false;
            ProcessEmitterActor("fx/fx_153.kfx");
            break;

        case 34:
            bStatic = false;
            ProcessEmitterActor("fx/fx_188.kfx");
            break;

        case 35:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_underwater_bubbles.kfx");
            break;

        case 331:
            bStatic = false;
            ProcessEmitterActor("fx/fx_263.kfx");
            break;

        case 347:
            bStatic = false;
            ProcessEmitterActor("fx/fx_171.kfx");
            break;

        case 348:
            bStatic = false;
            ProcessEmitterActor("fx/fx_262.kfx");
            break;

        case 349:
            bStatic = false;
            ProcessEmitterActor("fx/ambience_waterfall_steam.kfx");
            break;

        case 412:
            bStatic = false;
            ProcessEmitterActor("fx/fx_170.kfx");
            break;

        case 555:
            bStatic = false;
            ProcessEmitterActor("fx/fx_083.kfx");
            break;
        }

        Com_Strcat("bCollision = %i\n",
            GetAttribute(mapinst->attribute)->behavior1 & 1);
        Com_Strcat("bStatic = %i\n", bStatic);
        Com_Strcat("origin = { %f %f %f }\n",
            mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
        Com_Strcat("scale = { %f %f %f }\n",
            mapinst->scale[0], mapinst->scale[1], mapinst->scale[2]);
        Com_Strcat("rotation = { %f %f %f %f }\n",
            rotvec[0], rotvec[1], rotvec[2], rotvec[3]);
        Com_Strcat("plane = %i\n", mapinst->plane);
        Com_Strcat("radius = %f\n", GetAttribute(mapinst->attribute)->width);
        Com_Strcat("height = %f\n", GetAttribute(mapinst->attribute)->height);
        Com_Strcat("cullDistance = %f\n", bboxUnit + 4096.0f);

        Com_Strcat("}\n");
#endif
    }

    ProcessInstances(data2, count);

#ifndef FORMAT_BINARY
    Com_Strcat("}\n");
#else
    free(kmapInfo.staticStride);
#endif
}
#else
static void ProcessStaticInstances2(byte *data, byte *data2)
{
    int size;
    int count;
    int total;
    int i;

    size = Com_GetCartOffset(data, CHUNK_INSTANCE_SIZE, 0);
    count = Com_GetCartOffset(data, CHUNK_INSTANCE_COUNT, 0);
    total = (count+Com_GetCartOffset(data2, CHUNK_INSTANCE_COUNT, 0));

    DC_DecodeData(data, decode_buffer, 0);
    memcpy(data, decode_buffer, (size * count) + 8);

    for(i = 0; i < count; i++)
    {
        mapinsttype2_t *mapinst = (mapinsttype2_t*)(data + 8 + (i * size));
        float rotvec[4];
        float bboxUnit;
        dboolean bStatic = true;

        switch(mapinst->model)
        {
        /*case 9:
            ProcessEmitterActor(mapinst, "fx/fx_263.kfx");
            break;

        case 10:
            ProcessEmitterActor(mapinst, "fx/fx_347.kfx");
            break;

        case 11:
            ProcessEmitterActor(mapinst, "fx/ambience_bubbles01.kfx");
            break;

        case 12:
            ProcessEmitterActor(mapinst, "fx/fx_171.kfx");
            break;

        case 13:
            ProcessEmitterActor(mapinst, "fx/fx_262.kfx");
            break;

        case 14:
            ProcessEmitterActor(mapinst, "fx/fx_353.kfx");
            break;

        case 25:
            ProcessEmitterActor(mapinst, "fx/fx_169.kfx");
            break;

        case 26:
            ProcessEmitterActor(mapinst, "fx/ambience_tall_fire2.kfx");
            break;

        case 28:
            ProcessEmitterActor(mapinst, "fx/ambience_thunderstorm.kfx");
            break;

        case 29:
            ProcessEmitterActor(mapinst, "fx/ambience_tall_fire1.kfx");
            break;

        case 32:
            ProcessEmitterActor(mapinst, "fx/fx_342.kfx");
            break;

        case 33:
            ProcessEmitterActor(mapinst, "fx/fx_153.kfx");
            break;

        case 34:
            ProcessEmitterActor(mapinst, "fx/fx_188.kfx");
            break;

        case 35:
            ProcessEmitterActor(mapinst, "fx/ambience_underwater_bubbles.kfx");
            break;

        case 331:
            ProcessEmitterActor(mapinst, "fx/fx_263.kfx");
            break;

        case 347:
            ProcessEmitterActor(mapinst, "fx/fx_171.kfx");
            break;

        case 348:
            ProcessEmitterActor(mapinst, "fx/fx_262.kfx");
            break;

        case 349:
            ProcessEmitterActor(mapinst, "fx/ambience_waterfall_steam.kfx");
            break;

        case 412:
            ProcessEmitterActor(mapinst, "fx/fx_170.kfx");
            break;

        case 555:
            ProcessEmitterActor(mapinst, "fx/fx_083.kfx");
            break;*/
        default:
            Com_Strcat("actor \"kexWorldActor\"\n");
            Com_Strcat("{\n");
            rotvec[0] = (float)mapinst->angle[0] * ANGLE_INSTANCE;
            rotvec[1] = (float)mapinst->angle[1] * ANGLE_INSTANCE;
            rotvec[2] = (float)mapinst->angle[2] * ANGLE_INSTANCE;
            rotvec[3] = (float)mapinst->angle[3] * ANGLE_INSTANCE;
            Com_Strcat("mesh \"models/mdl%03d/mdl%03d.kmesh\"\n",
                mapinst->model, mapinst->model);
            Com_Strcat("bounds { %f %f %f } { %f %f %f }\n",
                CoerceFloat(mapinst->bbox[0]),
                CoerceFloat(mapinst->bbox[1]),
                CoerceFloat(mapinst->bbox[2]),
                CoerceFloat(mapinst->bbox[3]),
                CoerceFloat(mapinst->bbox[4]),
                CoerceFloat(mapinst->bbox[5]));

            bboxUnit = (float)sqrt(
                CoerceFloat(mapinst->bbox[0])*CoerceFloat(mapinst->bbox[0])+
                CoerceFloat(mapinst->bbox[2])*CoerceFloat(mapinst->bbox[2])+
                CoerceFloat(mapinst->bbox[3])*CoerceFloat(mapinst->bbox[3])+
                CoerceFloat(mapinst->bbox[5])*CoerceFloat(mapinst->bbox[5]));

            ProcessTextureOverrides(mapinst->model, GetAttribute(mapinst->attribute)->texture);
            Com_Strcat("bCollision %i\n",
                GetAttribute(mapinst->attribute)->behavior1 & 1);
             Com_Strcat("clipMesh { type 0 }\n");

            Com_Strcat("origin { %f %f %f }\n",
                mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
            Com_Strcat("scale { %f %f %f }\n",
                mapinst->scale[0], mapinst->scale[1], mapinst->scale[2]);
            Com_Strcat("rotation { %f %f %f %f }\n",
                rotvec[0], rotvec[1], rotvec[2], rotvec[3]);

            Com_Strcat("bStatic 1\n");
            Com_Strcat("radius %f\n", GetAttribute(mapinst->attribute)->width);
            Com_Strcat("height %f\n", GetAttribute(mapinst->attribute)->height);
            Com_Strcat("cullDistance %f\n", bboxUnit + 4096.0f);
            Com_Strcat("}\n");
        }
    }
}
#endif

//
// ProcessInstanceGroups
//

static void ProcessInstanceGroups(byte *data)
{
    /*int count = Com_GetCartOffset(data, CHUNK_INSTGROUP_COUNT, 0);
    int i;

    Com_Strcat("instancegroups =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        byte *group;
        byte *rncdata;
        int size;

        rncdata = Com_GetCartData(data, CHUNK_INSTGROUP_OFFSET(i), &size);
        group = RNC_ParseFile(rncdata, size, 0);

        Com_Strcat("{ // instancegroup %02d\n", i);
        Com_Strcat("staticinstances =\n");
        Com_Strcat("{\n");

        //ProcessStaticInstances1(Com_GetCartData(group, CHUNK_STATICINST_GROUP1, 0));
        ProcessStaticInstances2(Com_GetCartData(group, CHUNK_STATICINST_GROUP2, 0));

        Com_Strcat("}\n\n");

        Com_Strcat("instances =\n");
        Com_Strcat("{\n");

        ProcessInstances(Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));

        Com_Strcat("}\n\n");
        Com_Strcat("}\n\n");

        Com_Free(&group);
    }

    Com_Strcat("}\n\n");*/
}

//
// ProcessSkyTexture
//

static void ProcessSkyTexture(byte *data, int index)
{
    byte *rncdata;
    byte *texture;
    int size;
    int outsize;

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_SKYTEXTURE, &size);
    texture = RNC_ParseFile(rncdata, size, &outsize);

    AddTexture(texture, outsize, va("maps/map%02d/mapsky%02d", index, index));
    Com_Free(&texture);
}

//
// ProcessLevel
//

static void ProcessLevel(byte *data, int index)
{
    byte *rncdata;
    byte *actors;
    byte *grid;
    byte *inst;
    byte *info;
    int size;
    int outsize;
    char name[256];

    levelID = index;

#ifdef FORMAT_BINARY
    kmapInfo.buffer = Com_Alloc(0x850000);
    com_fileoffset = 0;
    Com_WriteBuffer32(kmapInfo.buffer, 1346456907);

    kmapInfo.size = (int*)&kmapInfo.buffer[com_fileoffset];
    Com_WriteBuffer32(kmapInfo.buffer, 0);
    kmapInfo.infoLookup = (int*)&kmapInfo.buffer[com_fileoffset];
    Com_WriteBuffer32(kmapInfo.buffer, 0);
    kmapInfo.actorLookup = (int*)&kmapInfo.buffer[com_fileoffset];
    Com_WriteBuffer32(kmapInfo.buffer, 0);
    kmapInfo.gridLookup = (int*)&kmapInfo.buffer[com_fileoffset];
    Com_WriteBuffer32(kmapInfo.buffer, 0);

    *kmapInfo.infoLookup = com_fileoffset;
    Com_WriteBufferString(kmapInfo.buffer, va("Map%02d", index));
    Com_WriteBuffer32(kmapInfo.buffer, index);
    Com_WriteBufferPad4(kmapInfo.buffer);

    *kmapInfo.actorLookup = com_fileoffset;
#else
    Com_StrcatClear();
#endif

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_ACTORS, &size);
    actors = RNC_ParseFile(rncdata, size, &outsize);
    DC_DecodeData(actors, decode_buffer, 0);
    memcpy(actors, decode_buffer, outsize);

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_GRIDBOUNDS, &size);
    grid = RNC_ParseFile(rncdata, size, &outsize);

    inst = Com_GetCartData(data, CHUNK_LVROOT_INSTANCES, 0);

    Com_SetDataProgress(Com_GetCartOffset(actors, CHUNK_ACTORS_COUNT, 0) +
        Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_COUNT, 0) +
        Com_GetCartOffset(data, CHUNK_AREAS_COUNT, 0));

#ifndef FORMAT_BINARY
    Com_Strcat("title = \"Map%02d\"\n", index);
    Com_Strcat("mapID = %i\n", index);

    ProcessHeader(data, index);
    Com_Strcat("\n");
#else
    kmapInfo.actorStride = (int**)Com_Alloc(sizeof(int*) *
        Com_GetCartOffset(actors, CHUNK_ACTORS_COUNT, 0));
#endif

#ifdef FORMAT_BINARY
    *kmapInfo.gridLookup = com_fileoffset;
#endif

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_INFO, &size);
    info = RNC_ParseFile(rncdata, size, &outsize);

    areadata = Com_GetCartData(info, CHUNK_LEVELINFO_AREAS, &size);
    DC_DecodeData(areadata, decode_buffer, 0);
    memcpy(areadata, decode_buffer, size);

    ProcessActors(actors);
    ProcessGridBounds(grid, inst);
#ifdef OLD_FORMAT
    ProcessAreas(areadata);
#endif

#ifndef FORMAT_BINARY
    sprintf(name, "maps/map%02d/map%02d.kmap", index, index);
    Com_StrcatAddToFile(name);
#endif

    Com_Free(&grid);
    Com_Free(&actors);
    Com_Free(&info);

#ifdef FORMAT_BINARY
    *kmapInfo.size = (com_fileoffset - 8);
    
    sprintf(name, "maps/map%02d/map%02d.kwld", index, index);
    PK_AddFile(name, kmapInfo.buffer, com_fileoffset, true);

    Com_Free(&kmapInfo.buffer);
    free(kmapInfo.actorStride);
    free(kmapInfo.gridStride);
#endif
}

//
// AddLevel
//

static void AddLevel(byte *data, int index)
{
    byte *level;

    actorTally = 0;

    level = Com_GetCartData(data, CHUNK_LEVEL_OFFSET(index), 0);

    ProcessNavigation(level, index);
    ProcessLevel(level, index);
    ProcessSkyTexture(level, index);
}

//
// InitAttribData
//

static void InitAttribData(void)
{
    byte *tmp;
    int size;

    tmp = Com_GetCartData(cartfile, CHUNK_DIRECTORY_ATTRIBUTES, &size);
    attribdata = RNC_ParseFile(tmp, size, 0);
    numattributes = Com_GetCartOffset(attribdata, CHUNK_ATTRIB_COUNT, 0);
}

//
// InitWarpData
//

static void InitWarpData(void)
{
    warpdata = Com_GetCartData(cartfile, CHUNK_DIRECTORY_WARPS, NULL);
}

//
// LVL_StoreLevels
//

void LV_StoreLevels(void)
{
    int i;

    leveldata = Com_GetCartData(cartfile, CHUNK_DIRECTORY_LEVEL, 0);
    numlevels = Com_GetCartOffset(leveldata, CHUNK_LEVEL_COUNT, 0);

    InitAttribData();
    InitWarpData();
    typedata = (short*)Com_GetCartData(cartfile, CHUNK_DIRECTORY_TYPES, 0);

    PK_AddFolder("maps/");

    /*const char *mapnames[] =
    {
        "campainger",
        "outro",
        "credits",
        "trexboss",
        "hubA",
        "hubB",
        "jungleA",
        "jungleB",
        "cityA",
        "cityB",
        "ruinsA",
        "ruinsB",
        "catacombsA",
        "catacombsB",
        "villageA",
        "villageB",
        "villageC",
        "lostlandA",
        "lostlandB",
        "lostlandC",
        "lostlandD",
        "finalA",
        "finalB",
        "finalC",
        "finalD",
        "finalE"
    };*/

    for(i = 0; i < numlevels; i++)
    {
        char name[256];

        sprintf(name, "maps/map%02d/", i);
        PK_AddFolder(name);

        AddLevel(leveldata, i);

        // do garbage collection / cleanup
        Z_FreeTags(PU_STATIC, PU_STATIC);
    }
}

