//-----------------------------------------------------------------------------
//
// TurokAIGrunt.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const GRUNT_STATE_IDLE      = 0;
const GRUNT_STATE_DEATH     = 1;

TurokAIGrunt = class.extendStatic(ComponentTurokAI, function()
{
    ComponentTurokAI.prototype.constructor.bind(this)();
});

class.properties(TurokAIGrunt,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    activeDistance  : 1126.4,
    anim_Idle       : null,
    anim_Walk       : null,
    anim_Run        : null,
    anim_STurn_L    : null,
    anim_STurn_R    : null,
    anim_STurn180   : null,
    anim_Death1     : null,
    state           : GRUNT_STATE_IDLE,
    destAngle       : 0.0,
    lookAngle       : 0.0,
    bTurning        : false,
    health          : 14,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    anglesToTarget : function(torg, aorg)
    {
        return Angle.invertClampSum(this.controller.angles.yaw,
            Vector.pointToAxis(torg, aorg).toYaw());
    },
    
    changeYaw : function(time)
    {
        this.controller.angles.yaw = Angle.clamp(this.controller.angles.yaw +
            (this.destAngle * time));
    },
    
    think : function(actor)
    {
        if(this.checkTargetRange(ClientPlayer.actor) == false)
            return;
        
        var aorg = this.controller.origin;
        var torg = ClientPlayer.actor.origin;
        
        switch(this.state)
        {
        case GRUNT_STATE_IDLE:
            if(!(actor.animState.flags & NRender.ANIM_BLEND) && !this.bTurning)
            {
                const ANGLE_TURN45_DELTA = Angle.degToRad(45);
                const ANGLE_TURN180_DELTA = Angle.degToRad(135);
                
                var bRight = false;
                
                var an = this.anglesToTarget(torg, aorg);
                
                if(an < 0)
                {
                    an = -an;
                    bRight = true;
                }
                
                if(an > ANGLE_TURN180_DELTA)
                {
                    this.bTurning = true;
                    this.destAngle = an * 0.908;
                    actor.animState.blendAnim(this.anim_STurn180, 4.0, 4.0,
                        NRender.ANIM_NOINTERRUPT | NRender.ANIM_ROOTMOTION);
                }
                else if(an > ANGLE_TURN45_DELTA)
                {
                    var turn;
                    
                    if(!bRight)
                    {
                        this.destAngle = an * 0.95;
                        turn = this.anim_STurn_R;
                    }
                    else
                    {
                        this.destAngle = an * 0.582;
                        this.destAngle = -this.destAngle;
                        turn = this.anim_STurn_L;
                    }
                    
                    this.bTurning = true;
                    actor.animState.blendAnim(turn, 4.0, 4.0,
                        NRender.ANIM_NOINTERRUPT | NRender.ANIM_ROOTMOTION);
                }
            }
            break;
        case GRUNT_STATE_DEATH:
            break;
        default:
            break;
        }
        
        if(this.bTurning == true)
            this.changeYaw(actor.timeStamp);
        
        if(actor.animState.flags & NRender.ANIM_ROOTMOTION &&
            !(actor.animState.flags & NRender.ANIM_STOPPED))
        {
            var dir = Vector.applyRotation(actor.animState.rootMotion, actor.rotation);
            dir.y = 0;
            dir.scale(0.5);
            this.controller.velocity.add(dir);
        }
    },
    
    death : function()
    {
        var actor = this.parent.owner;
        
        this.state = GRUNT_STATE_DEATH;
        actor.bCollision = false;
        this.bTurning = false;
        this.controller.velocity.clear();
        
        var rnd = Sys.rand(3);
        
        switch(rnd)
        {
            case 0:
                Snd.play('sounds/shaders/human_death_scream_3.ksnd', actor);
                break;
            case 1:
                Snd.play('sounds/shaders/human_death_scream_2.ksnd', actor);
                break;
            case 2:
                Snd.play('sounds/shaders/human_death_scream_1.ksnd', actor);
                break;
            default:
                break;
        }
        
        actor.animState.blendAnim(this.anim_Death1, 4.0, 4.0,
            NRender.ANIM_NOINTERRUPT | NRender.ANIM_ROOTMOTION);
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    start : function()
    {
    },
    
    onReady : function()
    {
        var actor = this.parent.owner;
        
        this.anim_Idle      = Sys.loadAnimation(actor.model, "anim00");
        this.anim_Walk      = Sys.loadAnimation(actor.model, "anim01");
        this.anim_Run       = Sys.loadAnimation(actor.model, "anim02");
        this.anim_STurn_L   = Sys.loadAnimation(actor.model, "anim03");
        this.anim_STurn_R   = Sys.loadAnimation(actor.model, "anim04");
        this.anim_STurn180  = Sys.loadAnimation(actor.model, "anim05");
        this.anim_Death1    = Sys.loadAnimation(actor.model, "anim21");
        
        ComponentTurokAI.prototype.onReady.bind(this)();
        
        actor.animState.setAnim(this.anim_Idle, 4.0, NRender.ANIM_LOOP);
    },
    
    onDamage : function(instigator)
    {
        this.health -= 7;
        if(this.health <= 0)
            this.death();
    },
    
    onTick : function()
    {
        var actor = this.parent.owner;
        
        if(actor.timeStamp >= 1 || actor.timeStamp <= 0)
            return;
            
        var c = this.controller;
        
        c.updateFromActor(actor, this);
        
        var plane = c.plane;
        
        if(plane != null)
            this.think(actor);
        
        c.gravity(c.mass);
        c.beginMovement();
        c.updateActor(actor);
        c.applyFriction(c.friction);
    },
    
    onLocalTick : function()
    {
        var actor = this.parent.owner;
        
        if(this.bTurning == true)
        {
            switch(this.state)
            {
                case GRUNT_STATE_IDLE:
                    if(actor.animState.flags & NRender.ANIM_STOPPED)
                    {
                        this.bTurning = false;
                        actor.animState.blendAnim(this.anim_Idle, 4.0, 8.0,
                            NRender.ANIM_NOINTERRUPT |
                            NRender.ANIM_LOOP |
                            NRender.ANIM_ROOTMOTION);
                    }
                    break;
                case GRUNT_STATE_DEATH:
                    break;
                default:
                    break;
            }
        }
        
        actor.animState.update();
    }
});
