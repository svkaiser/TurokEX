//-----------------------------------------------------------------------------
//
// TurokPickupKey.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupKey = class.extendStatic(TurokPickup);

class.properties(TurokPickupKey,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active      : true,
    amount      : 0,
    levelID     : 0,
    bits        : 0,
    message     : "",
    pickupSnd   : 'sounds/shaders/generic_4_non_weapon_pickup.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTouch : function(instigator)
    {
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player == null)
            return;
        
        TurokPickup.prototype.onTouch.bind(this)(instigator);
        
        CameraEventKeyPickup.prototype.levelID = this.levelID;
        
        player.cameraEvent = CameraEventKeyPickup;
        player.cameraEvent.prototype.startEvent();
        player.giveLevelKey(this.levelID, this.bits);
    }
});
