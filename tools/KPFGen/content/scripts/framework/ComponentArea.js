//-----------------------------------------------------------------------------
//
// ComponentArea.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentArea = class.extendStatic(Component);

class.properties(ComponentArea,
{
    onEnter : function(instigator) { },
    onExit : function(instigator) { },
    onPreRender : function(instigator) { },
    onRender : function(instigator) { },
    onPostRender : function(instigator) { },
    onTick : function() { },
    onLocalTick : function() { }
});
