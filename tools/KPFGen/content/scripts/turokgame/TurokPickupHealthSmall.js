//-----------------------------------------------------------------------------
//
// TurokPickupHealthSmall.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupHealthSmall = class.extendStatic(TurokPickupHealth);

class.properties(TurokPickupHealthSmall,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    amount : 2,
    message : "2 health",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
            
        if(player.health >= TUROK_HEALTH_MAX)
            return;
        
        TurokPickupHealth.prototype.onTouch.bind(this)(instigator);
        
        if(player.health > TUROK_HEALTH_MAX)
            player.health = TUROK_HEALTH_MAX;
    }
});
