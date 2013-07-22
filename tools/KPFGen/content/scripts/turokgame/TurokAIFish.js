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
            
        if((actor.animState.animID == AI_ANIM_WALKING ||
            actor.animState.animID == AI_STATE_RUNNING) && actor.animState.frame > 1)
            return;
            
        var maxTurn = Angle.degToRad(50);
        
        if(angles > maxTurn)
            angles = maxTurn;
            
        if(angles < -maxTurn)
            angles = maxTurn;
        
        actor.ai.setIdealYaw(actor.yaw + angles, 4.096);
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
        var actor = this.parent.owner;
        
        if(actor.physics == 0)
        {
            actor.physics = (
                Physics.PF_CLIPSTATICS |
                Physics.PF_CLIPGEOMETRY |
                Physics.PF_CLIPEDGES |
                Physics.PF_SLIDEMOVE |
                Physics.PF_NOEXITWATER);
        }
        
        actor.mass = 0.0;
        actor.ai.bFindPlayers = false;
        actor.ai.bAvoidWalls = true;
        actor.ai.bAvoidActors = true;
        actor.setAnim(AI_ANIM_FISH_SWIM, 4.0, NRender.ANIM_LOOP | NRender.ANIM_ROOTMOTION);
    }
});
