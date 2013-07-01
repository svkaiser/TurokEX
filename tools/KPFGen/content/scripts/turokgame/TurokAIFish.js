//-----------------------------------------------------------------------------
//
// TurokAIFish.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const AI_ANIM_FISH_SWIM         = 2500;
const AI_ANIM_FISH_SWIM_FAST    = 2505;

TurokAIFish = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIFish,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    extendedRadius  : 1.0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    idle : function(actor)
    {
        this.state = AI_STATE_STANDING;
        actor.blendAnim(AI_ANIM_FISH_SWIM, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    walk : function(actor)
    {
        this.state = AI_STATE_WALKING;
        actor.blendAnim(AI_ANIM_FISH_SWIM, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    run : function(actor)
    {
        this.state = AI_STATE_RUNNING;
        actor.blendAnim(AI_ANIM_FISH_SWIM_FAST, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    turn : function(actor, angles)
    {
        if(actor.ai.bTurning)
            return;
        
        actor.ai.setIdealYaw(actor.yaw + angles, 2.048);
    },
    
    onTargetFound : function()
    {
        this.idle(this.parent.owner);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        this.parent.owner.physics = Physics.PT_DEFAULT;
        
        var actor = this.parent.owner;
        
        actor.mass = 0.0;
        actor.bNoDropOff = false;
        actor.ai.bFindPlayers = false;
        actor.ai.bAvoidWalls = true;
        actor.ai.bAvoidActors = true;
        actor.setAnim(AI_ANIM_FISH_SWIM, 4.0, NRender.ANIM_LOOP | NRender.ANIM_ROOTMOTION);
    }
});
