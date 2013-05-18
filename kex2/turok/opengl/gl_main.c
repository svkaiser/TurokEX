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
// DESCRIPTION: OpenGL backend system
//
//-----------------------------------------------------------------------------

#include "SDL.h"
#include "SDL_opengl.h"

#include "common.h"
#include "type.h"
#include "gl.h"
#include "zone.h"
#include "kernel.h"

#define GL_MAX_INDICES  0x10000
#define GL_MAX_VERTICES 0x10000
static word indicecnt = 0;
static word drawIndices[GL_MAX_INDICES];
static word vertexcnt = 0;
static vtx_t drawVertices[GL_MAX_VERTICES];

static dtexture prev_texture = 0;

GL_ARB_multitexture_Define();
GL_EXT_compiled_vertex_array_Define();
GL_EXT_fog_coord_Define();
GL_ARB_texture_non_power_of_two_Define();
GL_ARB_texture_env_combine_Define();
GL_EXT_texture_env_combine_Define();
GL_EXT_texture_filter_anisotropic_Define();

CVAR(gl_filter, 0);
CVAR(gl_anisotropic, 0);

CVAR_EXTERNAL(gl_gamma);
CVAR_EXTERNAL(v_vsync);
CVAR_EXTERNAL(v_depthsize);
CVAR_EXTERNAL(v_buffersize);
CVAR_EXTERNAL(v_stencilsize);

int	ViewWindowX = 0;
int	ViewWindowY = 0;
int	ViewWidth	= 0;
int	ViewHeight	= 0;

int gl_max_texture_units;
int gl_max_texture_size;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;

int DGL_CLAMP = GL_CLAMP;
float max_anisotropic = 0;
kbool widescreen = false;

//
// FindExtension
//

static kbool FindExtension(const char *ext)
{
    const byte *extensions = NULL;
    const byte *start;
    byte *where, *terminator;
    
    // Extension names should not have spaces.
    where = (byte *) strrchr((char*)ext, ' ');
    if (where || *ext == '\0')
        return 0;
    
    extensions = dglGetString(GL_EXTENSIONS);
    
    start = extensions;
    for(;;)
    {
        where = (byte *)strstr(start, ext);
        if(!where)
            break;
        terminator = where + strlen(ext);
        if(where == start || *(where - 1) == ' ')
        {
            if(*terminator == ' ' || *terminator == '\0')
                return true;
            start = terminator;
        }
    }
    return false;
}

//
// GL_CheckExtension
//

kbool GL_CheckExtension(const char *ext)
{
    if(FindExtension(ext))
    {
        Com_Printf("GL Extension: %s = true\n", ext);
        return true;
    }
    else
        Com_Warning("GL Extension: %s = false\n", ext);
    
    return false;
}

//
// GL_RegisterProc
//

void* GL_RegisterProc(const char *address)
{
    void *proc = SDL_GL_GetProcAddress(address);
    
    if(!proc)
    {
        Com_Warning("GL_RegisterProc: Failed to get proc address: %s", address);
        return NULL;
    }
    
    return proc;
}

//
// GL_SetOrtho
//

void GL_SetOrtho(void)
{
    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();
    dglOrtho(0, video_width, video_height, 0, -1, 1);
    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
}

//
// GL_SwapBuffers
//

void GL_SwapBuffers(void)
{
    dglFinish();
    SDL_GL_SwapBuffers();
}

//
// GL_GetScreenBuffer
//

byte* GL_GetScreenBuffer(int x, int y, int width, int height)
{
    byte* buffer;
    byte* data;
    int i;
    int pack;
    int col;

    col     = (width * 3);
    data    = (byte*)Z_Calloc(height * width * 3, PU_STATIC, 0);
    buffer  = (byte*)Z_Calloc(col, PU_STATIC, 0);

    //
    // 20120313 villsa - force pack alignment to 1
    //
    dglGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    dglPixelStorei(GL_PACK_ALIGNMENT, 1);
    dglFlush();
    dglReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    dglPixelStorei(GL_PACK_ALIGNMENT, pack);

    //
    // Need to vertically flip the image
    // 20120313 villsa - better method to flip image. uses one buffer instead of two
    //
    for(i = 0; i < height / 2; i++)
    {
        memcpy(buffer, &data[i * col], col);
        memcpy(&data[i * col], &data[(height - (i + 1)) * col], col);
        memcpy(&data[(height - (i + 1)) * col], buffer, col);
    }
    
    Z_Free(buffer);

    return data;
}

//
// GL_ScreenToTexture
//

dtexture GL_ScreenToTexture(void)
{
    dtexture id;
    int width;
    int height;
    
    dglGenTextures(1, &id);
    dglBindTexture(GL_TEXTURE_2D, id);
    
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DGL_CLAMP);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    width = Tex_PadDims(video_width);
    height = Tex_PadDims(video_height);

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        0
        );

    dglCopyTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        0,
        0,
        width,
        height
        );
    
    return id;
}

//
// GL_SetTextureFilter
//

void GL_SetTextureFilter(void)
{
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)gl_filter.value == 0 ? GL_LINEAR : GL_NEAREST);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)gl_filter.value == 0 ? GL_LINEAR : GL_NEAREST);

    if(has_GL_EXT_texture_filter_anisotropic)
    {
        if(gl_anisotropic.value)
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropic);
        else
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
    }
}

//
// CalcViewSize
//

static void CalcViewSize(void)
{
    ViewWidth = video_width;
    ViewHeight = video_height;

    widescreen = !fcmp(((float)ViewWidth / (float)ViewHeight), (4.0f / 3.0f));

    ViewWindowX = (video_width - ViewWidth) / 2;

    if(ViewWidth == video_width)
        ViewWindowY = 0;
    else
        ViewWindowY = (ViewHeight) / 2;
}

//
// GetVersionInt
// Borrowed from prboom+
//

typedef enum
{
    OPENGL_VERSION_1_0,
    OPENGL_VERSION_1_1,
    OPENGL_VERSION_1_2,
    OPENGL_VERSION_1_3,
    OPENGL_VERSION_1_4,
    OPENGL_VERSION_1_5,
    OPENGL_VERSION_2_0,
    OPENGL_VERSION_2_1,
} glversion_t;

static int GetVersionInt(const char* version)
{
    int MajorVersion;
    int MinorVersion;
    int versionvar;

    versionvar = OPENGL_VERSION_1_0;

    if(sscanf(version, "%d.%d", &MajorVersion, &MinorVersion) == 2)
    {
        if(MajorVersion > 1)
        {
            versionvar = OPENGL_VERSION_2_0;

            if(MinorVersion > 0)
                versionvar = OPENGL_VERSION_2_1;
        }
        else
        {
            versionvar = OPENGL_VERSION_1_0;

            if(MinorVersion > 0) versionvar = OPENGL_VERSION_1_1;
            if(MinorVersion > 1) versionvar = OPENGL_VERSION_1_2;
            if(MinorVersion > 2) versionvar = OPENGL_VERSION_1_3;
            if(MinorVersion > 3) versionvar = OPENGL_VERSION_1_4;
            if(MinorVersion > 4) versionvar = OPENGL_VERSION_1_5;
        }
    }

    return versionvar;
}

//
// GL_SetState
//

static int glstate_flag = 0;

void GL_SetState(int bit, kbool enable)
{
#define TOGGLEGLBIT(flag, bit)                          \
    if(enable && !(glstate_flag & (1 << flag)))         \
    {                                                   \
        dglEnable(bit);                                 \
        glstate_flag |= (1 << flag);                    \
    }                                                   \
    else if(!enable && (glstate_flag & (1 << flag)))    \
    {                                                   \
        dglDisable(bit);                                \
        glstate_flag &= ~(1 << flag);                   \
    }

    switch(bit)
    {
    case GLSTATE_BLEND:
        TOGGLEGLBIT(GLSTATE_BLEND, GL_BLEND);
        break;
    case GLSTATE_CULL:
        TOGGLEGLBIT(GLSTATE_CULL, GL_CULL_FACE);
        break;
    case GLSTATE_TEXTURE0:
        TOGGLEGLBIT(GLSTATE_TEXTURE0, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE1:
        TOGGLEGLBIT(GLSTATE_TEXTURE1, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE2:
        TOGGLEGLBIT(GLSTATE_TEXTURE2, GL_TEXTURE_2D);
        break;
    case GLSTATE_TEXTURE3:
        TOGGLEGLBIT(GLSTATE_TEXTURE3, GL_TEXTURE_2D);
        break;
    case GLSTATE_ALPHATEST:
        TOGGLEGLBIT(GLSTATE_ALPHATEST, GL_ALPHA_TEST);
        break;
    case GLSTATE_TEXGEN_S:
        TOGGLEGLBIT(GLSTATE_TEXGEN_S, GL_TEXTURE_GEN_S);
        break;
    case GLSTATE_TEXGEN_T:
        TOGGLEGLBIT(GLSTATE_TEXGEN_T, GL_TEXTURE_GEN_T);
        break;
    case GLSTATE_DEPTHTEST:
        TOGGLEGLBIT(GLSTATE_DEPTHTEST, GL_DEPTH_TEST);
        break;
    default:
        Com_Warning("GL_SetState: unknown bit flag: %i\n", bit);
        break;
    }

#undef TOGGLEGLBIT
}

//
// GL_BindTexture
//

void GL_BindTexture(texture_t *texture)
{
    if(texture == NULL)
        return;

    if(texture->texid == prev_texture)
        return;

    dglBindTexture(GL_TEXTURE_2D, texture->texid);
    prev_texture = texture->texid;
}

//
// GL_BindTextureName
//

void GL_BindTextureName(const char *name)
{
    texture_t *texture;

    if(!(texture = Tex_CacheTextureFile(name, DGL_CLAMP, true)))
        return;

    GL_BindTexture(texture);
}

//
// GL_SetTextureUnit
//

void GL_SetTextureUnit(int unit, kbool enable)
{
    static int curunit = -1;

    if(!has_GL_ARB_multitexture)
        return;

    if(unit > 3)
        return;

    if(curunit == unit)
        return;

    curunit = unit;

    dglActiveTextureARB(GL_TEXTURE0_ARB + unit);
    GL_SetState(GLSTATE_TEXTURE0 + unit, enable);
}

//
// GL_ClearView
//

void GL_ClearView(float *clear)
{
    dglClearColor(clear[0], clear[1], clear[2], clear[3]);
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    dglViewport(ViewWindowX, ViewWindowY, ViewWidth, ViewHeight);
    dglScissor(ViewWindowX, ViewWindowY, ViewWidth, ViewHeight);
}

//
// GL_SetVertexPointer
//

void GL_SetVertexPointer(vtx_t *vtx)
{
    dglNormalPointer(GL_FLOAT, sizeof(vtx_t), &vtx->nx);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(vtx_t), &vtx->tu);
    dglVertexPointer(3, GL_FLOAT, sizeof(vtx_t), vtx);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vtx_t), &vtx->r);
}

//
// GL_Triangle
//

void GL_Triangle(int v0, int v1, int v2)
{
    if(indicecnt + 3 >= GL_MAX_INDICES)
        Com_Error("Static triangle indice overflow");

    drawIndices[indicecnt++] = v0;
    drawIndices[indicecnt++] = v1;
    drawIndices[indicecnt++] = v2;
}

//
// GL_Vertex
//

void GL_Vertex(float x, float y, float z,
               float tu, float tv,
               float nx, float ny, float nz,
               byte r, byte g, byte b, byte a)
{
    if(vertexcnt >= GL_MAX_VERTICES)
        Com_Error("Static vertex draw overflow");

    drawVertices[vertexcnt].x   = x;
    drawVertices[vertexcnt].y   = y;
    drawVertices[vertexcnt].z   = z;
    drawVertices[vertexcnt].tu  = tu;
    drawVertices[vertexcnt].tv  = tv;
    drawVertices[vertexcnt].nx  = nx;
    drawVertices[vertexcnt].ny  = ny;
    drawVertices[vertexcnt].nz  = nz;
    drawVertices[vertexcnt].r   = r;
    drawVertices[vertexcnt].g   = g;
    drawVertices[vertexcnt].b   = b;
    drawVertices[vertexcnt].a   = a;

    vertexcnt++;
}

//
// GL_DrawElements2
//

void GL_DrawElements2(void)
{
    GL_SetVertexPointer(drawVertices);
    dglDrawElements(GL_TRIANGLES, indicecnt, GL_UNSIGNED_SHORT, drawIndices);

    indicecnt = 0;
    vertexcnt = 0;
}

//
// GL_DrawElements
//

void GL_DrawElements(unsigned int count, vtx_t *vtx)
{
    if(has_GL_EXT_compiled_vertex_array)
        dglLockArraysEXT(0, count);

    dglDrawElements(GL_TRIANGLES, indicecnt, GL_UNSIGNED_SHORT, drawIndices);

    if(has_GL_EXT_compiled_vertex_array)
        dglUnlockArraysEXT();

    indicecnt = 0;
}

//
// GL_Register
//

void GL_Register(void)
{
    Cvar_Register(&gl_filter);
    Cvar_Register(&gl_anisotropic);
    Cvar_Register(&gl_gamma);
}

//
// GL_Init
//

void GL_Init(void)
{
    uint32 flags = 0;

    V_SetupScreen();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, (int)v_buffersize.value);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (int)v_depthsize.value);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, (int)v_stencilsize.value);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (int)v_vsync.value);
    
    flags |= SDL_OPENGL;
    
    if(!video_windowed)
        flags |= SDL_FULLSCREEN;
    
    if(SDL_SetVideoMode(video_width, video_height, 32, flags) == NULL)
        Com_Error("GL_Init: Failed to set opengl");

    gl_vendor = dglGetString(GL_VENDOR);
    Com_Printf("GL_VENDOR: %s\n", gl_vendor);
    gl_renderer = dglGetString(GL_RENDERER);
    Com_Printf("GL_RENDERER: %s\n", gl_renderer);
    gl_version = dglGetString(GL_VERSION);
    Com_Printf("GL_VERSION: %s\n", gl_version);
    dglGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
    Com_Printf("GL_MAX_TEXTURE_SIZE: %i\n", gl_max_texture_size);
    dglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &gl_max_texture_units);
    Com_Printf("GL_MAX_TEXTURE_UNITS_ARB: %i\n", gl_max_texture_units);
    
    CalcViewSize();

    dglViewport(0, 0, video_width, video_height);
    dglClearDepth(1.0f);
    dglClearStencil(0);
    dglDisable(GL_TEXTURE_2D);
    dglEnable(GL_CULL_FACE);
    dglDisable(GL_NORMALIZE);
    dglCullFace(GL_FRONT);
    dglShadeModel(GL_SMOOTH);
    dglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    dglDepthFunc(GL_LEQUAL);
    dglAlphaFunc(GL_GEQUAL, 0.01f);
    dglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    dglFogi(GL_FOG_MODE, GL_LINEAR);
    dglHint(GL_FOG_HINT, GL_NICEST);
    dglEnable(GL_SCISSOR_TEST);
    dglEnable(GL_DITHER);
    dglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	dglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    
    GL_SetState(GLSTATE_TEXTURE0, 1);
    GL_SetTextureFilter();

    GL_ARB_multitexture_Init();
    GL_EXT_compiled_vertex_array_Init();
    GL_EXT_fog_coord_Init();
    GL_ARB_texture_non_power_of_two_Init();
    GL_ARB_texture_env_combine_Init();
    GL_EXT_texture_env_combine_Init();
    GL_EXT_texture_filter_anisotropic_Init();

    if(!has_GL_ARB_multitexture)
    {
        Com_Warning("GL_ARB_multitexture not supported...\n");
    }

    dglEnableClientState(GL_VERTEX_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);
    dglEnableClientState(GL_NORMAL_ARRAY);

    DGL_CLAMP = (GetVersionInt(gl_version) >= OPENGL_VERSION_1_2 ? GL_CLAMP_TO_EDGE : GL_CLAMP);

    if(has_GL_EXT_texture_filter_anisotropic)
        dglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropic);

    Draw_Init();
    Tex_Init();
}

