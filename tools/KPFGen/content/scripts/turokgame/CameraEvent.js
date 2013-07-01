//-----------------------------------------------------------------------------
//
// CameraEvent.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

CameraEvent = class.define();

class.properties(CameraEvent,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    fade : 0.0,
    bFading : false,
    bFadeOut : false,
    stage : 0,
    time : 0.0,
    fadeSpeed : 1000,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    startEvent : function()
    {
        this.fadeOut();
        this.stage = 0;
    },
    
    fadeIn : function()
    {
        this.bFading = true;
        this.bFadeOut = false;
        this.fade = 255;
    },
    
    fadeOut : function()
    {
        this.bFading = true;
        this.bFadeOut = true;
        this.fade = 0;
    },
    
    tick : function(hud)
    {
        if(ClientPlayer.command.getAction('+attack'))
            this.onCancel();
            
        if(this.bFading == false)
            return;
            
        if(this.bFadeOut)
            this.fade += Math.round(this.fadeSpeed * Sys.deltatime());
        else
            this.fade -= Math.round(this.fadeSpeed * Sys.deltatime());
            
        if(this.fade > 255)
        {
            this.fade = 255;
            this.bFading = false;
            this.stage++;
        }
        
        if(this.fade < 0)
        {
            this.fade = 0;
            this.bFading = false;
            this.stage++;
        }
    },
    
    draw : function()
    {
        var hud = ClientPlayer.component.playerHud;
        var viewport = Render.viewport;
        
        GL.setBlend(1);
        
        hud.canvas.setDrawAlpha(this.fade);
        hud.canvas.drawTile('textures/black.tga', 0, 0,
            viewport.width, viewport.height);
            
        GL.setBlend(0);
    },
    
    onCancel : function()
    {
    },
    
    runEvent : function(player, camera)
    {
    }
});

//-----------------------------------------------------------------------------
//
// CameraEventKeyPickup.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

CameraEventKeyPickup = class.extends(CameraEvent);

class.properties(CameraEventKeyPickup,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    levelID : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onCancel : function()
    {
        if(this.stage == 3)
            this.time = 0.0;
    },
    
    draw : function()
    {
        if(this.stage >= 2 && this.stage <= 4)
        {
            var hud = ClientPlayer.component.playerHud;
            
            GL.setBlend(1);
            TurokString.setStringColor(180, 180, 124, 77, 63, 42);
            hud.canvas.setDrawScale(0.5);
            hud.canvas.setDrawAlpha(255);
            TurokString.drawText(hud.canvas, "Level " + this.levelID + " Key", 160, 174, true);
            GL.setBlend(0);
            
            hud.canvas.setDrawScale(1);
        }
        CameraEvent.prototype.draw.bind(this)();
    },
    
    runEvent : function(player, camera)
    {
        var actor = player.parent.owner;
        
        switch(this.stage)
        {
        case 1:
            this.fadeIn();
            this.time = 4000.0 + Level.time;
            this.stage++;
        
            // disconnect camera from player actor
            if(camera.owner)
                camera.owner = null;
                
            player.bHideHud = true;
            player.bLockControls = true;
            
            actor.modelVariant = 3;
            actor.setAnim("anim09", 8.0, NRender.ANIM_NOINTERRUPT);
            break;
        case 2:
        case 3:
        case 4:
            var origin = camera.origin;
            const cKeyDist = 61.44;
    
            origin.x = actor.origin.x + Math.sin(actor.yaw) * cKeyDist;
            origin.y = actor.origin.y + cKeyDist;
            origin.z = actor.origin.z + Math.cos(actor.yaw) * cKeyDist;
            
            var vec = new Vector(actor.origin.x, actor.origin.y + 71.48, actor.origin.z);
                
            camera.origin = origin;
            camera.yaw = Math.atan2(vec.x - origin.x, vec.z - origin.z);
            camera.pitch = Math.atan2(Vector.length3(vec, origin),
                vec.y - origin.y) - Angle.degToRad(90);
            
            if(this.time < Level.time && this.stage != 4)
            {
                this.fadeOut();
                this.stage++;
            }
            break;
        case 5:
            // reconnect player actor to camera
            camera.owner = ClientPlayer.actor;
            player.bHideHud = false;
            player.bLockControls = false;
            player.checkLevelKey(Level.mapID);
            this.fadeIn();
            this.stage++;
            break;
        case 7:
            player.cameraEvent = null;
            actor.modelVariant = 0;
            actor.setAnim("anim00", 4.0, NRender.ANIM_NOINTERRUPT|NRender.ANIM_STOPPED);
            break;
        }
    }
});

//-----------------------------------------------------------------------------
//
// CameraEventTeleport.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

CameraEventTeleport = class.extends(CameraEvent);

class.properties(CameraEventTeleport,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    fadeSpeed   : 350,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    draw : function()
    {
        var hud = ClientPlayer.component.playerHud;
        var viewport = Render.viewport;
        
        GL.setBlend(1);
        
        hud.canvas.setDrawAlpha(this.fade);
        hud.canvas.setDrawColor(255, 255, 255);
        hud.canvas.drawTile('textures/white.tga', 0, 0,
            viewport.width, viewport.height);
            
        GL.setBlend(0);
    },
    
    runEvent : function(player, camera)
    {
        var actor = player.parent.owner;
        
        switch(this.stage)
        {
        case 0:
            player.bLockControls = true;
            this.stage++;
            break;
        case 1:
            break;
        case 2:
            player.teleport();
            break;
        case 3:
            player.bLockControls = false;
            player.bTeleporting = false;
            break;
        case 4:
            player.cameraEvent = null;
            break;
        }
    }
});
