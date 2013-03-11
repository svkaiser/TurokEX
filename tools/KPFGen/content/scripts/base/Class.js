//-----------------------------------------------------------------------------
//
// Class.js
// DESCRIPTION: Pseudo-Class system for Javascript
//
//-----------------------------------------------------------------------------

class = new function()
{   
    //
    // defines a basic, extendable class prototype.
    // a subclass object is added which is used to
    // reference any parent or child class that
    // this class is extended off of
    //
    // @param f: constructor function
    //
    this.define = function(f)
    {
        var ok = (f && typeof f === "function");
        var newclass = function()
        {
            this.constructClass.apply(this, arguments);
            delete this.constructClass;
        }
        
        newclass.prototype.constructClass = (ok == true) ? f : function() { };
        newclass.prototype.constructor = newclass;
        
        Sys.unsetEnumerate(newclass.prototype, 'constructClass');
        
        return newclass;
    }
    
    //
    // defines a class that is depended on a existing
    // built-in native class. native classes cannot
    // be extended
    //
    // @param c: name of the built-in native class
    // @param f: constructor function
    //
    this.extendNative = function(c, f)
    {
        var ok = (f && typeof f === "function");
        var newclass = (ok == true) ? f : function() { };
        
        newclass.prototype = c;
        
        var newc = new newclass();
        
        newc.constructor = newclass;
        c.subclass = newc;
        newc.super = c;
        
        return newc;
    }
    
    //
    // defines a class that inherits off of a existing class
    // that's already defined. all exposed properties will
    // also be added to the extended class. a super object
    // is created which will reference any object from
    // the inherited class
    //
    // @param c: class prototype to inherit from
    // @param f: constructor function
    //
    this.extends = function(c, f)
    {
        var newclass = function()
        {
            var callSuperConstructor_r = function(sParent, dest, args)
            {
                if(sParent.prototype.super)
                    callSuperConstructor_r(sParent.prototype.super, dest, args);
                    
                sParent.prototype.constructClass.apply(dest, args);
            }
            
            callSuperConstructor_r(this.super, this, arguments);
            this.constructClass.apply(this, arguments);
            delete this.constructClass;
        };
        
        var ok = (f && typeof f === "function");
        
        newclass.prototype.constructClass = (ok == true) ? f : function() { };
        newclass.prototype.super = c;
        newclass.prototype.constructor = newclass;
        
        Sys.unsetEnumerate(newclass.prototype, 'constructClass');
        Sys.unsetEnumerate(newclass.prototype, 'super');

        for(var p in c.prototype)
            newclass.prototype[p] = c.prototype[p];
    
        return newclass;
    }
    
    //
    // defines a class that inherits off of a static
    // native class
    //
    // @param c: native class prototype to inherit from
    // @param f: constructor function
    //
    this.extendStatic = function(c, f)
    {
        var ok = (f && typeof f === "function");
        
        var newclass = (ok == true) ? f : function() { };
        newclass.prototype.constructor = newclass;

        if(c.prototype == undefined)
        {
            // static inheriting
            for(var p in c)
                newclass.prototype[p] = c[p];
        }
        else
        {
            // normal inheriting
            for(var p in c.prototype)
                newclass.prototype[p] = c.prototype[p];
        }
    
        return newclass;
    }
    
    //
    // adds properties to a class prototype which
    // can be accessed by console
    //
    // @param c: class prototype
    // @param items: object array with contained properties
    //
    this.properties = function(c, items)
    {
        for(var p in items)
            c.prototype[p] = items[p];
    }
    
    //
    // returns the class object matching the specified name
    //
    // @param cls: name of the class to search for
    //
    this.find = function(cls)
    {
        for(var name in Global)
        {
            if(typeof Global[name] === 'function' && cls === name)
                return Global[name];
        }
        
        if(typeof cls === 'object')
            return Object;
        else if(Array.isArray(cls))
            return Array;
        else if(typeof cls === 'string')
            return String;
        
        return null;
    }
    
    //
    // gets the subclass of a object that it is
    // inherited from
    //
    // @param c: class prototype
    //
    this.sub = function(c)
    {
        return c.prototype.subclass;
    }
    
    //
    // returns a new instance of the class prototype
    //
    // @param c: class prototype
    //
    this.create = function(c)
    {
        if(!arguments.length)
            return null;
        
        return new c();
    }
    
    //
    // prints out the name of all classes
    //
    this.listClasses = function()
    {
        for(var name in Global)
        {
            if(typeof Global[name] === 'function' ||
                typeof Global[name] === 'object' &&
                (Global[name]['subclass'] ||
                Global[name]['constructClass']))
            {
                Sys.print(name);
            }
        }
    }
    
    Sys.addCommand('listclasses', this.listClasses);
}
