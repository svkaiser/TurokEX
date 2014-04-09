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

#ifndef EDITOR
#include "common.h"
#else
#include "editorCommon.h"
#endif
#include "script.h"
#include "renderBackend.h"
#include "renderModel.h"
#ifndef EDITOR
#include "animation.h"
#endif

kexModelManager modelManager;
kexHeapBlock hb_model("model", false, NULL, NULL);

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
    scmdl_material,
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
    { scmdl_material,       "material"      },
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
    { "unknown1",       MDF_UNKNOWN1        },
    { "fullbright",     MDF_FULLBRIGHT      },
    { "nocullfaces",    MDF_NOCULLFACES     },
    { "renderspecular", MDF_RENDERSPECULAR  },
    { "shinysurface",   MDF_SHINYSURFACE    },
    { "unknown32",      MDF_UNKNOWN32       },
    { "solid",          MDF_SOLID           },
    { "masked",         MDF_MASKED          },
    { "transparent1",   MDF_TRANSPARENT1    },
    { "transparent2",   MDF_TRANSPARENT2    },
    { "colorize",       MDF_COLORIZE        },
    { "metalsurface",   MDF_METALSURFACE    },
    { "unknown4096",    MDF_UNKNOWN4096     },
    { "unknown8192",    MDF_UNKNOWN8192     },
    { "unknown16384",   MDF_UNKNOWN16384    },
    { "unknown32768",   MDF_UNKNOWN32768    },
    { "unknown65536",   MDF_UNKNOWN65536    }
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
// kexModelManager::Shutdown
//

void kexModelManager::Shutdown(void) {
    common.Printf("Freeing model resources\n");
    Mem_Purge(hb_model);
    Mem_Purge(hb_animation);
}

//
// kexModelManager::ParseKMesh
//

void kexModelManager::ParseKMesh(kexModel_t *model, kexLexer *lexer) {
    unsigned int i;
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
                model->nodes = (modelNode_t*)Mem_Malloc(sizeof(modelNode_t) *
                    model->numNodes, hb_model);

                for(i = 0; i < model->numNodes; i++) {
                    modelNode_t *node = &model->nodes[i];

                    // read into nested node block
                    lexer->ExpectNextToken(TK_LBRACK);

                    lexer->AssignFromTokenList(mdltokens, &node->numChildren,
                        scmdl_numchildren, true);

                    if(node->numChildren > 0) {
                        lexer->AssignFromTokenList(mdltokens, AT_SHORT,
                            (void**)&node->children, node->numChildren,
                            scmdl_children, true, hb_model);
                    }

                    lexer->AssignFromTokenList(mdltokens, &node->numSurfaces,
                        scmdl_numsections, true);

                    // read into surface block
                    bNested = false;
                    if(node->numSurfaces == 0) {
                        node->surfaces = NULL;
                        lexer->ExpectTokenListID(mdltokens, scmdl_sections);
                        lexer->ExpectNextToken(TK_LBRACK);
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                    else {
                        node->surfaces = (surface_t*)Mem_Malloc(sizeof(surface_t) *
                            node->numSurfaces, hb_model);

                        lexer->ExpectTokenListID(mdltokens, scmdl_sections);
                        lexer->ExpectNextToken(TK_LBRACK);

                        for(k = 0; k < node->numSurfaces; k++) {
                            surface_t *surface = &node->surfaces[k];

                            surface->color1 = 0;
                            surface->color2 = 0;
                            surface->flags = 0;
                            surface->numIndices = 0;
                            surface->numVerts = 0;
                            surface->texturePath[0] = 0;
                            surface->indices = NULL;
                            surface->normals = NULL;
                            surface->vertices = NULL;
                            surface->rgb = NULL;
                            surface->material = NULL;

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
                                case scmdl_material:
                                    lexer->ExpectNextToken(TK_EQUAL);
                                    lexer->GetString();
                                    surface->material = Mem_Strdup(lexer->StringToken(), hb_model);
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
                                        scmdl_triangles, false, hb_model);
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
                                        scmdl_xyz, true, hb_model);
                                    lexer->AssignFromTokenList(mdltokens, AT_FLOAT,
                                        (void**)&surface->coords, surface->numVerts * 2,
                                        scmdl_coords, true, hb_model);
                                    lexer->AssignFromTokenList(mdltokens, AT_FLOAT,
                                        (void**)&surface->normals, surface->numVerts * 3,
                                        scmdl_normals, true, hb_model);
                                    surface->rgb = (byte*)Mem_Malloc(surface->numVerts * 4, hb_model);
                                    for(l = 0; l < surface->numVerts; l++) {
                                        surface->rgb[l * 4 + 0] = 255;
                                        surface->rgb[l * 4 + 1] = 255;
                                        surface->rgb[l * 4 + 2] = 255;
                                        surface->rgb[l * 4 + 3] = 255;
                                    }
                                    lexer->ExpectNextToken(TK_RBRACK);
                                    break;
                                default:
                                    for(l = 0; l < 17; l++) {
                                        if(lexer->Matches(mdlflagnames[l].name)) {
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
#ifndef EDITOR
                if(model->numAnimations > 0) {
                    model->anims = (kexAnim_t*)Mem_Malloc(sizeof(kexAnim_t) *
                        model->numAnimations, hb_model);

                    for(i = 0; i < model->numAnimations; i++) {
                        lexer->ExpectNextToken(TK_LBRACK);
                        model->anims[i].animID = lexer->GetNumber();
                        lexer->GetString();
                        model->anims[i].alias = Mem_Strdup(lexer->StringToken(), hb_model);
                        lexer->GetString();
                        memcpy(model->anims[i].animFile, lexer->StringToken(), MAX_FILEPATH);
                        lexer->ExpectNextToken(TK_RBRACK);
                    }
                }
#else
                for(i = 0; i < model->numAnimations; i++) {
                    lexer->ExpectNextToken(TK_LBRACK);
                    lexer->GetNumber();
                    lexer->GetString();
                    lexer->GetString();
                    lexer->ExpectNextToken(TK_RBRACK);
                }
#endif
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
    kexArray<float>points;
    kexArray<float>coords;
    kexArray<float>normals;
    kexArray<int>indices;
    int numObjects = 0;
    int numGroups = 0;

    while(lexer->CheckState()) {
        lexer->Find();

        if(lexer->TokenType() != TK_IDENIFIER) {
            continue;
        }

        if(lexer->Matches("o")) {
            // TODO
            lexer->Find();
            numObjects++;
        }
        else if(lexer->Matches("g")) {
            // TODO
            lexer->Find();
            numGroups++;
        }
        else if(lexer->Matches("v")) {
            points.Push((float)lexer->GetFloat() * 256.0f);
            points.Push((float)lexer->GetFloat() * 256.0f);
            points.Push((float)lexer->GetFloat() * 256.0f);
        }
        // TODO
        /*else if(lexer->Matches("vt")) {
            coords.Push((float)lexer->GetFloat());
            coords.Push((float)lexer->GetFloat());
        }
        else if(lexer->Matches("vn")) {
            normals.Push((float)lexer->GetFloat());
            normals.Push((float)lexer->GetFloat());
            normals.Push((float)lexer->GetFloat());
        }*/
        else if(lexer->Matches("f")) {
            indices.Push(lexer->GetNumber());
            indices.Push(lexer->GetNumber());
            indices.Push(lexer->GetNumber());
        }
    }

    // TODO
    if(numObjects == 0) {
        numObjects = 1;
    }

    if(numGroups == 0) {
        numGroups = 1;
    }

    model->numNodes = numObjects;
    model->nodes = (modelNode_t*)Mem_Calloc(sizeof(modelNode_t) *
        model->numNodes, hb_model);

    // objects
    for(unsigned int i = 0; i < model->numNodes; i++) {
        modelNode_t *node = &model->nodes[i];
        node->numChildren = 0;

        // TODO
        node->numSurfaces = 1;
        node->surfaces = (surface_t*)Mem_Calloc(sizeof(surface_t) *
            node->numSurfaces, hb_model);

        // surfaces
        for(unsigned int k = 0; k < node->numSurfaces; k++) {
            surface_t *surface = &node->surfaces[k];
            surface->color1 = 0xFFFFFFFF;
            surface->flags |= MDF_SOLID;

            surface->numIndices = indices.Length();
            surface->indices = (word*)Mem_Calloc(sizeof(word) *
                surface->numIndices, hb_model);

            for(unsigned int ind = 0; ind < surface->numIndices; ind++) {
                surface->indices[ind] = indices[ind] - 1;
            }

            surface->numVerts = points.Length() / 3;
            surface->vertices = (kexVec3*)Mem_Calloc(sizeof(kexVec3) *
                surface->numVerts, hb_model);
            surface->coords = (float*)Mem_Calloc(sizeof(float) *
                surface->numVerts * 2, hb_model);
            surface->normals = (float*)Mem_Calloc(sizeof(float) *
                surface->numVerts * 3, hb_model);

            for(unsigned int v = 0; v < surface->numVerts; v++) {
                surface->vertices[v].x = points[v * 3 + 0];
                surface->vertices[v].y = points[v * 3 + 1];
                surface->vertices[v].z = points[v * 3 + 2];
                // TODO
                /*surface->coords[v * 2 + 0] = coords[v * 2 + 0];
                surface->coords[v * 2 + 1] = coords[v * 2 + 1];
                surface->normals[v * 2 + 0] = normals[v * 2 + 0];
                surface->normals[v * 2 + 1] = normals[v * 2 + 1];
                surface->normals[v * 2 + 2] = normals[v * 2 + 2];*/
            }

            strcpy(surface->texturePath, "textures/default.tga");
        }
    }
}

//
// kexModelManager::LoadModel
//

kexModel_t *kexModelManager::LoadModel(const char *file) {
    bool bUsingDefault = false;

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
            if(!(lexer = parser.Open("models/default.kmesh"))) {
                return NULL;
            }

            common.Warning("kexModelManager::LoadModel: %s not found\n", file);
            bUsingDefault = true;
        }

        model = modelList.Add(file);
        strncpy(model->filePath, file, MAX_FILEPATH);

        // begin parsing
        if(strstr(file, ".kmesh") || bUsingDefault) {
            ParseKMesh(model, lexer);
        }
        else if(strstr(file, ".obj")) {
            ParseWavefrontObj(model, lexer);
        }

        // we're done with the file
        parser.Close();

#ifndef EDITOR
        kexAnimState::LoadKAnim(model);
#endif
    }

    return model;
}
