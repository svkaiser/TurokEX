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
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderSystem.h"
#include "material.h"
#include "renderSurface.h"

const surface_t *kexRenderSurface::currentSurface = NULL;

//
// kexRenderSurface::DrawElements
//

void kexRenderSurface::DrawElements(const surface_t *surface)  {
    kexMaterial *material;
    
    if(surface == NULL) {
        return;
    }
    if(surface->material == NULL) {
        return;
    }
    if(surface->material->Flags() & MTF_NODRAW) {
        return;
    }
    
    material = surface->material;
    material->ShaderObj()->Enable();
    
    renderSystem.SetState(material->StateBits());
    renderSystem.SetAlphaFunc(material->AlphaFunction(), material->AlphaMask());
    renderSystem.SetCull(material->CullType());
    
    if(kexRenderSurface::currentSurface != surface) {
        dglNormalPointer(GL_FLOAT, sizeof(float)*3, surface->normals);
        dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, surface->coords);
        dglVertexPointer(3, GL_FLOAT, sizeof(kexVec3), surface->vertices);
        dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, surface->rgb);
        
        kexRenderSurface::currentSurface = surface;
    }
    
    for(unsigned int i = 0; i < material->NumUnits(); i++) {
        matSampler_t *sampler = material->Sampler(i);
        
        renderSystem.SetTextureUnit(sampler->unit);
        
        if(sampler->texture == NULL) {
            renderSystem.defaultTexture.Bind();
        }
        else {
            sampler->texture->Bind();
            sampler->texture->ChangeParameters(sampler->clamp, sampler->filter);
        }
    }
    
    renderSystem.SetTextureUnit(0);
    
    dglDrawElements(GL_TRIANGLES, surface->numIndices, GL_UNSIGNED_SHORT, surface->indices);
}
