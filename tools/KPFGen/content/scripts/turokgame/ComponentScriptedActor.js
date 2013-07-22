//-----------------------------------------------------------------------------
//
// ComponentScriptedActor.js
//
// DESCRIPTION: Base class for all Scripted Actor objects. Scripted Actors
// can be used to trigger special one-offs
//
//-----------------------------------------------------------------------------

ComponentScriptedActor = class.extendStatic(Component);

class.properties(ComponentScriptedActor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation        : -1,
    bTriggered              : false,
    bRemoveOnCompletion     : true,
    triggerDelay            : 0.0,
    bSleepUntilTriggered    : false,
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        if(!this.bSleepUntilTriggered)
            this.parent.owner.setAnim("anim00", 4.0,
                NRender.ANIM_LOOP|NRender.ANIM_ROOTMOTION);
        else
            this.parent.owner.bHidden = true;
    },
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        
        if(this.bSleepUntilTriggered)
            actor.bHidden = false;
        
        actor.blendAnim(this.triggerAnimation, 4.0, 4.0, NRender.ANIM_ROOTMOTION);
        this.bTriggered = true;
    },
    
    onLocalTick : function()
    {
        var actor = this.parent.owner;
        
        if(actor.animState.flags & NRender.ANIM_STOPPED)
        {
            if(this.bRemoveOnCompletion)
            {
                GameActor.remove(actor);
                return;
            }
        }
    }
});

// TODO - MOVE THESE INTO SEPERATE MODULES

//-----------------------------------------------------------------------------
//
// ScriptedMonkey.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ScriptedMonkey = class.extendStatic(ComponentScriptedActor);

class.properties(ScriptedMonkey,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation    : 200,
    bRemoveOnCompletion : true
});

//-----------------------------------------------------------------------------
//
// ScriptedBird.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ScriptedBird = class.extendStatic(ComponentScriptedActor);

class.properties(ScriptedBird,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation        : 200,
    bRemoveOnCompletion     : true,
    bSleepUntilTriggered    : true
});
