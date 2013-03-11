//-----------------------------------------------------------------------------
//
// Game.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Game = class.extendNative(NGame, function()
{
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.event_GameInitialized = function()
    {
    }
    
    // a new level has been initialized
    this.event_BeginLevel = function()
    {
        var pStart = null;
        
        for(var i = 0, actor = Level.getActor(i); actor != null; actor = Level.getActor(i++))
        {
            var components = actor.components;
            for(var p in components)
            {
                if(components[p].playerID !== undefined)
                {
                    pStart = actor;
                    break;
                }
            }
        }
        
        if(pStart == null)
        {
            Sys.error('No player start has been found');
            return;
        }
        
        // setup local player
        var lp      = Client.localPlayer;
        var c       = lp.controller;
        var angles  = pStart.angles;
        
        c.origin.copy(pStart.origin);
        
        c.owner             = pStart;
        //c.origin            = pStart.origin;
        c.angles.yaw        = angles.yaw;
        c.angles.pitch      = angles.pitch;
        c.angles.roll       = angles.roll;
        c.center_y          = pStart.centerHeight;
        c.view_y            = pStart.viewHeight;
        c.width             = pStart.radius;
        c.height            = pStart.height;
        c.plane             = Plane.fromIndex(pStart.plane);
        lp.command.angle_x  = angles.yaw;
        
        lp.prediction.updateFromController(c);
        
        for(var i = 0; i < Server.clients.length; i++)
        {
            if(!Server.clients[i])
            {
                Sys.log('client # ' + i + ' has not been created yet');
                continue;
            }
            
            var svcl = Server.clients[i];
            
            if(svcl.state != SVC_STATE_INACTIVE)
            {
                var c = svcl.controller;
                
                c.origin.copy(pStart.origin);
                
                svcl.state              = SVC_STATE_INGAME;
                c.owner                 = pStart;
                //c.origin                = pStart.origin;
                c.angles.yaw            = angles.yaw;
                c.angles.pitch          = angles.pitch;
                c.angles.roll           = angles.roll;
                c.center_y              = pStart.centerHeight;
                c.view_y                = pStart.viewHeight;
                c.width                 = pStart.radius;
                c.height                = pStart.height;
                c.plane                 = Plane.fromIndex(pStart.plane);
                svcl.command.angle_x    = angles.yaw;
                
                svcl.prediction.updateFromController(c);
                
                Sys.print(svcl.name + ' has joined');
            }
        }
    }
});
