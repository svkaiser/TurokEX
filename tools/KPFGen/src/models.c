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

extern const char *sndfxnames[];
void FX_GetName(int index, char *name);

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

#define CHUNK_UNKNOWNA_SIZE         0
#define CHUNK_UNKNOWNA_COUNT        4
#define CHUNK_UNKNOWNA_DATA(x)      (4 + (x * 4))

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
static short curnode;
static short curmesh;

short model_nodeCount[800];
short model_meshCount[800][100];
short section_count[800][100][100];
short section_textures[800][100][100][100];
byte model_masked[800];
bbox mdlboxes[1000];

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
            Com_Strcat("%s = 1\n", flagnames[i].name);
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
            Com_Strcat("\n    ");
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

    Com_Strcat("xyz =\n");
    Com_Strcat("{\n");

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

        Com_Strcat("%f %f %f\n", (float)xx, (float)yy, (float)zz);
    }
    Com_Strcat("}\n\n");

    Com_Strcat("coords =\n");
    Com_Strcat("{\n");

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

        Com_Strcat("%f %f\n", uv.uvf[0], uv.uvf[1]);
    }
    Com_Strcat("}\n\n");

    Com_Strcat("normals =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        float n1;
        float n2;
        float n3;
    
        n1 = (float)n[(count * 0) + i] / 127.0f;
        n2 = (float)n[(count * 1) + i] / 127.0f;
        n3 = (float)n[(count * 2) + i] / 127.0f;

        Com_Strcat("%f %f %f\n", n1, n2, n3);
    }
    Com_Strcat("}\n\n");
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

    section_textures[curmodel][curnode][curmesh][cursection] = header->texture;

    if(header->flags & 0x380)
    {
        model_masked[curmodel] = 1;
    }

    PrintFlags(header->flags);
    if(header->texture != -1)
    {
        Com_Strcat("texture = \"textures/tex%04d_00.tga\"\n",
            header->texture);
    }
    Com_Strcat("rgba = %i %i %i %i\n",
        header->rgba1[0], header->rgba1[1], header->rgba1[2], header->rgba1[3]);
    Com_Strcat("// rgba = %i %i %i %i\n",
        header->rgba2[0], header->rgba2[1], header->rgba2[2], header->rgba2[3]);

    Com_Strcat("\nnumtriangles = %i\n",
        Com_GetCartOffset(indices, CHUNK_INDICES_COUNT, 0));

    Com_Strcat("\ntriangles =\n");
    Com_Strcat("{\n");
    ProcessTriangles(indices);
    Com_Strcat("\n}\n\n");

    Com_Strcat("\nnumvertices = %i\n",
        vertexcount);

    Com_Strcat("vertices =\n");
    Com_Strcat("{\n");
    ProcessVertices(vertices);
    Com_Strcat("}\n\n");

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

    section_count[curmodel][curnode][curmesh] = sectioncount;

    Com_Strcat("numsections = %i\n\n", sectioncount);
    Com_Strcat("sections\n");
    Com_Strcat("{\n");
    for(i = 0; i < sectioncount; i++)
    {
        totalverts = 0;
        highidx = 0;
        hightri = 1;

        cursection = i;

        Com_Strcat("{ // section %02d\n", i);
        ProcessGeometry(Com_GetCartData(data, CHUNK_SECTIONLIST_OFFSET(i), 0));
        Com_Strcat("}\n");
    }
    Com_Strcat("}\n\n");
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

    model_meshCount[curmodel][curnode] = numobjects;

    children = Com_GetCartOffset(matrix, CHUNK_MATRIX_COUNT, 0);

    Com_Strcat("numchildren = %i\n", children);

    if(children > 0)
    {
        Com_Strcat("children = { ");

        for(i = 0; i < children; i++)
        {
            Com_Strcat("%i ",
                Com_GetCartOffset(matrix, CHUNK_MATRIX_OFFSET(i), 0));
        }

        Com_Strcat("}\n");
    }

    variantcount = Com_GetCartOffset(variants, CHUNK_VARIATIONS_COUNT, 0);
    Com_Strcat("\nnumvariants = %i\n", variantcount);
    Com_Strcat("variants = { ");
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

    Com_Strcat("numgroups = %i\n\n", numobjects);
    Com_Strcat("groups\n");
    Com_Strcat("{\n");
    for(i = 0; i < numobjects; i++)
    {
        Com_Strcat("{ // group %02d\n", i);
        curmesh = i;
        ProcessGroup(Com_GetCartData(objects, CHUNK_OBJECTS_OFFSET(i), 0));
        Com_Strcat("}\n");
    }
    Com_Strcat("}\n\n");
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

    Com_Strcat("numnodes = %i\n\n", limbcount);
    model_masked[curmodel] = 0;
    model_nodeCount[curmodel] = limbcount;

    Com_Strcat("nodes\n");
    Com_Strcat("{\n");
    for(i = 0; i < limbcount; i++)
    {
        Com_Strcat("{ // node %02d\n", i);
        curnode = i;
        ProcessLimbData(Com_GetCartData(data, CHUNK_LIMBINDEX_OFFSET(i), 0));
        Com_Strcat("}\n");
    }
    Com_Strcat("}\n\n");

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
        Com_Strcat("{ %i \"anim%02d\" \"%smdl%03d/mdl%03d_%02d.kanim\" }\n",
            Com_GetCartOffset(types, CHUNK_INFO_OFFSET(i), 0),
            i, GetModelNamespace(0), index, index, i);
    }
    Com_Strcat("}\n\n");

    Com_Strcat("info\n{\n");

    if(bboxsize < 24)
    {
        Com_Strcat("bbox = { 0.0 0.0 0.0 0.0 0.0 0.0 }\n");
    }
    else
    {
        float *fbbox = (float*)bbox;

        Com_Strcat("bbox = { %f %f %f %f %f %f }\n",
            fbbox[0], fbbox[1], fbbox[2], fbbox[3], fbbox[4], fbbox[5]);

        mdlboxes[index][0] = fbbox[0];
        mdlboxes[index][1] = fbbox[1];
        mdlboxes[index][2] = fbbox[2];
        mdlboxes[index][3] = fbbox[3];
        mdlboxes[index][4] = fbbox[4];
        mdlboxes[index][5] = fbbox[5];
    }

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
    Com_Strcat("numbehaviors = %i\n\n", count);

    for(i = 0; i < count; i++)
    {
        int offset = i * size;
        factions_t *fa = (factions_t*)(action_buffer + 8 + offset);

        Com_Strcat("{ %i %i %f %f %f %f }\n",
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
    Com_SetDataProgress(3);

    ProcessProperties(modeldata, index);
    Com_UpdateDataProgress();

    //ProcessSpawnActions(modeldata, index);
    Com_UpdateDataProgress();

    ProcessMeshes(modeldata, index);
    Com_UpdateDataProgress();

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

    Com_Strcat("numtranslationsets = %i\n", count);

    if(count <= 0)
        return;

    Com_Strcat("translationsets =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        byte *cmpdata = data + 8 + (size * i);

        Com_Strcat("{ // %i\n", i);

        if(encoded)
        {
            byte *tables[3];

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

                    Com_Strcat("{ %f %f %f }\n",
                        (float)dx * t * flt_48EC8C + x,
                        (float)dy * t * flt_48EC8C + y,
                        (float)dz * t * flt_48EC8C + z);
                }
            }
        }
        else
        {
            float *xyz = (float*)(cmpdata + 20);

            int j;

            for(j = 0; j < frames; j++, xyz += 3)
            {
                Com_Strcat("{ %f %f %f }\n",
                    xyz[0], xyz[1], xyz[2]);
            }
        }

        Com_Strcat("}\n");
    }

    Com_Strcat("}\n\n");
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

    Com_Strcat("numrotationsets = %i\n", count);

    if(count <= 0)
        return;

    Com_Strcat("rotationsets =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        byte* tables[4];

        tables[0] = (byte *)predtable + 0 * frames;
        tables[1] = (byte *)predtable + 2 * frames;
        tables[2] = (byte *)predtable + 4 * frames;
        tables[3] = (byte *)predtable + 6 * frames;

        DC_BuildAnimTable((int**)tables, data + 8 + (size * i), 4);

        Com_Strcat("{ // %i\n", i);

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

                Com_Strcat("{ %f %f %f %f }\n",
                    (float)p  * flt_48EC8C,
                    (float)r  * flt_48EC8C,
                    (float)ys * flt_48EC8C,
                    (float)yc * flt_48EC8C);
            }
        }

        Com_Strcat("}\n");
    }

    Com_Strcat("}\n\n");
}

//
// CheckIfFXEvent
//

static int CheckIfFXEvent(int id)
{
    int i;

    for(i = 125; i <= 154; i++)
    {
        if(i == id)
            return 86 + (i-125);
    }

    for(i = 161; i <= 230; i++)
    {
        if(i == id)
            return 124 + (i-161);
    }

    for(i = 260; i <= 319; i++)
    {
        if(i == 273)
            continue;

        if(i == id)
            return 202 + (i-260);
    }

    for(i = 321; i <= 360; i++)
    {
        if(i == 338)
            continue;

        if(i == 353)
            continue;

        if(i == id)
            return 262 + (i-321);
    }

    for(i = 408; i <= 467; i++)
    {
        if(i == 463)
            continue;

        if(i == id)
            return 315 + (i-408);
    }

    switch(id)
    {
    case 49:
        return 318;
    case 95:
        return 9;
    case 96:
        return 42;
    case 97:
        return 43;
    case 98:
        return 81;
    case 99:
        return 33;
    case 100:
        return 34;
    case 101:
        return 35;
    case 104:
        return 53;
    case 105:
        return 52;
    case 106:
        return 54;
    case 113:
        return 55;
    case 114:
        return 56;
    case 115:
        return 58;
    case 116:
        return 57;
    case 117:
        return 59;
    case 118:
        return 82;
    case 119:
        return 83;
    case 120:
        return 84;
    }

    return -1;
}

//
// GetActionName
//

char *GetActionName(int id, float arg0)
{
    static char actionName[512];

    sprintf(actionName, "unknown");

    switch(id)
    {
    case 18:
        sprintf(actionName, "spawnShockwave");
        break;
    case 19:
        sprintf(actionName, "shockwaveExplosion");
        break;
    case 20:
        sprintf(actionName, "shamanProjectile");
        break;
    case 22:
        sprintf(actionName, "swooshSound");
        break;
    case 25:
        sprintf(actionName, "fireWeapon");
        break;
    case 31:
        sprintf(actionName, "veryWimpyMelee");
        break;
    case 32:
        sprintf(actionName, "wimpyMelee");
        break;
    case 33:
        sprintf(actionName, "weakMelee");
        break;
    case 34:
        sprintf(actionName, "melee");
        break;
    case 35:
        sprintf(actionName, "strongMelee");
        break;
    case 36:
        sprintf(actionName, "veryStrongMelee");
        break;
    case 37:
        sprintf(actionName, "heavyMelee");
        break;
    case 38:
        sprintf(actionName, "veryHeavyMelee");
        break;
    case 39:
        sprintf(actionName, "fatalMelee");
        break;
    case 40:
        sprintf(actionName, "veryFatalMelee");
        break;
    case 41:
        sprintf(actionName, "deathBlow");
        break;
    case 44:
        sprintf(actionName, "footstepPuff");
        break;
    case 46:
        sprintf(actionName, "alertScream");
        break;
    case 48:
        sprintf(actionName, "roboShutdown");
        break;
    case 53:
        sprintf(actionName, "deathScream");
        break;
    case 55:
        sprintf(actionName, "footstepSound");
        break;
    case 56:
        sprintf(actionName, "injuryScream");
        break;
    case 58:
        sprintf(actionName, "stomp");
        break;
    case 70:
        sprintf(actionName, "violentDeathScream");
        break;
    case 72:
        sprintf(actionName, "deathYelp");
        break;
    case 92:
        sprintf(actionName, "weakKnockBack");
        break;
    case 122:
        sprintf(actionName, "arrowDamage");
        break;
    case 123:
        sprintf(actionName, "customDamage");
        break;
    case 160:
        sprintf(actionName, "shockwaveDamage");
        break;
    case 238:
        sprintf(actionName, "spawnItem");
        break;
    case 248:
        sprintf(actionName, "\"playSound\" \"sounds/shaders/%s.ksnd\"",
            sndfxnames[(int)arg0]);
        break;
    case 273:
        sprintf(actionName, "throwGrenade");
        break;
    case 338:
        sprintf(actionName, "fireRaptorMissile");
        break;
    case 353:
        sprintf(actionName, "fireDart");
        break;
    case 362:
        sprintf(actionName, "weakExplosion");
        break;
    case 368:
        sprintf(actionName, "grenadeExplode");
        break;
    case 373:
        sprintf(actionName, "strongKnockBack");
        break;
    case 374:
        sprintf(actionName, "veryWimpyFleshMelee");
        break;
    case 375:
        sprintf(actionName, "wimpyFleshMelee");
        break;
    case 376:
        sprintf(actionName, "weakFleshMelee");
        break;
    case 377:
        sprintf(actionName, "fleshMelee");
        break;
    case 378:
        sprintf(actionName, "strongFleshMelee");
        break;
    case 379:
        sprintf(actionName, "veryStrongFleshMelee");
        break;
    case 380:
        sprintf(actionName, "heavyFleshMelee");
        break;
    case 381:
        sprintf(actionName, "veryHeavyFleshMelee");
        break;
    case 382:
        sprintf(actionName, "fatalFleshMelee");
        break;
    case 383:
        sprintf(actionName, "veryFatalFleshMelee");
        break;
    case 384:
        sprintf(actionName, "fleshDeathBlow");
        break;
    case 385:
        sprintf(actionName, "veryWimpyBluntMelee");
        break;
    case 386:
        sprintf(actionName, "wimpyBluntMelee");
        break;
    case 387:
        sprintf(actionName, "weakBluntMelee");
        break;
    case 388:
        sprintf(actionName, "bluntMelee");
        break;
    case 389:
        sprintf(actionName, "strongBluntMelee");
        break;
    case 390:
        sprintf(actionName, "veryStrongBluntMelee");
        break;
    case 391:
        sprintf(actionName, "heavyBluntMelee");
        break;
    case 392:
        sprintf(actionName, "veryHeavyBluntMelee");
        break;
    case 393:
        sprintf(actionName, "fatalBluntMelee");
        break;
    case 394:
        sprintf(actionName, "veryFatalBluntMelee");
        break;
    case 395:
        sprintf(actionName, "bluntDeathBlow");
        break;
    case 463:
        sprintf(actionName, "demonProjectile");
        break;
    case 407:
        sprintf(actionName, "removeSelf");
        break;
    case 471:
        sprintf(actionName, "veryWimpyAcidDamage");
        break;
    default:
        strcpy(actionName, va("action_%03d", id));
        break;
    }

    return actionName;
}

//
// GetActionFunction
//

static char *GetActionFunction(factions_t *action)
{
    int fx = CheckIfFXEvent(action->type);

    if(fx != -1)
    {
        char name[256];

        FX_GetName(fx, name);

        return va("\"fx\" \"%s\" %f %f %f",
            name, action->args[1],
            action->args[2], action->args[3]);
    }

    if(action->type == 248)
    {
        return va("%s %f %f %f",
            GetActionName(action->type, action->args[0]),
            action->args[1], action->args[2], action->args[3]);
    }

    return va("\"%s\" %f %f %f %f",
            GetActionName(action->type, action->args[0]),
            action->args[0], action->args[1], action->args[2], action->args[3]);
}

//
// ProcessActions
//

static void ProcessActions(byte *data, int animIndex)
{
    int size        = Com_GetCartOffset(data, CHUNK_ACTIONS_SIZE, 0);
    int count       = Com_GetCartOffset(data, CHUNK_ACTIONS_COUNT, 0);
    int i;

    DC_DecodeData(data, action_buffer, 0);

    Com_Strcat("numactions = %i\n\n", count);

    if(count <= 0)
        return;

    Com_Strcat("actions =\n");
    Com_Strcat("{\n");

    for(i = 0; i < count; i++)
    {
        int offset = i * size;
        factions_t *fa = (factions_t*)(action_buffer + 8 + offset);

        // HACK
        if(curmodel == 396 && animIndex == 22 && fa->type == 407 && fa->frame == 62)
            fa->frame = 61;

        Com_Strcat("%i %s\n",
            fa->frame, GetActionFunction(fa));
    }

    Com_Strcat("\n    }\n\n");
}

//
// ProcessAnimation
//

static void ProcessAnimation(byte *data, int index, int animIndex)
{
    int numframes;
    int indexes;
    byte *frameinfo;
    byte *indextable;
    byte *initial;
    byte *lframe;
    short *lookup;
    int count;
    int i;

    frameinfo = Com_GetCartData(data, CHUNK_ANIMROOT_FRAMEINFO, 0);
    numframes = Com_GetCartOffset(frameinfo, CHUNK_FRAMEINFO_FRAMES, 0);

    Com_Strcat("numframes = %i\n", numframes);

    indextable  = Com_GetCartData(data, CHUNK_ANIMROOT_INDEXES, 0);
    indexes     = Com_GetCartOffset(indextable, CHUNK_INDEXES_ENTRIES, 0);
    initial     = Com_GetCartData(data, CHUNK_ANIMROOT_INITIAL, 0);
    lframe      = Com_GetCartData(data, CHUNK_ANIMROOT_UNKNOWN2, 0);
    lookup      = (short*)(indextable + CHUNK_INDEXES_TABLE);

    if(Com_GetCartOffset(lframe, CHUNK_UNKNOWNA_SIZE, 0) != 0 &&
        Com_GetCartOffset(lframe, CHUNK_UNKNOWNA_COUNT, 0) != 0)
    {
        short *rf = (short*)Com_GetCartData(lframe, CHUNK_UNKNOWNA_DATA(0), 0);
        Com_Strcat("// %i\n", rf[0]);
        Com_Strcat("loopframe = %i\n\n", rf[1]);
    }

    Com_Strcat("numnodes = %i\n\n", indexes);

    if(numframes > 0)
    {
        short *turninfo = (short*)(frameinfo+8);

        Com_Strcat("turninfo = {\n");
        for(i = 0; i < numframes; i++)
        {
            Com_Strcat("%f\n", *turninfo * 0.00003051850944757462f);
            turninfo++;
        }
        Com_Strcat("}\n");
    }

    ProcessMovement(Com_GetCartData(data, CHUNK_ANIMROOT_MOVEMENT, 0), numframes);
    ProcessRotation(Com_GetCartData(data, CHUNK_ANIMROOT_ROTATIONS, 0), numframes);

    Com_Strcat("nodeframes = // [translationset rotationset]\n");
    Com_Strcat("{\n");

    for(i = 0; i < indexes; i++)
    {
        Com_Strcat("{ ");

        Com_Strcat("%i ",    lookup[TRANSLATION_INDEX]);
        Com_Strcat("%i }\n", lookup[ROTATION_INDEX]);

        lookup += 2;
    }
    Com_Strcat("\n    }\n\n");

    ProcessActions(Com_GetCartData(data, CHUNK_ANIMROOT_ACTIONS, 0), animIndex);

    Com_Strcat("initialtranslation = // [vx vy vz]\n");
    Com_Strcat("{\n");

    count = Com_GetCartOffset(initial, CHUNK_INITIAL_COUNT, 0);
    for(i = 0; i < count; i++)
    {
        int size = Com_GetCartOffset(initial, CHUNK_INITIAL_SIZE, 0);
        float x = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 0));
        float y = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 1));
        float z = *(float*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 2));

        Com_Strcat("{ %f %f %f }\n", x, y, z);
    }

    if(count < indexes)
    {
        for(i = 0; i < (indexes - count); i++)
        {
            Com_Strcat("{ 0 0 0 }\n");
        }
    }

    Com_Strcat("\n}\n\n");

    Com_Strcat("initialrotation = // [qx qy qz qw]\n");
    Com_Strcat("{\n");

    count = Com_GetCartOffset(initial, CHUNK_INITIAL_COUNT, 0);
    for(i = 0; i < count; i++)
    {
        int size = Com_GetCartOffset(initial, CHUNK_INITIAL_SIZE, 0);
        short *r = (short*)((initial + CHUNK_INITIAL_DATA) + (size * i) + (4 * 3));

        Com_Strcat("{ %f %f %f %f }\n",
            (float)(r[0]) * flt_48EC8C,
            (float)(r[1]) * flt_48EC8C,
            (float)(r[2]) * flt_48EC8C,
            (float)(r[3]) * flt_48EC8C);
    }

    if(count < indexes)
    {
        for(i = 0; i < (indexes - count); i++)
        {
            Com_Strcat("{ 0 0 0 0 }\n");
        }
    }

    Com_Strcat("\n}\n\n");
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
        ProcessAnimation(animdata, index, i);
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

    memset(mdlboxes, 0, sizeof(bbox) * 1000);

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