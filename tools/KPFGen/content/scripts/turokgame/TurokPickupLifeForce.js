//-----------------------------------------------------------------------------
//
// TurokPickupLifeForce.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const MAX_LIFEFORCES = 100;
const MAX_LIVES = 9;
        
TurokPickupLifeForce = class.extendStatic(TurokPickup);

class.properties(TurokPickupLifeForce,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    amount : 1,
    message : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        player.lifeForces += this.amount;
        if(player.lifeForces >= MAX_LIFEFORCES)
        {
            Snd.play('sounds/shaders/generic_234.ksnd', this.parent.owner);
            player.lives++;
            if(player.lives > MAX_LIVES)
                player.lives = MAX_LIVES;
            
            player.lifeForces -= MAX_LIFEFORCES;
        }
        
        TurokPickup.prototype.onTouch.bind(this)(instigator);
    },
    
    start : function() { }
});
