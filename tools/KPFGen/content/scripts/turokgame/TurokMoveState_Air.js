//-----------------------------------------------------------------------------
//
// TurokMoveState_Air.js
// DESCRIPTION: Air movestate class
//
//-----------------------------------------------------------------------------

TurokMoveState_Air = class.extends(TurokMoveState);

class.properties(TurokMoveState_Air,
{
    stateproperties :
    {
        friction : 1.0,
        gravity : 0.62
    },
    
    Jump : function()
    {
        MoveController.velocity.y = 12.8;
    },
    
    OnCheck : function()
    {
        var plane = MoveController.plane;
        var origin = MoveController.origin;
        
        if(!this.super.OnCheck())
            return false;
        
        if(MoveController.state instanceof TurokMoveState_Air)
        {
            if(origin.y - plane.distance(origin) > 0.512)
                return true;
            else
                return false;
        }

        if(MoveController.cmd.action('+jump') &&
            !MoveController.cmd.actionHeld('+jump'))
        {
            var velocity = MoveController.velocity;
            
            // can't jump if standing on a steep slope
            if(plane.isAWall() && origin.y - plane.distance(origin) <= 8)
                return false;
            
            // must be standing on ground
            if(origin.y - plane.distance(origin) <= 0.512 ||
                (velocity.y < 0 && velocity.y > -12.8))
            {
                this.Jump();
                return true;
            }
        }
        
        return false;
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
        this.super.Move(properties);
    }
});

MoveController.addMovetype(new TurokMoveState_Air(), 2);
