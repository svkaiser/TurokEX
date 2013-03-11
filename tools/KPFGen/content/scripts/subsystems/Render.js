//-----------------------------------------------------------------------------
//
// Render.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Render = class.extendNative(NRender, function()
{
    const FOG_LERP_SPEED = 0.025;
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.fogColor = [0, 0, 0, 1];
    this.fogNear = 0.0;
    this.fogFar = 0.0;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.setupFog = function()
    {
        var plane = Client.localPlayer.prediction.plane;
        
        if(plane == null ||
            Sys.getCvar('r_fog') <= 0)
        {
            this.clearViewPort(0.25, 0.25, 0.25);
            return;
        }
        
        var area = plane.area;
        
        if(area == null)
            return;
        
        var areafog = area.ComponentAreaFog;
        
        if(areafog)
        {
            this.fogColor[0] = Math.lerp(this.fogColor[0], (areafog.fog_Color_r / 255.0), FOG_LERP_SPEED);
            this.fogColor[1] = Math.lerp(this.fogColor[1], (areafog.fog_Color_g / 255.0), FOG_LERP_SPEED);
            this.fogColor[2] = Math.lerp(this.fogColor[2], (areafog.fog_Color_b / 255.0), FOG_LERP_SPEED);
            this.fogColor[3] = 1;
            
            this.clearViewPort(
                this.fogColor[0],
                this.fogColor[1],
                this.fogColor[2]);
            
            this.fogFar = Math.lerp(this.fogFar, areafog.fog_Far, FOG_LERP_SPEED);
            this.fogNear = this.fogFar * 0.5;
            
            GL.enable(GL.FOG);
            GL.fog(GL.FOG_COORD_SRC, GL.FRAGMENT_DEPTH);
            GL.fog(GL.FOG_COLOR, this.fogColor);
            GL.fog(GL.FOG_START, this.fogNear);
            GL.fog(GL.FOG_END, this.fogFar);
        }
    }
    
    this.event_PreRender = function()
    {
        if(Client.localPlayer.viewCamera == null ||
            Client.localPlayer.viewCamera == undefined)
            return;
        
        this.setupFog();
        Client.localPlayer.viewCamera.draw();
    }
    
    this.event_OnRender = function()
    {
        var lp = Client.localPlayer;
        
        if(lp.controller.owner)
        {
            var components = lp.controller.owner.components;
            
            if(components.ComponentTurokPlayer)
            {
                var weapon = components.ComponentTurokPlayer.activeWeapon;
                
                weapon.tick();
                weapon.draw();
            }
        }
    }
    
    this.event_PostRender = function()
    {
        this.setOrtho();
        
        var lp = Client.localPlayer;
        
        if(lp.controller.owner)
        {
            var components = lp.controller.owner.components;
            for(var p in components)
            {
                if(components[p].playerHud)
                {
                    components[p].playerHud.onDraw();
                    break;
                }
            }
        }
        
        lp.console.draw();
    }
});
