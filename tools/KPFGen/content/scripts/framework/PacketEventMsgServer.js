//-----------------------------------------------------------------------------
//
// PacketEventMsgServer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

PacketEventMsgServer = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        packet.writeString(data);
    }
    
    this.recieve = function(ev, packet)
    {
        var svcl = Server.getSVClient(ev.peer);
        
        if(svcl.state == SVC_STATE_INACTIVE)
            return;
        
        var msg = packet.readString();
        
        Server.execMessageEvent(msg, svcl);
    }
});
