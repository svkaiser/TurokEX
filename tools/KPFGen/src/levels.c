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

void AddTexture(byte *data, int size, const char *path);

#define M_PI                        3.1415926535897932384626433832795
#define M_RAD                       (M_PI / 180.0)
#define M_DEG                       (180.0 / M_PI)

#define ANGLE_LEVELOBJECT           (255.0f / 180.0f)
#define ANGLE_INSTANCE              (ANGLE_LEVELOBJECT / 180.0f)

#define CHUNK_DIRECTORY_ATTRIBUTES  8
#define CHUNK_DIRECTORY_TYPES       12
#define CHUNK_DIRECTORY_LEVEL       28

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

static byte *leveldata;
static byte *attribdata;
static short *typedata;
static int numlevels;
static int numattributes;
static byte decode_buffer[0x40000];
static int actorTally = 0;

extern short section_count[800];
extern short texindexes[2000];
extern short section_textures[800][100];
extern bbox mdlboxes[1000];

typedef struct
{
    float u1;
    float uf2;
    float u3;
    float u4;
    float u5;
    float uf6;
    float meleerange;
    float width;
    float height;
    float viewheight;
    float u11;
    float u12;
    float u13;
    int u14;
    byte blockflags;
    byte u16[3];
    int u17;
    short u18;
    short health;
    short tid;
    short target;
    short skin;
    short u22;
    char variant1;
    char texture;
    short variant2;
    short u25;
    short u26;
    short u27;
    int flags;
} attribute_t;

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
    byte u3;
    byte u4;
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
// GetAttribute
//

static attribute_t *GetAttribute(int index)
{
    return (attribute_t*)(attribdata + 8 +
        (index * Com_GetCartOffset(attribdata, CHUNK_ATTRIB_SIZE, 0)));
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
    return (GetObjectType(model) >= 400 &&
            GetObjectType(model) <= 456);
}

//
// ProcessHeader
//

static void ProcessHeader(byte *data, int index)
{
    //Com_WriteCartData(data, CHUNK_LVROOT_HEADER, "lvlheader38.lmp");
}

//
// ProcessPoints
//

static void ProcessPoints(byte *data)
{
    int* rover = (int*)(data + 8);
    int count = Com_GetCartOffset(data, CHUNK_POINTS_COUNT, 0);
    int i;

    Com_Strcat("points =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        Com_Strcat("    %f %f %f %f\n",
            (float)rover[0] * 0.029296875f,
            (float)rover[1] * 0.029296875f,
            (float)rover[2] * 0.029296875f,
            (float)rover[3] * 0.029296875f);

        rover += 4;
        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n\n");
}

//
// ProcessLeafs
//

static void ProcessLeafs(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_LEAFS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_LEAFS_COUNT, 0);
    int i;

    Com_Strcat("leafs = // [area, flags, pt1, pt2, pt3, link1, link2, link3]\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        mapleaf_t *leaf = (mapleaf_t*)(data + 8 + (i * size));

        Com_Strcat("    %i %i %i %i %i %i %i %i\n",
            leaf->area_id, leaf->flags, leaf->pt1, leaf->pt2, leaf->pt3,
            leaf->link1, leaf->link2, leaf->link3);
        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n\n");
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

        Com_Strcat("    %f %f %f %f %i %i %i\n",
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

        if(area->flags & 0x1)       total++;
        if(area->flags & 0x10)      total++;
        if(area->flags & 0x80)      total++;
        if(area->flags & 0x100000)  total++;
        if(area->ambience > 0 && area->ambience <= 10) total++;

        Com_Strcat("    {\n");
        Com_Strcat("        components[%i] =\n", total);
        Com_Strcat("        {\n");

        if(area->flags & 0x1)
        {
            Com_Strcat("            BeginObject = \"ComponentAreaWater\"\n");
            Com_Strcat("                { \"active\" : true,\n");
            Com_Strcat("                \"waterYPlane\" : %f }\n", area->waterheight);
            Com_Strcat("            EndObject\n");
        }

        if(area->flags & 0x10)
        {
            Com_Strcat("            BeginObject = \"ComponentAreaClimb\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
        }

        if(area->flags & 0x80)
        {
            Com_Strcat("            BeginObject = \"ComponentAreaCrawl\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
        }

        if(area->flags & 0x100000)
        {
            Com_Strcat("            BeginObject = \"ComponentAreaSky\"\n");
            Com_Strcat("                { \"active\" : true,\n");
            Com_Strcat("                \"skyYPlane\" : %f }\n", area->skyheight);
            Com_Strcat("            EndObject\n");
        }

        switch(area->ambience)
        {
        case 1:
            Com_Strcat("            BeginObject = \"AmbienceJungle\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 2:
            Com_Strcat("            BeginObject = \"AmbienceSwamp\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 3:
            Com_Strcat("            BeginObject = \"AmbienceCave\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 4:
            Com_Strcat("            BeginObject = \"AmbienceCatcomb01\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 5:
            Com_Strcat("            BeginObject = \"AmbienceLostLand\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 6:
            Com_Strcat("            BeginObject = \"AmbienceCatcomb02\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 7:
            Com_Strcat("            BeginObject = \"AmbienceCatcomb01\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 8:
            Com_Strcat("            BeginObject = \"AmbienceRuins\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 9:
            Com_Strcat("            BeginObject = \"AmbienceValley\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        case 10:
            Com_Strcat("            BeginObject = \"AmbienceVillage\"\n");
            Com_Strcat("                { \"active\" : true }\n");
            Com_Strcat("            EndObject\n");
            break;
        default:
            break;
        }

        Com_Strcat("            BeginObject = \"ComponentAreaFog\"\n");
        Com_Strcat("                { \"fog_Color_r\" : %i,\n", area->fogrgba[0]);
        Com_Strcat("                \"fog_Color_g\" : %i,\n", area->fogrgba[1]);
        Com_Strcat("                \"fog_Color_b\" : %i,\n", area->fogrgba[2]);
        Com_Strcat("                \"fog_Far\" : %f }\n", area->fogzfar);
        Com_Strcat("            EndObject\n");
        Com_Strcat("        }\n");
        Com_Strcat("    }\n");

        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n");
}

//
// ProcessNavigation
//

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

    Com_StrcatClear();

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

    Com_Strcat("numpoints = %i\n", Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0));
    Com_Strcat("numleafs = %i\n\n", Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));
    //Com_Strcat("numzonebounds = %i\n\n", Com_GetCartOffset(zones, CHUNK_ZONE_COUNT, 0));

    Com_SetDataProgress(Com_GetCartOffset(points, CHUNK_POINTS_COUNT, 0) +
        Com_GetCartOffset(leafs, CHUNK_LEAFS_COUNT, 0));

    ProcessPoints(points);
    ProcessLeafs(leafs);
    //ProcessZones(zones);

    sprintf(name, "maps/map%02d/map%02d.kcm", index, index);
    Com_StrcatAddToFile(name);

    Com_Free(&info);
}

//
// ProcessActors
//

static void ProcessActors(byte *data)
{
    int size = Com_GetCartOffset(data, CHUNK_ACTORS_SIZE, 0);
    int count = Com_GetCartOffset(data, CHUNK_ACTORS_COUNT, 0);
    int i;

    Com_Strcat("actors[%i] =\n{\n", count);

    for(i = 0; i < count; i++)
    {
        mapactor_t *actor = (mapactor_t*)(data + 8 + (i * size));

        Com_Strcat("    {\n");

        Com_Strcat("        name = \"Actor_%i\"\n", actorTally++);
        
        if(actor->type == 0)
        {
            Com_Strcat("        components[1] =\n");
            Com_Strcat("        {\n");
            Com_Strcat("            BeginObject = \"ComponentTurokPlayer\"\n");
            Com_Strcat("                {\n");
			Com_Strcat("                    \"playerID\" : 0,\n");
			Com_Strcat("                    \"hudClass\" : \"TurokHud\"\n");
			Com_Strcat("                }\n");
            Com_Strcat("            EndObject\n");
            Com_Strcat("        }\n");
        }

        Com_Strcat("        mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            actor->model, actor->model);
        Com_Strcat("        bounds = { %f %f %f %f %f %f }\n",
            mdlboxes[actor->model][0],
            mdlboxes[actor->model][1],
            mdlboxes[actor->model][2],
            mdlboxes[actor->model][3],
            mdlboxes[actor->model][4],
            mdlboxes[actor->model][5]);

        Com_Strcat("        bCollision = %i\n",
            GetAttribute(actor->attribute)->blockflags & 1);
        Com_Strcat("        bStatic = 0\n");

        if(actor->flags & 1)
            Com_Strcat("        bOrientOnSlope = 0\n");
        else
            Com_Strcat("        bOrientOnSlope = 1\n");

        Com_Strcat("        plane = %i\n", actor->leaf);
        Com_Strcat("        origin = { %f %f %f }\n",
            actor->xyz[0], actor->xyz[1], actor->xyz[2]);
        Com_Strcat("        scale = { %f %f %f }\n",
            actor->scale[0], actor->scale[1], actor->scale[2]);
        Com_Strcat("        angles = { %f 0.0 0.0 }\n",
            ((((-(float)actor->angle / 180.0f) * M_RAD) + M_PI) / M_RAD) * M_RAD);
        Com_Strcat("        radius = %f\n", GetAttribute(actor->attribute)->width);
        Com_Strcat("        height = %f\n", GetAttribute(actor->attribute)->height);
        Com_Strcat("        centerheight = %f\n", GetAttribute(actor->attribute)->meleerange);
        Com_Strcat("        viewheight = %f\n", GetAttribute(actor->attribute)->viewheight);

        Com_Strcat("    }\n");

        Com_UpdateDataProgress();
    }

    Com_Strcat("}\n");
}

//
// ProcessGridBounds
//

static void ProcessStaticInstances2(byte *data, byte *data2);
static void ProcessInstances(byte *data);

static void ProcessGridBounds(byte *data, byte *inst)
{
    byte *grid = Com_GetCartData(data, CHUNK_LEVELGRID_BOUNDS, 0);
    int size = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_SIZE, 0);
    int count = Com_GetCartOffset(grid, CHUNK_GRIDBOUNDS_COUNT, 0);
    int total = count;
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

    Com_Strcat("gridbounds[%i] =\n{\n", total);

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

            Com_Strcat("    {\n");
            Com_Strcat("        bounds = %f %f %f %f\n", min_x, min_z, max_x, max_z);
            
            ProcessStaticInstances2(
                Com_GetCartData(group, CHUNK_STATICINST_GROUP2, 0),
                Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));
            //ProcessInstances(Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));

            Com_Strcat("    }\n");
        }

         Com_Free(&group);
         Com_UpdateDataProgress();
    }

    Com_Strcat("}\n");
}

//
// ProcessInstances
//

static void ProcessInstances(byte *data)
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

        Com_Strcat("            {\n");
        Com_Strcat("                name = \"Actor_%i\"\n", actorTally++);
        Com_Strcat("                origin = { %f %f %f }\n",
            mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
        Com_Strcat("                scale = { 0.35 0.35 0.35 }\n");
        Com_Strcat("                bStatic = 0\n");
        Com_Strcat("                bCollision = 0\n");
        Com_Strcat("                angles = { %f 0 0 }\n",
            (mapinst->angle * ANGLE_LEVELOBJECT) * M_RAD);
        Com_Strcat("                radius = %f\n", mapinst->bboxsize);
        Com_Strcat("                height = %f\n", mapinst->bboxsize);
        Com_Strcat("                mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            mapinst->model, mapinst->model);
        Com_Strcat("                bounds = { %f %f %f %f %f %f }\n",
            -mapinst->bboxsize, -mapinst->bboxsize, -mapinst->bboxsize,
            mapinst->bboxsize, mapinst->bboxsize, mapinst->bboxsize);

        if(IsAPickup(mapinst->model))
        {
            Com_Strcat("                bTouch = 1\n");
            Com_Strcat("                components[1] =\n");
            Com_Strcat("                {\n");
            switch(GetObjectType(mapinst->model))
            {
            case 409:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponPistol\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 410:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponRifle\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 415:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponPulseRifle\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 411:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponShotgun\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 412:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponRiotgun\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 413:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponMinigun\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 414:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponGrenadeLauncher\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 416:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponAlienRifle\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 419:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponFusion\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 417:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponRocketLauncher\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 418:
                Com_Strcat("                    BeginObject = \"TurokPickupWeaponAccelerator\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true\n");
                Com_Strcat("                    }\n");
                break;
            case 438:
                Com_Strcat("                    BeginObject = \"TurokPickupLifeForce\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true,\n");
                Com_Strcat("                        \"amount\" : 1\n");
                Com_Strcat("                    }\n");
                break;
            case 439:
                Com_Strcat("                    BeginObject = \"TurokPickupLifeForce\"\n");
                Com_Strcat("                    {\n");
                Com_Strcat("                        \"active\" : true,\n");
                Com_Strcat("                        \"amount\" : 10\n");
                Com_Strcat("                    }\n");
                break;
            default:
                Com_Strcat("                    BeginObject = \"TurokPickup\"\n");
                Com_Strcat("                        { \"active\" : true }\n");
                break;
            }
            Com_Strcat("                    EndObject\n");
            Com_Strcat("                }\n");
        }

        Com_Strcat("            }\n");
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
// ProcessTextureOverrides
//

static void ProcessTextureOverrides(short model, int textureid)
{
    int scount;
    int tcount;
    int i;

    if(textureid == -1 || textureid == 0)
    {
        return;
    }

    scount = section_count[model];

    Com_Strcat("                textureSwaps[%i] =\n", scount);
    Com_Strcat("                {\n");

    for(i = 0; i < scount; i++)
    {
        //Com_Strcat("                            { ");

        tcount = texindexes[section_textures[model][i]];

        if(textureid >= tcount)
        {
            Com_Strcat("                    \"-\"");
        }
        else
        {
            Com_Strcat("                    \"textures/tex%04d_%02d.tga\"",
                section_textures[model][i], textureid);
        }

        //Com_Strcat(" }");

        /*if(i != (scount-1))
        {
            Com_Strcat(",");
        }*/

        Com_Strcat("\n");
    }

    Com_Strcat("                }\n");
}

//
// ProcessStaticInstances2
//

static void ProcessStaticInstances2(byte *data, byte *data2)
{
    int size;
    int count;
    int i;

    size = Com_GetCartOffset(data, CHUNK_INSTANCE_SIZE, 0);
    count = Com_GetCartOffset(data, CHUNK_INSTANCE_COUNT, 0);

    Com_Strcat("        statics[%i] =\n        {\n",
        count + Com_GetCartOffset(data2, CHUNK_INSTANCE_COUNT, 0));

    DC_DecodeData(data, decode_buffer, 0);
    memcpy(data, decode_buffer, (size * count) + 8);

    for(i = 0; i < count; i++)
    {
        mapinsttype2_t *mapinst = (mapinsttype2_t*)(data + 8 + (i * size));
        float rotvec[4];

        Com_Strcat("            {\n");
        Com_Strcat("                name = \"Actor_%i\"\n", actorTally++);

        rotvec[0] = (float)mapinst->angle[0] * ANGLE_INSTANCE;
        rotvec[1] = (float)mapinst->angle[1] * ANGLE_INSTANCE;
        rotvec[2] = (float)mapinst->angle[2] * ANGLE_INSTANCE;
        rotvec[3] = (float)mapinst->angle[3] * ANGLE_INSTANCE;

        Com_Strcat("                mesh = \"models/mdl%03d/mdl%03d.kmesh\"\n",
            mapinst->model, mapinst->model);
        Com_Strcat("                bounds = { %f %f %f %f %f %f }\n",
            CoerceFloat(mapinst->bbox[0]),
            CoerceFloat(mapinst->bbox[1]),
            CoerceFloat(mapinst->bbox[2]),
            CoerceFloat(mapinst->bbox[3]),
            CoerceFloat(mapinst->bbox[4]),
            CoerceFloat(mapinst->bbox[5]));

        ProcessTextureOverrides(mapinst->model, GetAttribute(mapinst->attribute)->texture);

        Com_Strcat("                bCollision = %i\n",
            GetAttribute(mapinst->attribute)->blockflags & 1);
        Com_Strcat("                bStatic = 1\n");
        Com_Strcat("                origin = { %f %f %f }\n",
            mapinst->xyz[0], mapinst->xyz[1], mapinst->xyz[2]);
        Com_Strcat("                scale = { %f %f %f }\n",
            mapinst->scale[0], mapinst->scale[1], mapinst->scale[2]);
        Com_Strcat("                rotation = { %f %f %f %f }\n",
            rotvec[0], rotvec[1], rotvec[2], rotvec[3]);
        Com_Strcat("                plane = %i\n", mapinst->plane);
        Com_Strcat("                radius = %f\n", GetAttribute(mapinst->attribute)->width);
        Com_Strcat("                height = %f\n", GetAttribute(mapinst->attribute)->height);

        Com_Strcat("            }\n");
    }

    ProcessInstances(data2);

    Com_Strcat("        }\n");
}

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

        Com_Strcat("    { // instancegroup %02d\n", i);
        Com_Strcat("        staticinstances =\n");
        Com_Strcat("        {\n");

        //ProcessStaticInstances1(Com_GetCartData(group, CHUNK_STATICINST_GROUP1, 0));
        ProcessStaticInstances2(Com_GetCartData(group, CHUNK_STATICINST_GROUP2, 0));

        Com_Strcat("        }\n\n");

        Com_Strcat("        instances =\n");
        Com_Strcat("        {\n");

        ProcessInstances(Com_GetCartData(group, CHUNK_STATICINST_GROUP3, 0));

        Com_Strcat("        }\n\n");
        Com_Strcat("    }\n\n");

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
    byte *areas;
    int size;
    int outsize;
    char name[256];

    Com_StrcatClear();

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

    Com_Strcat("title = \"Map%02d\"\n", index);
    Com_Strcat("mapID = %i\n\n", index);

    ProcessActors(actors);
    ProcessGridBounds(grid, inst);

    rncdata = Com_GetCartData(data, CHUNK_LVROOT_INFO, &size);
    info = RNC_ParseFile(rncdata, size, &outsize);

    areas = Com_GetCartData(info, CHUNK_LEVELINFO_AREAS, &size);
    DC_DecodeData(areas, decode_buffer, 0);
    memcpy(areas, decode_buffer, size);

    ProcessAreas(areas);

    sprintf(name, "maps/map%02d/map%02d.kmap", index, index);
    Com_StrcatAddToFile(name);

    Com_Free(&grid);
    Com_Free(&actors);
    Com_Free(&info);
}

//
// AddLevel
//

static void AddLevel(byte *data, int index)
{
    byte *level;

    actorTally = 0;

    level = Com_GetCartData(data, CHUNK_LEVEL_OFFSET(index), 0);

    ProcessHeader(level, index);
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
// LVL_StoreLevels
//

void LV_StoreLevels(void)
{
    int i;

    leveldata = Com_GetCartData(cartfile, CHUNK_DIRECTORY_LEVEL, 0);
    numlevels = Com_GetCartOffset(leveldata, CHUNK_LEVEL_COUNT, 0);

    InitAttribData();
    typedata = (short*)Com_GetCartData(cartfile, CHUNK_DIRECTORY_TYPES, 0);

    PK_AddFolder("maps/");

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

