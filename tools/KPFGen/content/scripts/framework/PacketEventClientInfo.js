//-----------------------------------------------------------------------------
//
// PacketEventClientInfo.js
// DESCRIPTION: Assigns a peer ID to client to 'finalize' its connection process
//
//-----------------------------------------------------------------------------

PacketEventClientInfo = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        var clientID = data;
        
        packet.write8(clientID);
    }
    
    this.recieve = function(ev, packet)
    {
        Client.id = packet.read8();
        Client.state = Client.STATE_READY;
        
        Sys.log('Client ID: ' + Client.id);
    }
});
