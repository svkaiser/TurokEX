//-----------------------------------------------------------------------------
//
// ComponentAreaTrigger.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentAreaTrigger = class.extendStatic(ComponentArea);

class.properties(ComponentAreaTrigger,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active      : true,
    bRepeatable : false,
    targetID    : 0,
    args        : null,
    triggerSnd  : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onEnter : function(instigator)
    {
        if(this.active == false)
            return;
        
        if(this.triggerSnd !== "")
            Snd.play('sounds/shaders/' + this.triggerSnd + '.ksnd', instigator);
        
        Level.triggerActors(this.targetID, 0);
        this.active = false;
    }
});
