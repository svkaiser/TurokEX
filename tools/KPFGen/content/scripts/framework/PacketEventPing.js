//-----------------------------------------------------------------------------
//
// PacketEventPing.js
// DESCRIPTION: Ping server/client
//
//-----------------------------------------------------------------------------

PacketEventPing = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        packet.write8(data);
    }
    
    this.recieve = function(ev, packet)
    {
        var isClient = packet.read8();
        
        if(isClient)
        {
            Sys.print('Recieved ping from ' + Server.getPeerAddress(ev) +
                ' (channel ' + ev.channel + ')');
            
            PacketManager.send(PacketEventPing, ev.peer, false);
        }
        else
            Sys.print('Recieved acknowledgement from server');
    }
});
