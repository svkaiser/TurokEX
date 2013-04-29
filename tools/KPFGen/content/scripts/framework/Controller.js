//-----------------------------------------------------------------------------
//
// Controller.js
// DESCRIPTION: Controllers determines the actor's desired movement and direction
//
//-----------------------------------------------------------------------------

Controller = class.define(function()
{
    this.origin     = new Vector();
    this.velocity   = new Vector();
    this.forward    = new Vector();
    this.right      = new Vector();
    this.up         = new Vector();
    this.accel      = new Vector();
    this.angles     = { yaw : 0, pitch : 0, roll : 0 };
});

class.properties(Controller,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    width       : 0,
    height      : 0,
    movetime    : 0,
    timestamp   : 0.0,
    frametime   : 0.0,
    center_y    : 0.0,
    view_y      : 0.0,
    plane       : null,
    owner       : null,
    command     : null,     // player's command object is copied here
    local       : false,    // is this controller handled by server or client?
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    setDirection : function(pitch, yaw, roll)
    {
        var sy = Math.sin(yaw);
        var cy = Math.cos(yaw);
        var sp = Math.sin(pitch);
        var cp = Math.cos(pitch);
        var sr = Math.sin(roll);
        var cr = Math.cos(roll);

        this.forward.x = sy * cp;
        this.forward.y = -sp;
        this.forward.z = cy * cp;

        this.right.x = sr * sp * sy + cr * cy;
        this.right.y = sr * cp;
        this.right.z = sr * sp * cy + cr * -sy;

        this.up.x = cr * sp * sy + -sr * cy;
        this.up.y = cr * cp;
        this.up.z = cr * sp * cy + -sr * -sy;
    },
    
    updateFromWorldState : function(worldState, command)
    {
        this.origin         = worldState.origin;
        this.velocity       = worldState.velocity;
        this.accel          = worldState.accel;
        this.timestamp      = worldState.timeStamp;
        this.frametime      = worldState.frameTime;
        this.plane          = worldState.plane;
        this.angles.yaw     = worldState.yaw;
        this.angles.pitch   = worldState.pitch;
        this.angles.roll    = worldState.roll;
        this.owner          = worldState.actor;
        this.center_y       = this.owner.centerHeight;
        this.view_y         = this.owner.viewHeight;
        this.command        = command;
    },
    
    updateWorldState : function(worldState)
    {
        worldState.origin       = this.origin;
        worldState.velocity     = this.velocity;
        worldState.accel        = this.accel;
        worldState.timeStamp    = this.timestamp;
        worldState.frameTime    = this.frametime;
        worldState.plane        = this.plane;
        worldState.yaw          = this.angles.yaw;
        worldState.pitch        = this.angles.pitch;
        worldState.roll         = this.angles.roll;
    },
    
    applyFriction : function(friction)
    {
        var speed = this.velocity.unit3();
        
        if(speed < 0.0001)
        {
            this.velocity.x = 0;
            this.velocity.z = 0;
        }
        else
        {
            var clipspeed = speed - (speed * friction);
            
            if(clipspeed < 0) clipspeed = 0;
                clipspeed /= speed;
            
            this.velocity.x = this.velocity.x * clipspeed;
            this.velocity.z = this.velocity.z * clipspeed;
        }
    },
    
    applyVerticalFriction : function(friction)
    {
        var speed = this.velocity.y;
        
        if(speed < 0.0001)
            this.velocity.y = 0;
        else
        {
            var clipspeed = speed - (speed * friction);
            
            if(clipspeed < 0) clipspeed = 0;
                clipspeed /= speed;
            
            this.velocity.y = this.velocity.y * clipspeed;
        }
    },
    
    // time-critical lerping
    lerp : function(cur, next, time)
    {
        var t = (time * (60 * this.frametime));
        if(t > 1) return next;
        return (next - cur) * t + cur;
    },
    
    accelX : function(speed, acceleration)
    {
        this.accel.x = this.lerp(this.accel.x, speed, acceleration);
    },
    
    accelY : function(speed, acceleration)
    {
        this.accel.y = this.lerp(this.accel.y, speed, acceleration);
    },
    
    accelZ : function(speed, acceleration)
    {
        this.accel.z = this.lerp(this.accel.z, speed, acceleration);
    },
    
    deAccelX : function(deacceleration)
    {
        this.accel.x = this.lerp(this.accel.x, 0, deacceleration);
    },
    
    deAccelY : function(deacceleration)
    {
        this.accel.y = this.lerp(this.accel.y, 0, deacceleration);
    },
    
    deAccelZ : function(deacceleration)
    {
        this.accel.z = this.lerp(this.accel.z, 0, deacceleration);
    },
    
    gravity : function(amount)
    {
        if((this.origin.y - this.plane.distance(this.origin)) > 0.01)
            this.velocity.y -= (amount * this.frametime);
    },
    
    onGround : function()
    {
        if(this.plane == null)
            return false;
        
        return (this.origin.y -
            this.plane.distance(this.origin) <= 0.512);
    },
    
    hitFloor : function()
    {
        var c = this;
        
        if(c.plane != null)
        {
            var dist;
            
            if(c.plane.flags & 64)
            {
                dist = c.plane.heightDistance(c.origin);
                
                if((dist - c.origin.y) < 61.44)
                {
                    c.origin.y = dist - 61.44;
                    c.velocity.y = 0;
                }
            }

            dist = c.origin.y -
                c.plane.distance(c.origin);
                
            if(dist < 0.512)
            {
                c.origin.y = c.origin.y - dist;
                if(!c.plane.isAWall())
                    c.velocity.y = 0;
            }
        }
    },
    
    beginMovement : function()
    {
        Physics.move(this);
        this.hitFloor();
    }
});
