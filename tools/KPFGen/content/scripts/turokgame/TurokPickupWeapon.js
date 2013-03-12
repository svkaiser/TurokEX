//-----------------------------------------------------------------------------
//
// TurokPickupWeapon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupWeapon = class.extendStatic(TurokPickup);

class.properties(TurokPickupWeapon,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    weapon_id : 0,
    message : "Weapon",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.giveWeapon(this.weapon_id);
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    },
    
    start : function() { }
});
