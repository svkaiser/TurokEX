//-----------------------------------------------------------------------------
//
// Array.js
// DESCRIPTION: Extensions to Javascript Array class
//
//-----------------------------------------------------------------------------

Array.isArray = function(vArg)
{
    return Object.prototype.toString.call(vArg) === "[object Array]";
}
