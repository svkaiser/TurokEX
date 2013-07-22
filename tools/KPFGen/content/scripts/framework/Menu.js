//-----------------------------------------------------------------------------
//
// Menu.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const MENU_STATUS_READY         = 0;
const MENU_STATUS_FADEIN        = 1;
const MENU_STATUS_FADEOUT       = 2;

const MENU_STATUS_ITEM_HIDDEN   = 0;
const MENU_STATUS_ITEM_ENTEROK  = 1;
const MENU_STATUS_ITEM_DISABLED = 2;
const MENU_STATUS_ITEM_NOCURSOR = 3;
const MENU_STATUS_ITEM_OK       = 4;
const MENU_STATUS_ITEM_ARROWSOK = 5;
const MENU_STATUS_ITEM_SLIDER   = 6;

Menu = class.define();

class.properties(Menu,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    canvas          : null,
    items           : null,
    lastOn          : 0,
    numPageItems    : -1,
    pageOffset      : 0,
    hints           : null,
    thermoBars      : null,
    opacity         : 1.0,
    state           : MENU_STATUS_ITEM_OK,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    getActiveItemCount : function()
    {
        var numItems = 0;
        
        for(var i = 0; i < this.items.length; i++)
        {
            // ignore hidden items
            if(this.items[i].state == MENU_STATUS_ITEM_HIDDEN)
                continue;
                
            numItems++;
        }
        
        return numItems;
    },
    
    drawCenteredItems : function(canvas, y, spacing)
    {
        var row = y;
        
        TurokString.setStringColor(180, 180, 124, 77, 63, 42);
        canvas.setDrawScale(0.35);
        canvas.setDrawAlpha(Math.round(255.0 * this.opacity));
        
        for(var i = 0; i < this.items.length; i++)
        {
            // ignore hidden items
            if(this.items[i].state == MENU_STATUS_ITEM_HIDDEN)
                continue;
                
            TurokString.drawText(canvas, this.items[i].text, 160, row, true);
            row += spacing;
        }
    },
    
    draw : function(canvas)
    {
        var height = (15.0 * this.getActiveItemCount() + 8);
        var row = 178.0 - height / 2.0;
        var y = row;
        var alpha = Math.round(255.0 * ((this.opacity * 150.0) / 255.0));
        
        GL.setBlend(1);
        canvas.setDrawScale(1.0);
        this.drawFillBox(canvas, 70, y, 250, y + height, 2, false, 0, 0, 0, alpha);
        this.drawCenteredItems(canvas, y + 8.0, 15.0);
        GL.setBlend(0);
    },
    
    drawFillBox : function(canvas, x, y, w, h, borderSize, bTintSide, r, g, b, a)
    {
        var r1, r2;
        var g1, g2;
        var b1, b2;
        var ratiox, ratioy;
        var rx, ry, rw, rh;
        var viewport;
        
        viewport = NRender.viewport;
        ratiox = NRender.SCREEN_WIDTH / viewport.width;
        ratioy = NRender.SCREEN_HEIGHT / viewport.height;
        rx = x / ratiox;
        rw = (w-x) / ratiox;
        ry = y / ratioy;
        rh = (h-y) / ratioy;
        
        if(bTintSide)
        {
            r1 = r >> 1;
            g1 = g >> 1;
            b1 = b >> 1;
            r2 = r << 1;
            g2 = g << 1;
            b2 = b << 1;
        }
        else
        {
            r1 = r << 1;
            g1 = g << 1;
            b1 = b << 1;
            r2 = r >> 1;
            g2 = g >> 1;
            b2 = b >> 1;
        }
        
        // body
        canvas.setDrawColor(r, g, b);
        canvas.setDrawAlpha(a);
        canvas.drawTile('textures/white.tga', rx, ry, rw, rh);
        // border 1
        canvas.setDrawColor(r2, g2, b2);
        canvas.drawTile('textures/white.tga', rx, ry, rw, borderSize);
        // border 2
        canvas.setDrawColor(r1, g1, b1);
        canvas.drawTile('textures/white.tga', (rx+rw)-borderSize, ry, borderSize, rh);
        // border 3
        canvas.setDrawColor(r1, g1, b1);
        canvas.drawTile('textures/white.tga', rx, (ry+rh)-borderSize, rw, borderSize);
        // border 4
        canvas.setDrawColor(r2, g2, b2);
        canvas.drawTile('textures/white.tga', rx, ry, borderSize, rh);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onTick : function() { },
    onDraw : function() { }
});

//-----------------------------------------------------------------------------
//
// MainMenu.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

MainMenu = class.extends(Menu);

class.properties(MainMenu,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    items : [
                { state : 0, text : "start game",   callBack : null },
                { state : 0, text : "load game",    callBack : null },
                { state : 0, text : "options",      callBack : null },
                { state : 0, text : "training",     callBack : null },
                { state : 0, text : "enter cheat",  callBack : null },
                { state : 0, text : "cheat menu",   callBack : null },
                { state : 0, text : "quit",         callBack : null },
            ],
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onTick : function() { },
    onDraw : function() { }
});

