//-----------------------------------------------------------------------------
//
// TurokPickupMortalWound.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const TUROK_MORTAL_WOUND_MAX = 120;

TurokPickupMortalWound = class.extendStatic(TurokPickup);

class.properties(TurokPickupMortalWound,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    message : "mortal wound",
    pickupSnd : 'sounds/shaders/generic_4_non_weapon_pickup.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
            
        player.mortalWound += 5;
        if(player.mortalWound > TUROK_MORTAL_WOUND_MAX)
            player.mortalWound = TUROK_MORTAL_WOUND_MAX;
            
        if(player.health < player.mortalWound)
            player.health = player.mortalWound;
        
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
