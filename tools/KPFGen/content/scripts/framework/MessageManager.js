//-----------------------------------------------------------------------------
//
// MessageManager.js
// DESCRIPTION: Messages requested by Client to Server. These can be specific
// commands that only the Server can execute such as cheats or other commands
// that affect the gameplay
//
//-----------------------------------------------------------------------------

MessageManager = class.define(function()
{
    this.host = (arguments.length > 0 ? arguments[0] : NServer);
    this.events = new Array();
    
    this.add = function(func, name)
    {
        if(typeof func != "function")
            return;
        
        this.events[name] = func;
    }
    
    this.exec = function(name)
    {
        if(this.events[name])
            this.events[name]();
    }
});
