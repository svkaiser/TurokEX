//-----------------------------------------------------------------------------
//
// TurokHud.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Hud.js');
Sys.dependsOn('scripts/framework/FontHud.js');

TurokHud = class.extends(Hud, function()
{
    this.viewport = Render.viewport;
});

class.properties(TurokHud,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    opacity     : 192,
    bFlash      : false,
    flashAlpha  : 128,
    flash_r     : 0,
    flash_g     : 44,
    flash_b     : 148,
    viewport    : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    flash : function()
    {
        this.bFlash = true;
        this.flashAlpha = 128;
        
        if(arguments.length == 3)
        {
            this.flash_r = arguments[0];
            this.flash_g = arguments[1];
            this.flash_b = arguments[2];
        }
        else
        {
            this.flash_r = 0;
            this.flash_g = 44;
            this.flash_b = 148;
        }
    },
    onTick : function() { },
    onDraw : function()
    {
        var controller = Client.localPlayer.controller;
        var player = controller.owner.components.ComponentTurokPlayer;
        
        GL.setBlend(1);
        
        // flash overlay
        if(this.bFlash == true)
        {
            this.canvas.setDrawColor(this.flash_r, this.flash_g, this.flash_b);
            this.canvas.setDrawAlpha(this.flashAlpha);
            this.canvas.drawTile('textures/white.tga', 0, 0,
                this.viewport.width, this.viewport.height);
                
            this.flashAlpha -= 8;
            if(this.flashAlpha <= 0)
            {
                this.bFlash = false;
                this.flashAlpha = 128;
            }
        }
        
        // underwater overlay
        if(controller.waterlevel == WL_UNDER)
        {
            this.canvas.setDrawColor(0, 18, 95);
            this.canvas.setDrawAlpha(160);
            this.canvas.drawTile('textures/white.tga', 0, 0,
                this.viewport.width, this.viewport.height);
        }
        
        this.canvas.setFont(FontHud);
        this.canvas.setDrawColor(255, 255, 255);
        this.canvas.setDrawAlpha(this.opacity);
        this.canvas.setTextureTile(0, 1, 0, 1);
        this.canvas.setDrawScale(0.5);
        
        // icons
        this.canvas.drawFixedTile('hud/h_plaque1.tga', 32.95, 210);
        this.canvas.drawFixedTile('hud/h_health.tga', 12, 210);
        this.canvas.drawFixedTile('hud/h_turok.tga', 10, 10);
        this.canvas.drawFixedTile('hud/h_coin.tga', 286, 10);
        
        // numbers
        this.canvas.setDrawScale(0.325);
        this.canvas.drawFixedString(player.lives, 10 + 30, 10 + 32);
        
        var lf_x = 286 - 17;
        
        if(player.lifeForces < 10)
            lf_x += 9;
            
        this.canvas.drawFixedString(player.lifeForces, lf_x, 10 + 8);
        this.canvas.setDrawScale(1);
        
        GL.setBlend(0);
    },
    start : function()
    {
        Hud.prototype.start.bind(this)();
    }
});
