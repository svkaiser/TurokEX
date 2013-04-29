//-----------------------------------------------------------------------------
//
// TurokPickupArmor.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupArmor = class.extendStatic(TurokPickup);

class.properties(TurokPickupArmor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    message : "tek armor",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.bHasArmor = true;
        player.armor += 75;
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    }
});
