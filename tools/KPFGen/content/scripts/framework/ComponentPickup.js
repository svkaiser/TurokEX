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
    
    onTouch : function(instigator)
    {
        var owner = this.parent.owner;
        
        owner.bCollision = false;
        owner.bTouch = false;
        owner.bHidden = true;
        
        if(!this.bRespawn)
        {
            Sys.print(owner.origin.toString());
            GameActor.remove(this.parent.owner);
        }
        
        // TODO - TEMP
        Sys.print(this.message);
    },
    
    start : function() { }
});
