//-----------------------------------------------------------------------------
//
// TurokMoveState_Walk.js
// DESCRIPTION: Walking/Running movestate class
//
//-----------------------------------------------------------------------------

TurokMoveState_Walk = class.extends(TurokMoveState);

class.properties(TurokMoveState_Walk,
{
    stateproperties :
    {
        friction : 1.0,
        gravity : 0.62
    },
    
    OnCheck : function()
    {
        return this.super.OnCheck();
    },
    
    OnMove : function()
    {
        this.Move(this.stateproperties);
    },
    
    Move : function(properties)
    {
        MoveController.setDirection(
            0,
            MoveController.yaw,
            0);
        
        this.OnAccelerate(this.speedproperties);
            
        var plane = MoveController.plane;
        var origin = MoveController.origin;
        var velocity = MoveController.velocity;
        
        // slide down on a steep slope
        if(plane.isAWall() && origin.y - plane.distance(origin) <= 10.24)
        {
            velocity.y = (-16 - velocity.y) * 0.5 + velocity.y;
        }
        
        this.super.Move(properties);
    }
});

MoveController.addMovetype(new TurokMoveState_Walk(), 3);
