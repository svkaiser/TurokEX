//-----------------------------------------------------------------------------
//
// TurokAIInsect.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokAIInsect = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIInsect,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 10,
    meleeRange      : 204.8,
    bloodType       : BLOOD_TYPE_ALIEN,
    rangeDistance   : 0.0,
    bCanMelee       : true,
    lookNode        : 0,
    lookYawAxis_x   : 0,
    lookYawAxis_y   : 0,
    lookYawAxis_z   : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    meleeAttack : function(actor, ai)
    {
        return AI_ANIM_ATTACK_RANGE1;
    },
    
    walk : function(actor)
    {
        if(!actor.checkAnimID(AI_ANIM_WALKING))
            this.run(actor);
    },
    
    turnRunFront : function(actor, angles)
    {
    },
    
    run : function(actor)
    {
        this.state = AI_STATE_RUNNING;
        
        var anim = actor.checkAnimID(AI_ANIM_WALKING) ? AI_ANIM_WALKING : AI_ANIM_RUNNING;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    //------------------------------------------------------------------------
    // TURNING
    //------------------------------------------------------------------------
    
    turn : function(actor, angles)
    {
        if(this.health <= 0)
            return;
            
        var angle45 = Angle.degToRad(this.state == AI_STATE_STANDING ? 50 : 70);
        var angle135 = Angle.degToRad(150);
        
        var ai = actor.ai;
        
        if(ai.thinkTime <= 0.0)
            return;
            
        var an = angles;
            
        if(an < 0)
            an = -an;
        
        ai.setIdealYaw(actor.yaw + angles, (180.0 * (0.5 / ai.thinkTime)) * an);
        
        if(angles > angle45 || angles < -angle45)
        {
            actor.blendAnim(AI_ANIM_WALKING, 4.0, 8.0,
                NRender.ANIM_LOOP |
                NRender.ANIM_ROOTMOTION);
        }
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
});
