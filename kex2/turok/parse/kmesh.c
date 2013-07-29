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
// DESCRIPTION: Loading of KMESH files
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "render.h"
#include "parse.h"

static kmodel_t *mdl_hashlist[MAX_HASH];

enum
{
    scmdl_info = 0,
    scmdl_numanims,
    scmdl_animsets,
    scmdl_bbox,
    scmdl_types,
    scmdl_behaviors,
    scmdl_numbehaviors,
    scmdl_model,
    scmdl_numnodes,
    scmdl_nodes,
    scmdl_numchildren,
    scmdl_children,
    scmdl_numvariants,
    scmdl_variants,
    scmdl_numgroups,
    scmdl_groups,
    scmdl_numsections,
    scmdl_sections,
    scmdl_texture,
    scmdl_rgba1,
    scmdl_rgba2,
    scmdl_numtriangles,
    scmdl_triangles,
    scmdl_numvertices,
    scmdl_vertices,
    scmdl_xyz,
    scmdl_coords,
    scmdl_normals,
    scmdl_end
};

static const sctokens_t mdltokens[scmdl_end+1] =
{
    { scmdl_info,           "info"          },
    { scmdl_numanims,       "numanims"      },
    { scmdl_animsets,       "animsets"      },
    { scmdl_bbox,           "bbox"          },
    { scmdl_types,          "types"         },
    { scmdl_behaviors,      "behaviors"     },
    { scmdl_numbehaviors,   "numbehaviors"  },
    { scmdl_model,          "model"         },
    { scmdl_numnodes,       "numnodes"      },
    { scmdl_nodes,          "nodes"         },
    { scmdl_numchildren,    "numchildren"   },
    { scmdl_children,       "children"      },
    { scmdl_numvariants,    "numvariants"   },
    { scmdl_variants,       "variants"      },
    { scmdl_numgroups,      "numgroups"     },
    { scmdl_groups,         "groups"        },
    { scmdl_numsections,    "numsections"   },
    { scmdl_sections,       "sections"      },
    { scmdl_texture,        "texture"       },
    { scmdl_rgba1,          "rgba"          },
    { scmdl_rgba2,          "rgba2"         },
    { scmdl_numtriangles,   "numtriangles"  },
    { scmdl_triangles,      "triangles"     },
    { scmdl_numvertices,    "numvertices"   },
    { scmdl_vertices,       "vertices"      },
    { scmdl_xyz,            "xyz"           },
    { scmdl_coords,         "coords"        },
    { scmdl_normals,        "normals"       },
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
// Kmesh_ParseSectionBlock
//
// Majority of the model's geometry and data is contained in
// the section block
//

static void Kmesh_ParseSectionBlock(mdlmesh_t *mesh, scparser_t *parser)
{
    unsigned int i;
    kbool nested = false;

    mesh->sections = (mdlsection_t*)Z_Calloc(sizeof(mdlsection_t) *
        mesh->numsections, PU_MODEL, 0);

    SC_ExpectTokenID(mdltokens, scmdl_sections, parser);
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
            case scmdl_texture:
                SC_AssignString(mdltokens, section->texpath,
                    scmdl_texture, parser, false);
                break;
                // rgba
            case scmdl_rgba1:
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
            case scmdl_numtriangles:
                SC_AssignInteger(mdltokens, &section->numtris,
                    scmdl_numtriangles, parser, false);
                section->numtris *= 3;
                break;
                // triangle indice array
            case scmdl_triangles:
                nested = true;
                SC_AssignArray(mdltokens, AT_SHORT, &section->tris, section->numtris,
                    scmdl_triangles, parser, false, PU_MODEL);
                break;
                // number of vertices
            case scmdl_numvertices:
                SC_AssignInteger(mdltokens, &section->numverts,
                    scmdl_numvertices, parser, false);
                break;
                // vertex block: contains nested arrays (vertices, tex coords and normals)
            case scmdl_vertices:
                nested = true;
                SC_ExpectNextToken(TK_EQUAL);
                SC_ExpectNextToken(TK_LBRACK);
                SC_AssignArray(mdltokens, AT_VECTOR, (void*)&section->xyz, section->numverts,
                    scmdl_xyz, parser, true, PU_MODEL);
                SC_AssignArray(mdltokens, AT_FLOAT, &section->coords, section->numverts * 2,
                    scmdl_coords, parser, true, PU_MODEL);
                SC_AssignArray(mdltokens, AT_FLOAT, (void*)&section->normals, section->numverts * 3,
                    scmdl_normals, parser, true, PU_MODEL);
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
// Kmesh_ParseMeshBlock
//

static void Kmesh_ParseMeshBlock(mdlnode_t *node, scparser_t *parser)
{
    unsigned int i;

    node->meshes = (mdlmesh_t*)Z_Calloc(sizeof(mdlmesh_t) * node->nummeshes, PU_MODEL, 0);

    SC_ExpectTokenID(mdltokens, scmdl_groups, parser);
    SC_ExpectNextToken(TK_LBRACK);

    for(i = 0; i < node->nummeshes; i++)
    {
        mdlmesh_t *mesh = &node->meshes[i];

        // read into the nested mesh block
        SC_ExpectNextToken(TK_LBRACK);
        SC_AssignInteger(mdltokens, &mesh->numsections,
            scmdl_numsections, parser, true);

        // read into section block
        Kmesh_ParseSectionBlock(mesh, parser);

        // end of nested mesh block
        SC_ExpectNextToken(TK_RBRACK);
    }

    // end of mesh block
    SC_ExpectNextToken(TK_RBRACK);
}

//
// Kmesh_ParseNodeBlock
//

static void Kmesh_ParseNodeBlock(kmodel_t *model, scparser_t *parser)
{
    unsigned int i;

    model->nodes = (mdlnode_t*)Z_Calloc(sizeof(mdlnode_t) * model->numnodes, PU_MODEL, 0);

    for(i = 0; i < model->numnodes; i++)
    {
        mdlnode_t *node = &model->nodes[i];

        // read into nested node block
        SC_ExpectNextToken(TK_LBRACK);

        SC_AssignInteger(mdltokens, &node->numchildren,
            scmdl_numchildren, parser, true);

        if(node->numchildren > 0)
        {
            SC_AssignArray(mdltokens, AT_SHORT, &node->children, node->numchildren,
                scmdl_children, parser, true, PU_MODEL);
        }

        SC_AssignInteger(mdltokens, &node->numvariants,
            scmdl_numvariants, parser, true);

        // must have a variant
        if(node->numvariants <= 0)
        {
            SC_Error("Variant count for node %i should be at least 1 or more", i);
        }

        SC_AssignArray(mdltokens, AT_SHORT, &node->variants, node->numvariants,
            scmdl_variants, parser, true, PU_MODEL);
        SC_AssignInteger(mdltokens, &node->nummeshes,
            scmdl_numgroups, parser, true);

        // must have a mesh
        if(node->nummeshes <= 0)
        {
            SC_Error("Mesh count for node %i should be at least 1 or more", i);
        }

        // read into the mesh block
        Kmesh_ParseMeshBlock(node, parser);

        // end of nested node block
        SC_ExpectNextToken(TK_RBRACK);
    }
}

//
// Kmesh_ParseScript
//
// Main parsing routine
//

static void Kmesh_ParseScript(kmodel_t *model, scparser_t *parser)
{
    unsigned int i;

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
                case scmdl_info:
                    /*SC_ExpectNextToken(TK_LBRACK);
                    SC_ExpectTokenID(mdltokens, scmdl_bbox, parser);
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    model->bbox.min[0] = (float)SC_GetFloat();
                    model->bbox.min[1] = (float)SC_GetFloat();
                    model->bbox.min[2] = (float)SC_GetFloat();
                    model->bbox.max[0] = (float)SC_GetFloat();
                    model->bbox.max[1] = (float)SC_GetFloat();
                    model->bbox.max[2] = (float)SC_GetFloat();
                    SC_ExpectNextToken(TK_RBRACK);
                    SC_ExpectNextToken(TK_RBRACK);*/
                    break;
                    // behavior block (looping action, startup action, etc)
                case scmdl_behaviors:
                    break;
                    // model block (geometry)
                case scmdl_model:
                    SC_ExpectNextToken(TK_LBRACK);
                    SC_AssignInteger(mdltokens, &model->numnodes,
                        scmdl_numnodes, parser, true);

                    // begin reading into the node block
                    SC_ExpectTokenID(mdltokens, scmdl_nodes, parser);
                    SC_ExpectNextToken(TK_LBRACK);
                    Kmesh_ParseNodeBlock(model, parser);

                    // end of node block
                    SC_ExpectNextToken(TK_RBRACK);

                    // end of model block
                    SC_ExpectNextToken(TK_RBRACK);
                    break;
                    // numanims variable
                case scmdl_numanims:
                    SC_ExpectNextToken(TK_EQUAL);
                    model->numanimations = SC_GetNumber();
                    break;
                    // animsets
                case scmdl_animsets:
                    SC_ExpectNextToken(TK_EQUAL);
                    SC_ExpectNextToken(TK_LBRACK);
                    if(model->numanimations > 0)
                    {
                        model->anims = (anim_t*)Z_Calloc(sizeof(anim_t) *
                            model->numanimations, PU_MODEL, 0);

                        for(i = 0; i < model->numanimations; i++)
                        {
                            SC_ExpectNextToken(TK_LBRACK);
                            model->anims[i].animID = SC_GetNumber();
                            SC_GetString();
                            model->anims[i].alias = Z_Strdup(parser->stringToken, PU_MODEL, 0);
                            SC_GetString();
                            memcpy(model->anims[i].animpath, parser->stringToken, MAX_FILEPATH);
                            SC_ExpectNextToken(TK_RBRACK);
                        }
                    }
                    SC_ExpectNextToken(TK_RBRACK);
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
// Kmesh_Find
//

kmodel_t *Kmesh_Find(const char *name)
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
// Kmesh_Load
//
// Loads a kmesh file and parses the contents
//

kmodel_t *Kmesh_Load(const char *file)
{
    kmodel_t *model;

    if(file == NULL)
    {
        return NULL;
    }
    else if(file[0] == 0)
    {
        return NULL;
    }

    // is the model already parsed/allocated?
    if(!(model = Kmesh_Find(file)))
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
        Kmesh_ParseScript(model, parser);

        // add to hash for future reference
        hash = Com_HashFileName(model->mdlpath);
        model->next = mdl_hashlist[hash];
        mdl_hashlist[hash] = model;

        // we're done with the file
        SC_Close();

        Kanim_Load(model);
    }

    return model;
}
