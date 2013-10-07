// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: Model data management
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "renderModel.h"
#include "animation.h"

kexModelManager modelManager;

enum {
    scmdl_info = 0,
    scmdl_numanims,
    scmdl_animsets,
    scmdl_bbox,
    scmdl_types,
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

static const sctokens_t mdltokens[scmdl_end+1] = {
    { scmdl_info,           "info"          },
    { scmdl_numanims,       "numanims"      },
    { scmdl_animsets,       "animsets"      },
    { scmdl_bbox,           "bbox"          },
    { scmdl_types,          "types"         },
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

typedef struct {
    const char  *name;
    int         flag;
} mdlflagnames_t;

static const mdlflagnames_t mdlflagnames[17] = {
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
// kexModelManager::kexModelManager
//

kexModelManager::kexModelManager(void) {
}

//
// kexModelManager::~kexModelManager
//

kexModelManager::~kexModelManager(void) {
}

//
// kexModelManager::ParseKMesh
//

void kexModelManager::ParseKMesh(kexModel_t *model, kexLexer *lexer) {
    unsigned int i;
    unsigned int j;
    unsigned int k;
    unsigned int l;
    byte r, g, b, a;
    bool bNested = false;

    while(lexer->CheckState()) {
        lexer->Find();

        switch(lexer->TokenType()) {
        case TK_NONE:
            break;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            // there are three main blocks for a kmesh file
            switch(lexer->GetIDForTokenList(mdltokens, lexer->Token())) {
                // info block (bounding box, etc)
            case scmdl_info:
                break;
                // model block (geometry)
            case scmdl_model:
                lexer->ExpectNextToken(TK_LBRACK);
                lexer->AssignFromTokenList(mdltokens, &model->numNodes,
                    scmdl_numnodes, true);

                // begin reading into the node block
                lexer->ExpectTokenListID(mdltokens, scmdl_nodes);
                lexer->ExpectNextToken(TK_LBRACK);
                model->nodes = (modelNode_t*)Z_Calloc(sizeof(modelNode_t) *
                    model->numNodes, PU_MODEL, 0);

                for(i = 0; i < model->numNodes; i++) {
                    modelNode_t *node = &model->nodes[i];

                    // read into nested node block
                    lexer->ExpectNextToken(TK_LBRACK);

                    lexer->AssignFromTokenList(mdltokens, &node->numChildren,
                        scmdl_numchildren, true);

                    if(node->numChildren > 0) {
                        lexer->AssignFromTokenList(mdltokens, AT_SHORT,
                            (void**)&node->children, node->numChildren,
                            scmdl_children, true, PU_MODEL);
                    }

                    lexer->AssignFromTokenList(mdltokens, &node->numVariants,
                        scmdl_numvariants, true);

                    // must have a variant
                    if(node->numVariants <= 0) {
                        parser.Error("Variant count for node %i should be at least 1 or more", i);
                    }

                    lexer->AssignFromTokenList(mdltokens, AT_SHORT,
                        (void**)&node->variants, node->numVariants,
                        scmdl_variants, true, PU_MODEL);

                    lexer->AssignFromTokenList(mdltokens, &node->numSurfaceGroups,
                        scmdl_numgroups, true);

                    // must have a mesh
                    if(node->numSurfaceGroups <= 0) {
                        parser.Error("Surface group count for node %i should be at least 1 or more", i);
                    }

                    // read into the group block
                    node->surfaceGroups = (surfaceGroup_t*)Z_Calloc(sizeof(surfaceGroup_t) *
                        node->numSurfaceGroups, PU_MODEL, 0);

                    lexer->ExpectTokenListID(mdltokens, scmdl_groups);
                    lexer->ExpectNextToken(TK_LBRACK);

                    for(j = 0; j < node->numSurfaceGroups; j++) {
                        surfaceGroup_t *group = &node->surfaceGroups[j];

                        // read into the nested group block
                        lexer->ExpectNextToken(TK_LBRACK);
                        lexer->AssignFromTokenList(mdltokens, &group->numSurfaces,
                            scmdl_numsections, true);

                        // read into surface block
                        bNested = false;
                        if(group->numSurfaces == 0) {
                            group->surfaces = NULL;
                            lexer->ExpectTokenListID(mdltokens, scmdl_sections);
                            lexer->ExpectNextToken(TK_LBRACK);
                            lexer->ExpectNextToken(TK_RBRACK);
                        }
                        else {
                            group->surfaces = (surface_t*)Z_Calloc(sizeof(surface_t) *
                                group->numSurfaces, PU_MODEL, 0);

                            lexer->ExpectTokenListID(mdltokens, scmdl_sections);
                            lexer->ExpectNextToken(TK_LBRACK);

                            for(k = 0; k < group->numSurfaces; k++) {
                                surface_t *surface = &group->surfaces[k];

                                // read into the nested surface block
                                lexer->ExpectNextToken(TK_LBRACK);

                                // this is the only block that doesn't require the fields
                                // to be in a certain order
                                while(lexer->TokenType() != TK_RBRACK || bNested != false) {
                                    // required so the lexer doesn't get confused with the
                                    // closing bracket of the section block and the nested
                                    // section blocks
                                    bNested = false;

                                    lexer->Find();

                                    switch(lexer->GetIDForTokenList(mdltokens, lexer->Token())) {
                                    case scmdl_texture:
                                        lexer->AssignFromTokenList(mdltokens, surface->texturePath,
                                            scmdl_texture, false);
                                        break;
                                    case scmdl_rgba1:

                                        lexer->ExpectNextToken(TK_EQUAL);

                                        r = lexer->GetNumber();
                                        g = lexer->GetNumber();
                                        b = lexer->GetNumber();
                                        a = lexer->GetNumber();

                                        surface->color1 = RGBA(r, g, b, a);
                                        break;
                                    case scmdl_numtriangles:
                                        lexer->AssignFromTokenList(mdltokens, &surface->numIndices,
                                            scmdl_numtriangles, false);
                                        surface->numIndices *= 3;
                                        break;
                                    case scmdl_triangles:
                                        bNested = true;
                                        lexer->AssignFromTokenList(mdltokens, AT_SHORT,
                                            (void**)&surface->indices, surface->numIndices,
                                            scmdl_triangles, false, PU_MODEL);
                                        break;
                                    case scmdl_numvertices:
                                        lexer->AssignFromTokenList(mdltokens, &surface->numVerts,
                                            scmdl_numvertices, false);
                                        break;
                                    case scmdl_vertices:
                                        bNested = true;
                                        lexer->ExpectNextToken(TK_EQUAL);
                                        lexer->ExpectNextToken(TK_LBRACK);
                                        lexer->AssignFromTokenList(mdltokens, AT_VECTOR,
                                            (void**)&surface->vertices, surface->numVerts,
                                            scmdl_xyz, true, PU_MODEL);
                                        lexer->AssignFromTokenList(mdltokens, AT_FLOAT,
                                            (void**)&surface->coords, surface->numVerts * 2,
                                            scmdl_coords, true, PU_MODEL);
                                        lexer->AssignFromTokenList(mdltokens, AT_FLOAT,
                                            (void**)&surface->normals, surface->numVerts * 3,
                                            scmdl_normals, true, PU_MODEL);
                                        lexer->ExpectNextToken(TK_RBRACK);
                                        break;
                                    default:
                                        for(l = 0; l < 17; l++) {
                                            if(!strcmp(lexer->Token(), mdlflagnames[l].name)) {
                                                lexer->ExpectNextToken(TK_EQUAL);
                                                if(lexer->GetNumber()) {
                                                    surface->flags |= mdlflagnames[l].flag;
                                                    break;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                            }

                            // end of surface block
                            lexer->ExpectNextToken(TK_RBRACK);
                        }

                        // end of nested group block
                        lexer->ExpectNextToken(TK_RBRACK);
                    }

                    // end of group block
                    lexer->ExpectNextToken(TK_RBRACK);

                    // end of nested node block
                    lexer->ExpectNextToken(TK_RBRACK);
                }

                // end of node block
                lexer->ExpectNextToken(TK_RBRACK);

                // end of model block
                lexer->ExpectNextToken(TK_RBRACK);
                break;
                // numanims variable
            case scmdl_numanims:
                lexer->ExpectNextToken(TK_EQUAL);
                model->numAnimations = lexer->GetNumber();
                break;
                // animsets
            case scmdl_animsets:
                lexer->ExpectNextToken(TK_EQUAL);
                lexer->ExpectNextToken(TK_LBRACK);
                if(model->numAnimations > 0) {
                    model->anims = (kexAnim_t*)Z_Calloc(sizeof(kexAnim_t) *
                        model->numAnimations, PU_MODEL, 0);

                    for(i = 0; i < model->numAnimations; i++) {
                        lexer->ExpectNextToken(TK_LBRACK);
                        model->anims[i].animID = lexer->GetNumber();
                        lexer->GetString();
                        model->anims[i].alias = Z_Strdup(lexer->StringToken(), PU_MODEL, 0);
                        lexer->GetString();
                        memcpy(model->anims[i].animFile, lexer->StringToken(), MAX_FILEPATH);
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                }
                lexer->ExpectNextToken(TK_RBRACK);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

//
// kexModelManager::ParseWavefrontObj
//

void kexModelManager::ParseWavefrontObj(kexModel_t *model, kexLexer *lexer) {
}

//
// kexModelManager::LoadModel
//

kexModel_t *kexModelManager::LoadModel(const char *file) {
    if(file == NULL) {
        return NULL;
    }
    else if(file[0] == 0) {
        return NULL;
    }

    kexModel_t *model = NULL;

    // is the model already parsed/allocated?
    if(!(model = modelList.Find(file))) {
        kexLexer *lexer;

        if(!(lexer = parser.Open(file))) {
            return NULL;
        }

        model = modelList.Create(file);

        // begin parsing
        if(strstr(file, ".kmesh")) {
            ParseKMesh(model, lexer);
        }
        else if(strstr(file, ".obj")) {
            ParseWavefrontObj(model, lexer);
        }

        modelList.Add(model);

        // we're done with the file
        parser.Close();

        kexAnimState::LoadKAnim(model);
    }

    return model;
}
