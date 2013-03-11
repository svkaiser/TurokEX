//-----------------------------------------------------------------------------
//
// Object.js
// DESCRIPTION: Extensions to Javascript Object
//
//-----------------------------------------------------------------------------

Object.prototype.serialize = function()
{
    return JSON.encode(this);
}

Object.prototype.deSerialize = function(jsonData)
{
    // create json object
    var obj = JSON.parse(jsonData);
    
    // copy properties of json object our current object
    if(typeof obj === 'object')
    {
        var parseObject_r = function(srcObj, dstObj)
        {
            for(var prop in srcObj)
            {
                if(prop === 'classInstance')
                    continue;
                
                if(dstObj[prop] == undefined &&
                    (!(dstObj instanceof Vector) && !(dstObj instanceof Quaternion)))
                {
                    Sys.log('deSerialize - unknown property: ' + prop);
                    continue;
                }
                
                if(typeof srcObj[prop] === 'function')
                    continue;
                else if(Array.isArray(srcObj[prop]))
                {
                    for(var i = 0; i < srcObj[prop].length; i++)
                    {
                        if(typeof srcObj[prop][i] === 'object')
                        {
                            if(dstObj[prop][i] == undefined)
                            {
                                var cls = Global[srcObj[prop][i].classInstance];
                                if(cls == undefined)
                                {
                                    Sys.log('deSerialize - unknown class: ' + srcObj[prop][i].classInstance);
                                    continue;
                                }
                                
                                dstObj[prop][i] = new cls();
                            }
                            parseObject_r(srcObj[prop][i], dstObj[prop][i]);
                        }
                        else
                            dstObj[prop][i] = srcObj[prop][i];
                    }
                }
                else if(typeof srcObj[prop] === 'object')
                    parseObject_r(srcObj[prop], dstObj[prop]);
                else
                    dstObj[prop] = srcObj[prop];
            }
            
            dstObj.start();
        }
        
        parseObject_r(obj, this);
    }
    
    obj = null;
}

Object.prototype.start = function() {}

Object.prototype.classname = function()
{
    for(var name in Global)
    {
        if(name === 'Global' || name === 'class')
            continue;
            
        if(typeof Global[name] !== 'function' &&
            typeof Global[name] !== 'object')
        {
            continue;
        }
        
        if(Global[name].prototype == undefined)
        {
            if(Global[name] === this)
                return name;
            else
                continue;
        }
        
        if(this instanceof Global[name])
            return name;
    }
    
    if(this instanceof String)      return 'String';
    if(Array.isArray(this))         return 'Array';
    if(this instanceof Number)      return 'Number';
    if(this instanceof Boolean)     return 'Boolean';
    
    if(typeof this === 'function')
        return 'Function';
    
    return 'Object';
}

Object.prototype.config = function()
{
    if(this.hasOwnProperty('configProperties'))
        return;
    
    if(arguments.length <= 0)
        return;
        
    this.configProperties = new Array();
        
    for(var i = 0; i < arguments.length; i++)
    {
        if(typeof arguments[i] !== 'string')
            continue;
            
        this.configProperties.push(arguments[i]);
    }
}

Sys.unsetEnumerate(Object.prototype, 'serialize');
Sys.unsetEnumerate(Object.prototype, 'deSerialize');
Sys.unsetEnumerate(Object.prototype, 'start');
Sys.unsetEnumerate(Object.prototype, 'classname');
Sys.unsetEnumerate(Object.prototype, 'config');
