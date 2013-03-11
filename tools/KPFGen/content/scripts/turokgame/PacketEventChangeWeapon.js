//-----------------------------------------------------------------------------
//
// PacketEventChangeWeapon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

PacketEventChangeWeapon = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        packet.write8(data);
    }
    
    this.recieve = function(ev, packet)
    { 
        var wpn_id = packet.read8();
        var tPlayer = Client.localPlayer.controller.owner.components.ComponentTurokPlayer;
        
        if(tPlayer)
            tPlayer.changeWeapon(wpn_id);
    }
});
