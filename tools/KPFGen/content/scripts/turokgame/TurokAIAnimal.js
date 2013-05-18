//-----------------------------------------------------------------------------
//
// TurokAIAnimal.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const ANIMAL_STATE_IDLE     = 0;
const ANIMAL_STATE_FLEE     = 1;
const ANIMAL_STATE_DEATH    = 2;

TurokAIAnimal = class.extendStatic(ComponentTurokAI, function()
{
    ComponentTurokAI.prototype.constructor.bind(this)();
});

class.properties(TurokAIAnimal,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    activeDistance  : 0,
    anim_Idle       : null,
    anim_Run        : null,
    anim_Death      : null,
    state           : ANIMAL_STATE_IDLE,
    health          : 0,
    fleeLoopFrame   : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    think : function(actor)
    {
        if(this.checkTargetRange(ClientPlayer.actor) == false)
        {
            if(this.state == ANIMAL_STATE_FLEE)
            {
                this.state = ANIMAL_STATE_IDLE;
                this.bTurning = false;
                actor.animState.blendAnim(this.anim_Idle, 4.0, 8.0,
                    NRender.ANIM_NOINTERRUPT |
                    NRender.ANIM_LOOP |
                    NRender.ANIM_ROOTMOTION);
            }
            return;
        }
        
        var actor = this.parent.owner;
        var aorg = this.controller.origin;
        var torg = ClientPlayer.actor.origin;
        
        this.controller.setDirection(actor.pitch, actor.yaw, actor.roll);
        
        switch(this.state)
        {
            case ANIMAL_STATE_IDLE:
                if(!(actor.animState.flags & NRender.ANIM_BLEND) && !this.bTurning)
                {
                    var an = this.anglesToTarget(torg, aorg);
            
                    if(an < 1.57 && an > -1.57)
                    {
                        this.state = ANIMAL_STATE_FLEE;
                        actor.animState.blendAnim(this.anim_Run, 4.0, 8.0,
                            NRender.ANIM_NOINTERRUPT |
                            NRender.ANIM_ROOTMOTION |
                            NRender.ANIM_LOOP);
                            
                        actor.animState.restartFrame = this.fleeLoopFrame;
                        this.bTurning = true;
                        this.destAngle =
                            this.angleRound(Vector.pointToAxis(torg, aorg).toYaw() + Math.PI);
                    }
                }
                break;
                
            case ANIMAL_STATE_FLEE:
                if(!(actor.animState.flags & NRender.ANIM_BLEND))
                { 
                    if(this.bTurning == false)
                    {
                        var yaw = this.controller.angles.yaw;
                        var an = Angle.degToRad(150);
                        
                        if(!this.checkPosition(this.controller.origin, 1.5, yaw + an))
                        {
                            this.bTurning = true;
                            this.destAngle = this.angleRound(yaw + an);
                        }
                        
                        if(!this.checkPosition(this.controller.origin, 1.5, yaw - an))
                        {
                            this.bTurning = true;
                            this.destAngle = this.angleRound(yaw - an);
                        }
                    }
                    
                    if(this.bTurning == true)
                        this.changeAngle(this.destAngle, 2.03125 * actor.timeStamp);
                }
                break;
                
            case ANIMAL_STATE_DEATH:
                break;
        }
        
        this.rootMotionMove();
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        var actor = this.parent.owner;
        
        this.anim_Idle      = Sys.loadAnimation(actor.model, "anim00");
        this.anim_Run       = Sys.loadAnimation(actor.model, "anim01");
        this.anim_Death     = Sys.loadAnimation(actor.model, "anim02");
        
        ComponentTurokAI.prototype.onReady.bind(this)();
        
        actor.animState.setAnim(this.anim_Idle, 4.0, NRender.ANIM_LOOP);
    },
    
    onDamage : function(instigator)
    {
    },
    
    onDeath : function(instigator)
    {
        var actor = this.parent.owner;
        
        this.state = ANIMAL_STATE_DEATH;
        actor.bCollision = false;
        actor.animState.blendAnim(this.anim_Death, 4.0, 4.0,
            NRender.ANIM_NOINTERRUPT | NRender.ANIM_LOOP);
    },
});
