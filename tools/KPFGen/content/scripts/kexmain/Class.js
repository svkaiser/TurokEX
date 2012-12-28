//-----------------------------------------------------------------------------
//
// Class.js
// DESCRIPTION: Pseudo-Class system for Javascript
//
//-----------------------------------------------------------------------------

var class = new function()
{   
    this.define = function()
    {
        var newclass = function() { };
        return newclass;
    }
    
    this.extends = function(c)
    {
        var args = arguments.length;
        
        var newclass = function() { };
        
        newclass.prototype = new c();
        newclass.prototype.super = c.prototype;
        newclass.base = this.prototype;
        
        return newclass;
    }
    
    this.properties = function(c, items)
    {
        for(var p in items)
            c.prototype[p] = items[p];
    }
}