//-----------------------------------------------------------------------------
//
// TurokAIDimetrodon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokAIDimetrodon = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIDimetrodon,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 50,
    meleeRange      : 122.88,
    runRange        : 163.84,
    bloodType       : BLOOD_TYPE_HUMAN,
    rangeDistance   : 0.0,
    bCanMelee       : true,
    lookNode        : 11,
    lookYawAxis_x   : 0,
    lookYawAxis_y   : 0,
    lookYawAxis_z   : -1.0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/dimetrodon_footfall.ksnd', this.parent.owner);
    },
    
    violentDeathScream : function()
    {
        Snd.play('sounds/shaders/dimetrodon_violent_death.ksnd', this.parent.owner);
    },
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/dimetrodon_death.ksnd', this.parent.owner);
    },
    
    walk : function(actor)
    {
        this.state = AI_STATE_WALKING;
        actor.blendAnim(AI_ANIM_RUNNING, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkLeft : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_L_RUN, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkRight : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_R_RUN, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkFront : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_RUNNING, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
    }
});
