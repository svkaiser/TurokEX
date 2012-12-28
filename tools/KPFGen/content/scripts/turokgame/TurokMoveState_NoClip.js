//-----------------------------------------------------------------------------
//
// TurokMoveState_NoClip.js
// DESCRIPTION: NoClip movestate class
//
//-----------------------------------------------------------------------------

TurokMoveState_NoClip = class.extends(TurokMoveState);

class.properties(TurokMoveState_NoClip,
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
            forwardspeed : 8.192,
            backspeed : -8.192,
            acceleration : 0.1,
            deacceleration : 0.1
        },
        
        right :
        {
            forwardspeed : 8.192,
            backspeed : -8.192,
            acceleration : 0.1,
            deacceleration : 0.1
        },
    },
    
    OnCheck : function()
    {
        return true;
    },
    
    OnMove : function()
    {
        this.Move(this.stateproperties);
    },
    
    Move : function(properties)
    {
        MoveController.setDirection(
            MoveController.pitch,
            MoveController.yaw,
            0);
        
        this.OnAccelerate(this.speedproperties);
            
        if(MoveController.cmd.action('+jump'))
            this.Accelerate_Y(8.192, 0.1);
        else
            this.Deaccelerate_Y(0.1);
            
        MoveController.velocity.y += MoveController.accel.y;
        MoveController.origin.add(MoveController.velocity);
        MoveController.velocity.x = 0;
        MoveController.velocity.y = 0;
        MoveController.velocity.z = 0;
    }
});

MoveController.addMovetype(new TurokMoveState_NoClip(), 0);
