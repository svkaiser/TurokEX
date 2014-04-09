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
// DESCRIPTION: Renderer backend 
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "memHeap.h"
#include "renderBackend.h"
#include "renderMain.h"
#include "renderWorld.h"
#include "defs.h"

kexCvar cvarRenderFinish("r_finish", CVF_BOOL|CVF_CONFIG, "0", "Force a GL command sync");
extern kexCvar cvarVidDepthSize;

kexRenderBackend renderBackend;

GL_ARB_multitexture_Define();
GL_EXT_compiled_vertex_array_Define();
GL_EXT_fog_coord_Define();
GL_ARB_texture_non_power_of_two_Define();
GL_ARB_texture_env_combine_Define();
GL_EXT_texture_env_combine_Define();
GL_EXT_texture_filter_anisotropic_Define();
GL_ARB_vertex_buffer_object_Define();
GL_ARB_shader_objects_Define();

//
// FindExtension
//

static bool FindExtension(const char *ext) {
    const byte *extensions = NULL;
    const byte *start;
    byte *wh, *terminator;
    
    // Extension names should not have spaces.
    wh = (byte *)strrchr((char*)ext, ' ');
    if(wh || *ext == '\0') {
        return 0;
    }
    
    extensions = dglGetString(GL_EXTENSIONS);
    
    start = extensions;
    for(;;) {
        wh = (byte *)strstr((char*)start, ext);
        if(!wh) {
            break;
        }
        terminator = wh + strlen(ext);
        if(wh == start || *(wh - 1) == ' ') {
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

bool GL_CheckExtension(const char *ext) {
    if(FindExtension(ext)) {
        common.Printf("GL Extension: %s = true\n", ext);
        return true;
    }
    else {
        common.Warning("GL Extension: %s = false\n", ext);
    }
    
    return false;
}

//
// GL_RegisterProc
//

void* GL_RegisterProc(const char *address) {
    void *proc = SDL_GL_GetProcAddress(address);
    
    if(!proc) {
        common.Warning("GL_RegisterProc: Failed to get proc address: %s", address);
        return NULL;
    }
    
    return proc;
}

//
// kexRenderBackend::kexRenderBackend
//

kexRenderBackend::kexRenderBackend(void) {
    this->viewWidth                 = this->SCREEN_WIDTH;
    this->viewHeight                = this->SCREEN_HEIGHT;
    this->viewWindowX               = 0;
    this->viewWindowY               = 0;
    this->maxTextureUnits           = 1;
    this->maxTextureSize            = 64;
    this->maxAnisotropic            = 0;
    this->bWideScreen               = false;
    this->bFullScreen               = false;
    this->bIsInit                   = false;
    this->glState.glStateBits       = 0;
    this->glState.alphaFunction     = -1;
    this->glState.blendDest         = -1;
    this->glState.blendSrc          = -1;
    this->glState.cullType          = -1;
    this->glState.depthMask         = -1;
    this->glState.currentUnit       = -1;
    this->glState.currentProgram    = 0;
    this->frameBuffer               = NULL;
}

//
// kexRenderBackend::~kexRenderBackend
//

kexRenderBackend::~kexRenderBackend(void) {
}

//
// kexRenderBackend::SetViewDimensions
//

void kexRenderBackend::SetViewDimensions(void) {
    viewWidth = sysMain.VideoWidth();
    viewHeight = sysMain.VideoHeight();

    bWideScreen = !fcmp(((float)viewWidth / (float)viewHeight), (4.0f / 3.0f));

    viewWindowX = (sysMain.VideoWidth() - viewWidth) / 2;

    if(viewWidth == sysMain.VideoWidth()) {
        viewWindowY = 0;
    }
    else {
        viewWindowY = (viewHeight) / 2;
    }
}

//
// kexRenderBackend::GetOGLVersion
//

typedef enum {
    OPENGL_VERSION_1_0,
    OPENGL_VERSION_1_1,
    OPENGL_VERSION_1_2,
    OPENGL_VERSION_1_3,
    OPENGL_VERSION_1_4,
    OPENGL_VERSION_1_5,
    OPENGL_VERSION_2_0,
    OPENGL_VERSION_2_1,
} glversion_t;

int kexRenderBackend::GetOGLVersion(const char* version) {
    int MajorVersion;
    int MinorVersion;
    int versionvar;

    versionvar = OPENGL_VERSION_1_0;

    if(sscanf(version, "%d.%d", &MajorVersion, &MinorVersion) == 2) {
        if(MajorVersion > 1) {
            versionvar = OPENGL_VERSION_2_0;

            if(MinorVersion > 0) {
                versionvar = OPENGL_VERSION_2_1;
            }
        }
        else {
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
// kexRenderBackend::SetDefaultState
//

void kexRenderBackend::SetDefaultState(void) {
    SetViewDimensions();

    glState.glStateBits     = 0;
    glState.alphaFunction   = -1;
    glState.blendDest       = -1;
    glState.blendSrc        = -1;
    glState.cullType        = -1;
    glState.depthMask       = -1;
    glState.currentUnit     = -1;
    glState.currentProgram  = 0;
    
    dglViewport(0, 0, sysMain.VideoWidth(), sysMain.VideoHeight());
    dglClearDepth(1.0f);
    dglClearStencil(0);
    
    SetState(GLSTATE_TEXTURE0, true);
    SetState(GLSTATE_CULL, true);
    SetState(GLSTATE_LIGHTING, false);
    SetState(GLSTATE_FOG, false);
    SetCull(GLCULL_FRONT);
    SetDepth(GLFUNC_LEQUAL);
    SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    SetDepthMask(GLDEPTHMASK_YES);

    dglDisable(GL_NORMALIZE);
    dglShadeModel(GL_SMOOTH);
    dglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    dglFogi(GL_FOG_MODE, GL_LINEAR);
    dglHint(GL_FOG_HINT, GL_NICEST);
    dglEnable(GL_SCISSOR_TEST);
    dglEnable(GL_DITHER);
    dglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    dglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    dglColorMaterial(GL_FRONT, GL_DIFFUSE);
    dglColorMaterial(GL_BACK, GL_DIFFUSE);

    dglEnableClientState(GL_VERTEX_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);
    dglEnableClientState(GL_NORMAL_ARRAY);
}

//
// kexRenderBackend::Init
//

void kexRenderBackend::Init(void) {
    gl_vendor = (const char*)dglGetString(GL_VENDOR);
    gl_renderer = (const char*)dglGetString(GL_RENDERER);
    gl_version = (const char*)dglGetString(GL_VERSION);
    
    dglGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    dglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
    
    common.Printf("GL_VENDOR: %s\n", gl_vendor);
    common.Printf("GL_RENDERER: %s\n", gl_renderer);
    common.Printf("GL_VERSION: %s\n", gl_version);
    common.Printf("GL_MAX_TEXTURE_SIZE: %i\n", maxTextureSize);
    common.Printf("GL_MAX_TEXTURE_UNITS_ARB: %i\n", maxTextureUnits);

    GL_ARB_multitexture_Init();
    GL_EXT_compiled_vertex_array_Init();
    GL_EXT_fog_coord_Init();
    GL_ARB_texture_non_power_of_two_Init();
    GL_ARB_texture_env_combine_Init();
    GL_EXT_texture_env_combine_Init();
    GL_EXT_texture_filter_anisotropic_Init();
    GL_ARB_vertex_buffer_object_Init();
    GL_ARB_shader_objects_Init();

    SetDefaultState();

    byte *data;

    if((data = defaultTexture.LoadFromFile("textures/default.tga"))) {
        defaultTexture.Upload(&data, TC_CLAMP, TF_LINEAR);
        Mem_Free(data);
    }

    if((data = whiteTexture.LoadFromFile("textures/white.tga"))) {
        whiteTexture.Upload(&data, TC_CLAMP, TF_LINEAR);
        Mem_Free(data);
    }

    if((data = blackTexture.LoadFromFile("textures/black.tga"))) {
        blackTexture.Upload(&data, TC_CLAMP, TF_LINEAR);
        Mem_Free(data);
    }
    
    // create framebuffer texture
    frameBuffer = textureList.Add("framebuffer", kexTexture::hb_texture);
    frameBuffer->SetParameters();

    // create depthbuffer texture
    depthBuffer = textureList.Add("depthbuffer", kexTexture::hb_texture);
    depthBuffer->SetParameters();

    consoleFont.LoadKFont("fonts/confont.kfont");

    defaultProg.InitProgram();
    defaultProg.Compile("progs/default.vert", RST_VERTEX);
    defaultProg.Compile("progs/default.frag", RST_FRAGMENT);
    defaultProg.Link();

    if(has_GL_EXT_texture_filter_anisotropic) {
        dglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropic);
    }

    renderer.Init();
    kexRenderWorld::Init();

    bIsInit = true;
    common.Printf("Renderer Initialized\n");
}

//
// kexRenderBackend::Shutdown
//

void kexRenderBackend::Shutdown(void) {
    kexTexture *texture;
    kexMaterial *material;
    kexShaderObj *shader;

    common.Printf("Shutting down render system\n");

    defaultTexture.Delete();
    whiteTexture.Delete();
    blackTexture.Delete();
    consoleFont.Material()->Delete();

    for(int i = 0; i < MAX_HASH; i++) {
        for(material = materials.GetData(i); material; material = materials.Next()) {
            material->Delete();
        }
        for(texture = textureList.GetData(i); texture; texture = textureList.Next()) {
            texture->Delete();
        }
        for(shader = shaders.GetData(i); shader; shader = shaders.Next()) {
            shader->Delete();
        }
    }

    // do last round of texture flushing to make sure we freed everything
    Mem_Purge(kexTexture::hb_texture);
}

//
// kexRenderBackend::SetOrtho
//

void kexRenderBackend::SetOrtho(void) {
    kexMatrix mtx;

    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();

    mtx.SetOrtho(0, (float)sysMain.VideoWidth(), (float)sysMain.VideoHeight(), 0, -1, 1);
    dglLoadMatrixf(mtx.ToFloatPtr());

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();
}

//
// kexRenderBackend::SwapBuffers
//

void kexRenderBackend::SwapBuffers(void) {
    if(cvarRenderFinish.GetBool()) {
        dglFinish();
    }

    sysMain.SwapBuffers();
}

//
// kexRenderBackend::SetState
//

void kexRenderBackend::SetState(const int bits, bool bEnable) {
#define TOGGLEGLBIT(flag, bit)                                  \
    if(bEnable && !(glState.glStateBits & (1 << flag))) {       \
        dglEnable(bit);                                         \
        glState.glStateBits |= (1 << flag);                     \
    }                                                           \
    else if(!bEnable && (glState.glStateBits & (1 << flag))) {  \
        dglDisable(bit);                                        \
        glState.glStateBits &= ~(1 << flag);                    \
    }
    
    switch(bits) {
        case GLSTATE_BLEND:
            TOGGLEGLBIT(GLSTATE_BLEND, GL_BLEND);
            break;
        case GLSTATE_CULL:
            TOGGLEGLBIT(GLSTATE_CULL, GL_CULL_FACE);
            break;
        case GLSTATE_TEXTURE0:
            SetTextureUnit(0);
            TOGGLEGLBIT(GLSTATE_TEXTURE0, GL_TEXTURE_2D);
            break;
        case GLSTATE_TEXTURE1:
            SetTextureUnit(1);
            TOGGLEGLBIT(GLSTATE_TEXTURE1, GL_TEXTURE_2D);
            break;
        case GLSTATE_TEXTURE2:
            SetTextureUnit(2);
            TOGGLEGLBIT(GLSTATE_TEXTURE2, GL_TEXTURE_2D);
            break;
        case GLSTATE_TEXTURE3:
            SetTextureUnit(3);
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
        case GLSTATE_LIGHTING:
            TOGGLEGLBIT(GLSTATE_LIGHTING, GL_LIGHTING);
            break;
        case GLSTATE_FOG:
            TOGGLEGLBIT(GLSTATE_FOG, GL_FOG);
            break;
        default:
            common.Warning("kexRenderBackend::SetState: unknown bit flag: %i\n", bits);
            break;
    }
    
#undef TOGGLEGLBIT
}

//
// kexRenderBackend::SetState
//

void kexRenderBackend::SetState(unsigned int flags) {
    for(int i = 0; i < NUMGLSTATES; i++) {
        if(!(flags & BIT(i))) {
            SetState(i, false);
            continue;
        }

        SetState(i, true);
    }
}

//
// kexRenderBackend::SetFunc
//

void kexRenderBackend::SetAlphaFunc(int func, float val) {
    int pFunc = (glState.alphaFunction ^ func) |
        (glState.alphaFuncThreshold != val);
        
    if(pFunc == 0) {
        return;
    }
    
    int glFunc = 0;

    switch(func) {
        case GLFUNC_EQUAL:
            glFunc = GL_EQUAL;
            break;
        case GLFUNC_ALWAYS:
            glFunc = GL_ALWAYS;
            break;
        case GLFUNC_LEQUAL:
            glFunc = GL_LEQUAL;
            break;
        case GLFUNC_GEQUAL:
            glFunc = GL_GEQUAL;
            break;
        case GLFUNC_NEVER:
            glFunc = GL_NEVER;
            break;
    }
    
    dglAlphaFunc(glFunc, val);   
    glState.alphaFunction = func;
    glState.alphaFuncThreshold = val;
}

//
// kexRenderBackend::SetDepth
//

void kexRenderBackend::SetDepth(int func) {
    int pFunc = glState.depthFunction ^ func;
    
    if(pFunc == 0) {
        return;
    }
    
    int glFunc = 0;
        
    switch(func) {
        case GLFUNC_EQUAL:
            glFunc = GL_EQUAL;
            break;
        case GLFUNC_ALWAYS:
            glFunc = GL_ALWAYS;
            break;
        case GLFUNC_LEQUAL:
            glFunc = GL_LEQUAL;
            break;
        case GLFUNC_GEQUAL:
            glFunc = GL_GEQUAL;
            break;
        case GLFUNC_NEVER:
            glFunc = GL_NEVER;
            break;
    }
    
    dglDepthFunc(glFunc);
    glState.depthFunction = func;
}

//
// kexRenderBackend::SetBlend
//

void kexRenderBackend::SetBlend(int src, int dest) {
    int pBlend = (glState.blendSrc ^ src) | (glState.blendDest ^ dest);
    
    if(pBlend == 0) {
        return;
    }
    
    int glSrc = GL_ONE;
    int glDst = GL_ONE;
    
    switch(src) {
        case GLSRC_ZERO:
            glSrc = GL_ZERO;
            break;
        case GLSRC_ONE:
            glSrc = GL_ONE;
            break;
        case GLSRC_DST_COLOR:
            glSrc = GL_DST_COLOR;
            break;
        case GLSRC_ONE_MINUS_DST_COLOR:
            glSrc = GL_ONE_MINUS_DST_COLOR;
            break;
        case GLSRC_SRC_ALPHA:
            glSrc = GL_SRC_ALPHA;
            break;
        case GLSRC_ONE_MINUS_SRC_ALPHA:
            glSrc = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case GLSRC_DST_ALPHA:
            glSrc = GL_DST_ALPHA;
            break;
        case GLSRC_ONE_MINUS_DST_ALPHA:
            glSrc = GL_ONE_MINUS_DST_ALPHA;
            break;
        case GLSRC_ALPHA_SATURATE:
            glSrc = GL_SRC_ALPHA_SATURATE;
            break;
    }
    
    switch(dest) {
        case GLDST_ZERO:
            glDst = GL_ZERO;
            break;
        case GLDST_ONE:
            glDst = GL_ONE;
            break;
        case GLDST_SRC_COLOR:
            glDst = GL_SRC_COLOR;
            break;
        case GLDST_ONE_MINUS_SRC_COLOR:
            glDst = GL_ONE_MINUS_SRC_COLOR;
            break;
        case GLDST_SRC_ALPHA:
            glDst = GL_SRC_ALPHA;
            break;
        case GLDST_ONE_MINUS_SRC_ALPHA:
            glDst = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case GLDST_DST_ALPHA:
            glDst = GL_DST_ALPHA;
            break;
        case GLDST_ONE_MINUS_DST_ALPHA:
            glDst = GL_ONE_MINUS_DST_ALPHA;
            break;
    }
    
    dglBlendFunc(glSrc, glDst);
    glState.blendSrc = src;
    glState.blendDest = dest;
}

//
// kexRenderBackend::SetEnv
//

void kexRenderBackend::SetEnv(int env) {
}

//
// kexRenderBackend::SetCull
//

void kexRenderBackend::SetCull(int type) {
    int pCullType = glState.cullType ^ type;
    
    if(pCullType == 0) {
        return;
    }
        
    int cullType = 0;
    
    switch(type) {
        case GLCULL_FRONT:
            cullType = GL_FRONT;
            break;
        case GLCULL_BACK:
            cullType = GL_BACK;
            break;
        default:
            return;
    }
    
    dglCullFace(cullType);
    glState.cullType = type;
}

//
// kexRenderBackend::SetPolyMode
//

void kexRenderBackend::SetPolyMode(int type) {
    int pPolyMode = glState.polyMode ^ type;
    
    if(pPolyMode == 0) {
        return;
    }
        
    int polyMode = 0;
    
    switch(type) {
        case GLPOLY_FILL:
            polyMode = GL_FILL;
            break;
        case GLPOLY_LINE:
            polyMode = GL_LINE;
            break;
        default:
            return;
    }
    
    dglPolygonMode(GL_FRONT_AND_BACK, polyMode);
    glState.polyMode = type;
}

//
// kexRenderBackend::SetDepthMask
//

void kexRenderBackend::SetDepthMask(int enable) {
    int pEnable = glState.depthMask ^ enable;
    
    if(pEnable == 0) {
        return;
    }
    
    int flag = 0;
    
    switch(enable) {
        case GLDEPTHMASK_YES:
            flag = GL_TRUE;
            break;
        case GLDEPTHMASK_NO:
            flag = GL_FALSE;
            break;
        default:
            return;
    }
    
    dglDepthMask(flag);
    glState.depthMask = enable;
}

//
// kexRenderBackend::SetTextureUnit
//

void kexRenderBackend::SetTextureUnit(int unit) {
    if(unit > MAX_TEXTURE_UNITS || unit < 0) {
        return;
    }
    
    if(unit == glState.currentUnit) {
        return;
    }
        
    dglActiveTextureARB(GL_TEXTURE0_ARB + unit);
    dglClientActiveTextureARB(GL_TEXTURE0_ARB + unit);
    glState.currentUnit = unit;
}

//
// kexRenderBackend::CacheTexture
//

kexTexture *kexRenderBackend::CacheTexture(const char *name, texClampMode_t clampMode,
                                          texFilterMode_t filterMode) {
    kexTexture *texture = NULL;

    if(name == NULL || name[0] == 0) {
        return NULL;
    }

    if(!(texture = textureList.Find(name))) {
        byte *data;

        texture = textureList.Add(name, kexTexture::hb_texture);
        texture->SetMasked(true);
        strcpy(texture->filePath, name);

        data = texture->LoadFromFile(name);
        texture->Upload(&data, clampMode, filterMode);

        if(data != NULL) {
            Mem_Free(data);
        }
    }

    return texture;
}

//
// kexRenderBackend::LoadShader
//

kexShaderObj *kexRenderBackend::CacheShader(const char *file) {
    kexShaderObj *sobj;
    
    if(file == NULL) {
        return NULL;
    }
    else if(file[0] == 0) {
        return NULL;
    }
    
    if(!(sobj = shaders.Find(file))) {
        kexKeyMap *def;
        
        if((def = defManager.FindDefEntry(file))) {
            sobj = shaders.Add(file);
            strncpy(sobj->fileName, file, MAX_FILEPATH);

            sobj->InitFromDefinition(def);

            if(sobj->HasErrors()) {
                common.Warning("There were some errors when compiling %s\n", file);
            }
            
        }
        else {
            return NULL;
        }
    }
    
    return sobj;
}

//
// kexRenderBackend::CacheFont
//

kexFont *kexRenderBackend::CacheFont(const char *name) {
    kexFont *font = NULL;

    if(!(font = fontList.Find(name))) {
        font = fontList.Add(name);
        font->LoadKFont(name);
    }

    return font;
}

//
// kexRenderBackend::CacheMaterial
//

kexMaterial *kexRenderBackend::CacheMaterial(const char *file) {
    kexMaterial *material;
    
    if(file == NULL) {
        return NULL;
    }
    else if(file[0] == 0) {
        return NULL;
    }
    
    if(!(material = materials.Find(file))) {
        kexLexer *lexer;
        filepath_t tStr;
        int pos;
        int len;
        bool bFoundMaterial = false;
        
        pos = kexStr::IndexOf(file, "@");
        
        if(pos == -1) {
            return NULL;
        }
        
        len = strlen(file);
        strncpy(tStr, file, pos);
        tStr[pos] = 0;
        
        if(!(lexer = parser.Open(tStr))) {
            common.Warning("kexMaterialManager::LoadMaterial: %s not found\n", tStr);
            return NULL;
        }

        strncpy(tStr, file + pos + 1, len - pos);
        tStr[len - pos] = 0;

        while(lexer->CheckState()) {
            lexer->Find();

            if(lexer->Matches(tStr)) {
                bFoundMaterial = true;
                material = materials.Add(file);
                strncpy(material->fileName, file, MAX_FILEPATH);

                material->Init();
                material->Parse(lexer);
                break;
            }
        }

        if(bFoundMaterial == false) {
            common.Warning("kexMaterialManager::LoadMaterial: %s not found\n", tStr);
            parser.Close();
            return NULL;
        }
        
        // we're done with the file
        parser.Close();
    }
    
    return material;
}

//
// kexRenderBackend::DisableShaders
//

void kexRenderBackend::DisableShaders(void) {
    if(glState.currentProgram != 0) {
        dglUseProgramObjectARB(0);
        glState.currentProgram = 0;
    }

    renderer.currentSurface = NULL;
}

//
// kexRenderBackend::GetDepthSizeComponent
//

const int kexRenderBackend::GetDepthSizeComponent(void) {
    int depthSize = cvarVidDepthSize.GetInt();
    
    switch(depthSize) {
        case 16:
            return GL_DEPTH_COMPONENT16_ARB;
        case 24:
            return GL_DEPTH_COMPONENT24_ARB;
        case 32:
            return GL_DEPTH_COMPONENT32_ARB;
        default:
            common.Warning("GetDepthSizeComponent: unknown depth size (%i)", depthSize);
            break;
    }
    
    return GL_DEPTH_COMPONENT;
}

//
// kexRenderBackend::DrawLoadingScreen
//

void kexRenderBackend::DrawLoadingScreen(const char *text) {
    rcolor c = 0xffffffff;

    dglClearColor(0, 0, 0, 1.0f);
    dglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SetOrtho();
    consoleFont.DrawString(
        text,
        (float)sysMain.VideoWidth() * 0.5f,
        (float)sysMain.VideoHeight() * 0.5f,
        1,
        true,
        (byte*)&c,
        (byte*)&c);

    SwapBuffers();
}

//
// kexRenderBackend::BindDrawPointers
//

void kexRenderBackend::BindDrawPointers(void) {
    dglNormalPointer(GL_FLOAT, sizeof(float)*3, drawVertices);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, drawTexCoords);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, drawVertices);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, drawRGB);
}

//
// kexRenderBackend::AddTriangle
//

void kexRenderBackend::AddTriangle(int v0, int v1, int v2) {
    if(indiceCount + 3 >= GL_MAX_INDICES) {
        common.Warning("Static triangle indice overflow");
        return;
    }

    drawIndices[indiceCount++] = v0;
    drawIndices[indiceCount++] = v1;
    drawIndices[indiceCount++] = v2;
}

//
// kexRenderBackend::AddVertex
//

void kexRenderBackend::AddVertex(float x, float y, float z, float s, float t,
                                byte r, byte g, byte b, byte a) {
    if((vertexCount * 4 + 3) >= GL_MAX_VERTICES) {
        common.Warning("Static vertex draw overflow");
        return;
    }

    drawVertices[vertexCount * 3 + 0]   = x;
    drawVertices[vertexCount * 3 + 1]   = y;
    drawVertices[vertexCount * 3 + 2]   = z;
    drawTexCoords[vertexCount * 2 + 0]  = s;
    drawTexCoords[vertexCount * 2 + 1]  = t;
    drawRGB[vertexCount * 4 + 0]        = r;
    drawRGB[vertexCount * 4 + 1]        = g;
    drawRGB[vertexCount * 4 + 2]        = b;
    drawRGB[vertexCount * 4 + 3]        = a;

    vertexCount++;
}

//
// kexRenderBackend::AddLine
//

void kexRenderBackend::AddLine(float x1, float y1, float z1,
                              float x2, float y2, float z2,
                              byte r, byte g, byte b, byte a) {
    
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x1, y1, z1, 0, 0, r, g, b, a);
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x2, y2, z2, 0, 0, r, g, b, a);
}

//
// kexRenderBackend::AddLine
//

void kexRenderBackend::AddLine(float x1, float y1, float z1,
                              float x2, float y2, float z2,
                              byte r1, byte g1, byte b1, byte a1,
                              byte r2, byte g2, byte b2, byte a2) {
    
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x1, y1, z1, 0, 0, r1, g1, b1, a1);
    drawIndices[indiceCount++] = vertexCount;
    AddVertex(x2, y2, z2, 0, 0, r2, g2, b2, a2);
}

//
// kexRenderBackend::DrawElements
//

void kexRenderBackend::DrawElements(const bool bClearCount) {
    DisableShaders();
    dglDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);

    if(bClearCount) {
        indiceCount = 0;
        vertexCount = 0;
    }
}

//
// kexRenderBackend::DrawElements
//
// Draws using the specified material. A temp. surface
// is created in order to draw the material
//

void kexRenderBackend::DrawElements(const kexMaterial *material, const bool bClearCount) {
    surface_t surf;
    
    surf.numVerts   = vertexCount;
    surf.numIndices = indiceCount;
    surf.vertices   = reinterpret_cast<kexVec3*>(drawVertices);
    surf.coords     = drawTexCoords;
    surf.rgb        = drawRGB;
    surf.normals    = drawVertices;
    surf.indices    = drawIndices;
    
    renderer.DrawSurface(&surf, (kexMaterial*)material);
    
    if(bClearCount) {
        indiceCount = 0;
        vertexCount = 0;
    }
}

//
// kexRenderBackend::DrawLineElements
//

void kexRenderBackend::DrawLineElements(void) {
    DisableShaders();
    dglDrawElements(GL_LINES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);
    
    indiceCount = 0;
    vertexCount = 0;
}
