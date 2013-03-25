//-----------------------------------------------------------------------------
//
// Render.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Render = class.extendNative(NRender, function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.fogColor = [0, 0, 0, 1];
    this.fogNear = 0.0;
    this.fogFar = 0.0;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.event_PreRender = function()
    {
        var lp = Client.localPlayer;
        
        if(lp.controller == null)
        {
            this.clearViewPort(0.25, 0.25, 0.25);
            return;
        }
            
        if(lp.viewCamera == null ||
            lp.viewCamera == undefined)
            return;
        
        var plane = lp.prediction.plane;
        
        if(plane == null)
            this.clearViewPort(0.25, 0.25, 0.25);
        else
        {
            for(var component in lp.controller.plane.area)
                lp.controller.plane.area[component].onPreRender();
        }
        
        lp.viewCamera.draw();
    }
    
    this.event_OnRender = function()
    {
        var lp = Client.localPlayer;
        
        if(lp.controller == null)
            return;
            
        if(lp.controller.plane != null)
        {
            for(var component in lp.controller.plane.area)
                lp.controller.plane.area[component].onRender();
        }
        
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
        
        if(lp.controller && lp.controller.owner)
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
