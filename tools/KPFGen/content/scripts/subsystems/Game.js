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
        var playerComponent = null;
        
        for(var i = 0, actor = Level.getActor(i); actor != null; actor = Level.getActor(i++))
        {
            var components = actor.components;
            for(var p in components)
            {
                if(components[p].playerID !== undefined)
                {
                    playerComponent = components[p];
                    pStart = actor;
                    break;
                }
            }
            
            if(pStart != null)
                break;
        }
        
        if(pStart == null)
        {
            Sys.error('No player start has been found');
            return;
        }
        
        // tell the client that we're possessing this actor
        Client.playerActor = pStart;
        
        // setup local player
        var lp      = Client.localPlayer;
        var ctrl    = class.find(playerComponent.controller);
        
        if(ctrl == null)
            Sys.error('Missing controller class (' + playerComponent.controller + ') not found');
        
        lp.controller = new ctrl();
        
        var c       = lp.controller;
        var angles  = pStart.angles;
        
        c.origin.copy(pStart.origin);
        
        c.owner             = pStart;
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
                svcl.controller = new ctrl();
                var c = svcl.controller;
                
                c.origin.copy(pStart.origin);
                
                svcl.state              = SVC_STATE_INGAME;
                c.owner                 = pStart;
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
