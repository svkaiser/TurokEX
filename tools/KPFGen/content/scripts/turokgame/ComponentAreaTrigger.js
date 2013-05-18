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
        
        // find an actor within the level that has a matching name
        var actorList = Level.findActor(this.targetID);
        
        if(actorList != null)
        {
            for(var j = 0; j < actorList.length; j++)
            {
                var actor = actorList[j];
                
                // walk through all of its components
                for(var i in actor.components)
                {
                    var component = actor.components[i];
                    
                    // fire trigger event
                    if(component.onTrigger)
                        component.onTrigger(instigator, this.args);
                }
            }
        }
        
        this.active = false;
    }
});
