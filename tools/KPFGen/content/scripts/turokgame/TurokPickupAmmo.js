//-----------------------------------------------------------------------------
//
// TurokPickupAmmo.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmo = class.extendStatic(TurokPickup);

class.properties(TurokPickupAmmo,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    message : "ammo",
    amount : 0,
    id : 0,
    pickupSnd : 'sounds/shaders/generic_1_bullet_pickup.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.giveAmmo(this.id, this.amount);
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
