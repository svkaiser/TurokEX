//-----------------------------------------------------------------------------
//
// Server.js
// DESCRIPTION: Main server system. Handles gameplay and client movement correction.
// Must hook directly to the native NServer class.
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/PacketManager.js');
Sys.dependsOn('scripts/framework/SVClient.js');

const SERVER_TIME = (0.1 * 1000);

Server = class.extendNative(NServer, function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.clients = new Array(this.MAXCLIENTS);
    this.messageEvents = new Array();
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    /*this.addSVClient = function(ev)
    {
        for(var i = 0; i < this.clients.length; i++)
        {
            if(!this.clients[i])
            {
                // if one doesn't exist already, create a new svclient
                this.clients[i] = new SVClient();
            }
            
            if(this.clients[i].state == SVC_STATE_INACTIVE)
            {
                this.clients[i].create(ev, i);
                PacketManager.send(PacketEventClientInfo,
                    this.clients[i].peer,
                    this.clients[i].clientID);
                
                return;
            }
        }
    }
    
    this.getSVClient = function(peer)
    {
        for(var i = 0; i < this.clients.length; i++)
        {
            if(this.clients[i].state == SVC_STATE_INACTIVE)
                continue;

            if(peer.connectID == this.clients[i].clientID)
                return this.clients[i];
        }

       return null;
    }
    
    this.updateClients = function()
    {
        for(var i = 0; i < this.clients.length; i++)
        {
            if(!this.clients[i])
                continue;
            
            if(this.clients[i].state == SVC_STATE_INGAME)
                PacketManager.send(PacketEventUpdateClient,
                    this.clients[i].peer,
                    this.clients[i]);
        }

       return this.clients[0];
    }
    
    this.addMessageEvent = function(func, name)
    {
        this.messageEvents[name] = func;
    }
    
    this.execMessageEvent = function(name, svclient)
    {
        if(this.messageEvents[name])
            this.messageEvents[name].apply(this, arguments);
    }
    
    this.netUpdate = function()
    {
        var ev = this.netEvent;
        
        switch(ev.type)
        {
        case Net.connect:
            this.addSVClient(ev);
            break;
            
        case Net.disconnect:
            Sys.print(Server.getPeerAddress(ev) + ' disconnected...');
            break;
            
        case Net.packet:
            PacketManager.recieve(ev);
            break;
        }
    }
    
    // called every 'SERVER_TIME' seconds
    this.tick = function()
    {
    }
    
    // always called but before 'this.tick()'
    this.update = function()
    {
        this.updateClients();
    }
    
    //------------------------------------------------------------------------
    // COMMANDS
    //------------------------------------------------------------------------
    
    this.cmdNoClip = function()
    {
        var svcl = arguments[1];
        
        var controller = svcl.controller;
            
        if(svcl.state == SVC_STATE_INGAME)
        {
            if(controller.bNoClip == false)
            {
                controller.bFlying = false;
                controller.bNoClip = true;
                // TODO - TEMP
                Sys.print('NoClip enabled');
            }
            else
            {
                var nPlane = Level.findPlane(controller.origin);
                
                if(nPlane == null)
                    return;
                
                controller.plane = nPlane;
                svcl.prediction.plane = nPlane;
                controller.owner.plane = nPlane.toIndex();
                
                // TODO - TEMP
                nPlane = Level.findPlane(Client.localPlayer.controller.origin);
                Client.localPlayer.controller.plane = nPlane;
                Client.localPlayer.prediction.plane = nPlane;
                Client.localPlayer.controller.owner.plane = nPlane.toIndex();
                
                controller.bNoClip = false;
                // TODO - TEMP
                Sys.print('NoClip Disabled');
            }
        }
    }
    
    this.cmdFly = function()
    {
        var svcl = arguments[1];
        
        var controller = svcl.controller;
            
        if(svcl.state == SVC_STATE_INGAME)
        {
            if(controller.bFlying == false)
            {
                controller.bNoClip = false;
                controller.bFlying = true;
                // TODO - TEMP
                Sys.print('You feel lightweight');
            }
            else
            {
                var nPlane = Level.findPlane(controller.origin);
                
                if(nPlane == null)
                    return;
                
                controller.plane = nPlane;
                svcl.prediction.plane = nPlane;
                controller.owner.plane = nPlane.toIndex();
                
                // TODO - TEMP
                nPlane = Level.findPlane(Client.localPlayer.controller.origin);
                Client.localPlayer.controller.plane = nPlane;
                Client.localPlayer.prediction.plane = nPlane;
                Client.localPlayer.controller.owner.plane = nPlane.toIndex();
                
                controller.bFlying = false;
                // TODO - TEMP
                Sys.print('Gravity pulls you down');
            }
        }
    }
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    this.addMessageEvent(this.cmdNoClip, 'noclip');
    this.addMessageEvent(this.cmdFly, 'fly');
    
    PacketManager.add(PacketEventClientInfo);
    PacketManager.add(PacketEventUpdateClient);*/
});
