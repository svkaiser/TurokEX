//-----------------------------------------------------------------------------
//
// TurokPickupHealthMedium.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupHealthMedium = class.extendStatic(TurokPickupHealth);

class.properties(TurokPickupHealthMedium,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    amount : 10,
    message : "10 health",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
            
        if(player.health >= player.mortalWound)
            return;
        
        TurokPickupHealth.prototype.onTouch.bind(this)(instigator);
        
        if(player.health > player.mortalWound)
            player.health = player.mortalWound;
    }
});
