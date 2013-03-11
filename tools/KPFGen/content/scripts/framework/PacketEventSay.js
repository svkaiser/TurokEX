//-----------------------------------------------------------------------------
//
// PacketEventSay.js
// DESCRIPTION: Send a message to server
//
//-----------------------------------------------------------------------------

PacketEventSay = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        packet.writeString(data);
    }
    
    this.recieve = function(ev, packet)
    {
        Sys.print(Server.getPeerAddress(ev) + ': ' + packet.readString());
    }
});
