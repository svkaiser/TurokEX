// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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

#ifndef __LENSFLARES_H__
#define __LENSFLARES_H__

class kexLensFlares {
public:
                        kexLensFlares(void);
                        ~kexLensFlares(void);

    void                LoadKLF(const char *file);
    void                Draw(void);
    void                Delete(void);

    const bool          IsLoaded(void) const { return bLoaded; }
    kexVec3             &Origin(void) { return origin; }

    filepath_t          filePath;
    kexLensFlares       *next;

private:
    typedef struct {
        kexMaterial     *material;
        float           scale;
        float           offset;
    } lfData_t;

    lfData_t            *lens;
    kexVec3             origin;
    int                 numlens;
    bool                bLoaded;
};

#endif
