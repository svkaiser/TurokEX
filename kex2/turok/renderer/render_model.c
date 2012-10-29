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
// DESCRIPTION: Model System
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "kernel.h"
#include "zone.h"
#include "gl.h"
#include "script.h"
#include "render.h"

static kmodel_t *mdl_hashlist[MAX_HASH];

#define SCMDL_NOP           0
#define SCMDL_INFO          1
#define SCMDL_BBOX          2
#define SCMDL_TYPES         3
#define SCMDL_BEHAVIORS     4
#define SCMDL_NUMBEHAVIORS  5
#define SCMDL_MODEL         6
#define SCMDL_NUMNODES      7
#define SCMDL_NODES         8
#define SCMDL_NUMCHILDREN   9
#define SCMDL_CHILDREN      10
#define SCMDL_NUMVARIANTS   11
#define SCMDL_VARIANTS      12
#define SCMDL_NUMGROUPS     13
#define SCMDL_GROUPS        14
#define SCMDL_NUMSECTIONS   15
#define SCMDL_SECTIONS      16
#define SCMDL_TEXTURE       17
#define SCMDL_RGBA1         18
#define SCMDL_RGBA2         19
#define SCMDL_NUMTRIANGLES  20
#define SCMDL_TRIANGLES     21
#define SCMDL_NUMVERTICES   22
#define SCMDL_VERTICES      23
#define SCMDL_XYZ           24
#define SCMDL_COORDS        25
#define SCMDL_NORMALS       26

static const sctokens_t mdltokens[] =
{
    { SCMDL_NOP,            NULL            },
    { SCMDL_INFO,           "info"          },
    { SCMDL_BBOX,           "bbox"          },
    { SCMDL_TYPES,          "types"         },
    { SCMDL_BEHAVIORS,      "behaviors"     },
    { SCMDL_NUMBEHAVIORS,   "numbehaviors"  },
    { SCMDL_MODEL,          "model"         },
    { SCMDL_NUMNODES,       "numnodes"      },
    { SCMDL_NODES,          "nodes"         },
    { SCMDL_NUMCHILDREN,    "numchildren"   },
    { SCMDL_CHILDREN,       "children"      },
    { SCMDL_NUMVARIANTS,    "numvariants"   },
    { SCMDL_VARIANTS,       "variants"      },
    { SCMDL_NUMGROUPS,      "numgroups"     },
    { SCMDL_GROUPS,         "groups"        },
    { SCMDL_NUMSECTIONS,    "numsections"   },
    { SCMDL_SECTIONS,       "sections"      },
    { SCMDL_TEXTURE,        "texture"       },
    { SCMDL_RGBA1,          "rgba"          },
    { SCMDL_RGBA2,          "rgba2"         },
    { SCMDL_NUMTRIANGLES,   "numtriangles"  },
    { SCMDL_TRIANGLES,      "triangles"     },
    { SCMDL_NUMVERTICES,    "numvertices"   },
    { SCMDL_VERTICES,       "vertices"      },
    { SCMDL_XYZ,            "xyz"           },
    { SCMDL_COORDS,         "coords"        },
    { SCMDL_NORMALS,        "normals"       },
    { -1,                   NULL            }
};

typedef struct
{
    const char  *name;
    int         flag;
} mdlflagnames_t;

static const mdlflagnames_t mdlflagnames[17] =
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

//
// Mdl_ParseSectionBlock
//
// Majority of the model's geometry and data is contained in
// the section block
//

static void Mdl_ParseSectionBlock(mdlmesh_t *mesh, scparser_t *parser)
{
    unsigned int i;
    kbool nested = false;

    mesh->sections = (mdlsection_t*)Z_Calloc(sizeof(mdlsection_t) *
        mesh->numsections, PU_MODEL, 0);

    SC_ExpectTokenID(mdltokens, SCMDL_SECTIONS, parser);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < mesh->numsections; i++)
    {
        mdlsection_t *section = &mesh->sections[i];

        // read into the nested section block
        SC_ExpectNextToken(TK_LBRACK);
        
        // this is the only block that doesn't require the fields
        // to be in a certain order
        while(parser->tokentype != TK_RBRACK || nested != false)
        {
            // required so the parser doesn't get confused with the
            // closing bracket of the section block and the nested
            // section blocks
            nested = false;

            SC_Find();

            switch(SC_GetIDForToken(mdltokens, parser->token))
            {
                // texture
            case SCMDL_TEXTURE:
                SC_AssignString(mdltokens, section->texpath,
                    SCMDL_TEXTURE, parser, false);
                break;
                // rgba
            case SCMDL_RGBA1:
                {
                    byte r, g, b, a;

                    SC_ExpectNextToken(TK_EQUAL);

                    r = SC_GetNumber();
                    g = SC_GetNumber();
                    b = SC_GetNumber();
                    a = SC_GetNumber();

                    section->color1 = RGBA(r, g, b, a);
                }
                break;
                // number of triangle indices
            case SCMDL_NUMTRIANGLES:
                SC_AssignInteger(mdltokens, &section->numtris,
                    SCMDL_NUMTRIANGLES, parser, false);
                section->numtris *= 3;
                break;
                // triangle indice array
            case SCMDL_TRIANGLES:
                nested = true;
                SC_AssignArray(mdltokens, AT_SHORT, &section->tris, section->numtris,
                    SCMDL_TRIANGLES, parser, false, PU_MODEL);
                break;
                // number of vertices
            case SCMDL_NUMVERTICES:
                SC_AssignInteger(mdltokens, &section->numverts,
                    SCMDL_NUMVERTICES, parser, false);
                break;
                // vertex block: contains nested arrays (vertices, tex coords and normals)
            case SCMDL_VERTICES:
                nested = true;
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);
                SC_AssignArray(mdltokens, AT_VECTOR, (void*)&section->xyz, section->numverts,
                    SCMDL_XYZ, parser, true, PU_MODEL);
                SC_AssignArray(mdltokens, AT_FLOAT, &section->coords, section->numverts * 2,
                    SCMDL_COORDS, parser, true, PU_MODEL);
                SC_AssignArray(mdltokens, AT_FLOAT, (void*)&section->normals, section->numverts * 3,
                    SCMDL_NORMALS, parser, true, PU_MODEL);
                SC_ExpectNextToken(TK_RBRACK);
                break;
                // misc tokens will be considered as a flag or property name
            default:
                {
                    int j;

                    for(j = 0; j < 17; j++)
                    {
                        if(!strcmp(parser->token, mdlflagnames[j].name))
                        {
                            SC_ExpectNextToken(TK_EQUAL);
                            if(SC_GetNumber())
                            {
                                section->flags |= mdlflagnames[j].flag;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    // end of section block
    SC_ExpectNextToken(TK_RBRACK);
}

//
// Mdl_ParseMeshBlock
//

static void Mdl_ParseMeshBlock(mdlnode_t *node, scparser_t *parser)
{
    unsigned int i;

    node->meshes = (mdlmesh_t*)Z_Calloc(sizeof(mdlmesh_t) * node->nummeshes, PU_MODEL, 0);

    SC_ExpectTokenID(mdltokens, SCMDL_GROUPS, parser);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < node->nummeshes; i++)
    {
        mdlmesh_t *mesh = &node->meshes[i];

        // read into the nested mesh block
        SC_ExpectNextToken(TK_LBRACK);
        SC_AssignInteger(mdltokens, &mesh->numsections,
            SCMDL_NUMSECTIONS, parser, true);

        // read into section block
        Mdl_ParseSectionBlock(mesh, parser);

        // end of nested mesh block
        SC_ExpectNextToken(TK_RBRACK);
    }

    // end of mesh block
    SC_ExpectNextToken(TK_RBRACK);
}

//
// Mdl_ParseNodeBlock
//

static void Mdl_ParseNodeBlock(kmodel_t *model, scparser_t *parser)
{
    unsigned int i;

    model->nodes = (mdlnode_t*)Z_Calloc(sizeof(mdlnode_t) * model->numnodes, PU_MODEL, 0);

    for(i = 0; i < model->numnodes; i++)
    {
        mdlnode_t *node = &model->nodes[i];

        // read into nested node block
        SC_ExpectNextToken(TK_LBRACK);

        SC_AssignInteger(mdltokens, &node->numchildren,
            SCMDL_NUMCHILDREN, parser, true);

        if(node->numchildren > 0)
        {
            SC_AssignArray(mdltokens, AT_SHORT, &node->children, node->numchildren,
                SCMDL_CHILDREN, parser, true, PU_MODEL);
        }

        SC_AssignInteger(mdltokens, &node->numvariants,
            SCMDL_NUMVARIANTS, parser, true);

        // must have a variant
        if(node->numvariants <= 0)
        {
            SC_Error("Variant count for node %i should be at least 1 or more", i);
        }

        SC_AssignArray(mdltokens, AT_SHORT, &node->variants, node->numvariants,
            SCMDL_VARIANTS, parser, true, PU_MODEL);
        SC_AssignInteger(mdltokens, &node->nummeshes,
            SCMDL_NUMGROUPS, parser, true);

        // must have a mesh
        if(node->nummeshes <= 0)
        {
            SC_Error("Mesh count for node %i should be at least 1 or more", i);
        }

        // read into the mesh block
        Mdl_ParseMeshBlock(node, parser);

        // end of nested node block
        SC_ExpectNextToken(TK_RBRACK);
    }
}

//
// Mdl_ParseModelBlock
//

static void Mdl_ParseModelBlock(kmodel_t *model, scparser_t *parser)
{
    SC_ExpectNextToken(TK_LBRACK);
    SC_AssignInteger(mdltokens, &model->numnodes,
        SCMDL_NUMNODES, parser, true);

    // begin reading into the node block
    SC_ExpectTokenID(mdltokens, SCMDL_NODES, parser);
    SC_ExpectNextToken(TK_LBRACK);
    Mdl_ParseNodeBlock(model, parser);

    // end of node block
    SC_ExpectNextToken(TK_RBRACK);

    // end of model block
    SC_ExpectNextToken(TK_RBRACK);
}

//
// Mdl_ParseScript
//
// Main parsing routine
//

static void Mdl_ParseScript(kmodel_t *model, scparser_t *parser)
{
    while(SC_CheckScriptState())
    {
        SC_Find();

        switch(parser->tokentype)
        {
        case TK_NONE:
            break;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            {
                // there are three main blocks for a kmesh file
                switch(SC_GetIDForToken(mdltokens, parser->token))
                {
                    // info block (bounding box, etc)
                case SCMDL_INFO:
                    break;
                    // behavior block (looping action, startup action, etc)
                case SCMDL_BEHAVIORS:
                    break;
                    // model block (geometry)
                case SCMDL_MODEL:
                    Mdl_ParseModelBlock(model, parser);
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
}

//
// Mdl_DrawSection
//

void Mdl_DrawSection(kmodel_t *model, mdlsection_t *section, char *texture)
{
    texture_t *tex;
    char *texturepath;

    if(texture == NULL)
    {
        texturepath = section->texpath;
    }
    else
    {
        texturepath = texture;
    }

    if(section->flags & MDF_NOCULLFACES)
    {
        GL_SetState(GLSTATE_CULL, false);
    }
    else
    {
        GL_SetState(GLSTATE_CULL, true);
    }

    if(section->flags & MDF_MASKED)
    {
        GL_SetState(GLSTATE_BLEND, true);
        dglEnable(GL_ALPHA_TEST);
    }
    else
    {
        GL_SetState(GLSTATE_BLEND, false);
        dglDisable(GL_ALPHA_TEST);
    }

    if(section->flags & MDF_SHINYSURFACE)
    {
        dglEnable(GL_TEXTURE_GEN_S);
        dglEnable(GL_TEXTURE_GEN_T);
    }
    else
    {
        dglDisable(GL_TEXTURE_GEN_S);
        dglDisable(GL_TEXTURE_GEN_T);
    }

    if(section->flags & MDF_COLORIZE)
    {
        dglColor4ubv((byte*)&section->color1);
    }
    else
    {
        dglColor4ub(255, 255, 255, 255);
    }

    dglNormalPointer(GL_FLOAT, sizeof(float), section->normals);
    dglTexCoordPointer(2, GL_FLOAT, 0, section->coords);
    dglVertexPointer(3, GL_FLOAT, sizeof(vec3_t), section->xyz);

    tex = Tex_CacheTextureFile(texturepath, GL_REPEAT,
        section->flags & MDF_MASKED);

    if(tex)
    {
        GL_SetState(GLSTATE_TEXTURE0, true);
        GL_BindTexture(tex);
    }
    else
    {
        GL_SetState(GLSTATE_TEXTURE0, false);
        dglColor4ubv((byte*)&section->color1);
    }

    dglDrawElements(GL_TRIANGLES, section->numtris, GL_UNSIGNED_SHORT, section->tris);
}

//
// Mdl_TraverseDrawNode
//

void Mdl_TraverseDrawNode(kmodel_t *model, mdlnode_t *node, char **textures)
{
    unsigned int i;
    unsigned int j;

    dglPushMatrix();

    if(node->nummeshes > 0)
    {
        for(i = 0; i < node->nummeshes; i++)
        {
            for(j = 0; j < node->meshes[i].numsections; j++)
            {
                mdlsection_t *section = &node->meshes[i].sections[j];
                char *texturepath = NULL;

                if(textures != NULL)
                {
                    if(textures[j][0] != '-')
                    {
                        texturepath = textures[j];
                    }
                }

                Mdl_DrawSection(model, section, texturepath);
            }
        }
    }

    for(i = 1; i < node->numchildren; i++)
    {
        Mdl_TraverseDrawNode(model,
            &model->nodes[node->children[i]], textures);
    }

    dglPopMatrix();
}

//
// Mdl_Find
//

kmodel_t *Mdl_Find(const char *name)
{
    kmodel_t *model;
    unsigned int hash;

    hash = Com_HashFileName(name);

    for(model = mdl_hashlist[hash]; model; model = model->next)
    {
        if(!strcmp(name, model->mdlpath))
        {
            return model;
        }
    }

    return NULL;
}

//
// Mdl_Load
//
// Loads a kmesh file and parses the contents
//

kmodel_t *Mdl_Load(const char *file)
{
    kmodel_t *model;

    // is the model already parsed/allocated?
    if(!(model = Mdl_Find(file)))
    {
        scparser_t *parser;
        unsigned int hash;

        if(!(parser = SC_Open(file)))
        {
            return NULL;
        }

        model = (kmodel_t*)Z_Calloc(sizeof(kmodel_t), PU_MODEL, 0);
        strncpy(model->mdlpath, file, MAX_FILEPATH);

        // begin parsing
        Mdl_ParseScript(model, parser);

        // add to hash for future reference
        hash = Com_HashFileName(model->mdlpath);
        model->next = mdl_hashlist[hash];
        mdl_hashlist[hash] = model;

        // we're done with the file
        SC_Close();
    }

    return model;
}

//
// FCmd_LoadTestModel
//

static void FCmd_LoadTestModel(void)
{
    kmodel_t *model;
    int time;

    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    time = Sys_GetMilliseconds();

    model = Mdl_Load(Cmd_GetArgv(1));
    if(model == NULL)
    {
        return;
    }

    Com_DPrintf("\nloadtime: %f seconds\n\n",
        (float)(Sys_GetMilliseconds() - time) / 1000.0f);
}

//
// Mdl_Init
//

void Mdl_Init(void)
{
    Cmd_AddCommand("loadmodel", FCmd_LoadTestModel);
}
