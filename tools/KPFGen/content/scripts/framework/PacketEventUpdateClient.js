//-----------------------------------------------------------------------------
//
// PacketEventUpdateClient.js
// DESCRIPTION: Packet data for prediction movement. Sent to client to compare
// and correct movement
//
//-----------------------------------------------------------------------------

PacketEventUpdateClient = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        var svcl = data;
        
        if(svcl.prediction.plane == null)
            return;
        
        packet.write32(Server.ticks);
        packet.writeVector(svcl.prediction.origin);
        packet.writeVector(svcl.prediction.velocity);
        packet.write32(svcl.prediction.plane.toIndex());
        packet.write8(svcl.controller.bNoClip);
        packet.write32(svcl.netsequence.ingoing);
        packet.write32(svcl.netsequence.outgoing);
        
        svcl.netsequence.outgoing++;
    }
    
    this.recieve = function(ev, packet)
    {
        if(Client.state != Client.STATE_READY)
            return;
            
        if(!Client.inLevel())
            return;
            
        var lp = Client.localPlayer;
            
        // TODO
        packet.read32();
        var origin = packet.readVector();
        var velocity = packet.readVector();
        var plane = Plane.fromIndex(packet.read32());
        
        lp.controller.bNoClip = packet.read8();
        lp.netsequence.acks = packet.read32();
        lp.netsequence.inGoing = packet.read32();
        
        var oldOrigin = lp.oldMoves[lp.netsequence.acks & (NETBACKUPS-1)];
        var diff = new Vector();
        diff.copy(origin);
        diff.sub(oldOrigin);
        
        //Sys.print(diff.toString());
        
        if(diff.unit3 > 0.1)
            lp.prediction.origin.copy(oldOrigin);
            
        //Sys.print(oldOrigin.toString() + ' ' + origin.toString());
    }
});
