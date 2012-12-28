//-----------------------------------------------------------------------------
//
// TurokMoveState.js
// DESCRIPTION: Base movestate class for Turok
//
//-----------------------------------------------------------------------------

TurokMoveState = class.extends(Movestate);

class.properties(TurokMoveState,
{
    speedproperties :
    {
        forward :
        {
            forwardspeed : 6.4,
            backspeed : -4.864,
            acceleration : 0.09,
            deacceleration : 0.5
        },
        
        right :
        {
            forwardspeed : 6.4,
            backspeed : -6.4,
            acceleration : 0.25,
            deacceleration : 0.5
        },
    },
    
    OnAccelerate : function(properties)
    {
        var forwardspeed = properties.forward;
        var rightspeed = properties.right;
        
        // accel Z axis
        if(MoveController.cmd.action('+forward'))
            this.Accelerate_Z(forwardspeed.forwardspeed, forwardspeed.acceleration);
        else if(MoveController.cmd.action('+back'))
            this.Accelerate_Z(forwardspeed.backspeed, forwardspeed.acceleration);
        else
            this.Deaccelerate_Z(forwardspeed.deacceleration);
        
        // accel X axis
        if(MoveController.cmd.action('+strafeleft'))
            this.Accelerate_X(rightspeed.forwardspeed, rightspeed.acceleration);
        else if(MoveController.cmd.action('+straferight'))
            this.Accelerate_X(rightspeed.backspeed, rightspeed.acceleration);
        else
            this.Deaccelerate_X(rightspeed.deacceleration);
        
        //
        // apply acceleration to velocity
        //
        MoveController.velocity.add(Vector.scale(
            MoveController.forward, MoveController.accel.z));
            
        MoveController.velocity.add(Vector.scale(
            MoveController.right, MoveController.accel.x));
    },
    
    OnCheck : function()
    {
        return !(MoveController.state instanceof TurokMoveState_NoClip);
    },
    
    OnMove : function()
    {
    },
    
    Move : function(properties)
    {
        this.super.Move(properties);
    }
});
