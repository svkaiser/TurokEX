//-----------------------------------------------------------------------------
//
// TurokPickupHealth.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const TUROK_HEALTH_MAX = 250;

TurokPickupHealth = class.extendStatic(TurokPickup);

class.properties(TurokPickupHealth,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    amount : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.health += this.amount;
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
