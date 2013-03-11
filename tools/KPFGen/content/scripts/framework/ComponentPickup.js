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
    angle : 0.0,
    rotation :
    {
        x : 0.0,
        y : 0.0,
        z : 0.0,
        w : 0.0
    },
    message : "Pickup",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onLocalTick : function()
    {
        const ROTATE_RADIANS = 0.017;
        
        var owner = this.parent.owner;
        var rot = this.rotation;

        rot.y  = Math.sin(this.angle * 0.5);
        rot.w  = Math.cos(this.angle * 0.5);

        Math.qNormalize(rot);
        
        owner.rotation = rot;
        owner.updateTransform();
        
        this.angle += ROTATE_RADIANS;
    },
    
    onTouch : function(instigator)
    {
        var owner = this.parent.owner;
        
        owner.bCollision = false;
        owner.bTouch = false;
        owner.bHidden = true;
        
        if(!this.bRespawn)
            delete this;
        
        // TODO - TEMP
        Sys.print(this.message);
    },
    
    start : function() { }
});
