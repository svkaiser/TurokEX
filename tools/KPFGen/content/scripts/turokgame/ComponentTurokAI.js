//-----------------------------------------------------------------------------
//
// ComponentTurokAI.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ControllerAI.js');

ComponentTurokAI = class.extendStatic(Component, function()
{
    var controller = class.find(this.controllerClass);
        
    if(controller != null)
        this.controller = new controller();
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
    
    lookForTarget : function(target)
    {
    },
    
    angleRound : function(angle)
    {
        var an = Math.round(Angle.radToDeg(angle) * (65536 / 360.0)) & 65535;
        return Angle.clamp(Angle.degToRad((360.0 / 65536) * an));
    },
    
    turn : function(angles, time)
    {
        this.controller.angles.yaw =
            this.controller.lerp(this.controller.angles.yaw, angles, time);
    },
    
    start : function() { },
    
    onReady : function()
    {
        this.controller.owner = this.parent.owner;
    },
    
    onTick : function()
    {
    },
    
    onLocalTick : function()
    {
    }
});
