//-----------------------------------------------------------------------------
//
// ComponentPickup.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentPickup = class.extendStatic(Component);

class.properties(ComponentPickup,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    bRespawn : false,
    message : "Pickup",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onLocalTick : function()
    {
        const ROTATE_RADIANS = 0.017;
        
        var actor = this.parent.owner;
        
        actor.yaw += ROTATE_RADIANS;
        actor.updateTransform();
    },
    
    onTouch : function(instigator)
    {
        var owner = this.parent.owner;
        
        owner.bCollision = false;
        owner.bTouch = false;
        owner.bHidden = true;
        
        if(!this.bRespawn)
        {
            Sys.print(owner.origin.toString());
            delete this;
        }
        
        // TODO - TEMP
        Sys.print(this.message);
    },
    
    start : function() { }
});
