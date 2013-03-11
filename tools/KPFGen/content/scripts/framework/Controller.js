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
    plane       : null,
    owner       : null,
    
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
    
    // time-critical lerping
    lerp : function(cur, next, time)
    {
        var t = (time * (60 * this.frametime));
        if(t > 1) return next;
        return (next - cur) * t + cur;
    },
    
    accelX : function(speed, acceleration)
    {
        this.accel.x = this.lerp(this.accel.x, speed * this.frametime, acceleration);
    },
    
    accelY : function(speed, acceleration)
    {
        this.accel.y = this.lerp(this.accel.y, speed * this.frametime, acceleration);
    },
    
    accelZ : function(speed, acceleration)
    {
        this.accel.z = this.lerp(this.accel.z, speed * this.frametime, acceleration);
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
            this.velocity.y -= ((amount * this.frametime) * this.frametime);
    },
    
    hitFloor : function()
    {
        var c = this;
        
        if(c.plane != null)
        {
            var dist;
            
            if(c.plane.flags & 64)
            {
                dist = c.plane.heightDistance(c.origin) - 61.44;
                
                if((c.origin.y + c.velocity.y) > dist)
                {
                    c.origin.y = dist;
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
        Physics.move(
            this.origin,
            this.velocity,
            this.plane,
            this.owner,
            this.angles.yaw);
            
        this.hitFloor();
    }
});
