//-----------------------------------------------------------------------------
//
// TurokPickupSpiritual.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupSpiritual = class.extendStatic(TurokPickup);

class.properties(TurokPickupSpiritual,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    message : "spiritual invincibility",
    pickupSnd : 'sounds/shaders/generic_4_non_weapon_pickup.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.spiritTime = 450;
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
