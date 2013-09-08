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
#include "renderSystem.h"

kexRenderSystem renderSystem;

//
// kexRenderSystem::kexRenderSystem
//

kexRenderSystem::kexRenderSystem(void) {
    this->viewWidth         = this->SCREEN_WIDTH;
    this->viewHeight        = this->SCREEN_HEIGHT;
    this->viewWindowX       = 0;
    this->viewWindowY       = 0;
    this->maxTextureUnits   = 1;
    this->maxTextureSize    = 64;
    this->maxAnisotropic    = 0;
    this->bWideScreen       = false;
    this->bFullScreen       = false;
}

//
// kexRenderSystem::~kexRenderSystem
//

kexRenderSystem::~kexRenderSystem(void) {
}

//
// kexRenderSystem::Init
//

void kexRenderSystem::Init(void) {
}

//
// kexRenderSystem::InitOpenGL
//

void kexRenderSystem::InitOpenGL(void) {
}

//
// kexRenderSystem::Shutdown
//

void kexRenderSystem::Shutdown(void) {
}

//
// kexRenderSystem::SetOrtho
//

void kexRenderSystem::SetOrtho(void) {
}

//
// kexRenderSystem::SwapBuffers
//

void kexRenderSystem::SwapBuffers(void) {
}

//
// kexRenderSystem::GetScreenBuffer
//

byte *kexRenderSystem::GetScreenBuffer(int x, int y, int width, int height, bool bFlip) {
    return NULL;
}

//
// kexRenderSystem::ScreenToTexture
//

dtexture kexRenderSystem::ScreenToTexture(void) {
    return 0;
}

//
// kexRenderSystem::SetState
//

void kexRenderSystem::SetState(int bits, bool bEnable) {
}

//
// kexRenderSystem::SetFunc
//

void kexRenderSystem::SetFunc(int type, int func, float val) {
}

//
// kexRenderSystem::SetEnv
//

void kexRenderSystem::SetEnv(int env) {
}

//
// kexRenderSystem::SetCull
//

void kexRenderSystem::SetCull(int type) {
}

//
// kexRenderSystem::SetTextureUnit
//

void kexRenderSystem::SetTextureUnit(int unit) {
}
