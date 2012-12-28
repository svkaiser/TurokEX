//-----------------------------------------------------------------------------
//
// TurokMoveState_Climb.js
// DESCRIPTION: Climbing movestate class
//
//-----------------------------------------------------------------------------

TurokMoveState_Climb = class.extends(TurokMoveState);

class.properties(TurokMoveState_Climb,
{
    stateproperties :
    {
        friction : 1.0,
        gravity : 0.0
    },
    
    speedproperties :
    {
        forward :
        {
            forwardspeed : 3.584,
            backspeed : 3.584,
            acceleration : 0.25,
            deacceleration : 0.5
        },
        
        right :
        {
            forwardspeed : 0,
            backspeed : 0,
            acceleration : 0,
            deacceleration : 0
        },
    },
    
    GetClimbDistance : function()
    {
        var n = MoveController.plane.normal;
        
        return (Vector.dot(MoveController.origin, n) - 
                Vector.dot(MoveController.plane.pt1, n));
    },
    
    OnCheck : function()
    {
        if(!this.super.OnCheck())
            return false;
            
        if(MoveController.plane.area['climb'])
        {
            if(!(MoveController.state instanceof TurokMoveState_Climb))
            {
                if(this.GetClimbDistance() >= 0.25)
                    return false;
            }
            
            MoveController.velocity.x = 0;
            MoveController.velocity.y = 0;
            MoveController.velocity.z = 0;
            return true;
        }
            
        return false;
    },
    
    OnMove : function()
    {
        this.Move(this.stateproperties);
    },
    
    Move : function(properties)
    {
        var forwardspeed = this.speedproperties.forward;
        var yaw;
        
        yaw = Angle.diff(MoveController.yaw + Math.PI, MoveController.plane.getYaw());
        Angle.clamp(yaw);
        MoveController.yaw = -yaw * 0.084 + MoveController.yaw;
        
        MoveController.setDirection(
            0,
            MoveController.plane.normal.toYaw(),
            0);
        
        if(MoveController.cmd.action('+forward'))
            this.Accelerate_Y(forwardspeed.forwardspeed, forwardspeed.acceleration);
        else
            this.Deaccelerate_Y(forwardspeed.deacceleration);
            
        MoveController.velocity.y = MoveController.accel.y;
        
        this.OnAccelerate(this.speedproperties);
        this.super.Move(properties);
        MoveController.velocity.y = 0;
    }
});

MoveController.addMovetype(new TurokMoveState_Climb(), 1);
