//-----------------------------------------------------------------------------
//
// TurokPickupBackpack.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupBackpack = class.extendStatic(TurokPickup);

class.properties(TurokPickupBackpack,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    message : "backpack",
    pickupSnd : 'sounds/shaders/generic_4_non_weapon_pickup.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
            
        player.bHasBackpack = true;
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
