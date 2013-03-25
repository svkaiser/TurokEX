//-----------------------------------------------------------------------------
//
// Client.js
// DESCRIPTION: Main client system. Takes in user-inputs, rendering, or anything
// that is handled locally. Must hook directly to the native NClient class
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/PacketManager.js');
Sys.dependsOn('scripts/framework/LocalPlayer.js');

const NETBACKUPS = 64;

Client = class.extendNative(NClient, function()
{
    //------------------------------------------------------------------------
    // OBJECTS
    //------------------------------------------------------------------------
    
    // keep track of server information
    this.serverstate =
    {
        time        : 0,
        tics        : 0
    };
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.localPlayer = new LocalPlayer();
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // TODO - temporarily called by native code
    this._tempResponder = function()
    {
        if(this.state != this.STATE_READY)
            return;
            
        var ev = this.getEvent();
        
        if(ev == null)
            return;
        
        this.localPlayer.processInput(ev);
    }
    
    this.buildCommand = function()
    {
        if(this.state != this.STATE_READY)
            return;
        
        this.localPlayer.buildCommand();
    }
    
    this.sendCommand = function()
    {
        if(this.state != this.STATE_READY || Sys.deltatime() >= 1)
            return;
        
        PacketManager.send(PacketEventCommand, this.peer, this.localPlayer);
    }
    
    this.netUpdate = function()
    {
        var ev = this.netEvent;
        
        switch(ev.type)
        {
        case Net.connect:
            Sys.print('connected to host');
            this.localPlayer.resetNetSequence();
            break;
            
        case Net.disconnect:
            Sys.print('disconnected from host');
            break;
            
        case Net.packet:
            PacketManager.recieve(ev);
            break;
        }
    }
    
    this.tick = function()
    {
        Input.process();
        
        this.buildCommand();
        
        this.sendCommand();
        
        this.localPlayer.tick();
    }
    
    //------------------------------------------------------------------------
    // COMMANDS
    //------------------------------------------------------------------------
    
    this.ping = function()
    {
        PacketManager.send(PacketEventPing, Client.peer, true);
    }
    
    this.msgServer = function()
    {
        if(arguments.length == 0)
        {
            Sys.print('Usage: msgserver <message name>');
            return;
        }
        
        PacketManager.send(PacketEventMsgServer, Client.peer, arguments[0]);
    }
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    PacketManager.add(PacketEventPing);
    PacketManager.add(PacketEventSay);
    PacketManager.add(PacketEventCommand);
    PacketManager.add(PacketEventMsgServer);
    
    Sys.addCommand('ping', this.ping);
    Sys.addCommand('msgserver', this.msgServer);
});
