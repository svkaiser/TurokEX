//-----------------------------------------------------------------------------
//
// TurokPickup.js
//
// DESCRIPTION: Base class for all Turok-specifc item pickups
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
    pickupSnd : 'sounds/shaders/health_pickup_1.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.playerHud.flash(this.flash_r, this.flash_g, this.flash_b);
        player.playerHud.notify(this.message);
        ComponentPickup.prototype.onTouch.bind(this)(instigator);
        Snd.play(this.pickupSnd);
    },
    
    start : function() { }
});
