//-----------------------------------------------------------------------------
//
// ComponentDoor.js
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const ANIM_DOOR_IDLE    = 2600;
const ANIM_DOOR_OPEN    = 2601;
const ANIM_DOOR_CLOSE   = 2602;

ComponentDoor = class.extendStatic(Component);

class.properties(ComponentDoor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    delayTime   : 0.0,
    currentTime : 0.0,
    bTriggered  : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // used to unblock a path when a door opens
    // shared across all door types
    action_232 : function()
    {
        var actor = this.parent.owner;
        
        if(actor.plane == -1)
            return;
        
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        var plane = Plane.fromIndex(actor.plane);
        var vec = actor.getLocalVector(x, y, z);
        var len = Math.round(vec.unit3());
        var origin = actor.origin;
        
        vec.sub(origin);
        vec.normalize();
        
        // looked at the disassembly but not sure why they would do it like this..
        // seems like a workaround due to its origin being far off from the door mesh.
        // find the plane facing the origin and then toggle the solid/blocking flags off
        var trace = Physics.rayTrace(origin, vec, 1, len, plane);
            
        if(trace && trace.hitPlane)
            plane = trace.hitPlane;
            
        Level.toggleBlockingPlanes(plane, true);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        this.parent.owner.setAnim(ANIM_DOOR_IDLE, 4.0, NRender.ANIM_LOOP);
    },
    
    onTrigger : function(instigator, args)
    {
        if(this.bTriggered)
            return;
        
        this.parent.owner.setAnim(ANIM_DOOR_OPEN, 4.0, 0);
        this.currentTime = this.delayTime;
        this.bTriggered = true;
    },
    
    onTick : function()
    {
        var actor = this.parent.owner;
        
        switch(actor.animState.animID)
        {
        case ANIM_DOOR_OPEN:
            if(this.delayTime != 0 && actor.animState.flags & NRender.ANIM_STOPPED)
            {
                this.currentTime -= this.parent.owner.timeStamp;
        
                if(this.currentTime > 0)
                    return;
                    
                this.parent.owner.setAnim(ANIM_DOOR_CLOSE, 4.0, 0);
            }
            break;
        case ANIM_DOOR_CLOSE:
            if(actor.animState.flags & NRender.ANIM_STOPPED)
            {
                this.bTriggered = true;
                this.parent.owner.setAnim(ANIM_DOOR_IDLE, 4.0, 0);
            }
            break;
        }
    }
});
