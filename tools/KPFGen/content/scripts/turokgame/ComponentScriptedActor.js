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
    bRootMotion             : false,
    triggerDelay            : 0.0,
    bSleepUntilTriggered    : false,
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        if(!this.bSleepUntilTriggered)
            this.parent.owner.setAnim("anim00", 4.0, NRender.ANIM_LOOP);
        else
            this.parent.owner.bHidden = true;
    },
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        var flags = 0;
        
        if(this.bSleepUntilTriggered)
            actor.bHidden = false;
        
        if(this.bRootMotion == true)
            flags |= NRender.ANIM_ROOTMOTION;
        
        actor.setAnim(this.triggerAnimation, 4.0, flags);
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
    bRemoveOnCompletion : true,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onTrigger : function(instigator, args)
    {
        ComponentScriptedActor.prototype.onTrigger.bind(this)();
        
        var actor = this.parent.owner;
        
        // TODO - handle bounding box updates in engine
        var box = actor.bbox;
        box.min_z = -1024;
        
        actor.setBounds(box.min_x, box.min_y, box.min_z, box.max_x, box.max_y, box.max_z);
    }
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
    bRootMotion             : false,
    bSleepUntilTriggered    : true,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onTrigger : function(instigator, args)
    {
        ComponentScriptedActor.prototype.onTrigger.bind(this)();
        
        var actor = this.parent.owner;
        
        // TODO - handle bounding box updates in engine
        var box = actor.bbox;
        box.min_z = -4096;
        
        actor.setBounds(box.min_x, box.min_y, box.min_z, box.max_x, box.max_y, box.max_z);
    }
});
