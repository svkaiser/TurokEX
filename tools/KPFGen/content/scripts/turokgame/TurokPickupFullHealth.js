//-----------------------------------------------------------------------------
//
// TurokPickupFullHealth.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupFullHealth = class.extendStatic(TurokPickup);

class.properties(TurokPickupFullHealth,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    amount : 0,
    message : "full health",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
            
        player.health = player.mortalWound;
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
