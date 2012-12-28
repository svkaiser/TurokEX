//-----------------------------------------------------------------------------
//
// Movestate.js
// DESCRIPTION: Base movestate class. Handles basic
// acceleration, friction, gravity, etc
//
//-----------------------------------------------------------------------------

Movestate = class.define();

class.properties(Movestate,
{
    stateproperties :
    {
        friction : 1.0,
        gravity : 0.5
    },
    
    Accelerate_X : function(speed, acceleration)
    {
        MoveController.accel.x = (speed - MoveController.accel.x) * acceleration + MoveController.accel.x;
    },
    
    Accelerate_Y : function(speed, acceleration)
    {
        MoveController.accel.y = (speed - MoveController.accel.y) * acceleration + MoveController.accel.y;
    },
    
    Accelerate_Z : function(speed, acceleration)
    {
        MoveController.accel.z = (speed - MoveController.accel.z) * acceleration + MoveController.accel.z;
    },
    
    Deaccelerate_X : function(deacceleration)
    {
        MoveController.accel.x = -MoveController.accel.x * deacceleration + MoveController.accel.x;
    },
    
    Deaccelerate_Y : function(deacceleration)
    {
        MoveController.accel.y = -MoveController.accel.y * deacceleration + MoveController.accel.y;
    },
    
    Deaccelerate_Z : function(deacceleration)
    {
        MoveController.accel.z = -MoveController.accel.z * deacceleration + MoveController.accel.z;
    },
    
    // used to determine if we should use this movestate or not based on user-conditions
    OnCheck : function()
    {
        return true;
    },
    
    // begin movement
    OnMove : function()
    {
    },
    
    // handle actual movement here
    Move : function(properties)
    {
        MoveController.move(properties.friction, properties.gravity);
    }
});