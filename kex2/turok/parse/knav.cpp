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
// DESCRIPTION: Loading of KNAV files
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"
#include "script.h"
#include "level.h"

enum
{
    scnav_numpoints = 0,
    scnav_numleafs,
    scnav_points,
    scnav_leafs,
    scnav_end
};

static const sctokens_t navtokens[scnav_end+1] =
{
    { scnav_numpoints,      "numpoints"     },
    { scnav_numleafs,       "numleafs"      },
    { scnav_points,         "points"        },
    { scnav_leafs,          "leafs"         },
    { -1,                   NULL            }
};

//
// Knav_ParseCollisionPlanes
//

static void Knav_ParseCollisionPlanes(kexLexer *lexer,
                                     unsigned int numpoints, float *points)
{
    unsigned int i;

    if(gLevel.numplanes <= 0)
    {
#if 0
        parser.Error("numplanes is 0 or hasn't been set yet");
#endif
        return;
    }

    if(numpoints <= 0)
    {
#if 0
        parser.Error("numpoints is 0 or hasn't been set yet");
#endif
        return;
    }

    if(points == NULL)
    {
        parser.Error("points hasn't been allocated yet");
        return;
    }

    gLevel.planes = (plane_t*)Z_Calloc(sizeof(plane_t) *
        gLevel.numplanes, PU_LEVEL, 0);

    lexer->ExpectNextToken(TK_EQUAL);
    lexer->ExpectNextToken(TK_LBRACK);

    for(i = 0; i < gLevel.numplanes; i++)
    {
        int p;
        plane_t *pl = &gLevel.planes[i];

        pl->area_id = lexer->GetNumber();
        pl->flags = lexer->GetNumber();

        for(p = 0; p < 3; p++)
        {
            int index = lexer->GetNumber();

            pl->points[p][0] = points[index * 4 + 0];
            pl->points[p][1] = points[index * 4 + 1];
            pl->points[p][2] = points[index * 4 + 2];
            pl->height[p]    = points[index * 4 + 3];
        }

        for(p = 0; p < 3; p++)
        {
            int link = lexer->GetNumber();
            pl->link[p] = link == -1 ? NULL : &gLevel.planes[link];
        }

        Plane_GetNormal(pl->normal, pl);
        Vec_Normalize3(pl->normal);
        Plane_GetCeilingNormal(pl->ceilingNormal, pl);
        Vec_Normalize3(pl->ceilingNormal);
    }

    lexer->ExpectNextToken(TK_RBRACK);
}

//
// Knav_ParseNavScript
//

static void Knav_ParseNavScript(kexLexer *lexer)
{
    unsigned int numpoints = 0;
    float *points = NULL;

    while(lexer->CheckState())
    {
        lexer->Find();

        switch(lexer->TokenType())
        {
        case TK_NONE:
            break;
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            {
                switch(lexer->GetIDForTokenList(navtokens, lexer->Token()))
                {
                case scnav_numpoints:
                    lexer->AssignFromTokenList(navtokens, &numpoints,
                        scnav_numpoints, false);
                    break;

                case scnav_numleafs:
                    lexer->AssignFromTokenList(navtokens, &gLevel.numplanes,
                        scnav_numleafs, false);
                    break;

                case scnav_points:
                    if((numpoints * 4) > 0)
                    {
                        lexer->AssignFromTokenList(navtokens, AT_FLOAT,
                            (void**)&points, numpoints * 4, scnav_points,
                            false, PU_LEVEL);
                    }
                    break;

                case scnav_leafs:
                    Knav_ParseCollisionPlanes(lexer, numpoints, points);
                    break;

                default:
                    if(lexer->TokenType() == TK_IDENIFIER)
                    {
                        common.DPrintf("Knav_ParseNavScript: Unknown token: %s\n",
                            lexer->Token());
                    }
                    break;
                }
            }
            break;

        default:
            break;
        }
    }

    if(points != NULL)
        Z_Free(points);
}

//
// Knav_Load
//

void Knav_Load(int map)
{
    kexLexer *lexer;
    filepath_t file;

    sprintf(file, "maps/map%02d/map%02d.kcm", map, map);
    common.Printf("Knav_Load: Loading %s...\n", file);

    if(lexer = parser.Open(file))
    {
        Knav_ParseNavScript(lexer);
        // we're done with the file
        parser.Close();
    }
}
