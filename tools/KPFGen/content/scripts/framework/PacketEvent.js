//-----------------------------------------------------------------------------
//
// PacketEvent.js
// DESCRIPTION: Base class for all packet events
//
//-----------------------------------------------------------------------------

PacketEvent = class.define(function()
{
    this.id = 0;
    
    this.send = function(packet, data) { }
    this.recieve = function(ev, packet) { }
});
