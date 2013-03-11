//-----------------------------------------------------------------------------
//
// PacketEventRequestWeapon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

PacketEventRequestWeapon = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        packet.write8(data);
    }
    
    this.recieve = function(ev, packet)
    {
        var svcl = Server.getSVClient(ev.peer);
        
        if(svcl.state == SVC_STATE_INACTIVE)
            return;
            
        var wpn_id = packet.read8();
        PacketManager.send(PacketEventChangeWeapon, svcl.peer, wpn_id);
    }
});
