//-----------------------------------------------------------------------------
//
// TurokHud.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Hud.js');
Sys.dependsOn('scripts/framework/FontHud.js');
Sys.dependsOn('scripts/turokgame/HudMessage.js');
Sys.dependsOn('scripts/turokgame/TurokString.js');

TurokHud = class.extends(Hud, function()
{
    this.viewport = Render.viewport;
    this.messages = new Array(8);
    
    for(var i = 0; i < 8; i++)
        this.messages[i] = new HudMessage();
});

class.properties(TurokHud,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    opacity     : 255,
    bFlash      : false,
    flashAlpha  : 128,
    flash_r     : 0,
    flash_g     : 44,
    flash_b     : 148,
    viewport    : null,
    messages    : null,
    strCounter  : 0,
    
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
        var controller = ClientPlayer.component.controller;
        var player = ClientPlayer.component;
        
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
        
        this.canvas.setDrawColor(255, 255, 255);
        this.canvas.setDrawAlpha(this.opacity);
        this.canvas.setTextureTile(0, 1, 0, 1);
        this.canvas.setDrawScale(0.5);
        
        // health
        this.canvas.drawFixedTile('hud/h_plaque1.tga', 32.95, 210);
        this.canvas.drawFixedTile('hud/h_health.tga', 12, 210);
        TurokString.drawNumber(this.canvas, player.health, 36, 215);
        
        // lives
        this.canvas.drawFixedTile('hud/h_turok.tga', 10, 10);
        TurokString.drawNumber(this.canvas, player.lives, 10 + 30, 10 + 32);
        
        // ammo
        if(player.activeWeaponID != WP_KNIFE)
        {
            var ammo_id = 0;
            var am_icon_x = 82;
            
            this.canvas.drawFixedTile('hud/h_plaque1.tga', 100, 210);
            
            switch(player.activeWeaponID)
            {
            case WP_BOW:
                ammo_id = AM_ARROWS;
                break;
                
            case WP_PISTOL:
            case WP_RIFLE:
            case WP_MINIGUN:
                am_icon_x = 86;
                ammo_id = AM_CLIPS;
                break;
                
            case WP_SHOTGUN:
            case WP_AUTOSHOTGUN:
                am_icon_x = 79;
                ammo_id = AM_SHELLS;
                break;
                
            case WP_PULSERIFLE:
            case WP_ALIENRIFLE:
                ammo_id = AM_CELLS;
                break;
                
            case WP_GRENADE_L:
                am_icon_x = 79;
                ammo_id = AM_GRENADES;
                break;
                
            case WP_ROCKET_L:
                am_icon_x = 82;
                ammo_id = AM_ROCKETS;
                break;
            }
            
            TurokString.drawNumber(this.canvas, player.ammo[ammo_id].amount, 102, 215);
            this.canvas.drawFixedTile(player.ammo[ammo_id].hudIcon, am_icon_x, 210);
        }
        
        // life forces
        this.canvas.drawFixedTile('hud/h_coin.tga', 286, 10);
        
        var lf_x = 286 - 17;
        
        if(player.lifeForces < 10)
            lf_x += 9;
            
        TurokString.drawNumber(this.canvas, player.lifeForces, lf_x, 10 + 8);
        
        // messages
        this.drawMessages();
        this.canvas.setDrawScale(1);
        
        // debug stats
        if(Sys.getCvar('g_displayplayerinfo'))
        {
            var pmove = ClientPlayer.worldState;
            
            this.canvas.setFont(FontArial);
            this.canvas.setDrawScale(1);
            this.canvas.setDrawColor(255, 255, 0);
            this.canvas.setDrawAlpha(255);
            this.canvas.drawString('---Player Info---', 32, 160, false);
            this.canvas.setDrawColor(0, 255, 0);
            this.canvas.drawString('origin: ' + pmove.origin.toString(), 32, 176, false);
            this.canvas.drawString('acceleration: ' + pmove.accel.toString(), 32, 192, false);
            this.canvas.drawString('velocity: ' + pmove.velocity.toString(), 32, 208, false);
            
            if(pmove.plane != null)
                this.canvas.drawString('plane: ' + pmove.plane.toIndex(), 32, 224, false);
            
            this.canvas.drawString('yaw: ' + pmove.yaw, 32, 240, false);
            this.canvas.drawString('pitch: ' + pmove.pitch, 32, 256, false);
            this.canvas.drawString('roll: ' + pmove.roll, 32, 272, false);
        }
        
        GL.setBlend(0);
    },
    
    notify : function(text)
    {
        this.messages[this.strCounter].addText(text);
        this.strCounter = (this.strCounter + 1) & 7;
    },
    
    drawMessages : function()
    {
        TurokString.setStringColor(180, 180, 124, 77, 63, 42);
        
        var cnt = this.strCounter;
        var y = 48;
        
        for(var i = 0; i < 8; i++)
        {
            var msg = this.messages[cnt];
            msg.update();
            
            if(msg.state != 0)
            {
                this.canvas.setDrawAlpha(msg.alpha);
                this.canvas.setDrawScale(msg.scale * 0.5);
                TurokString.drawText(this.canvas, msg.text, 160, y, true);
            }
            
            cnt = (cnt + 1) & 7;
            y += 18;
        }
    },
    
    start : function()
    {
        Hud.prototype.start.bind(this)();
    }
});
