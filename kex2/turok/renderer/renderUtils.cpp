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
// DESCRIPTION: Drawing utilities
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderBackend.h"
#include "material.h"
#include "renderUtils.h"

float kexRenderUtils::debugLineNum = 0;

//
// kexRenderUtils::DrawBoundingBox
//

void kexRenderUtils::DrawBoundingBox(const kexBBox &bbox, byte r, byte g, byte b) {
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_LIGHTING, false);

    renderBackend.DisableShaders();

#define ADD_LINE(ba1, ba2, ba3, bb1, bb2, bb3)                          \
    renderBackend.AddLine(bbox[ba1][0], bbox[ba2][1], bbox[ba3][2],     \
                         bbox[bb1][0], bbox[bb2][1], bbox[bb3][2],      \
                         r, g, b, 255)
    
    renderBackend.BindDrawPointers();
    ADD_LINE(0, 0, 0, 1, 0, 0);
    ADD_LINE(1, 0, 0, 1, 0, 1);
    ADD_LINE(1, 0, 1, 0, 0, 1);
    ADD_LINE(0, 0, 1, 0, 0, 0);
    ADD_LINE(0, 0, 0, 0, 1, 0);
    ADD_LINE(0, 1, 0, 0, 1, 1);
    ADD_LINE(0, 1, 1, 0, 0, 1);
    ADD_LINE(0, 0, 1, 0, 0, 0);
    ADD_LINE(0, 1, 0, 1, 1, 0);
    ADD_LINE(1, 1, 0, 1, 1, 1);
    ADD_LINE(1, 1, 1, 0, 1, 1);
    ADD_LINE(0, 1, 1, 0, 1, 0);
    ADD_LINE(1, 0, 0, 1, 1, 0);
    ADD_LINE(1, 1, 0, 1, 1, 1);
    ADD_LINE(1, 1, 1, 1, 0, 1);
    ADD_LINE(1, 0, 1, 1, 0, 0);
    renderBackend.DrawLineElements();
    
#undef ADD_LINE

    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderUtils::DrawRadius
//

void kexRenderUtils::DrawRadius(float x, float y, float z,
                             float radius, float height,
                             byte r, byte g, byte b) {
    float an;
    int i;

    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_LIGHTING, false);

    renderBackend.DisableShaders();
    renderBackend.BindDrawPointers();

    an = DEG2RAD(360 / 32);

    for(i = 0; i < 32; i++) {
        float s1 = kexMath::Sin(an * i);
        float c1 = kexMath::Cos(an * i);
        float s2 = kexMath::Sin(an * ((i+1)%31));
        float c2 = kexMath::Cos(an * ((i+1)%31));
        float x1 = x + (radius * s1);
        float x2 = x + (radius * s2);
        float y1 = y;
        float y2 = y + height;
        float z1 = z + (radius * c1);
        float z2 = z + (radius * c2);
        
        renderBackend.AddLine(x1, y1, z1, x1, y2, z1, r, g, b, 255);
        renderBackend.AddLine(x1, y1, z1, x2, y1, z2, r, g, b, 255);
        renderBackend.AddLine(x1, y2, z1, x2, y2, z2, r, g, b, 255);
    }

    renderBackend.DrawLineElements();
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderUtils::DrawOrigin
//

void kexRenderUtils::DrawOrigin(float x, float y, float z, float size) {
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_FOG, false);
    renderBackend.SetState(GLSTATE_LIGHTING, false);

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);

    renderBackend.DisableShaders();
    
    renderBackend.BindDrawPointers();
    renderBackend.AddLine(x, y, z, x + size, y, z, 255, 0, 0, 255); // x
    renderBackend.AddLine(x, y, z, x, y + size, z, 0, 255, 0, 255); // y
    renderBackend.AddLine(x, y, z, x, y, z + size, 0, 0, 255, 255); // z
    renderBackend.DrawLineElements();
    
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
    
    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderUtils::DrawSphere
//

void kexRenderUtils::DrawSphere(float x, float y, float z, float radius, byte r, byte g, byte b) {
    float points[72];
    int count;
    int i;
    int j;
    int k;
    float s;
    float c;
    float v1[3];
    float v2[3];

    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    renderBackend.SetState(GLSTATE_CULL, false);

    renderBackend.DisableShaders();
    renderBackend.BindDrawPointers();

    count = (360 / 15);
    points[0 * 3 + 0] = x;
    points[0 * 3 + 1] = y;
    points[0 * 3 + 2] = z + radius;

    for(i = 1; i < count; i++) {
        points[i * 3 + 0] = points[0 * 3 + 0];
        points[i * 3 + 1] = points[0 * 3 + 1];
        points[i * 3 + 2] = points[0 * 3 + 2];
    }

    for(i = 15; i <= 360; i += 15) {
        s = kexMath::Sin(DEG2RAD(i));
        c = kexMath::Cos(DEG2RAD(i));

        v1[0] = x;
        v1[1] = y + radius * s;
        v1[2] = z + radius * c;

        for(k = 0, j = 15; j <= 360; j += 15, k++) {
            v2[0] = x + kexMath::Sin(DEG2RAD(j)) * radius * s;
            v2[1] = y + kexMath::Cos(DEG2RAD(j)) * radius * s;
            v2[2] = v1[2];

            renderBackend.AddLine(v1[0], v1[1], v2[2], v2[0], v2[1], v2[2], r, g, b, 255);
            renderBackend.AddLine(v1[0], v1[1], v2[2],
                points[k * 3 + 0],
                points[k * 3 + 1],
                points[k * 3 + 2],
                r, g, b, 255);

            points[k * 3 + 0] = v1[0];
            points[k * 3 + 1] = v1[1];
            points[k * 3 + 2] = v1[2];

            v1[0] = v2[0];
            v1[1] = v2[1];
            v1[2] = v2[2];
        }
    }

    renderBackend.DrawLineElements();
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
}

//
// kexRenderUtils::PrintStatsText
//

void kexRenderUtils::PrintStatsText(const char *title, const char *s, ...) {
    va_list v;
    static char vastr[1024];
    unsigned int c;
    byte *cb;
	
    cb = (byte*)&c;
    
    if(title != NULL) {
        c = RGBA(0, 255, 0, 255);
        renderBackend.consoleFont.DrawString(title, 32, debugLineNum, 1, false, cb, cb);
    }
    
    if(s != NULL) {
        va_start(v, s);
        vsprintf(vastr, s, v);
        va_end(v);
        
        c = RGBA(255, 255, 0, 255);
        renderBackend.consoleFont.DrawString(vastr, 192, debugLineNum, 1, false, cb, cb);
    }
    
    debugLineNum += 16.0f;
}

