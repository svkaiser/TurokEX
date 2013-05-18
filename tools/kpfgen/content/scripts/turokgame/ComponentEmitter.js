//-----------------------------------------------------------------------------
//
// ComponentEmitter.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentEmitter = class.extendStatic(Component);

class.properties(ComponentEmitter,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bEmit : false,
    fx : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        this.bEmit = true;
    },
    
    onTrigger : function(instigator, args)
    {
    },
    
    onTick : function()
    {
        /*if(this.bEmit == false)
            return;
            
        var actor = this.parent.owner;
        
        Sys.spawnFx(this.fx, actor, actor.origin, actor.rotation,
            Plane.fromIndex(actor.plane), null, null);
            
        this.bEmit = false;*/
    },
    
    onLocalTick : function()
    {
    }
});
