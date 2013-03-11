//-----------------------------------------------------------------------------
//
// PacketEventCommand.js
// DESCRIPTION: Client input data. Sent to server
//
//-----------------------------------------------------------------------------

PacketEventCommand = class.extends(PacketEvent, function()
{
    this.send = function(packet, data)
    {
        var lp = data;
        var command = lp.command;
        var inputlist = Input.getActions();
        var numactions = 0;
        
        for(var i = 0; i < inputlist.length; i++)
        {
            if(command.buttons[i])
                numactions++;
        }
        
        packet.writeFloat(command.angle_x);
        packet.writeFloat(command.angle_y);
        packet.writeFloat(command.timestamp);
        packet.writeFloat(command.frametime);
        packet.write32(numactions);
        
        for(var i = 0; i < inputlist.length; i++)
        {
            if(command.buttons[i])
            {
                packet.write8(i);
                packet.write8(command.heldtime[i]);
            }
        }
        
        packet.write32(lp.netsequence.ingoing);
        packet.write32(lp.netsequence.outgoing);
        
        lp.netsequence.outgoing++;
    }
    
    this.recieve = function(ev, packet)
    {
        var svcl = Server.getSVClient(ev.peer);
        
        if(svcl == null)
            return;
            
        var numactions = 0;
        
        svcl.command.angle_x = packet.readFloat();
        svcl.command.angle_y = packet.readFloat();
        svcl.command.timestamp = packet.readFloat();
        svcl.command.frametime = packet.readFloat();
        
        numactions = packet.read32();
        
        svcl.command.clearButtons();
        
        for(var i = 0; i < numactions; i++)
        {
            var btn = packet.read8();
            
            svcl.command.buttons[btn] = true;
            svcl.command.heldtime[btn] = packet.read8();
        }
        
        svcl.netsequence.acks = packet.read32();
        svcl.netsequence.ingoing = packet.read32();
        
        svcl.prediction.serverMove(svcl);
    }
});
