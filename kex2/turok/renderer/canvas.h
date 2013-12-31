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

//-----------------------------------------------------------------------------
//
// kexCanvasObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexCanvasObject, kexObject);
public:
                                    kexCanvasObject(void);
                                    ~kexCanvasObject(void);

    virtual void                    Draw(void) = 0;

    void                            IncRef(void);
    void                            DecRef(void);

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

private:
    int                             refCount;
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

    kexCanvasObject*                operator[](int index);

    kexLinklist<kexCanvasObject>    children;
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
    kexCanvasText                   *CreateText(const char *font);
    kexCanvasText                   *CreateText(const kexStr &font);

    void                            AddChild(kexCanvasObject *object);
    void                            RemoveChild(kexCanvasObject *object);

    kexCanvasObject*                operator[](int index);

    void                            Draw(void);

    static void                     InitObject(void);
    static kexHeapBlock             hb_canvas;

    kexLinklist<kexCanvasObject>    children;
    kexLinklist<kexCanvasObject>    objects;
};

#endif
