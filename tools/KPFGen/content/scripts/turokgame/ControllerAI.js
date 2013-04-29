//-----------------------------------------------------------------------------
//
// ControllerAI.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ControllerAI = class.extends(Controller);

class.properties(ControllerAI,
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    aiComponent     : null,
    waterheight     : 0.0,
    mass            : 1200,
    friction        : 1.0,
    bounceDamp      : 1.0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    updateFromActor : function(actor, component)
    {
        this.origin         = actor.origin;
        this.plane          = Plane.fromIndex(actor.plane);
        this.angles.yaw     = actor.yaw;
        this.angles.pitch   = actor.pitch;
        this.angles.roll    = 0.0;
        this.frametime      = actor.timeStamp;
        this.center_y       = this.owner.centerHeight;
        this.view_y         = this.owner.viewHeight;
        this.aiComponent    = component;
    },
    
    updateActor : function(actor)
    {
        actor.origin       = this.origin;
        actor.plane        = this.plane.toIndex();
        actor.yaw          = this.angles.yaw;
        actor.pitch        = this.angles.pitch;
        
        actor.updateTransform();
    },
    
    beginMovement : function()
    {
        if(this.frametime >= 1 || this.frametime <= 0)
            return;
            
        Physics.move(this);
    }
});
