//-----------------------------------------------------------------------------
//
// ComponentTurokAI.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ControllerAI.js');

ComponentTurokAI = class.extendStatic(Component, function()
{
    var ctrl = class.find(this.controllerClass);
        
    if(ctrl != null)
        this.controller = new ctrl();
});

class.properties(ComponentTurokAI,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    controllerClass : "ControllerAI",
    controller      : null,
    active          : true,
    activeDistance  : 0.0,
    destAngle       : 0.0,
    bTurning        : false,
    health          : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkTargetRange : function(target)
    {
        var self = this.parent.owner;
        
        var x = self.origin.x - target.origin.x;
        var z = self.origin.z - target.origin.z;
        
        if(this.activeDistance < Math.sqrt(x * x + z * z))
            return false;
            
        return true;
    },
    
    anglesToTarget : function(torg, aorg)
    {
        return Angle.invertClampSum(this.controller.angles.yaw,
            Vector.pointToAxis(torg, aorg).toYaw());
    },
    
    changeYaw : function(time)
    {
        this.controller.angles.yaw = Angle.clamp(this.controller.angles.yaw +
            (this.destAngle * time));
    },
    
    checkTargetLOS : function(target)
    {
        var t = Physics.rayTrace(this.controller.origin, this.controller.forward, 4, 64,
            this.controller.plane, this.controller.owner);
            
        if(t != null && t.hitActor != null && GameActor.compare(t.hitActor, target))
            return true;
            
        return false;
    },
    
    rootMotionMove : function()
    {
        var actor = this.parent.owner;
        
        if(actor.animState.flags & NRender.ANIM_ROOTMOTION &&
            !(actor.animState.flags & NRender.ANIM_STOPPED))
        {
            var dir = Vector.applyRotation(actor.animState.rootMotion, actor.rotation);
            dir.y = 0;
            dir.scale(31.25 * actor.timeStamp);
            this.controller.velocity.add(dir);
        }
    },
    
    checkPosition : function(origin, extraradius, angle)
    {
        var actor = this.parent.owner;
        
        var x = origin.x + (actor.radius * extraradius * Math.sin(angle + Math.PI));
        var y = origin.y;
        var z = origin.z + (actor.radius * extraradius * Math.cos(angle + Math.PI));
        
        var pos = new Vector(x, y, z);
        
        return (Physics.checkPosition(actor, origin, pos,
            this.controller.plane) == Physics.TRT_NOHIT);
    },
    
    changeAngle : function(angle, speed)
    {
        var current = this.angleRound(this.controller.angles.yaw);
        
        if(current == angle)
        {
            this.bTurning = false;
            return;
        }
            
        var move = Angle.clamp(angle - current);
        
        if(move > 0)
        {
            if(move > speed)
                move = speed;
        }
        else if(move < -speed)
            move = -speed;
        
        this.controller.angles.yaw = this.angleRound(current + move);
    },
    
    lookForTarget : function(target)
    {
    },
    
    angleRound : function(angle)
    {
        var an = Math.round(Angle.radToDeg(angle) * (65536 / 360.0)) & 65535;
        return Angle.clamp(Angle.degToRad((360.0 / 65536) * an));
    },
    
    lerpAngle : function(angles, time)
    {
        this.controller.angles.yaw =
            this.controller.lerp(this.controller.angles.yaw, angles, time);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    start : function()
    {
    },
    
    onReady : function()
    {
        this.controller.owner = this.parent.owner;
    },
    
    onDamage : function(instigator)
    {
    },
    
    onDeath : function(instigator)
    {
    },
    
    onTick : function()
    {
        var actor = this.parent.owner;
        
        if(actor.timeStamp >= 1 || actor.timeStamp <= 0)
            return;
            
        var c = this.controller;
        
        c.updateFromActor(actor, this);
        
        var plane = c.plane;
        
        if(plane != null)
            this.think(actor);
        
        c.gravity(c.mass);
        c.beginMovement();
        c.applyFriction(c.friction);
        c.updateActor(actor);
    },
    
    onLocalTick : function()
    {
        this.parent.owner.animState.update();
    }
});
