//-----------------------------------------------------------------------------
//
// TurokPickup.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickup = class.extendStatic(ComponentPickup);

class.properties(TurokPickup,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    flash_r : 0,
    flash_g : 44,
    flash_b : 148,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.playerHud.flash(this.flash_r, this.flash_g, this.flash_b);
        ComponentPickup.prototype.onTouch.bind(this)(instigator);
    },
    
    start : function() { }
});
