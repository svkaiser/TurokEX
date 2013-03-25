//-----------------------------------------------------------------------------
//
// ComponentAreaCrawl.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/ComponentArea.js');

ComponentAreaCrawl = class.extendStatic(ComponentArea);

class.properties(ComponentAreaCrawl,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onLocalTick : function()
    {
        var ctrl = Client.localPlayer.controller;
        
        if(ctrl == null || ctrl == undefined)
            return; 
        if(!(ctrl instanceof ControllerPlayer))
            return;
            
        if(ctrl.bCrawling)
            ctrl.view_y = 3.84;
    },
    
    onEnter : function(instigator)
    {
        const CRAWL_FLOORHEIGHT = 15.36;
        var ctrl = instigator;
        
        if(ctrl == null || ctrl == undefined)
            return;
        if(!(ctrl instanceof ControllerPlayer))
            return;
            
        if(ctrl.plane.distance(ctrl.origin) - ctrl.origin.y <= CRAWL_FLOORHEIGHT)
            ctrl.bCrawling = true;
    },
    
    onExit : function(instigator)
    {
        var ctrl = instigator;
        
        if(ctrl == null || ctrl == undefined)
            return;
        if(!(ctrl instanceof ControllerPlayer))
            return;
            
        ctrl.bCrawling = false;
        ctrl.view_y = ctrl.owner.viewHeight;
    }
});
