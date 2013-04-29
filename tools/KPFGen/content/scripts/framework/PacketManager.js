//-----------------------------------------------------------------------------
//
// PacketManager.js
// DESCRIPTION: Stores all packet event classes in a array. Each class is
// identified by a refID which is also used as the 'header' when sending out
// new packets between client and server
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/PacketEvent.js');

PacketManager = new function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.events = new Array();
    this.refID = 1;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.add = function(packetClass)
    {
        packetClass.prototype.id = this.refID;
        this.events[this.refID] = new packetClass;
        this.events[this.refID].id = this.refID;
        this.refID++;
    }
    
    this.send = function(packetClass, peer, data)
    {
        var id = packetClass.prototype.id;
        var packet = Net.newPacket();
        
        if(packet == null)
            return;
        
        // write header
        packet.write8(id);
        
        // handle event
        this.events[id].send(packet, data);
        
        // done
        packet.send(peer);
    }
    
    this.recieve = function(ev)
    {
        var packet = ev.packet;
        var id = packet.read8();
        
        if(this.events[id])
            this.events[id].recieve(ev, packet);
        else
            Sys.print('Recieved unknown packet type: ' + id);
            
        packet.destroy();
    }
}
