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
// DESCRIPTION: Canvas system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "renderSystem.h"
#include "memHeap.h"
#include "canvas.h"
#include "system.h"
#include "scriptAPI/scriptSystem.h"

kexHeapBlock kexCanvas::hb_canvas("canvas", false, NULL, NULL);

//-----------------------------------------------------------------------------
//
// kexCanvasObject
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_CLASS(kexCanvasObject, kexObject)

//
// kexCanvasObject::kexCanvasObject
//

kexCanvasObject::kexCanvasObject(void) {
    this->x         = 0;
    this->y         = 0;
    this->scaleX    = 1;
    this->scaleY    = 1;
    this->regX      = 0;
    this->regY      = 0;
    this->alpha     = 1;
    this->bVisible  = true;
    this->parent    = NULL;
    this->refCount  = 0;

    this->link.SetData(this);
}

//
// kexCanvasObject::~kexCanvasObject
//

kexCanvasObject::~kexCanvasObject(void) {
    this->link.Remove();
}

//
// kexCanvasObject::IncRef
//

void kexCanvasObject::IncRef(void) {
    refCount++;
}

//
// kexCanvasObject::DecRef
//

void kexCanvasObject::DecRef(void) {
    refCount--;
}

//-----------------------------------------------------------------------------
//
// kexCanvasImage
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexCanvasImage, kexCanvasObject)

//
// kexCanvasImage::kexCanvasImage
//

kexCanvasImage::kexCanvasImage(void) {
    this->width = 0;
    this->height = 0;
}

//
// kexCanvasImage::~kexCanvasImage
//

kexCanvasImage::~kexCanvasImage(void) {
}

//
// kexCanvasImage::SetRGB
//

void kexCanvasImage::SetRGB(const int index, const byte r, const byte g, const byte b) {
    if(index < 0 || index >= 4) {
        return;
    }

    rgba[index * 4 + 0] = r;
    rgba[index * 4 + 1] = g;
    rgba[index * 4 + 2] = b;
}

//
// kexCanvasImage::Draw
//

void kexCanvasImage::Draw(void) {
    float ratiox;
    float ratioy;
    int i;
    float rx;
    float ry;
    float rw;
    float rh;
    float w;
    float h;
    byte a[4];

    if(bVisible == false) {
        return;
    }

    renderSystem.SetState(GLSTATE_BLEND, true);
    renderSystem.BindDrawPointers();
    texture->Bind();

    w = (width <= 0) ? (float)texture->Width() : width * 2;
    h = (height <= 0) ? (float)texture->Height() : height * 2;

    ratiox = (float)FIXED_WIDTH / sysMain.VideoWidth();
    ratioy = (float)FIXED_HEIGHT / sysMain.VideoHeight();
    rx = x / ratiox;
    rw = rx + (w / ratiox) * 0.5f * scaleX;
    ry = y / ratioy;
    rh = ry + (h / ratioy) * 0.5f * scaleY;

    for(i = 0; i < 4; i++) {
        a[i] = (byte)((float)rgba[i * 4 + 3] * alpha);
    }

    if(parent != NULL) {
        rx += parent->x / ratiox;
        rw += parent->x / ratiox;
        ry += parent->y / ratioy;
        rh += parent->y / ratioy;

        rw *= parent->scaleX;
        rh *= parent->scaleY;

        for(i = 0; i < 4; i++) {
            a[i] = (byte)((float)a[i] * parent->alpha);
        }
    }

    renderSystem.AddVertex(
        rx,
        ry,
        0,
        0,
        0,
        rgba[0 * 4 + 0],
        rgba[0 * 4 + 1],
        rgba[0 * 4 + 2],
        a[0]);
    renderSystem.AddVertex(
        rw,
        ry,
        0,
        1,
        0,
        rgba[1 * 4 + 0],
        rgba[1 * 4 + 1],
        rgba[1 * 4 + 2],
        a[1]);
    renderSystem.AddVertex(
        rx,
        rh,
        0,
        0,
        1,
        rgba[2 * 4 + 0],
        rgba[2 * 4 + 1],
        rgba[2 * 4 + 2],
        a[2]);
    renderSystem.AddVertex(
        rw,
        rh,
        0,
        1,
        1,
        rgba[3 * 4 + 0],
        rgba[3 * 4 + 1],
        rgba[3 * 4 + 2],
        a[3]);

    renderSystem.AddTriangle(0, 1, 2);
    renderSystem.AddTriangle(2, 1, 3);

    renderSystem.DrawElements();
    renderSystem.SetState(GLSTATE_BLEND, false);
}

//-----------------------------------------------------------------------------
//
// kexCanvasText
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexCanvasText, kexCanvasObject)

//
// kexCanvasText::kexCanvasText
//

kexCanvasText::kexCanvasText(void) {
    this->bCentered = false;
}

//
// kexCanvasText::~kexCanvasText
//

kexCanvasText::~kexCanvasText(void) {
}

//
// kexCanvasText::SetRGB
//

void kexCanvasText::SetRGB(const int index, const byte r, const byte g, const byte b) {
    if(index < 0 || index >= 4) {
        return;
    }

    rgba[index * 4 + 0] = r;
    rgba[index * 4 + 1] = g;
    rgba[index * 4 + 2] = b;
}

//
// kexCanvasText::Draw
//

void kexCanvasText::Draw(void) {
    byte color[32];
    float dx, dy;
    float ds;
    float ratiox;
    float ratioy;

    if(bVisible == false) {
        return;
    }

    for(int i = 0; i < 4; i++) {
        color[i * 4 + 0] = rgba[i * 4 + 0];
        color[i * 4 + 1] = rgba[i * 4 + 1];
        color[i * 4 + 2] = rgba[i * 4 + 2];
        color[i * 4 + 3] = (byte)((float)rgba[i * 4 + 3] * alpha);
    }

    dx = x;
    dy = y;
    ds = scaleX;

    ratiox = (float)FIXED_WIDTH / sysMain.VideoWidth();
    ratioy = (float)FIXED_HEIGHT / sysMain.VideoHeight();
    dx /= ratiox;
    dy /= ratioy;
    ds /= ratiox;

    ds *= 0.5f;

    if(parent != NULL) {
        dx += (parent->x / ratiox);
        dy += (parent->y / ratioy);
        ds *= parent->scaleX;

        for(int i = 0; i < 4; i++) {
            color[i * 4 + 3] = (byte)((float)color[i * 4 + 3] * parent->alpha);
        }
    }

    renderSystem.SetState(GLSTATE_BLEND, true);
    font->DrawString(text.c_str(), dx, dy, ds, bCentered, (byte*)&color[0 * 4], (byte*)&color[2 * 4]);
    renderSystem.SetState(GLSTATE_BLEND, false);
}

//-----------------------------------------------------------------------------
//
// kexContainer
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexContainer, kexCanvasObject)

//
// kexContainer::kexContainer
//

kexContainer::kexContainer(void) {
}

//
// kexContainer::~kexContainer
//

kexContainer::~kexContainer(void) {
}

//
// kexContainer::Draw
//

void kexContainer::Draw(void) {
    if(bVisible == false) {
        return;
    }

    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        obj->Draw();
    }
}

//
// kexContainer::AddChild
//

void kexContainer::AddChild(kexCanvasObject *object) {
    object->link.Add(children);
    object->parent = this;
}

//
// kexContainer::RemoveChild
//

void kexContainer::RemoveChild(kexCanvasObject *object) {
    object->link.Remove();
    object->parent = NULL;
    object->DecRef();
}

//
// kexContainer::operator[]
//

kexCanvasObject *kexContainer::operator[](int index) {
    int idx = 0;
    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        if(idx++ == index) {
            return obj;
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------------
//
// kexCanvas
//
//-----------------------------------------------------------------------------

//
// kexCanvas::kexCanvas
//

kexCanvas::kexCanvas(void) {
}

//
// kexCanvas::~kexCanvas
//

kexCanvas::~kexCanvas(void) {
}

//
// kexCanvas::CreateImage
//

kexCanvasImage *kexCanvas::CreateImage(const char *texture) {
    kexCanvasImage *img = static_cast<kexCanvasImage*>(kexObject::Create("kexCanvasImage"));

    if(img == NULL) {
        return NULL;
    }

    img->texture = renderSystem.CacheTexture(texture, TC_CLAMP, TF_LINEAR);

    img->link.Clear();
    img->mainLink.Clear();
    img->mainLink.Add(objects);

    for(int i = 0; i < 4; i++) {
        img->rgba[i * 4 + 0] = 0xff;
        img->rgba[i * 4 + 1] = 0xff;
        img->rgba[i * 4 + 2] = 0xff;
        img->rgba[i * 4 + 3] = 0xff;
    }

    return img;
}

//
// kexCanvas::CreateImage
//

kexCanvasImage *kexCanvas::CreateImage(const kexStr &texture) {
    return CreateImage(texture.c_str());
}

//
// kexCanvas::CreateContainer
//

kexContainer *kexCanvas::CreateContainer(void) {
    kexContainer *container = static_cast<kexContainer*>(kexObject::Create("kexContainer"));

    container->link.Clear();
    container->mainLink.Clear();
    container->mainLink.Add(objects);

    return container;
}

//
// kexCanvas::CreateText
//

kexCanvasText *kexCanvas::CreateText(const char *font) {
    kexCanvasText *str = static_cast<kexCanvasText*>(kexObject::Create("kexCanvasText"));

    str->font = renderSystem.CacheFont(font);

    str->link.Clear();
    str->mainLink.Clear();
    str->mainLink.Add(objects);

    for(int i = 0; i < 4; i++) {
        str->rgba[i * 4 + 0] = 0xff;
        str->rgba[i * 4 + 1] = 0xff;
        str->rgba[i * 4 + 2] = 0xff;
        str->rgba[i * 4 + 3] = 0xff;
    }

    return str;
}

//
// kexCanvas::CreateText
//

kexCanvasText *kexCanvas::CreateText(const kexStr &font) {
    return CreateText(font.c_str());
}

//
// kexCanvas::AddChild
//

void kexCanvas::AddChild(kexCanvasObject *object) {
    object->link.Add(children);
}

//
// kexCanvas::RemoveChild
//

void kexCanvas::RemoveChild(kexCanvasObject *object) {
    object->link.Remove();
    object->DecRef();
}

//
// kexCanvas::operator[]
//

kexCanvasObject *kexCanvas::operator[](int index) {
    int idx = 0;
    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        if(idx++ == index) {
            return obj;
        }
    }

    return NULL;
}

//
// kexCanvas::Draw
//

void kexCanvas::Draw(void) {
    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        obj->Draw();
    }
}

//
// RegisterCanvasObjectProperties
//

template<class type>
static void RegisterCanvasObject(const char *name) {
    kexScriptManager::RegisterRefObject<type>(name);
    kexScriptManager::RegisterIncRef<type>(name);
    kexScriptManager::RegisterDecRef<type>(name);
    scriptManager.Engine()->RegisterObjectProperty(name, "float x",
        asOFFSET(type, x));
    scriptManager.Engine()->RegisterObjectProperty(name, "float y",
        asOFFSET(type, y));
    scriptManager.Engine()->RegisterObjectProperty(name, "float scaleX",
        asOFFSET(type, scaleX));
    scriptManager.Engine()->RegisterObjectProperty(name, "float scaleY",
        asOFFSET(type, scaleY));
    scriptManager.Engine()->RegisterObjectProperty(name, "float regX",
        asOFFSET(type, regX));
    scriptManager.Engine()->RegisterObjectProperty(name, "float regY",
        asOFFSET(type, regY));
    scriptManager.Engine()->RegisterObjectProperty(name, "float alpha",
        asOFFSET(type, alpha));
    scriptManager.Engine()->RegisterObjectProperty(name, "bool bVisible",
        asOFFSET(type, bVisible));

    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        kva("void AddChild(%s@)", name),
        asMETHODPR(kexCanvas, AddChild, (kexCanvasObject*), void),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        kva("void RemoveChild(%s@)", name),
        asMETHODPR(kexCanvas, RemoveChild, (kexCanvasObject*), void),
        asCALL_THISCALL);
}

//
// RegisterContainerChildMethods
//

static void RegisterContainerChildMethods(const char *name) {
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvasContainer",
        kva("void AddChild(%s@)", name),
        asMETHODPR(kexContainer, AddChild, (kexCanvasObject*), void),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvasContainer",
        kva("void RemoveChild(%s@)", name),
        asMETHODPR(kexContainer, RemoveChild, (kexCanvasObject*), void),
        asCALL_THISCALL);
}

//
// kexCanvas::InitObject
//

void kexCanvas::InitObject(void) {
    kexScriptManager::RegisterDataObject<kexCanvas>("kCanvas");
    RegisterCanvasObject<kexCanvasImage>("kCanvasImage");
    RegisterCanvasObject<kexCanvasText>("kCanvasText");
    RegisterCanvasObject<kexContainer>("kCanvasContainer");

    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        "kCanvasImage @CreateImage(const kStr &in)",
        asMETHODPR(kexCanvas, CreateImage, (const kexStr&), kexCanvasImage*),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        "kCanvasContainer @CreateContainer(void)",
        asMETHODPR(kexCanvas, CreateContainer, (void), kexContainer*),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        "kCanvasText @CreateText(const kStr &in)",
        asMETHODPR(kexCanvas, CreateText, (const kexStr&), kexCanvasText*),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvas",
        "kCanvasImage @opIndex(const int)",
        asMETHODPR(kexCanvas, operator[], (const int), kexCanvasObject*),
        asCALL_THISCALL);

    RegisterContainerChildMethods("kCanvasImage");
    RegisterContainerChildMethods("kCanvasText");
    RegisterContainerChildMethods("kCanvasContainer");

    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvasContainer",
        "kCanvasImage @opIndex(const int)",
        asMETHODPR(kexContainer, operator[], (const int), kexCanvasObject*),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvasImage",
        "void SetRGB(const int, const uint8, const uint8, const uint8)",
        asMETHODPR(kexCanvasImage, SetRGB, (const int, const byte, const byte, const byte), void),
        asCALL_THISCALL);
    scriptManager.Engine()->RegisterObjectMethod(
        "kCanvasText",
        "void SetRGB(const int, const uint8, const uint8, const uint8)",
        asMETHODPR(kexCanvasText, SetRGB, (const int, const byte, const byte, const byte), void),
        asCALL_THISCALL);

    scriptManager.Engine()->RegisterObjectProperty("kCanvasImage", "float width",
        asOFFSET(kexCanvasImage, width));
    scriptManager.Engine()->RegisterObjectProperty("kCanvasImage", "float height",
        asOFFSET(kexCanvasImage, height));

    scriptManager.Engine()->RegisterObjectProperty("kCanvasText", "kStr text",
        asOFFSET(kexCanvasText, text));
    scriptManager.Engine()->RegisterObjectProperty("kCanvasText", "bool bCentered",
        asOFFSET(kexCanvasText, bCentered));

    scriptManager.Engine()->RegisterGlobalProperty("kCanvas Canvas", &renderSystem.Canvas());
}
