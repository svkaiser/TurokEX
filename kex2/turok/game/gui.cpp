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
// DESCRIPTION: GUI system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "memHeap.h"
#include "fileSystem.h"
#include "renderBackend.h"
#include "renderMain.h"
#include "renderUtils.h"
#include "canvas.h"
#include "scriptAPI/scriptSystem.h"
#include "gui.h"
#include "tinyxml2.h"

using namespace tinyxml2;

kexGuiManager guiManager;

//
// FCmd_DebugGuiButtons
//

static void FCmd_DebugGuiButtons(void) {
    if(command.GetArgc() < 1) {
        return;
    }
    
    guiManager.bDebugButtons ^= 1;
}

//
// kexGuiManager::kexGuiManager
//

kexGuiManager::kexGuiManager(void) {
    this->guis.Clear();
    this->cursor_x = 0;
    this->cursor_y = 0;
    this->bEnabled = false;
    this->bDebugButtons = false;
    this->mainGui = NULL;
}

//
// kexGuiManager::~kexGuiManager
//

kexGuiManager::~kexGuiManager(void) {
}

//
// kexGuiManager::Init
//

void kexGuiManager::Init(void) {
    cursorMaterial = renderBackend.CacheMaterial("gui/cursor.kmat@cursor");
    command.Add("debugguibuttons", FCmd_DebugGuiButtons);
    
    kexStrList list;
    fileSystem.GetMatchingFiles(list, "gui/");
    
    for(unsigned int i = 0; i < list.Length(); i++) {
        if(list[i].IndexOf(".xml") == -1) {
            continue;
        }
        
        LoadGui(kexStr("gui/") + list[i]);
    }
    
    list.Empty();
}

//
// kexGuiManager::DeleteGuis
//

void kexGuiManager::DeleteGuis(void) {
    kexGui *next;
    
    for(kexGui *obj = guis.Next(); obj; obj = next) {
        next = obj->link.Next();
        delete obj;
        obj = next;
    }
}

//
// kexGuiManager::LoadGui
//

kexGui *kexGuiManager::LoadGui(const char *guiFile) {
    int buffsize;
    byte *buffer;
    kexGui *gui;
    
    if(guiFile == NULL || guiFile[0] == 0) {
        return NULL;
    }
    
    buffsize = fileSystem.OpenFile(guiFile, (byte**)(&buffer), hb_static);
    
    if(buffsize <= 0) {
        common.Warning("kexGuiManager::LoadGui: %s not found\n", guiFile);
        return NULL;
    }
    
    // force namespace so msvc can shut up
    tinyxml2::XMLDocument xmldoc;
    xmldoc.Parse((char*)buffer);
    
    Mem_Free(buffer);
    
    if(xmldoc.ErrorID() != XML_SUCCESS) {
        xmldoc.PrintError();
        return NULL;
    }
    
    XMLElement *root = xmldoc.RootElement();
    
    if(kexStr::Compare(root->Value(), "gui")) {
        common.Warning("kexGuiManager::LoadGui: found root element '%s', expected 'gui'\n",
                       root->Value());
        return NULL;
    }
    
    gui = new kexGui;
    gui->link.Clear();
    gui->link.Add(guis);
    
    for(const XMLAttribute *attrib = root->FirstAttribute(); attrib; attrib = attrib->Next()) {
        if(!kexStr::Compare(attrib->Name(), "name")) {
            gui->name = kexStr(attrib->Value());
        }
    }
    
    ParseNode(root, gui, NULL);
    return gui;
}

//
// kexGuiManager::ParseSimpleProperties
//

void kexGuiManager::ParseSimpleProperties(XMLNode *node, kexCanvasObject *object) {
    const XMLAttribute *attrib;
    
    for(attrib = node->ToElement()->FirstAttribute(); attrib; attrib = attrib->Next()) {
        if(!kexStr::Compare(attrib->Name(), "scaleX")) {
            object->scaleX = kexStr(attrib->Value()).Atof();
        }
        else if(!kexStr::Compare(attrib->Name(), "scaleY")) {
            object->scaleY = kexStr(attrib->Value()).Atof();
        }
        else if(!kexStr::Compare(attrib->Name(), "x")) {
            object->x = kexStr(attrib->Value()).Atof();
        }
        else if(!kexStr::Compare(attrib->Name(), "y")) {
            object->y = kexStr(attrib->Value()).Atof();
        }
        else if(!kexStr::Compare(attrib->Name(), "visible")) {
            object->bVisible = !kexStr::CompareCase(attrib->Value(), "true");
        }
        else if(!kexStr::Compare(attrib->Name(), "opacity")) {
            object->alpha = kexStr(attrib->Value()).Atof();
        }
    }
}

//
// kexGuiManager::ParseNode
//

void kexGuiManager::ParseNode(XMLElement *element, kexGui *gui, kexContainer *container) {
    for(XMLNode *node = element->FirstChild(); node; node = node->NextSibling()) {
        if(!kexStr::Compare(node->Value(), "container")) {
            kexContainer *canvasContainer = gui->canvas.CreateContainer();
            
            if(container == NULL) {
                gui->canvas.AddChild(canvasContainer);
            }
            else {
                container->AddChild(canvasContainer);
            }
            
            ParseSimpleProperties(node, canvasContainer);
            ParseNode(node->ToElement(), gui, canvasContainer);
        }
        else if(!kexStr::Compare(node->Value(), "image")) {
            const XMLAttribute *attrib = node->ToElement()->FirstAttribute();
            
            if(!kexStr::Compare(attrib->Name(), "texture")) {
                kexCanvasImage *canvasImage = gui->canvas.CreateImage(attrib->Value());
                
                if(container == NULL) {
                    gui->canvas.AddChild(canvasImage);
                }
                else {
                    container->AddChild(canvasImage);
                }
                
                ParseSimpleProperties(node, canvasImage);
            }
        }
        else if(!kexStr::Compare(node->Value(), "text")) {
            const XMLAttribute *attrib = node->ToElement()->FirstAttribute();
            
            if(!kexStr::Compare(attrib->Name(), "font")) {
                kexCanvasText *canvasText = gui->canvas.CreateText(attrib->Value());
                
                if(container == NULL) {
                    gui->canvas.AddChild(canvasText);
                }
                else {
                    container->AddChild(canvasText);
                }
                
                ParseSimpleProperties(node, canvasText);

                for(XMLElement *child = node->FirstChildElement(); child;
                    child = child->NextSiblingElement()) {
                    if(!kexStr::Compare(child->Value(), "string")) {
                        canvasText->text = kexStr(child->GetText());
                    }
                }
            }
        }
        else if(!kexStr::Compare(node->Value(), "object")) {
            const XMLAttribute *attrib = node->ToElement()->FirstAttribute();
            
            if(!kexStr::Compare(attrib->Name(), "class")) {
                kexCanvasScriptObject *cso = gui->canvas.CreateScriptObject(attrib->Value());
                
                if(cso == NULL) {
                    common.Warning("kexGuiManager::ParseNode: script class %s not found\n",
                                   attrib->Value());
                    continue;
                }
                
                cso->container = gui->canvas.CreateContainer();
                
                // we add the scripted canvas object's container to the parent container
                // because we want that container to easily inherit the parent's transform.
                // the actual scripted canvas object will be added to the root instead
                if(container) {
                    container->AddChild(cso->container);
                }
                else {
                    gui->canvas.AddChild(cso->container);
                }
                
                // scripted canvas objects are always added to the root gui
                gui->canvas.AddChild(cso);
                
                ParseSimpleProperties(node, cso);
                
                for(XMLElement *child = node->FirstChildElement(); child;
                    child = child->NextSiblingElement()) {
                    if(!kexStr::Compare(child->Value(), "properties")) {
                        for(XMLElement *pnode = child->FirstChildElement(); pnode;
                            pnode = pnode->NextSiblingElement()) {
                            cso->SetProperty(pnode->Value(), pnode->GetText());
                        }
                        
                        cso->component.CallFunction(cso->component.onInit);
                    }
                    else if(!kexStr::Compare(child->Value(), "container")) {
                        ParseSimpleProperties(child, cso->container);
                        ParseNode(child, gui, cso->container);
                    }
                }
            }
        }
        else if(!kexStr::Compare(node->Value(), "button")) {
            ParseButton(node, gui, container);
        }
        else if(!kexStr::Compare(node->Value(), "slider")) {
            ParseSlider(node, gui, container);
        }
    }
}

//
// kexGuiManager::ParseButton
//

void kexGuiManager::ParseButton(XMLNode *node, kexGui *gui, kexContainer *container) {
    const XMLAttribute *attrib = node->ToElement()->FirstAttribute();
    
    if(!kexStr::Compare(attrib->Name(), "name")) {
        guiButton_t *button = new guiButton_t;
        
        button->name = kexStr(attrib->Value());
        button->state = GBS_NONE;
        button->link.SetData(button);
        button->container = gui->canvas.CreateContainer();
        
        Mem_CacheRef((void**)&button->container);
        
        if(container) {
            container->AddChild(button->container);
        }
        else {
            gui->canvas.AddChild(button->container);
        }
        
        button->link.Add(gui->buttons);
        
        ParseSimpleProperties(node, button->container);
        ParseButtonEvent(node, gui, button->container, button);
        ParseNode(node->ToElement(), gui, button->container);
    }
}

//
// kexGuiManager::ParseSlider
//

void kexGuiManager::ParseSlider(XMLNode *node, kexGui *gui, kexContainer *container) {
    const XMLAttribute *attrib = node->ToElement()->FirstAttribute();
    
    if(!kexStr::Compare(attrib->Name(), "name")) {
        guiSlider_t *slider = new guiSlider_t;
        
        slider->name = kexStr(attrib->Value());
        slider->state = GBS_NONE;
        slider->link.SetData(slider);
        slider->container = gui->canvas.CreateContainer();
        slider->barImage = NULL;
        slider->sliderImage = NULL;
        slider->barWidth = 0;
        slider->position = 0;
        slider->bGrabbed = false;
        
        Mem_CacheRef((void**)&slider->container);
        
        if(container) {
            container->AddChild(slider->container);
        }
        else {
            gui->canvas.AddChild(slider->container);
        }
        
        slider->link.Add(gui->sliders);
        
        ParseSimpleProperties(node, slider->container);
        
        for(XMLElement *child = node->FirstChildElement(); child; child = child->NextSiblingElement()) {
            if(!kexStr::Compare(child->Value(), "properties")) {
                for(XMLElement *pnode = child->FirstChildElement(); pnode;
                    pnode = pnode->NextSiblingElement()) {
                    if(!kexStr::Compare(pnode->Value(), "barimage")) {
                        slider->barImage = gui->canvas.CreateImage(pnode->ToElement()->GetText());
                        
                        if(slider->barImage) {
                            slider->container->AddChild(slider->barImage);
                        }
                    }
                    else if(!kexStr::Compare(pnode->Value(), "sliderimage")) {
                        slider->sliderImage = gui->canvas.CreateImage(pnode->ToElement()->GetText());
                        
                        if(slider->sliderImage) {
                            slider->container->AddChild(slider->sliderImage);
                        }
                    }
                }
            }
        }
        
        if(slider->barImage && slider->sliderImage) {
            slider->barImage->regX = (float)slider->barImage->texture->Width() * 0.5f;
            slider->barImage->regY = (float)slider->barImage->texture->Height() * 0.5f;
            slider->sliderImage->regX = (float)slider->sliderImage->texture->Width() * 0.5f;
            slider->sliderImage->regY = (float)slider->sliderImage->texture->Height() * 0.5f;
            
            slider->barWidth = (float)slider->barImage->texture->Width();
            slider->position = slider->barImage->regX;
        }
        
        ParseNode(node->ToElement(), gui, slider->container);
    }
}

//
// kexGuiManager::ParseButtonEvent
//

void kexGuiManager::ParseButtonEvent(XMLNode *node, kexGui *gui,
                                     kexContainer *container, guiButton_t *button) {
    for(XMLNode *child = node->ToElement()->FirstChild(); child; child = child->NextSibling()) {
        if(!kexStr::Compare(child->Value(), "event")) {
            guiEvent_t guiEvent;
            
            guiEvent.event = GEVT_NONE;
            guiEvent.action = GAT_NONE;
            
            for(const XMLAttribute *attrib = child->ToElement()->FirstAttribute();
                attrib; attrib = attrib->Next()) {
                if(!kexStr::Compare(attrib->Name(), "type")) {
                    if(!kexStr::Compare(attrib->Value(), "ondown")) {
                        guiEvent.event = GEVT_ONDOWN;
                    }
                    else if(!kexStr::Compare(attrib->Value(), "onrelease")) {
                        guiEvent.event = GEVT_ONRELEASE;
                    }
                }
                else if(!kexStr::Compare(attrib->Name(), "action")) {
                    if(!kexStr::Compare(attrib->Value(), "changegui")) {
                        guiEvent.action = GAT_CHANGEGUI;
                    }
                    else if(!kexStr::Compare(attrib->Value(), "callcommand")) {
                        guiEvent.action = GAT_CALLCOMMAND;
                    }
                }
            }
            
            button->events.Push(guiEvent);
            
            for(XMLNode *evChild = child->ToElement()->FirstChild(); evChild;
                evChild = evChild->NextSibling()) {
                // need to set args from within the array rather than from the
                // temporarily variable
                guiEvent_t *refEvent = &button->events[button->events.Length()-1];
                refEvent->args.Add(evChild->Value(), evChild->ToElement()->GetText());
            }
        }
    }
}

//
// kexGuiManager::FindGuiByName
//

kexGui *kexGuiManager::FindGuiByName(const char *name) {
    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        if(!kexStr::Compare(obj->name.c_str(), name)) {
            return obj;
        }
    }
    
    return NULL;
}

//
// kexGuiManager::ClearGuis
//

void kexGuiManager::ClearGuis(const float fadeSpeed) {
    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        if(obj->status == GUIS_DISABLED) {
            continue;
        }

        obj->FadeOut(fadeSpeed);
    }
}

//
// kexGuiManager::UpdateGuis
//

void kexGuiManager::UpdateGuis(void) {
    bool bAllDisabled = true;

    if(bEnabled == false) {
        return;
    }

    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        if(obj->status == GUIS_DISABLED) {
            continue;
        }
        else if(obj->status == GUIS_FADEIN) {
            obj->canvas.alpha += (obj->fadeSpeed * client.GetRunTime());
            kexMath::Clamp(obj->canvas.alpha, 0, 1);

            if(obj->canvas.alpha >= 1) {
                obj->status = GUIS_READY;
            }
        }
        else if(obj->status == GUIS_FADEOUT) {
            obj->canvas.alpha -= (obj->fadeSpeed * client.GetRunTime());
            kexMath::Clamp(obj->canvas.alpha, 0, 1);

            if(obj->canvas.alpha <= 0) {
                obj->status = GUIS_DISABLED;
            }
        }

        bAllDisabled = false;
    }

    if(bAllDisabled) {
        bEnabled = false;
    }
}

//
// kexGuiManager::DrawGuis
//

void kexGuiManager::DrawGuis(void) {
    if(bEnabled == false) {
        return;
    }

    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        if(obj->status == GUIS_DISABLED) {
            continue;
        }

        obj->Draw();
        
        if(bDebugButtons) {
            guiButton_t *button;
            guiSlider_t *slider;
            kexBBox box;
            float w, h;

            w = (float)sysMain.VideoWidth() / (float)FIXED_WIDTH;
            h = (float)sysMain.VideoHeight() / (float)FIXED_HEIGHT;

            for(slider = obj->sliders.Next(); slider; slider = slider->link.Next()) {
                box.min.Set(slider->sliderImage->min[0], slider->sliderImage->min[1], 0);
                box.max.Set(slider->sliderImage->max[0], slider->sliderImage->max[1], 0);

                box.min[0] *= w; box.min[1] *= h;
                box.max[0] *= w; box.max[1] *= h;
                
                kexRenderUtils::DrawBoundingBox(box, 255, 0, 255);
            }
            
            for(button = obj->buttons.Next(); button; button = button->link.Next()) {
                box.min.Set(button->container->min[0], button->container->min[1], 0);
                box.max.Set(button->container->max[0], button->container->max[1], 0);

                box.min[0] *= w; box.min[1] *= h;
                box.max[0] *= w; box.max[1] *= h;
                
                kexRenderUtils::DrawBoundingBox(box, 255, 0, 255);
            }
        }
    }
    
    DrawCursor();
}

//
// kexGuiManager::ProcessInput
//

extern kexCvar cvarMSensitivityX;
extern kexCvar cvarMSensitivityY;
    
bool kexGuiManager::ProcessInput(const event_t *ev) {
    bool ok = false;
    
    if(bEnabled == false) {
        return false;
    }
    
    if(ev->type == ev_mouse) {
        float t = (float)sysMain.VideoWidth() / 32.0f;
        
        cursor_x += ((float)ev->data2 * cvarMSensitivityX.GetFloat()) / t;
        cursor_y -= ((float)ev->data3 * cvarMSensitivityX.GetFloat()) / t;
        kexMath::Clamp(cursor_x, 0.0f, (float)sysMain.VideoWidth());
        kexMath::Clamp(cursor_y, 0.0f, (float)sysMain.VideoHeight());
        
        ok = true;
    }
    
    for(kexGui *gui = guis.Next(); gui; gui = gui->link.Next()) {
        if(gui->status != GUIS_READY) {
            continue;
        }
        ok |= gui->CheckEvents(ev);
    }
    
    return ok;
}

//
// kexGuiManager::DrawCursor
//

void kexGuiManager::DrawCursor(void) {
    if(bEnabled == false) {
        return;
    }
    
    renderer.AddVertex(cursor_x, cursor_y, 0, 0, 0, 255, 255, 255, 255);
    renderer.AddVertex(32+ cursor_x, cursor_y, 0, 1, 0, 255, 255, 255, 255);
    renderer.AddVertex(cursor_x, 32 + cursor_y, 0, 0, 1, 255, 255, 255, 255);
    renderer.AddVertex(32 + cursor_x, 32 + cursor_y, 0, 1, 1, 255, 255, 255, 255);
    renderer.AddTriangle(0, 1, 2);
    renderer.AddTriangle(2, 1, 3);
    renderer.DrawElements(cursorMaterial);
    
    renderBackend.DisableShaders();
}

//
// kexGui::kexGui
//

kexGui::kexGui(void) {
    this->status = GUIS_DISABLED;
    this->fadeSpeed = 1.0f;
    this->canvas.alpha = 0;
    this->name = (kexStr("gui_") + sysMain.GetMS()).c_str();
    this->link.SetData(this);
}

//
// kexGui::~kexGui
//

kexGui::~kexGui(void) {
    guiButton_t *next;
    
    this->link.Remove();
    canvas.Empty();
    
    for(guiButton_t *button = buttons.Next(); button;) {
        next = button->link.Next();
        button->link.Remove();
        button->events.Empty();
        delete button;
        button = next;
    }
}

//
// kexGui::Draw
//

void kexGui::Draw(void) {
    canvas.Draw();
}

//
// kexGui::FadeIn
//

void kexGui::FadeIn(const float speed) {
    status = GUIS_FADEIN;
    fadeSpeed = speed;
}

//
// kexGui::FadeOut
//

void kexGui::FadeOut(const float speed) {
    guiButton_t *button;
    
    status = GUIS_FADEOUT;
    fadeSpeed = speed;
    
    for(button = buttons.Next(); button; button = button->link.Next()) {
        if(button->state == GBS_DISABLED) {
            continue;
        }
        
        ExecuteButtonEvent(button, GBS_RELEASED);
        ExecuteButtonEvent(button, GBS_EXITED);
    }
}

//
// kexGui::ExecuteEvent
//

void kexGui::ExecuteEvent(guiEvent_t *event) {
    switch(event->action) {
        case GAT_CHANGEGUI:
            ChangeGuis(event);
            break;
            
        case GAT_CALLCOMMAND:
            CallCommand(event);
            break;
            
        default:
            break;
    }
}

//
// kexGui::ChangeGuis
//

void kexGui::ChangeGuis(guiEvent_t *guiEvent) {
    kexStr guiName;
    kexGui *nextGui;
    float speed;
    
    guiEvent->args.GetString("gui", guiName);
    guiEvent->args.GetFloat("fadeSpeed", speed);
    
    nextGui = guiManager.FindGuiByName(guiName.c_str());
    
    if(nextGui == NULL) {
        return;
    }
    
    FadeOut(speed);
    nextGui->FadeIn(speed);
}

//
// kexGui::CallCommand
//

void kexGui::CallCommand(guiEvent_t *event) {
    kexStr cmd;
    
    event->args.GetString("command", cmd);
    command.Execute(cmd.c_str());
}

//
// kexGui::ExecuteButtonEvent
//

void kexGui::ExecuteButtonEvent(guiButton_t *button, const guiButtonState_t btnState) {
    if(button->state == btnState) {
        return;
    }
    
    for(kexCanvasObject *obj = canvas.children.Next(); obj; obj = obj->link.Next()) {
        if(!obj->InstanceOf(&kexCanvasScriptObject::info)) {
            continue;
        }
        
        kexCanvasScriptObject *cso = static_cast<kexCanvasScriptObject*>(obj);
        if(cso->container->parent != button->container) {
            continue;
        }

        switch(btnState) {
            case GBS_HOVER:
                if(cso->component.onHover) {
                    cso->component.CallFunction(cso->component.onHover);
                }
                break;
                
            case GBS_DOWN:
                if(cso->component.onDown) {
                    cso->component.CallFunction(cso->component.onDown);
                }
                break;
                
            case GBS_RELEASED:
                if(cso->component.onRelease) {
                    cso->component.CallFunction(cso->component.onRelease);
                }
                break;

            case GBS_EXITED:
                if(cso->component.onExit) {
                    cso->component.CallFunction(cso->component.onExit);
                }
                
                if(button->state == GBS_DOWN) {
                    if(cso->component.onRelease) {
                        cso->component.CallFunction(cso->component.onRelease);
                    }
                }
                break;

            default:
                break;
        }
    }
    
    for(unsigned int i = 0; i < button->events.Length(); i++) {
        if(btnState == GBS_DOWN) {
            if(button->events[i].event != GEVT_ONDOWN) {
                continue;
            }
            
            ExecuteEvent(&button->events[i]);
        }
    }

    button->state = btnState;
}

//
// kexGui::CheckEvents
//

bool kexGui::CheckEvents(const event_t *ev) {
    float x = guiManager.cursor_x;
    float y = guiManager.cursor_y;
    guiButton_t *button;
    guiSlider_t *slider;
    float w;
    float h;
    bool ok = false;

    w = (float)sysMain.VideoWidth() / (float)FIXED_WIDTH;
    h = (float)sysMain.VideoHeight() / (float)FIXED_HEIGHT;
    
    for(slider = sliders.Next(); slider; slider = slider->link.Next()) {
        if(x > slider->sliderImage->min[0] * w && x < slider->sliderImage->max[0] * w &&
           y > slider->sliderImage->min[1] * h && y < slider->sliderImage->max[1] * h) {
            if(ev->type == ev_mousedown) {
                slider->bGrabbed = true;
            }
            else if(ev->type == ev_mouseup) {
                slider->bGrabbed = false;
            }
        }
        else {
            slider->bGrabbed = false;
        }
    }
    
    for(button = buttons.Next(); button; button = button->link.Next()) {
        if(button->state == GBS_DISABLED) {
            continue;
        }
        
        if(x > button->container->min[0] * w && x < button->container->max[0] * w &&
           y > button->container->min[1] * h && y < button->container->max[1] * h) {
            if(ev->type == ev_mousedown && button->state != GBS_DOWN) {
                ExecuteButtonEvent(button, GBS_DOWN);
                ok = true;
            }
            else if(ev->type == ev_mouseup && button->state == GBS_DOWN) {
                ExecuteButtonEvent(button, GBS_RELEASED);
                ok = true;
            }
            else if(button->state != GBS_HOVER && button->state != GBS_DOWN) {
                ExecuteButtonEvent(button, GBS_HOVER);
                ok = true;
            }
        }
        else {
            if(button->state != GBS_EXITED) {
                ExecuteButtonEvent(button, GBS_EXITED);
            }
        }
    }
    
    return ok;
}
