//-----------------------------------------------------------------------------
//
// PredictionMove.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Controller.js');

PredictionMove = class.define(function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.origin     = new Vector();
    this.velocity   = new Vector();
    this.accel      = new Vector();
    this.timestamp  = 0.0;
    this.frametime  = 0.0;
    this.angles     = { yaw : 0.0, pitch : 0.0, roll : 0.0 };
    this.plane      = null;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.buildAngles = function(command)
    { 
        this.angles.yaw     = Angle.clamp(command.angle_x);
        this.angles.pitch   = Angle.clamp(command.angle_y);
    }
    
    this.updateFromController = function(controller)
    {
        this.origin         = controller.origin;
        this.velocity       = controller.velocity;
        this.accel          = controller.accel;
        this.plane          = controller.plane;
        this.angles.yaw     = controller.angles.yaw;
        this.angles.pitch   = controller.angles.pitch;
        this.angles.roll    = controller.angles.roll;
    }
    
    this.clientMove = function(p)
    {
        if(Client.state != Client.STATE_READY)
            return;
            
        if(!Client.inLevel())
            return;
            
        this.buildAngles(p.command);
        this.timestamp = p.command.timestamp;
        this.frametime = p.command.frametime;
        
        p.controller.updateFromPrediction(this);
        //Sys.print('cl: ' + this.timestamp + ' ' + this.frametime + ' ' + this.origin.toString());
        p.controller.command = p.command;
        p.controller.beginMovement();
        
        this.updateFromController(p.controller);
        p.command = p.controller.command;
        
        var current = (p.netsequence.outgoing-1) & (NETBACKUPS-1);
        p.oldMoves[current].copy(this.origin);
        //p.oldCommands[current] = p.command;
        p.latency[current] = Sys.time();
    }
    
    this.serverMove = function(svcl)
    {
        if(svcl.state != SVC_STATE_INGAME)
            return;
            
        this.buildAngles(svcl.command);
        this.timestamp = svcl.command.timestamp;
        this.frametime = svcl.command.frametime;
        
        svcl.controller.updateFromPrediction(this);
        //Sys.print('sv: ' + this.timestamp + ' ' + this.frametime + ' ' + this.origin.toString());
        svcl.controller.command = svcl.command;
        svcl.controller.beginMovement();
        
        this.updateFromController(svcl.controller);
        svcl.command = svcl.controller.command;
    }
});
