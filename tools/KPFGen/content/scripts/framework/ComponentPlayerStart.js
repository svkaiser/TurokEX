//-----------------------------------------------------------------------------
//
// ComponentPlayerStart.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentPlayerStart = class.extendStatic(Component, function()
{
    var hud = class.find(this.hudClass);
    
    if(hud != null)
        this.playerHud = new hud();
});

class.properties(ComponentPlayerStart,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    playerID    : 0,
    hudClass    : "Hud",
    playerHud   : null,
    active      : true,
    controller  : "Controller"
});
