//-----------------------------------------------------------------------------
//
// ControllerProjectile.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ControllerProjectile = class.extends(Controller);

class.properties(ControllerProjectile,
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    mass        : 0.0,
    friction    : 0.0,
    airFriction : 0.0,
    traceResult : null,
    
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
        this.mass           = component.mass;
        this.friction       = component.friction;
        this.airFriction    = component.airFriction;
        
        this.setDirection(actor.pitch, actor.yaw, actor.roll);
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
            
        this.gravity(this.mass);
        this.traceResult = Physics.move(this, true);
        
        if(this.onGround())
            this.applyFriction(this.friction);
        else
            this.applyFriction(this.airFriction);
        
        this.updateActor(this.owner);
    }
});
