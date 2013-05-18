//-----------------------------------------------------------------------------
//
// ComponentScriptedActor.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentScriptedActor = class.extendStatic(Component);

class.properties(ComponentScriptedActor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation    : "",
    bRemoveOnCompletion : true,
    anim                : null,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        this.anim = Sys.loadAnimation(this.parent.owner.model, this.triggerAnimation);
    },
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        var flags = NRender.ANIM_NOINTERRUPT;
        
        if(this.bRootMotion == true)
            flags |= NRender.ANIM_ROOTMOTION;
        
        actor.animState.blendAnim(this.anim, 4.0, 4.0, flags);
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
        
        actor.animState.update();
        
        if(actor.animState.flags & NRender.ANIM_ROOTMOTION &&
            !(actor.animState.flags & NRender.ANIM_STOPPED))
        {
            var dir = Vector.applyRotation(actor.animState.rootMotion, actor.rotation);
            dir.y = 0;
            dir.scale(Sys.deltatime() * 0.5);
            dir.add(actor.origin);
            
            actor.origin = dir;
        }
        
        actor.updateTransform();
    }
});

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
    
    triggerAnimation    : "anim01",
    bRemoveOnCompletion : true,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        var actor = this.parent.owner;
        
        this.anim = Sys.loadAnimation(actor.model, this.triggerAnimation);
        actor.animState.setAnim(Sys.loadAnimation(actor.model, "anim00"),
            4.0, NRender.ANIM_LOOP);
    },
    
    onTrigger : function(instigator, args)
    {
        ComponentScriptedActor.prototype.onTrigger.bind(this)();
        
        var actor = this.parent.owner;
        
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
    
    triggerAnimation    : "anim00",
    bRemoveOnCompletion : true,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        var actor = this.parent.owner;
        
        this.anim = Sys.loadAnimation(actor.model, this.triggerAnimation);
        actor.bHidden = true;
    },
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        actor.bHidden = false;
        
        var flags = NRender.ANIM_NOINTERRUPT;
        
        if(this.bRootMotion == true)
            flags |= NRender.ANIM_ROOTMOTION;
        
        actor.animState.setAnim(this.anim, 4.0, flags);
        
        var box = actor.bbox;
        box.min_z = -4096;
        
        actor.setBounds(box.min_x, box.min_y, box.min_z, box.max_x, box.max_y, box.max_z);
    }
});

//-----------------------------------------------------------------------------
//
// ScriptedPressurePlate.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ScriptedPressurePlate = class.extendStatic(ComponentScriptedActor);

class.properties(ScriptedPressurePlate,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation    : "anim01",
    bRemoveOnCompletion : false,
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
    }
});
