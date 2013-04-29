//-----------------------------------------------------------------------------
//
// ComponentArea.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentArea = class.extendStatic(Component);

class.properties(ComponentArea,
{
    // actor enters this area
    onEnter : function(instigator) { },
    
    // actor leaves this area
    onExit : function(instigator) { },
    
    // before world rendering
    onPreRender : function(instigator) { },
    
    // during world rendering
    onRender : function(instigator) { },
    
    // after world rendering
    onPostRender : function(instigator) { },
    
    // called every server tick
    onTick : function() { },
    
    // called every client tick
    onLocalTick : function() { }
});
