//-----------------------------------------------------------------------------
//
// ComponentProjectile.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ControllerProjectile.js');

ComponentProjectile = class.extendStatic(Component, function()
{
    var controller = class.find(this.controllerClass);
        
    if(controller != null)
        this.controller = new controller();
});

class.properties(ComponentProjectile,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    controllerClass : "ControllerProjectile",
    controller      : null,
    active          : true,
    actor           : null,
    damage          : 0,
    speed           : 1024,
    mass            : 0,
    friction        : 0.0,
    airFriction     : 0.0,
    sourceActor     : null,
    bTouchActors    : true,
    bTouchSurface   : true,
    lifeTime        : 0.0,
    damageClass     : Damage,
    impactFX        : "",
    impactSnd       : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    destroy : function(origin)
    {
        Sys.spawnFx(this.impactFX, null, origin,
            this.actor.rotation, Plane.fromIndex(this.actor.plane), null, this.impactSnd);
            
        GameActor.remove(this.actor);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    start : function()
    {
    },
    
    onReady : function()
    {
        this.actor = this.parent.owner;
        this.controller.owner = this.actor;
        this.controller.setDirection(this.actor.pitch, this.actor.yaw, this.actor.roll);
    },
    
    onTick : function()
    {
        var c = this.controller;
        
        if(this.actor.plane == -1 || this.lifeTime <= 0)
        {
            this.destroy(this.actor.origin);
            return;
        }
        
        c.updateFromActor(this.actor, this);
        c.beginMovement();
            
        if(c.traceResult != null)
        {
            var t = c.traceResult;
            
            if(this.bTouchActors && t.hitActor != null &&
                GameActor.compare(t.hitActor, this.sourceActor) == false)
            {
                this.damageClass.prototype.inflict(t.hitActor, actor);
                this.destroy(t.hitVector);
            }
            else if(this.bTouchSurface)
            {
                var t = c.traceResult;
                this.destroy(t.hitVector);
            }
        }
        
        this.lifeTime -= ((1.0 / 60.0) * 1000.0) * this.actor.timeStamp;
    },
    
    onLocalTick : function()
    {
    }
});
