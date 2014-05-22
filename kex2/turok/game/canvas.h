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

#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "linkedlist.h"
#include "scriptAPI/component.h"

class kexCanvasComponent : public kexComponent {
public:
                                    kexCanvasComponent(void);
                                    ~kexCanvasComponent(void);
    
    virtual void                    Construct(const char *className);
    static void                     Init(void);
    
    asIScriptFunction               *onUpdate;
    asIScriptFunction               *onInit;
    asIScriptFunction               *onHover;
    asIScriptFunction               *onExit;
    asIScriptFunction               *onDown;
    asIScriptFunction               *onRelease;
};

//-----------------------------------------------------------------------------
//
// kexCanvasObject
//
//-----------------------------------------------------------------------------

class kexCanvas;

BEGIN_EXTENDED_CLASS(kexCanvasObject, kexObject);
    friend class kexCanvas;
    friend class kexContainer;
public:
                                    kexCanvasObject(void);
    virtual                         ~kexCanvasObject(void);

    virtual void                    Draw(void) = 0;

    static int                      objId;

    int                             IncRef(void);
    int                             DecRef(void);

    kexCanvasObject                 *parent;
    kexLinklist<kexCanvasObject>    link;
    kexLinklist<kexCanvasObject>    mainLink;

    float                           x;
    float                           y;
    float                           scaleX;
    float                           scaleY;
    float                           regX;
    float                           regY;
    float                           rotation;
    float                           alpha;
    bool                            bVisible;
    float                           min[2];
    float                           max[2];

protected:
    int                             scriptRef;
    int                             curId;
    kexCanvas                       *refCanvas;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexCanvasImage
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexCanvasImage, kexCanvasObject);
public:
                                    kexCanvasImage(void);
                                    ~kexCanvasImage(void);

    virtual void                    Draw(void);

    void                            SetRGB(const int index, const byte r, const byte g, const byte b);

    byte                            rgba[32];
    kexTexture                      *texture;
    float                           width;
    float                           height;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexCanvasText
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexCanvasText, kexCanvasObject);
public:
                                    kexCanvasText(void);
                                    ~kexCanvasText(void);

    virtual void                    Draw(void);

    void                            SetRGB(const int index, const byte r, const byte g, const byte b);

    byte                            rgba[32];
    kexStr                          text;
    kexFont                         *font;
    bool                            bCentered;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexContainer
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexContainer, kexCanvasObject);
public:
                                    kexContainer(void);
                                    ~kexContainer(void);

    virtual void                    Draw(void);

    void                            AddChild(kexCanvasObject *object);
    void                            RemoveChild(kexCanvasObject *object);
    void                            Empty(void);

    kexCanvasObject*                operator[](int index);

    kexLinklist<kexCanvasObject>    children;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexCanvasScriptObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexCanvasScriptObject, kexCanvasObject);
public:
                                    kexCanvasScriptObject(void);
                                    ~kexCanvasScriptObject(void);

    virtual void                    Draw(void);
    void                            SetProperty(const char *name, const char *value);

    kexCanvasComponent              component;
    kexContainer                    *container;
END_CLASS();

//-----------------------------------------------------------------------------
//
// kexCanvas
//
//-----------------------------------------------------------------------------

class kexCanvas {
public:
                                    kexCanvas(void);
                                    ~kexCanvas(void);

    kexCanvasImage                  *CreateImage(const char *texture);
    kexCanvasImage                  *CreateImage(const kexStr &texture);
    kexContainer                    *CreateContainer(void);
    kexCanvasScriptObject           *CreateScriptObject(const char *className);
    kexCanvasText                   *CreateText(const char *font);
    kexCanvasText                   *CreateText(const kexStr &font);

    void                            AddChild(kexCanvasObject *object);
    void                            RemoveChild(kexCanvasObject *object);
    void                            Empty(void);

    kexCanvasObject*                operator[](int index);

    void                            Draw(void);

    static void                     InitObject(void);

    kexLinklist<kexCanvasObject>    children;
    kexLinklist<kexCanvasObject>    objects;
    float                           alpha;
};

#endif
