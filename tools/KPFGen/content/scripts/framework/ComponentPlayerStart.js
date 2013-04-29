//-----------------------------------------------------------------------------
//
// ComponentPlayerStart.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentPlayerStart = class.extendStatic(Component, function()
{
    var hud = class.find(this.hudClass);
    var controller = class.find(this.controllerClass);
    
    if(hud != null)
        this.playerHud = new hud();
        
    if(controller != null)
        this.controller = new controller();
});

class.properties(ComponentPlayerStart,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    playerID        : 0,
    controllerClass : "Controller",
    hudClass        : "Hud",
    playerHud       : null,
    active          : true,
    controller      : null
});
