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
#include "pak.h"
#include "rnc.h"
#include "decoders.h"
#include "zone.h"

#define CHUNK_DIRECTORY_MODEL       4

/**************************************************
***************************************************
                    MODEL DATA
***************************************************
***************************************************/

// model index
#define CHUNK_MODEL_COUNT           0
#define CHUNK_MODEL_OFFSET(x)       (4 + (x * 4))

// root
#define CHUNK_MDLROOT_ENTIRES       0
#define CHUNK_MDLROOT_INFO          4
#define CHUNK_MDLROOT_DATA          8
#define CHUNK_MDLROOT_ANIMATIONS    12
#define CHUNK_MDLROOT_ACTIONS       16

// info
#define CHUNK_MDLINFO_ENTIRES       0
#define CHUNK_MDLINFO_BBOX          4
#define CHUNK_MDLINFO_TYPE          8

// info->type
#define CHUNK_INFO_SIZE             0
#define CHUNK_INFO_COUNT            4
#define CHUNK_INFO_OFFSET(x)        (8 + (x * 4))

// data
#define CHUNK_MDLDATA_ENTIRES       0
#define CHUNK_MDLDATA_UNKNOWN1      4
#define CHUNK_MDLDATA_LIMBINDEX     8

// limb index
#define CHUNK_LIMBINDEX_COUNT       0
#define CHUNK_LIMBINDEX_OFFSET(x)   (4 + (x * 4))

// limb
#define CHUNK_LIMB_ENTIRES          0
#define CHUNK_LIMB_MATRIX           4
#define CHUNK_LIMB_VARIATIONS       8
#define CHUNK_LIMB_OBJECTS          12

// limb->matrix
#define CHUNK_MATRIX_SIZE           0
#define CHUNK_MATRIX_COUNT          4
#define CHUNK_MATRIX_OFFSET(x)      (8 + (x * 4))

// limb->variations
#define CHUNK_VARIATIONS_SIZE       0
#define CHUNK_VARIATIONS_COUNT      4
#define CHUNK_VARIATIONS_OFFSET(x)  (8 + (x * 4))

// limb->objects
#define CHUNK_OBJECTS_COUNT         0
#define CHUNK_OBJECTS_OFFSET(x)     (4 + (x * 4))

// limb->objects->sectionlist
#define CHUNK_SECTIONLIST_COUNT     0
#define CHUNK_SECTIONLIST_OFFSET(x) (4 + (x * 4))

// sections
#define CHUNK_SECTION_ENTIRES       0
#define CHUNK_SECTION_HEADER        4
#define CHUNK_SECTION_INDICES       8
#define CHUNK_SECTION_VERTEX        12

// sections->indices
#define CHUNK_INDICES_SIZE          0
#define CHUNK_INDICES_COUNT         4
#define CHUNK_INDICES_OFFSET(x)     (8 + (x * 2))

// sections->vertices
#define CHUNK_VERTEX_SIZE           0
#define CHUNK_VERTEX_COUNT          4

/**************************************************
***************************************************
                  ANIMATION DATA
***************************************************
***************************************************/

// anim index
#define CHUNK_ANIMATION_COUNT       0
#define CHUNK_ANIMATION_OFFSET(x)   (4 + (x * 4))

// anim root
#define CHUNK_ANIMROOT_ENTIRES      0
#define CHUNK_ANIMROOT_FRAMEINFO    4
#define CHUNK_ANIMROOT_INDEXES      8
#define CHUNK_ANIMROOT_INITIAL      12
#define CHUNK_ANIMROOT_MOVEMENT     16
#define CHUNK_ANIMROOT_ROTATIONS    20
#define CHUNK_ANIMROOT_UNKNOWN2     24
#define CHUNK_ANIMROOT_ACTIONS      28

// anim frameinfo
#define CHUNK_FRAMEINFO_SIZE        0
#define CHUNK_FRAMEINFO_FRAMES      4

// anim indexes
#define CHUNK_INDEXES_SIZE          0
#define CHUNK_INDEXES_ENTRIES       4
#define CHUNK_INDEXES_TABLE         8

#define TRANSLATION_INDEX           0
#define ROTATION_INDEX              1

// anim initial
#define CHUNK_INITIAL_SIZE          0
#define CHUNK_INITIAL_COUNT         4
#define CHUNK_INITIAL_DATA          8

// anim movement
#define CHUNK_MOVEMENT_SIZE         0
#define CHUNK_MOVEMENT_COUNT        4
#define CHUNK_MOVEMENT_ENCODED      8

// anim rotations
#define CHUNK_ROTATIONS_SIZE        0
#define CHUNK_ROTATIONS_COUNT       4

// anim actions
#define CHUNK_ACTIONS_SIZE          0
#define CHUNK_ACTIONS_COUNT         4
#define CHUNK_ACTIONS_FRAME         8
#define CHUNK_ACTIONS_TYPE          10
#define CHUNK_ACTIONS_ARGS          12

#define CHUNK_UNKNOWN_ENTRIES       0
#define CHUNK_UNKNOWN_A             4
#define CHUNK_UNKNOWN_B             8

// unknown
#define CHUNK_UNKNOWNB_SIZE         0
#define CHUNK_UNKNOWNB_COUNT        4
#define CHUNK_UNKNOWNB_DATA         8

static byte* modeldata;
static int nummodels;
static int totalverts;
static int highidx;
static int hightri;
static byte action_buffer[0x4000];
static short curmodel;

short section_count[800];
short section_textures[800][100];

typedef struct
{
    unsigned int    texture;
    int             flags;
    byte            rgba1[4];
    byte            rgba2[4];
    short           tex_w;
    short           tex_h;
    int             unk3;
} geomheader_t;

typedef struct
{
    char *name;
    int flag;
} flagnames_t;

typedef struct
{
    short frame;
    short type;
    float args[4];
} factions_t;

typedef struct
{
    short flags;
    short frame;
} unknownstruct1_t;

typedef struct
{
    int u1;
    int u2;
    int u3;
} unknownstruct2_t;

static const flagnames_t flagnames[17] =
{
    { "unknown1",       1       },
    { "fullbright",     2       },
    { "nocullfaces",    4       },
    { "renderspecular", 8       },
    { "shinysurface",   16      },
    { "unknown32",      32      },
    { "solid",          64      },
    { "masked",         128     },
    { "transparent1",   256     },
    { "transparent2",   512     },
    { "colorize",       1024    },
    { "metalsurface",   2048    },
    { "unknown4096",    4096    },
    { "unknown8192",    8192    },
    { "unknown16384",   16384   },
    { "unknown32768",   32768   },
    { "unknown65536",   65536   }
};

static const int mdl_namespace[] =
{
    0
};

//
// GetModelNamespace
//

static char *GetModelNamespace(int type)
{
    /*switch(type)
    {
    case 0:
        return "models/caves/";
        break;
    case 1:
        return "models/char/";
        break;
    case 2:
        return "models/items/";
        break;
    case 3:
        return "models/jungle/";
        break;
    case 4:
        return "models/misc/";
        break;
    case 5:
        return "models/ruins/";
        break;
    case 6:
        return "models/tech/";
        break;
    case 7:
        return "models/temple/";
        break;
    case 8:
        return "models/village/";
        break;
    case 9:
        return "models/weapons/";
        break;
    case 10:
        return "models/lostworld/";
        break;
    case 11:
        return "models/arenas/";
        break;
    }*/

    return "models/";
}

//
// PrintFlags
//

static void PrintFlags(int flags)
{
    int i;

    for(i = 0; i < 17; i++)
    {
        if(flags & flagnames[i].flag)
        {
            Com_Strcat("                            %s = 1\n", flagnames[i].name);
        }
    }
}

//
// ProcessTriangles
//

static void ProcessTriangles(byte *data)
{
    int i;
    int max;
    int inc;
    int previnc;
    int count;

    count = Com_GetCartOffset(data, CHUNK_INDICES_COUNT, 0);

    for(i = 0, max = 0, inc = 0, previnc = 0; i < count; i++)
    {
        word val;
        short v0;
        short v1;
        short v2;
        short tri0;
        short tri1;
        short tri2;

        val = Com_GetCartOffset(data, CHUNK_INDICES_OFFSET(i), 0);

        v0 = val & 0x1F;
        v1 = ((val >> 5) & 0x1F);
        v2 = ((val >> 10) & 0x1F);

        if(v0 < (max & 0x1F))
            v0 += inc;
        else
            v0 += previnc;

        if(v1 < (max & 0x1F))
            v1 += inc;
        else
            v1 += previnc;

        if(v2 < (max & 0x1F))
            v2 += inc;
        else
            v2 += previnc;

        tri0 = v0 + 1 + totalverts;
        tri1 = v1 + 1 + totalverts;
        tri2 = v2 + 1 + totalverts;

        if(tri0 >= highidx)
            highidx = tri0 + 1;
        if(tri1 >= highidx)
            highidx = tri1 + 1;
        if(tri2 >= highidx)
            highidx = tri2 + 1;

        if((i % 4) == 0)
        {
            Com_Strcat("\n                                ");
        }

        Com_Strcat("%i %i %i ",
            tri2 - hightri,
            tri1 - hightri,
            tri0 - hightri);

        if((val >> 15) & 1)
        {
            max += 16;

            if((max & 0x1F))
                inc += 32;
            else
                previnc = inc;
        }
    }
}

//
// ProcessVertices
//

static void ProcessVertices(byte *data)
{
    short *x;
    short *y;
    short *z;
    short *tu1;
    short *tu2;
    short *tv1;
    short *tv2;
    char *n;
    int count;
    int i;

    count = Com_GetCartOffset(data, CHUNK_VERTEX_COUNT, 0);

    x   = (short*)(data + 8 + ((count * 2) * 0));
    y   = (short*)(data + 8 + ((count * 2) * 1));
    z   = (short*)(data + 8 + ((count * 2) * 2));
    tu1 = (short*)(data + 8 + ((count * 2) * 4));
    tu2 = (short*)(data + 8 + ((count * 2) * 5));
    tv1 = (short*)(data + 8 + ((count * 2) * 6));
    tv2 = (short*)(data + 8 + ((count * 2) * 7));
    n   = (byte*)( data + 8 + ((count * 2) * 8));

    Com_Strcat("                                xyz =\n");
    Com_Strcat("                                {\n");

    for(i = 0; i < count; i++)
    {
        byte *fx;
        byte *fy;
        byte *fz;
        signed short xx;
        signed short yy;
        signed short zz;
    
        fx = (byte*)x;
        fy = (byte*)y;
        fz = (byte*)z;

        xx = (signed short)((fx[count + i] << 8) | fx[i]);
        yy = (signed short)((fy[count + i] << 8) | fy[i]);
        zz = (signed short)((fz[count + i] << 8) | fz[i]);

        Com_Strcat("                                    ");
        Com_Strcat("%f %f %f\n", (float)xx, (float)yy, (float)zz);
    }
    Com_Strcat("                                }\n\n");

    Com_Strcat("                                coords =\n");
    Com_Strcat("                                {\n");

    for(i = 0; i < count; i++)
    {
        byte *fx1;
        byte *fy1;
        byte *fx2;
        byte *fy2;
        word tmu1;
        word tmu2;
        word tmv1;
        word tmv2;

        union
        {
            unsigned int uvi[2];
            float uvf[2];
        } uv;

        fx1 = (byte*)tu1;
        fx2 = (byte*)tu2;
        fy1 = (byte*)tv1;
        fy2 = (byte*)tv2;

        tmu1 = (word)((fx1[count + i] << 8) | fx1[i]);
        tmu2 = (word)((fx2[count + i] << 8) | fx2[i]);
        tmv1 = (word)((fy1[count + i] << 8) | fy1[i]);
        tmv2 = (word)((fy2[count + i] << 8) | fy2[i]);

        uv.uvi[0] = (tmu2 << 16) | tmu1;
        uv.uvi[1] = (tmv2 << 16) | tmv1;

        Com_Strcat("                                    ");
        Com_Strcat("%f %f\n", uv.uvf[0], uv.uvf[1]);
    }
    Com_Strcat("                                }\n\n");

    Com_Strcat("                                normals =\n");
    Com_Strcat("                                {\n");

    for(i = 0; i < count; i++)
    {
        float n1;
        float n2;
        float n3;
    
        n1 = (float)n[(count * 0) + i] / 127.0f;
        n2 = (float)n[(count * 1) + i] / 127.0f;
        n3 = (float)n[(count * 2) + i] / 127.0f;

        Com_Strcat("                                    ");
        Com_Strcat("%f %f %f\n", n1, n2, n3);
    }
    Com_Strcat("                                }\n\n");
}

//
// ProcessGeometry
//

static int cursection = 0;

static void ProcessGeometry(byte *data)
{
    geomheader_t *header;
    int headersize;
    byte *indices;
    int indicesize;
    byte *vertices;
    int vertexsize;
    int vertexcount;

    header = (geomheader_t*)Com_GetCartData(data, CHUNK_SECTION_HEADER, &headersize);
    indices = Com_GetCartData(data, CHUNK_SECTION_INDICES, &indicesize);
    vertices = Com_GetCartData(data, CHUNK_SECTION_VERTEX, &vertexsize);
    vertexcount = Com_GetCartOffset(vertices, CHUNK_VERTEX_COUNT, 0);

    section_textures[curmodel][cursection] = header->texture;

    PrintFlags(header->flags);
    if(header->texture != -1)
    {
        Com_Strcat("                            texture = \"textures/tex%04d_00.tga\"\n",
            header->texture);
    }
    Com_Strcat("                            rgba = %i %i %i %i\n",
        header->rgba1[0], header->rgba1[1], header->rgba1[2], header->rgba1[3]);
    Com_Strcat("                            // rgba = %i %i %i %i\n",
        header->rgba2[0], header->rgba2[1], header->rgba2[2], header->rgba2[3]);

    Com_Strcat("\n                            numtriangles = %i\n",
        Com_GetCartOffset(indices, CHUNK_INDICES_COUNT, 0));

    Com_Strcat("\n                            triangles =\n");
    Com_Strcat("                            {\n");
    ProcessTriangles(indices);
    Com_Strcat("\n                            }\n\n");

    Com_Strcat("\n                            numvertices = %i\n",
        vertexcount);

    Com_Strcat("                            vertices =\n");
    Com_Strcat("                            {\n");
    ProcessVertices(vertices);
    Com_Strcat("                            }\n\n");

    hightri = highidx;
    totalverts += vertexcount;
}

//
// ProcessGroup
//

static void ProcessGroup(byte *data)
{
    int sectioncount;
    int i;

    sectioncount = Com_GetCartOffset(data, CHUNK_SECTIONLIST_COUNT, 0);

    section_count[curmodel] = sectioncount;

    Com_Strcat("                    numsections = %i\n\n", sectioncount);
    Com_Strcat("                    sections\n");
    Com_Strcat("                    {\n");
    for(i = 0; i < sectioncount; i++)
    {
        totalverts = 0;
        highidx = 0;
        hightri = 1;

        cursection = i;

        Com_Strcat("                        { // section %02d\n", i);
        ProcessGeometry(Com_GetCartData(data, CHUNK_SECTIONLIST_OFFSET(i), 0));
        Com_Strcat("                        }\n");
    }
    Com_Strcat("                    }\n\n");
}

//
// ProcessLimbData
//

static void ProcessLimbData(byte *data)
{
    byte *matrix;
    int matrixsize;
    int children;
    byte *variants;
    int variantsize;
    int variantcount;
    byte *objects;
    int numobjects;
    int i;

    matrix = Com_GetCartData(data, CHUNK_LIMB_MATRIX, &matrixsize);
    variants = Com_GetCartData(data, CHUNK_LIMB_VARIATIONS, &variantsize);
    objects = Com_GetCartData(data, CHUNK_LIMB_OBJECTS, 0);
    numobjects = Com_GetCartOffset(objects, CHUNK_OBJECTS_COUNT, 0);

    children = Com_GetCartOffset(matrix, CHUNK_MATRIX_COUNT, 0);

    Com_Strcat("            numchildren = %i\n", children);

    if(children > 0)
    {
        Com_Strcat("            children = { ");

        for(i = 0; i < children; i++)
        {
            Com_Strcat("%i ",
                Com_GetCartOffset(matrix, CHUNK_MATRIX_OFFSET(i), 0));
        }

        Com_Strcat("}\n");
    }

    variantcount = Com_GetCartOffset(variants, CHUNK_VARIATIONS_COUNT, 0);
    Com_Strcat("\n            numvariants = %i\n", variantcount);
    Com_Strcat("            variants = { ");
    if(variantcount <= 0)
    {
        Com_Strcat("0 ");
    }
    else
    {
        for(i = 0; i < variantcount; i++)
        {
            Com_Strcat("%i ",
                Com_GetCartOffset(variants, CHUNK_VARIATIONS_OFFSET(i), 0));
        }
    }
    Com_Strcat("}\n");

    Com_Strcat("            numgroups = %i\n\n", numobjects);
    Com_Strcat("            groups\n");
    Com_Strcat("            {\n");
    for(i = 0; i < numobjects; i++)
    {
        Com_Strcat("                { // group %02d\n", i);
        ProcessGroup(Com_GetCartData(objects, CHUNK_OBJECTS_OFFSET(i), 0));
        Com_Strcat("                }\n");
    }
    Com_Strcat("            }\n\n");
}

//
// ProcessRoot
//

static void ProcessRoot(byte *data, int index)
{
    int limbcount;
    int i;

    Com_Strcat("model\n{\n");

    limbcount = Com_GetCartOffset(data, CHUNK_LIMBINDEX_COUNT, 0);

    Com_Strcat("    numnodes = %i\n\n", limbcount);

    Com_Strcat("    nodes\n");
    Com_Strcat("    {\n");
    for(i = 0; i < limbcount; i++)
    {
        Com_Strcat("        { // node %02d\n", i);
        ProcessLimbData(Com_GetCartData(data, CHUNK_LIMBINDEX_OFFSET(i), 0));
        Com_Strcat("        }\n");
    }
    Com_Strcat("    }\n\n");

    Com_Strcat("}\n\n");
}

//
// ProcessMeshes
//

static void ProcessMeshes(byte *data, int index)
{
    byte *rncdata;
    byte *mdldata;
    int size;
    int outsize;

    rncdata = Com_GetCartData(
        Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0),
        CHUNK_MDLROOT_DATA, &size);

    mdldata = RNC_ParseFile(rncdata, size, &outsize);
    ProcessRoot(Com_GetCartData(mdldata, CHUNK_MDLDATA_LIMBINDEX, 0), index);

    Com_Free(&mdldata);
}

//
// ProcessProperties
//

static void ProcessProperties(byte *data, int index)
{
    byte *bbox;
    int bboxsize;
    byte *types;
    int typecount;
    int animcount;
    int i;

    bbox = Com_GetCartData(Com_GetCartData(
        Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0), CHUNK_MDLROOT_INFO, 0),
        CHUNK_MDLINFO_BBOX, &bboxsize);

    types = Com_GetCartData(Com_GetCartData(
        Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0), CHUNK_MDLROOT_INFO, 0),
        CHUNK_MDLINFO_TYPE, 0);

    typecount = Com_GetCartOffset(types, CHUNK_INFO_COUNT, 0);

    animcount = Com_GetCartOffset(Com_GetCartData(
        Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0),
        CHUNK_MDLROOT_ANIMATIONS, 0), CHUNK_ANIMATION_COUNT, 0);

    Com_Strcat("numanims = %i\n", animcount);

    Com_Strcat("animsets =\n{\n");
    for(i = 0; i < animcount; i++)
    {
        Com_Strcat("    { \"anim%02d\" \"%smdl%03d/mdl%03d_%02d.kanim\" }\n",
            i, GetModelNamespace(0), index, index, i);
    }
    Com_Strcat("}\n\n");

    Com_Strcat("info\n{\n");

    if(bboxsize < 24)
    {
        Com_Strcat("    bbox = { 0.0 0.0 0.0 0.0 0.0 0.0 }\n");
    }
    else
    {
        float *fbbox = (float*)bbox;

        Com_Strcat("    bbox = { %f %f %f %f %f %f }\n",
            fbbox[0], fbbox[1], fbbox[2], fbbox[3], fbbox[4], fbbox[5]);
    }

    Com_Strcat("    types =\n    {");
    if(typecount <= 0)
    {
        Com_Strcat("\n        0 ");
    }
    else
    {
        for(i = 0; i < typecount; i++)
        {
            if((i % 4) == 0)
            {
                Com_Strcat("\n        ");
            }

            Com_Strcat("%i ",
                Com_GetCartOffset(types, CHUNK_INFO_OFFSET(i), 0));
        }
    }
    Com_Strcat("\n    }\n\n");
    Com_Strcat("}\n\n");
}

//
// ProcessSpawnActions
//

static void ProcessSpawnActions(byte *data, int index)
{
    byte *actions   = Com_GetCartData(
        Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0),
        CHUNK_MDLROOT_ACTIONS, 0);
    int size        = Com_GetCartOffset(actions, CHUNK_ACTIONS_SIZE, 0);
    int count       = Com_GetCartOffset(actions, CHUNK_ACTIONS_COUNT, 0);
    int i;

    DC_DecodeData(actions, action_buffer, 0);

    Com_Strcat("behaviors = // [frame## action## arg0 arg1 arg2 arg3]\n");
    Com_Strcat("{\n");
    Com_Strcat("    numbehaviors = %i\n\n", count);

    for(i = 0; i < count; i++)
    {
        int offset = i * size;
        factions_t *fa = (factions_t*)(action_buffer + 8 + offset);

        Com_Strcat("    { %i %i %f %f %f %f }\n",
            fa->frame, fa->type, fa->args[0], fa->args[1], fa->args[2], fa->args[3]);
    }

    Com_Strcat("\n}\n\n");
}

//
// AddModel
//

static void AddModel(byte *data, int index)
{
    char name[256];

    Com_StrcatClear();

    ProcessProperties(modeldata, index);
    ProcessSpawnActions(modeldata, index);
    ProcessMeshes(modeldata, index);

    sprintf(name, "%smdl%03d/mdl%03d.kmesh",
        GetModelNamespace(0), index, index);
    Com_StrcatAddToFile(name);
}

//
// ProcessMovement
//

static void ProcessMovement(byte *data, int frames)
{
    int* predtable      = off_49CF1C;
    int size            = Com_GetCartOffset(data, CHUNK_MOVEMENT_SIZE, 0);
    int count           = Com_GetCartOffset(data, CHUNK_MOVEMENT_COUNT, 0);
    dboolean encoded    = Com_GetCartOffset(data, CHUNK_MOVEMENT_ENCODED, 0);
    int i;

    Com_Strcat("    numtranslationsets = %i\n", count);

    if(count <= 0)
        return;

    Com_Strcat("    translationsets =\n");
    Com_Strcat("    {\n");

    for(i = 0; i < count; i++)
    {
        Com_Strcat("        { // %i\n", i);
        if(encoded)
        {
            byte *tables[3];
            byte *cmpdata;

            cmpdata = data + 8 + (size * i);

            tables[0] = (byte *)predtable + 0 * size;
            tables[1] = (byte *)predtable + 2 * size;
            tables[2] = (byte *)predtable + 4 * size;

            DC_BuildAnimTable((int**)tables, cmpdata + 20, 3);

            if(frames > 0)
            {
                int j;

                for(j = 0; j < frames; j++)
                {
                    double x;
                    double y;
                    double z;
                    double t;
                    int dx;
                    int dy;
                    int dz;

                    x   = *(float*)(cmpdata + 0x4);
                    y   = *(float*)(cmpdata + 0x8);
                    z   = *(float*)(cmpdata + 0xc);
                    t   = *(float*)(cmpdata + 0x10);
                    dx  = *(short*)&tables[0][j * 2];
                    dy  = *(short*)&tables[1][j * 2];
                    dz  = *(short*)&tables[2][j * 2];

                    Com_Strcat("            { %f %f %f }\n",
                        (float)dx * t * flt_48EC8C + x,
                        (float)dy * t * flt_48EC8C + y,
                        (float)dz * t * flt_48EC8C + z);
                }
            }
        }
        else
        {
        }

        Com_Strcat("        }\n");
    }

    Com_Strcat("    }\n\n");
}

//
// ProcessRotation
//

static void ProcessRotation(byte *data, int frames)
{
    int* predtable  = off_49CF1C;
    int size        = Com_GetCartOffset(data, CHUNK_ROTATIONS_SIZE, 0);
    int count       = Com_GetCartOffset(data, CHUNK_ROTATIONS_COUNT, 0);
    int i;

    Com_Strcat("    numrotationsets = %i\n", count);

    if(count <= 0)
        return;

    Com_Strcat("    rotationsets =\n");
    Com_Strcat("    {\n");

    for(i = 0; i < count; i++)
    {
        byte* tables[4];

        tables[0] = (byte *)predtable + 0 * frames;
        tables[1] = (byte *)predtable + 2 * frames;
        tables[2] = (byte *)predtable + 4 * frames;
        tables[3] = (byte *)predtable + 6 * frames;

        DC_BuildAnimTable((int**)tables, data + 8 + (size * i), 4);

        Com_Strcat("        { // %i\n", i);

        if(frames > 0)
        {
            int j;

            for(j = 0; j < frames; j++)
            {
                short p;
                short r;
                short ys;
                short yc;

                p   = *(short*)&tables[0][j * 2];
                r   = *(short*)&tables[1][j * 2];
                ys  = *(short*)&tables[2][j * 2];
                yc  = *(short*)&tables[3][j * 2];

                Com_Strcat("            { %f %f %f %f }\n",
                    (float)p  * flt_48EC8C,
                    (float)r  * flt_48EC8C,
                    (float)ys * flt_48EC8C,
                    (float)yc * flt_48EC8C);
            }
        }

        Com_Strcat("        }\n");
    }

    Com_Strcat("    }\n\n");
}

//
// ProcessActions
//

static void ProcessActions(byte *data)
{
    int size        = Com_GetCartOffset(data, CHUNK_ACTIONS_SIZE, 0);
    int count       = Com_GetCartOffset(data, CHUNK_ACTIONS_COUNT, 0);
    int i;

    DC_DecodeData(data, action_buffer, 0);

    Com_Strcat("    numactions = %i\n\n", count);

    if(count <= 0)
        return;

    Com_Strcat("    actions = // [frame## action## arg0 arg1 arg2 arg3]\n");
    Com_Strcat("    {\n");

    for(i = 0; i < count; i++)
    {
        int offset = i * size;
        factions_t *fa = (factions_t*)(action_buffer + 8 + offset);

        Com_Strcat("        %i %i %f %f %f %f\n",
            fa->frame, fa->type, fa->args[0], fa->args[1], fa->args[2], fa->args[3]);
    }

    Com_Strcat("\n    }\n\n");
}

//
// ProcessAnimation
//

static void ProcessAnimation(byte *data, int index)
{
    int numframes;
    int indexes;
    byte *indextable;
    byte *initial;
    short *lookup;
    int i;

    numframes = Com_GetCartOffset(Com_GetCartData(data, CHUNK_ANIMROOT_FRAMEINFO, 0),
        CHUNK_FRAMEINFO_FRAMES, 0);

    Com_Strcat("    numframes = %i\n", numframes);

    indextable  = Com_GetCartData(data, CHUNK_ANIMROOT_INDEXES, 0);
    indexes     = Com_GetCartOffset(indextable, CHUNK_INDEXES_ENTRIES, 0);
    initial     = Com_GetCartData(data, CHUNK_ANIMROOT_INITIAL, 0);
    lookup      = (short*)(indextable + CHUNK_INDEXES_TABLE);

    Com_Strcat("    numnodes = %i\n\n", indexes);

    ProcessMovement(Com_GetCartData(data, CHUNK_ANIMROOT_MOVEMENT, 0), numframes);
    ProcessRotation(Com_GetCartData(data, CHUNK_ANIMROOT_ROTATIONS, 0), numframes);

    Com_Strcat("    nodeframes = // [translationset rotationset]\n");
    Com_Strcat("    {\n");

    for(i = 0; i < indexes; i++)
    {
        Com_Strcat("        { ");

        Com_Strcat("%i ",    lookup[TRANSLATION_INDEX]);
        Com_Strcat("%i }\n", lookup[ROTATION_INDEX]);

        lookup += 2;
    }
    Com_Strcat("\n    }\n\n");

    ProcessActions(Com_GetCartData(data, CHUNK_ANIMROOT_ACTIONS, 0));

    Com_Strcat("    initialtranslation = // [vx vy vz]\n");
    Com_Strcat("    {\n");

    for(i = 0; i < Com_GetCartOffset(initial, CHUNK_INITIAL_COUNT, 0); i++)
    {
        int size = Com_GetCartOffset(initial, CHUNK_INITIAL_SIZE, 0);
        float x = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 0));
        float y = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 1));
        float z = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 2));

        Com_Strcat("        { %f %f %f }\n", x, y, z);
    }

    Com_Strcat("\n    }\n\n");

    Com_Strcat("    initialrotation = // [qx qy qz qw]\n");
    Com_Strcat("    {\n");

    for(i = 0; i < Com_GetCartOffset(initial, CHUNK_INITIAL_COUNT, 0); i++)
    {
        int size = Com_GetCartOffset(initial, CHUNK_INITIAL_SIZE, 0);
        short *r = (short*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 3));

        Com_Strcat("        { %f %f %f %f }\n",
            (float)(r[0]) * flt_48EC8C,
            (float)(r[1]) * flt_48EC8C,
            (float)(r[2]) * flt_48EC8C,
            (float)(r[3]) * flt_48EC8C);
    }

    Com_Strcat("\n    }\n\n");
}

//
// AddAnimations
//

static void AddAnimations(byte *data, int index)
{
    int i;
    byte *anims;
    int count;

    anims = Com_GetCartData(Com_GetCartData(data, CHUNK_MODEL_OFFSET(index), 0),
        CHUNK_MDLROOT_ANIMATIONS, 0);

    count = Com_GetCartOffset(anims, CHUNK_ANIMATION_COUNT, 0);

    for(i = 0; i < count; i++)
    {
        byte *rncdata;
        byte *animdata;
        int size;
        int outsize;
        char name[256];

        rncdata = Com_GetCartData(anims, CHUNK_ANIMATION_OFFSET(i), &size);
        animdata = RNC_ParseFile(rncdata, size, &outsize);

        Com_StrcatClear();

        Com_Strcat("anim\n{\n");
        ProcessAnimation(animdata, index);
        Com_Strcat("}\n\n");

        sprintf(name, "%smdl%03d/mdl%03d_%02d.kanim",
            GetModelNamespace(0), index, index, i);

        Com_StrcatAddToFile(name);
        Com_Free(&animdata);
    }
}

//
// MDL_StoreModels
//

void MDL_StoreModels(void)
{
    int i;

    modeldata = Com_GetCartData(cartfile, CHUNK_DIRECTORY_MODEL, 0);
    nummodels = Com_GetCartOffset(modeldata, CHUNK_MODEL_COUNT, 0);

    PK_AddFolder("models/");
    StoreExternalFile("default.kmesh", "models/default.kmesh");
    /*PK_AddFolder("models/actors/");
    PK_AddFolder("models/arenas/");
    PK_AddFolder("models/caves/");
    PK_AddFolder("models/dynamic/");
    PK_AddFolder("models/items/");
    PK_AddFolder("models/jungle/");
    PK_AddFolder("models/lostworld/");
    PK_AddFolder("models/misc/");
    PK_AddFolder("models/ruins/");
    PK_AddFolder("models/tech/");
    PK_AddFolder("models/temple/");
    PK_AddFolder("models/village/");
    PK_AddFolder("models/weapons/");*/

    for(i = 0; i < nummodels; i++)
    {
        char name[256];

        sprintf(name, "%smdl%03d/", GetModelNamespace(0), i);
        PK_AddFolder(name);

        curmodel = i;

        AddModel(modeldata, i);
        AddAnimations(modeldata, i);

        // do garbage collection / cleanup
        Z_FreeTags(PU_STATIC, PU_STATIC);
    }
}