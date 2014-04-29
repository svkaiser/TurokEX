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
// DESCRIPTION: Framebuffer objects
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderBackend.h"
#include "fbo.h"

//
// kexFBO::kexFBO
//

kexFBO::kexFBO(void) {
    this->fboId = 0;
    this->rboId = 0;
    this->fboTexId = 0;
    this->bLoaded = false;
}

//
// kexFBO::~kexFBO
//

kexFBO::~kexFBO(void) {
    Delete();
}

//
// kexFBO::CheckStatus
//

void kexFBO::CheckStatus(void) {
    GLenum status = dglCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
    
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        common.Warning("kexFBO::Init: frame buffer creation didn't complete\n");
        bLoaded = false;
    }
    else {
        bLoaded = true;
    }
}

//
// kexFBO::InitColorAttachment
//

void kexFBO::InitColorAttachment(const int attachment, const int width, const int height) {
    if(bLoaded) {
        return;
    }
    
    if(attachment < 0 || attachment >= renderBackend.MaxColorAttachments()) {
        return;
    }
    
    fboAttachment = GL_COLOR_ATTACHMENT0_EXT + attachment;
    
    fboWidth = width;
    fboHeight = height;
    
    // texture
    dglGenTextures(1, &fboTexId);
    dglBindTexture(GL_TEXTURE_2D, fboTexId);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    dglTexImage2D(GL_TEXTURE_2D,
                  0,
                  GL_RGBA,
                  fboWidth,
                  fboHeight,
                  0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE,
                  0);
    
    // renderbuffer
    dglGenRenderbuffers(1, &rboId);
    dglBindRenderbuffer(GL_RENDERBUFFER_EXT, rboId);
    dglRenderbufferStorage(GL_RENDERBUFFER_EXT,
                           GL_DEPTH_COMPONENT,
                           fboWidth,
                           fboHeight);
    
    // framebuffer
    dglGenFramebuffers(1, &fboId);
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, fboId);
    dglDrawBuffer(GL_NONE);
    dglReadBuffer(GL_NONE);
    dglFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
                            fboAttachment,
                            GL_TEXTURE_2D,
                            fboTexId,
                            0);
    
    dglFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
                               GL_DEPTH_ATTACHMENT_EXT,
                               GL_RENDERBUFFER_EXT,
                               rboId);
    
    CheckStatus();
    
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    dglBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
    dglBindTexture(GL_TEXTURE_2D, 0);
    dglDrawBuffer(GL_BACK);
    dglReadBuffer(GL_BACK);
}

//
// kexFBO::InitColorAttachment
//

void kexFBO::InitColorAttachment(const int attachment) {
    int width = kexMath::RoundPowerOfTwo(sysMain.VideoWidth()) >> 1;
    int height = kexMath::RoundPowerOfTwo(sysMain.VideoHeight()) >> 1;
    
    InitColorAttachment(attachment, width, height);
}

//
// kexFBO::InitDepthAttachment
//

void kexFBO::InitDepthAttachment(const int width, const int height) {
    if(bLoaded) {
        return;
    }
    
    fboWidth = width;
    fboHeight = height;
    
    // texture
    dglGenTextures(1, &fboTexId);
    dglBindTexture(GL_TEXTURE_2D, fboTexId);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    dglTexImage2D(GL_TEXTURE_2D,
                  0,
                  GL_DEPTH_COMPONENT,
                  fboWidth,
                  fboHeight,
                  0,
                  GL_DEPTH_COMPONENT,
                  GL_UNSIGNED_BYTE,
                  0);
    
    // framebuffer
    dglGenFramebuffers(1, &fboId);
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, fboId);
    dglDrawBuffer(GL_NONE);
    dglReadBuffer(GL_NONE);
    dglFramebufferTexture2D(GL_FRAMEBUFFER_EXT,
                            GL_DEPTH_ATTACHMENT_EXT,
                            GL_TEXTURE_2D,
                            fboTexId,
                            0);
    
    CheckStatus();
    
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    dglBindTexture(GL_TEXTURE_2D, 0);
    dglDrawBuffer(GL_BACK);
    dglReadBuffer(GL_BACK);
}

//
// kexFBO::Delete
//

void kexFBO::Delete(void) {
    if(!bLoaded) {
        return;
    }
    
    if(fboTexId != 0) {
        dglDeleteTextures(1, &fboTexId);
        fboTexId = 0;
    }
    if(fboId != 0) {
        dglDeleteFramebuffers(1, &fboId);
        fboId = 0;
    }
    if(rboId != 0) {
        dglDeleteRenderbuffers(1, &rboId);
        rboId = 0;
    }
    
    bLoaded = false;
}

//
// kexFBO::Bind
//

void kexFBO::Bind(void) {
    if(fboId == renderBackend.glState.currentFBO) {
        return;
    }
    
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, fboId);
    dglReadBuffer(fboAttachment);
    dglDrawBuffer(fboAttachment);
    renderBackend.glState.currentFBO = fboId;
}

//
// kexFBO::UnBind
//

void kexFBO::UnBind(void) {
    if(renderBackend.glState.currentFBO == 0) {
        return;
    }
    
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    dglDrawBuffer(GL_BACK);
    dglReadBuffer(GL_BACK);
    
    renderBackend.glState.currentFBO = 0;
}

//
// kexFBO::BindImage
//

void kexFBO::BindImage(void) {
    int unit = renderBackend.glState.currentUnit;
    dtexture currentTexture = renderBackend.glState.textureUnits[unit].currentTexture;
    
    if(fboTexId == currentTexture) {
        return;
    }
    
    dglBindTexture(GL_TEXTURE_2D, fboTexId);
    renderBackend.glState.textureUnits[unit].currentTexture = fboTexId;
}
