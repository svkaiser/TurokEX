//-----------------------------------------------------------------------------
//
// ComponentScriptedActor.js
//
// DESCRIPTION: Base class for all Scripted Actor objects. Scripted Actors
// can be used to trigger special one-offs or function as a door or switch
//
//-----------------------------------------------------------------------------

ComponentScriptedActor = class.extendStatic(Component);

class.properties(ComponentScriptedActor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    triggerAnimation    : "",
    bTriggered          : false,
    bRemoveOnCompletion : true,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // used to unblock a path when a door opens
    // shared across all door types
    action_232 : function()
    {
        var actor = this.parent.owner;
        
        if(actor.plane == -1)
            return;
        
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        var plane = Plane.fromIndex(actor.plane);
        var vec = actor.getLocalVector(x, y, z);
        var len = Math.round(vec.unit3());
        var origin = actor.origin;
        
        vec.sub(origin);
        vec.normalize();
        
        // looked at the disassembly but not sure why they would do it like this..
        // seems like a workaround due to its origin being far off from the door mesh.
        // find the plane facing the origin and then toggle the solid/blocking flags off
        var trace = Physics.rayTrace(origin, vec, 1, len, plane);
            
        if(trace && trace.hitPlane)
            plane = trace.hitPlane;
            
        Level.toggleBlockingPlanes(plane, true);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        var flags = NRender.ANIM_NOINTERRUPT;
        
        if(this.bRootMotion == true)
            flags |= NRender.ANIM_ROOTMOTION;
        
        actor.blendAnim(this.triggerAnimation, 4.0, 4.0, flags);
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
        this.parent.owner.setAnim("anim00", 4.0, NRender.ANIM_LOOP);
    },
    
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
        this.parent.owner.bHidden = true;
    },
    
    onTrigger : function(instigator, args)
    {
        var actor = this.parent.owner;
        actor.bHidden = false;
        
        var flags = NRender.ANIM_NOINTERRUPT;
        
        if(this.bRootMotion == true)
            flags |= NRender.ANIM_ROOTMOTION;
        
        actor.setAnim(this.triggerAnimation, 4.0, flags);
        
        // TODO - handle bounding box updates in engine
        var box = actor.bbox;
        box.min_z = -4096;
        
        actor.setBounds(box.min_x, box.min_y, box.min_z, box.max_x, box.max_y, box.max_z);
    }
});

//-----------------------------------------------------------------------------
//
// ScriptedSwingingHook.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ScriptedSwingingHook = class.extendStatic(ComponentScriptedActor);

class.properties(ScriptedSwingingHook,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bRemoveOnCompletion : false,
    bRootMotion         : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        this.parent.owner.setAnim("anim00", 4.0, NRender.ANIM_LOOP);
    },
    
    // kinda sucks that I have to define the same function twice. though it would probably
    // make more since to have the AI component inherit off of the scripted actor component
    melee : function()
    {
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        var self = this.parent.owner;
        var target = ClientPlayer.actor;
        var torg = target.origin;
        var aorg = self.getLocalVector(x, y, z);
        
        aorg.y += (target.viewHeight * 0.5);
        torg.sub(aorg);
        
        if(torg.unit3() <= (10.0 * 10.24) + target.radius)
            DamageMelee.prototype.inflict(ClientPlayer.actor, self);
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
