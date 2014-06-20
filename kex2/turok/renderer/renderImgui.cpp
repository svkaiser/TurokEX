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
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderBackend.h"
#include "renderImgui.h"
#include "renderMain.h"

kexRenderImgui renderImgui;

//
// kexRenderImgui::kexRenderImgui
//

kexRenderImgui::kexRenderImgui(void) {
    for (int i = 0; i < CIRCLE_VERTS; ++i) {
        float a = (float)i / (float)CIRCLE_VERTS * M_PI*2;
        
        circleVerts[i*2+0] = kexMath::Cos(a);
        circleVerts[i*2+1] = kexMath::Sin(a);
    }
}

//
// kexRenderImgui::~kexRenderImgui
//

kexRenderImgui::~kexRenderImgui(void) {
}

//
// kexRenderImgui::DrawPolygon
//

void kexRenderImgui::DrawPolygon(const float *coords, unsigned int numCoords, float r, rcolor col) {
    if(numCoords > TEMP_COORD_COUNT) {
        numCoords = TEMP_COORD_COUNT;
    }
    
    for(unsigned i = 0, j = numCoords-1; i < numCoords; j = i++) {
        const float* v0 = &coords[j*2];
        const float* v1 = &coords[i*2];
        float dx = v1[0] - v0[0];
        float dy = v1[1] - v0[1];
        float d = kexMath::Sqrt(dx*dx+dy*dy);
        if(d > 0) {
            d = 1.0f/d;
            dx *= d;
            dy *= d;
        }
        tempNormals[j*2+0] = dy;
        tempNormals[j*2+1] = -dx;
    }
    
    for(unsigned i = 0, j = numCoords-1; i < numCoords; j = i++) {
        float dlx0 = tempNormals[j*2+0];
        float dly0 = tempNormals[j*2+1];
        float dlx1 = tempNormals[i*2+0];
        float dly1 = tempNormals[i*2+1];
        float dmx = (dlx0 + dlx1) * 0.5f;
        float dmy = (dly0 + dly1) * 0.5f;
        float dmr2 = dmx*dmx + dmy*dmy;
        if(dmr2 > 0.000001f) {
            float scale = 1.0f / dmr2;
            if(scale > 10.0f) {
                scale = 10.0f;
            }
            
            dmx *= scale;
            dmy *= scale;
        }
		tempCoords[i*2+0] = coords[i*2+0]+dmx*r;
		tempCoords[i*2+1] = coords[i*2+1]+dmy*r;
	}
    
    byte r1 = (byte)(col&0xff);
    byte g1 = (byte)((col>>8)&0xff);
    byte b1 = (byte)((col>>16)&0xff);
    byte a1 = (byte)((col>>24)&0xff);
    
    cpuVertList.BindDrawPointers();
    renderBackend.SetState(GLSTATE_TEXTURE0, false);
    
    int tri = 0;
    
    for(unsigned int i = 0, j = numCoords-1; i < numCoords; j = i++) {
        cpuVertList.AddVertex(coords[i*2+0], coords[i*2+1], 0, 0, 0, r1, g1, b1, a1);
        cpuVertList.AddVertex(coords[j*2+0], coords[j*2+1], 0, 0, 0, r1, g1, b1, a1);
        cpuVertList.AddVertex(tempCoords[j*2+0], tempCoords[j*2+1], 0, 0, 0, r1, g1, b1, 0);
        
        cpuVertList.AddTriangle(tri+0, tri+1, tri+2);
        tri += 3;
        
        cpuVertList.AddVertex(tempCoords[j*2+0], tempCoords[j*2+1], 0, 0, 0, r1, g1, b1, 0);
        cpuVertList.AddVertex(tempCoords[i*2+0], tempCoords[i*2+1], 0, 0, 0, r1, g1, b1, 0);
        cpuVertList.AddVertex(coords[i*2+0], coords[i*2+1], 0, 0, 0, r1, g1, b1, a1);
        
        cpuVertList.AddTriangle(tri+0, tri+1, tri+2);
        tri += 3;
    }
    
    const float *tcoords;
    
    for(unsigned int i = 2; i < numCoords; ++i) {
        cpuVertList.AddVertex(coords[0], coords[1], 0, 0, 0, r1, g1, b1, a1);
        tcoords = &coords[(i-1)*2];
        cpuVertList.AddVertex(tcoords[0], tcoords[1], 0, 0, 0, r1, g1, b1, a1);
        tcoords = &coords[i*2];
        cpuVertList.AddVertex(tcoords[0], tcoords[1], 0, 0, 0, r1, g1, b1, a1);
        
        cpuVertList.AddTriangle(tri+0, tri+1, tri+2);
        tri += 3;
    }
    
    cpuVertList.DrawElementsNoShader();
}

//
// kexRenderImgui::DrawRect
//

void kexRenderImgui::DrawRect(const float x, const float y, const float w, const float h,
                              const float fth, rcolor col) {
    const float verts[4*2] = {
        x+0.5f, y+0.5f,
        x+w-0.5f, y+0.5f,
        x+w-0.5f, y+h-0.5f,
        x+0.5f, y+h-0.5f
    };
    
    DrawPolygon(verts, 4, fth, col);
}

//
// kexRenderImgui::DrawRoundRect
//

void kexRenderImgui::DrawRoundRect(const float x, const float y, const float w, const float h,
                              const float r, const float fth, rcolor col) {
    const unsigned int n = CIRCLE_VERTS / 4;
    float verts[(n+1)*4*2];
    const float *cverts = circleVerts;
    float *v = verts;
    
    for(unsigned int i = 0; i <= n; ++i) {
        *v++ = x+w-r + cverts[i*2] * r;
        *v++ = y+h-r + cverts[i*2+1] * r;
    }
    
    for(unsigned int i = n; i<= n*2; ++i) {
        *v++ = x+r + cverts[i*2] * r;
        *v++ = y+h-r + cverts[i*2+1] * r;
    }
    
    for(unsigned int i = n*2; i <= n*3; ++i) {
        *v++ = x+r + cverts[i*2] * r;
        *v++ = y+r + cverts[i*2+1] * r;
    }
    
    for(unsigned int i = n*3; i < n*4; ++i) {
        *v++ = x+w-r + cverts[i*2] * r;
        *v++ = y+r + cverts[i*2+1] * r;
    }
    
    *v++ = x+w-r + cverts[0] * r;
    *v++ = y+r + cverts[1] * r;
    
    DrawPolygon(verts, (n+1)*4, fth, col);
}

//
// kexRenderImgui::DrawLine
//

void kexRenderImgui::DrawLine(const float x0, const float y0, const float x1, const float y1,
                              const float r, const float fth, rcolor col) {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float d = kexMath::Sqrt(dx*dx+dy*dy);
    
    if(d > 0.0001f) {
        d = 1.0f / d;
        dx *= d;
        dy *= d;
    }
    
    float nx = dy;
    float ny = -dx;
    float verts[4*2];
    float rr = r;
    
    rr -= fth;
    rr *= 0.5f;
    
    if(rr < 0.01f) {
        rr = 0.01f;
    }
    
    dx *= rr;
    dy *= rr;
    nx *= rr;
    ny *= rr;
    
    verts[0] = x0-dx-nx;
    verts[1] = y0-dy-ny;
    verts[2] = x0-dx+nx;
    verts[3] = y0-dy+ny;
    verts[4] = x1+dx+nx;
    verts[5] = y1+dy+ny;
    verts[6] = x1+dx-nx;
    verts[7] = y1+dy-ny;
    
    DrawPolygon(verts, 4, fth, col);
}

//
// kexRenderImgui::DrawText
//

void kexRenderImgui::DrawText(const float x, const float y, const char *text,
                              const int align, rcolor color) {
    if(!text) {
        return;
    }
    
    float tx = x;
    
    if(align == IMGUI_ALIGN_RIGHT) {
        tx -= (renderBackend.consoleFont.StringWidth(text, 1.0f, 0)) * 0.5f;
    }
    
    renderBackend.consoleFont.DrawString(text, tx, y, 1.0f, align == IMGUI_ALIGN_CENTER,
                                         (byte*)&color, (byte*)&color);
}

//
// kexRenderImgui::Draw
//

void kexRenderImgui::Draw(const int width, const int height) {
    const float s = 1.0f / 8.0f;
    const imguiGfxCmd* q = imguiGetRenderQueue();
    int nq = imguiGetRenderQueueSize();
    
    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_SCISSOR, false);
    renderBackend.SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    renderBackend.SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    
    for(int i = 0; i < nq; i++) {
        const imguiGfxCmd &cmd = q[i];
        
        switch(cmd.type) {
            case IMGUI_GFXCMD_RECT:
                if(cmd.rect.r == 0) {
                    DrawRect((float)cmd.rect.x*s+0.5f,
                             (float)cmd.rect.y*s+0.5f,
                             (float)cmd.rect.w*s-1,
                             (float)cmd.rect.h*s-1,
                             1.0f,
                             cmd.col);
                }
                else {
                    DrawRoundRect((float)cmd.rect.x*s+0.5f,
                                  (float)cmd.rect.y*s+0.5f,
                                  (float)cmd.rect.w*s-1,
                                  (float)cmd.rect.h*s-1,
                                  (float)cmd.rect.r*s,
                                  1.0f,
                                  cmd.col);
                }
                break;
            case IMGUI_GFXCMD_LINE:
                DrawLine(cmd.line.x0*s,
                         cmd.line.y0*s,
                         cmd.line.x1*s,
                         cmd.line.y1*s,
                         cmd.line.r*s,
                         1.0f,
                         cmd.col);
                break;
            case IMGUI_GFXCMD_TRIANGLE:
                if(cmd.flags == 1) {
                    const float verts[3*2] = {
                        (float)cmd.rect.x*s+0.5f, (float)cmd.rect.y*s+0.5f,
                        (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1,
                        (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s/2-0.5f,
                        (float)cmd.rect.x*s+0.5f,
                        (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1
                    };
                    
                    DrawPolygon(verts, 3, 1.0f, cmd.col);
                }
                if(cmd.flags == 2) {
                    const float verts[3*2] = {
                        (float)cmd.rect.x*s+0.5f,
                        (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1,
                        (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s/2-0.5f,
                        (float)cmd.rect.y*s+0.5f,
                        (float)cmd.rect.x*s+0.5f+(float)cmd.rect.w*s-1,
                        (float)cmd.rect.y*s+0.5f+(float)cmd.rect.h*s-1
                    };
                    
                    DrawPolygon(verts, 3, 1.0f, cmd.col);
                }
                break;
            case IMGUI_GFXCMD_TEXT:
                DrawText(cmd.text.x, cmd.text.y, cmd.text.text, cmd.text.align, cmd.col);
                break;
            case IMGUI_GFXCMD_SCISSOR:
                if(cmd.flags) {
                    renderBackend.SetState(GLSTATE_SCISSOR, true);
                    renderBackend.SetScissorRect(cmd.rect.x, cmd.rect.y, cmd.rect.w, cmd.rect.h);
                }
                else {
                    renderBackend.SetState(GLSTATE_SCISSOR, false);
                }
                break;
            default:
                break;
        }
    }
    
    renderBackend.SetState(GLSTATE_SCISSOR, false);
}
