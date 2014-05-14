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
#include "canvas.h"
#include "scriptAPI/scriptSystem.h"
#include "gui.h"
#include "tinyxml2.h"

using namespace tinyxml2;

kexGuiManager guiManager;

//
// kexGuiManager::kexGuiManager
//

kexGuiManager::kexGuiManager(void) {
    this->guis.Clear();
}

//
// kexGuiManager::~kexGuiManager
//

kexGuiManager::~kexGuiManager(void) {
    kexGui *next;
    
    for(kexGui *obj = guis.Next(); obj != NULL;) {
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
                
                if(container == NULL) {
                    gui->canvas.AddChild(cso);
                }
                else {
                    container->AddChild(cso);
                }
                
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
                        ParseSimpleProperties(child, &cso->container);
                        ParseNode(child, gui, &cso->container);
                    }
                }
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
    for(kexGui *obj = guis.Next(); obj; obj = obj->link.Next()) {
        obj->Draw();
    }
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
    this->link.Remove();
    canvas.Empty();
}

//
// kexGui::Draw
//

void kexGui::Draw(void) {
    canvas.Draw();
}
