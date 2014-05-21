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
    
    buffsize = fileSystem.ReadExternalTextFile(guiFile, (byte**)(&buffer));
    if(buffsize <= 0) {
        buffsize = fileSystem.OpenFile(guiFile, (byte**)(&buffer), hb_static);
    }
    
    if(buffsize <= 0) {
        common.Warning("kexGuiManager::LoadGui: %s not found\n", guiFile);
        return NULL;
    }
    
    XMLDocument xmldoc;
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
        if(kexStr::Compare(attrib->Name(), "name")) {
            gui->name = kexStr(attrib->Value());
        }
        else if(kexStr::Compare(attrib->Name(), "prevGUI")) {
            gui->prevGui = FindGuiByName(attrib->Value());
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
                ParseNode(node->ToElement(), gui, button->container);
            }
        }
    }
}

//
// kexGuiManager::FindGuiByName
//

kexGui *kexGuiManager::FindGuiByName(const char *name) {
    kexStr compareName(name);
    
    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        if(obj->name == compareName) {
            return obj;
        }
    }
    
    return NULL;
}

//
// kexGuiManager::DrawGuis
//

void kexGuiManager::DrawGuis(void) {
    if(bEnabled == false) {
        return;
    }
    
    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        obj->Draw();
        
        if(bDebugButtons) {
            guiButton_t *button;
            kexBBox box;
            
            for(button = obj->buttons.Next(); button; button = button->link.Next()) {
                box.min.Set(button->container->min[0], button->container->min[1], 0);
                box.max.Set(button->container->max[0], button->container->max[1], 0);
                
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
    if(bEnabled == false) {
        return false;
    }
    
    if(ev->type == ev_mouse) {
        cursor_x += ((float)ev->data2 * cvarMSensitivityX.GetFloat()) / 32.0f;
        cursor_y -= ((float)ev->data3 * cvarMSensitivityX.GetFloat()) / 32.0f;
        kexMath::Clamp(cursor_x, 0.0f, (float)sysMain.VideoWidth());
        kexMath::Clamp(cursor_y, 0.0f, (float)sysMain.VideoHeight());
        
        for(kexGui *gui = guis.Next(); gui; gui = gui->link.Next()) {
            if(gui->status == GUIS_DISABLED) {
                continue;
            }
            gui->CheckEvents();
        }
        return true;
    }
    
    return false;
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
    this->prevGui = NULL;
    this->status = GUIS_READY;
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
// kexGui::ExecuteButtonEvent
//

void kexGui::ExecuteButtonEvent(guiButton_t *button, const guiButtonState_t btnState) {

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

            case GBS_NONE:
                if(button->state == GBS_HOVER) {
                    if(cso->component.onExit) {
                        cso->component.CallFunction(cso->component.onExit);
                    }
                }
                break;

            default:
                break;
        }
    }

    button->state = btnState;
}

//
// kexGui::CheckEvents
//

void kexGui::CheckEvents(void) {
    float x = guiManager.cursor_x;
    float y = guiManager.cursor_y;
    guiButton_t *button;
    
    for(button = buttons.Next(); button; button = button->link.Next()) {
        if(button->state == GBS_DISABLED) {
            continue;
        }
        
        if(x > button->container->min[0] && x < button->container->max[0] &&
           y > button->container->min[1] && y < button->container->max[1]) {
            if(button->state != GBS_HOVER) {
                ExecuteButtonEvent(button, GBS_HOVER);
            }
        }
        else {
            if(button->state != GBS_NONE) {
                ExecuteButtonEvent(button, GBS_NONE);
            }
        }
    }
}
