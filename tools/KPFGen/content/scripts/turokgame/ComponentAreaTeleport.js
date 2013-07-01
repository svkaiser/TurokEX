//-----------------------------------------------------------------------------
//
// ComponentAreaTeleport.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentAreaTeleport = class.extendStatic(ComponentArea);

class.properties(ComponentAreaTeleport,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active      : true,
    x           : 0.0,
    y           : 0.0,
    z           : 0.0,
    yaw         : 0.0,
    level       : -1,
    plane       : -1,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    start : function()
    {
        this.active = false;
    },
    
    onEnter : function(instigator)
    {
        this.active = true;
    },
    
    onExit : function(instigator)
    {
        this.active = false;
    },
    
    onLocalTick : function(player)
    {
        if(this.active == false)
            return;
        
        if(player == undefined || player == null)
            return;
        
        if(player.cameraEvent != null)
            return;
        
        if(!player.controller.onGround())
            return;
        
        player.teleportInfo.x       = this.x;
        player.teleportInfo.y       = this.y;
        player.teleportInfo.z       = this.z;
        player.teleportInfo.yaw     = this.yaw;
        player.teleportInfo.level   = this.level;
        player.teleportInfo.plane   = this.plane;
        player.bTeleporting         = true;
        
        player.cameraEvent = CameraEventTeleport;
        player.cameraEvent.prototype.startEvent();
        
        this.active = false;
    }
});
