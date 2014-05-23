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
#include "renderBackend.h"
#include "renderMain.h"
#include "memHeap.h"
#include "canvas.h"
#include "system.h"
#include "scriptAPI/scriptSystem.h"

//
// kexCanvasComponent::kexCanvasComponent
//

kexCanvasComponent::kexCanvasComponent(void) {
    this->onUpdate = NULL;
    this->onInit = NULL;
    this->onHover = NULL;
    this->onExit = NULL;
    this->onDown = NULL;
    this->onRelease = NULL;
}

//
// kexCanvasComponent::~kexCanvasComponent
//

kexCanvasComponent::~kexCanvasComponent(void) {
}

//
// kexCanvasComponent::Init
//

void kexCanvasComponent::Init(void) {
    scriptManager.Engine()->RegisterInterface("CanvasComponent");
    scriptManager.Engine()->RegisterInterfaceMethod("CanvasComponent", "void OnUpdate(void)");
    scriptManager.Engine()->RegisterInterfaceMethod("CanvasComponent", "void OnInit(void)");
}

//
// kexCanvasComponent::Construct
//

void kexCanvasComponent::Construct(const char *className) {
    if(!Spawn(className)) {
        return;
    }
    
    CallConstructor((kexStr(className) + " @" + className + "(kCanvasScriptObject@)").c_str());
    onUpdate = type->GetMethodByDecl("void OnUpdate(void)");
    onInit = type->GetMethodByDecl("void OnInit(void)");
    onHover = type->GetMethodByDecl("void OnHover(void)");
    onExit = type->GetMethodByDecl("void OnExit(void)");
    onDown = type->GetMethodByDecl("void OnDown(void)");
    onRelease = type->GetMethodByDecl("void OnRelease(void)");
}

//-----------------------------------------------------------------------------
//
// kexCanvasObject
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_CLASS(kexCanvasObject, kexObject)

int kexCanvasObject::objId = 0;

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
    this->scriptRef = 0;
    this->refCanvas = NULL;
    this->max[0]    = -M_INFINITY;
    this->max[1]    = -M_INFINITY;
    this->min[0]    =  M_INFINITY;
    this->min[1]    =  M_INFINITY;
    this->curId     = kexCanvasObject::objId++;

    this->link.SetData(this);
}

//
// kexCanvasObject::~kexCanvasObject
//

kexCanvasObject::~kexCanvasObject(void) {
    scriptRef = 0;
    this->link.Remove();
}

//
// kexCanvasObject::IncRef
//

int kexCanvasObject::IncRef(void) {
    return ++scriptRef;
}

//
// kexCanvasObject::DecRef
//

int kexCanvasObject::DecRef(void) {
    return --scriptRef;
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

void kexCanvasImage::Draw(kexMatrix &curMatrix, const float &curAlpha) {
    int i;
    float w;
    float h;
    byte a[4];

    if(bVisible == false) {
        return;
    }

    kexMatrix mtx;

    renderBackend.SetState(GLSTATE_BLEND, true);
    renderBackend.SetState(GLSTATE_ALPHATEST, true);
    renderBackend.SetState(GLSTATE_TEXTURE0, true);
    renderBackend.SetState(GLSTATE_CULL, false);
    renderBackend.SetState(GLSTATE_DEPTHTEST, false);

    renderer.BindDrawPointers();
    texture->Bind();

    w = ((width <= 0) ? (float)texture->Width() : (width * 2.0f)) * 0.5f;
    h = ((height <= 0) ? (float)texture->Height() : (height * 2.0f)) * 0.5f;

    for(i = 0; i < 4; i++) {
        a[i] = (byte)((float)rgba[i * 4 + 3] * alpha * curAlpha);
    }

    mtx.Scale(scaleX, scaleY, 1);
    mtx.SetTranslation(x - (regX * 0.5f), y - (regY * 0.5f), 0);

    matrix = mtx * curMatrix;
    
    min[0] = 0;
    min[1] = 0;
    max[0] = w;
    max[1] = h;

    renderer.AddVertex(
        0,
        0,
        0,
        0,
        0,
        rgba[0 * 4 + 0],
        rgba[0 * 4 + 1],
        rgba[0 * 4 + 2],
        a[0]);
    renderer.AddVertex(
        w,
        0,
        0,
        1,
        0,
        rgba[1 * 4 + 0],
        rgba[1 * 4 + 1],
        rgba[1 * 4 + 2],
        a[1]);
    renderer.AddVertex(
        0,
        h,
        0,
        0,
        1,
        rgba[2 * 4 + 0],
        rgba[2 * 4 + 1],
        rgba[2 * 4 + 2],
        a[2]);
    renderer.AddVertex(
        w,
        h,
        0,
        1,
        1,
        rgba[3 * 4 + 0],
        rgba[3 * 4 + 1],
        rgba[3 * 4 + 2],
        a[3]);

    dglPushMatrix();
    dglMultMatrixf(matrix.ToFloatPtr());

    renderer.AddTriangle(0, 1, 2);
    renderer.AddTriangle(2, 1, 3);

    renderer.DrawElementsNoShader();
    dglPopMatrix();

    min *= matrix;
    max *= matrix;
}

//-----------------------------------------------------------------------------
//
// kexCanvasScriptObject
//
//-----------------------------------------------------------------------------

DECLARE_CLASS(kexCanvasScriptObject, kexCanvasObject)

//
// kexCanvasScriptObject::kexCanvasScriptObject
//

kexCanvasScriptObject::kexCanvasScriptObject(void) {
    this->component.SetOwner(this);
}

//
// kexCanvasScriptObject::~kexCanvasScriptObject
//

kexCanvasScriptObject::~kexCanvasScriptObject(void) {
    if(container) {
        container->Empty();
    }

    if(component.ScriptObject() != NULL) {
        component.Release();
    }
}

//
// kexCanvasScriptObject::Draw
//

void kexCanvasScriptObject::Draw(kexMatrix &curMatrix, const float &curAlpha) {
    if(component.onUpdate) {
        component.CallFunction(component.onUpdate);
    }
}

//
// kexCanvasScriptObject::SetProperty
//

void kexCanvasScriptObject::SetProperty(const char *name, const char *value) {
    bool ok = false;
    int state;
    kexStr decl;
    int type = -1;
    int intVal = 0;
    
    
    if(name == NULL || value == NULL) {
        return;
    }
    
    decl = "void Set_";
    
    if(value[0] >= '0' && value[0] <= '9') {
        if(kexStr::IndexOf(value, ".") != -1) {
            decl = decl + name + "(const float)";
            type = 0;
        }
        else {
            decl = decl + name + "(const int)";
            type = 1;
        }
    }
    else if(value[0] == '#') {
        char *hex = (char*)(value+1);
        
        decl = decl + name + "(const int)";
        type = 2;
        
        intVal = strtol(hex, &hex, 16);
    }
    else if(!kexStr::CompareCase(value, "false")) {
        decl = decl + name + "(const bool)";
        type = 3;
    }
    else if(!kexStr::CompareCase(value, "true")) {
        decl = decl + name + "(const bool)";
        type = 4;
    }
    else {
        decl = decl + name + "(const kStr &in)";
        type = 5;
    }
    
    if(type == -1) {
        return;
    }
    
    state = component.PrepareFunction(decl.c_str());
    if(state == -1) {
        return;
    }
    
    switch(type) {
        case 0:
            component.SetCallArgument(0, (float)atof(value));
            break;
        case 1:
            component.SetCallArgument(0, atoi(value));
            break;
        case 2:
            component.SetCallArgument(0, intVal);
            break;
        case 3:
            component.SetCallArgument(0, false);
            break;
        case 4:
            component.SetCallArgument(0, true);
            break;
        case 5:
            component.SetCallArgument(0, new kexStr(value));
            break;
        default:
            return;
    }
    
    if(!component.ExecuteFunction(state)) {
        return;
    }
    
    component.FinishFunction(state, &ok);
    
    if(ok == false) {
        return;
    }
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

void kexCanvasText::Draw(kexMatrix &curMatrix, const float &curAlpha) {
    byte color[32];

    if(bVisible == false) {
        return;
    }

    kexMatrix mtx;

    for(int i = 0; i < 4; i++) {
        color[i * 4 + 0] = rgba[i * 4 + 0];
        color[i * 4 + 1] = rgba[i * 4 + 1];
        color[i * 4 + 2] = rgba[i * 4 + 2];
        color[i * 4 + 3] = (byte)((float)rgba[i * 4 + 3] * alpha * curAlpha);
    }

    mtx.Scale(scaleX, scaleY, 1);
    mtx.SetTranslation(x - (regX * 0.5f), y - (regY * 0.5f), 0);

    matrix = mtx * curMatrix;

    dglPushMatrix();
    dglMultMatrixf(matrix.ToFloatPtr());

    renderBackend.SetState(GLSTATE_BLEND, true);
    font->DrawString(text.c_str(), x, y, 0.5f, bCentered, (byte*)&color[0 * 4], (byte*)&color[2 * 4]);
    renderBackend.SetState(GLSTATE_BLEND, false);

    dglPopMatrix();
    
    float w = font->StringWidth(text.c_str(), 0.5f, 0);
    float h = font->StringHeight(text.c_str(), 0.5f, 0);

    min[0] = 0;
    min[1] = 0;
    max[0] = w;
    max[1] = h;

    if(bCentered) {
        min[0] -= (w * 0.5f);
        max[0] -= (w * 0.5f);
    }
    
    min *= matrix;
    max *= matrix;
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
    Empty();
}

//
// kexContainer::Draw
//

void kexContainer::Draw(kexMatrix &curMatrix, const float &curAlpha) {
    if(bVisible == false) {
        return;
    }

    kexMatrix mtx;

    mtx.Scale(scaleX, scaleY, 1);
    mtx.SetTranslation(x, y, 0);

    matrix = mtx * curMatrix;

    float a = alpha * curAlpha;

    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        obj->Draw(matrix, a);

        if(obj->min[0] < min[0]) min[0] = obj->min[0];
        if(obj->max[0] > max[0]) max[0] = obj->max[0];
        if(obj->min[1] < min[1]) min[1] = obj->min[1];
        if(obj->max[1] > max[1]) max[1] = obj->max[1];
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
    
    if(object->InstanceOf(&kexContainer::info)) {
        static_cast<kexContainer*>(object)->Empty();
    }

    if(object->scriptRef <= 0) {
        delete object;
    }
}

//
// kexContainer::Empty
//

void kexContainer::Empty(void) {
    kexCanvasObject *next;
    
    for(kexCanvasObject *obj = children.Next(); obj != NULL;) {
        next = obj->link.Next();
        RemoveChild(obj);
        obj = next;
    }
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
    this->alpha = 1.0f;
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

    img->texture = renderBackend.CacheTexture(texture, TC_CLAMP, TF_LINEAR);

    img->link.Clear();
    img->mainLink.Clear();
    img->mainLink.Add(objects);

    for(int i = 0; i < 4; i++) {
        img->rgba[i * 4 + 0] = 0xff;
        img->rgba[i * 4 + 1] = 0xff;
        img->rgba[i * 4 + 2] = 0xff;
        img->rgba[i * 4 + 3] = 0xff;
    }
    
    img->refCanvas = this;

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
    
    container->refCanvas = this;

    return container;
}

//
// kexCanvas::CreateScriptObject
//

kexCanvasScriptObject *kexCanvas::CreateScriptObject(const char *className) {
    kexCanvasScriptObject *cso = static_cast<kexCanvasScriptObject*>
    (kexObject::Create("kexCanvasScriptObject"));
    
    cso->link.Clear();
    cso->mainLink.Clear();
    cso->mainLink.Add(objects);
    
    cso->component.Construct(className);
    if(cso->component.ScriptObject() == NULL) {
        delete cso;
        return NULL;
    }
    
    cso->refCanvas = this;
    
    return cso;
}

//
// kexCanvas::CreateText
//

kexCanvasText *kexCanvas::CreateText(const char *font) {
    kexCanvasText *str = static_cast<kexCanvasText*>(kexObject::Create("kexCanvasText"));

    str->font = renderBackend.CacheFont(font);

    str->link.Clear();
    str->mainLink.Clear();
    str->mainLink.Add(objects);

    for(int i = 0; i < 4; i++) {
        str->rgba[i * 4 + 0] = 0xff;
        str->rgba[i * 4 + 1] = 0xff;
        str->rgba[i * 4 + 2] = 0xff;
        str->rgba[i * 4 + 3] = 0xff;
    }
    
    str->refCanvas = this;

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
    
    if(object->scriptRef <= 0) {
        delete object;
    }
}

//
// kexCanvas::Empty
//

void kexCanvas::Empty(void) {
    kexCanvasObject *next;
    
    for(kexCanvasObject *obj = children.Next(); obj != NULL;) {
        next = obj->link.Next();
        RemoveChild(obj);
        obj = next;
    }
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
    kexMatrix matrix;
    float _alpha = this->alpha;

    dglPushMatrix();
    matrix.SetOrtho(0, (float)FIXED_WIDTH, (float)FIXED_HEIGHT, 0, -1, 1);
    dglLoadMatrixf(matrix.ToFloatPtr());

    matrix.Identity();

    for(kexCanvasObject *obj = children.Next(); obj != NULL; obj = obj->link.Next()) {
        obj->Draw(matrix, _alpha);
    }

    dglPopMatrix();
}

//
// RegisterCanvasObjectProperties
//

template<class type>
static void RegisterCanvasObject(const char *name) {
    kexScriptManager::RegisterRefObjectNoCount<type>(name);
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
    kexCanvasComponent::Init();
    kexScriptManager::RegisterDataObject<kexCanvas>("kCanvas");
    
    RegisterCanvasObject<kexCanvasImage>("kCanvasImage");
    RegisterCanvasObject<kexCanvasText>("kCanvasText");
    RegisterCanvasObject<kexContainer>("kCanvasContainer");
    RegisterCanvasObject<kexCanvasScriptObject>("kCanvasScriptObject");

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

    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasScriptObject",
        "ref @obj",
        asOFFSET(kexCanvasScriptObject, component.Handle()));
    
    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasScriptObject",
        "kCanvasContainer @container",
        asOFFSET(kexCanvasScriptObject, container));
    
    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasImage",
        "float width",
        asOFFSET(kexCanvasImage, width));
    
    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasImage",
        "float height",
        asOFFSET(kexCanvasImage, height));
    
    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasText",
        "kStr text",
        asOFFSET(kexCanvasText, text));
    
    scriptManager.Engine()->RegisterObjectProperty(
        "kCanvasText",
        "bool bCentered",
        asOFFSET(kexCanvasText, bCentered));

    scriptManager.Engine()->RegisterGlobalProperty("kCanvas Canvas", &renderBackend.Canvas());
}
