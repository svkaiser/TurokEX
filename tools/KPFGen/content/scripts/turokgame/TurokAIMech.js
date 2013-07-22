//-----------------------------------------------------------------------------
//
// TurokAIMech.js
//
// DESCRIPTION: Mech AI
//
//-----------------------------------------------------------------------------

TurokAIMech = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIMech,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 150,
    meleeRange      : 122.88,
    runRange        : 180.0,
    bloodType       : BLOOD_TYPE_MECH,
    rangeDistance   : 1024.0,
    bCanMelee       : true,
    bCanRangeAttack : true,
    rangedAttacks   : FLAG_ATTACK_RANGE2,
    lookNode        : 0,
    lookYawAxis_x   : 0,
    lookYawAxis_y   : 0,
    lookYawAxis_z   : 0,
    
    //------------------------------------------------------------------------
    // ANIMATION TWEEN FUNCTIONS
    //------------------------------------------------------------------------
    
    run : function(actor)
    {
        this.state = AI_STATE_RUNNING;
        actor.blendAnim(AI_ANIM_WALKING, 4.0, 8.0,
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
        
        if(actor.animState.animID == AI_ANIM_WALKING && actor.animState.frame > 1)
            return;
        
        var ai = actor.ai;
        
        if(ai.thinkTime <= 0.0)
            return;
        
        var angle45 = Angle.degToRad(this.state == AI_STATE_STANDING ? 50 : 70);
        var curAnim = actor.animState.animID;
        
        // don't try turning if we're already blending an animation
        if(!(actor.animState.flags & NRender.ANIM_BLEND))
        {
            var anim;
            var speed;
            var turnSpeed;
            
            this.turnDirection = 0;
            
            if(angles > angle45 || angles < -angle45)
            {
                this.bTurning = true;
                ai.bTurning = false;
                ai.bLookAtTarget = true;
                
                if(angles < 0)
                    this.turnDirection = 1;
                
                if(this.turnDirection == 0)
                    this.turnStandRight(actor, angles);
                else
                    this.turnStandLeft(actor, angles);
                
                if(curAnim != actor.animState.animID)
                    this.turnAnim = actor.animState.animID;
                    
                return;
            }
        }
        
        if(!this.bTurning)
            ai.setIdealYaw(actor.yaw + angles, 7.85);
    },
    
    //------------------------------------------------------------------------
    // ACTION CALLBACKS
    //------------------------------------------------------------------------
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/robot_footfall.ksnd', this.parent.owner);
    },
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/robot_shutdown.ksnd', this.parent.owner);
    },
    
    injuryScream : function()
    {
        Snd.play('sounds/shaders/robot_short_1.ksnd', this.parent.owner);
    },
    
    roboShutdown : function()
    {
        Snd.play('sounds/shaders/robot_shutdown.ksnd', this.parent.owner);
    },
    
    fireWeapon : function()
    {
        var actor = this.parent.owner; 
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        if(arguments[0] == 1)
        {
            Snd.play('sounds/shaders/auto_shotgun_shot.ksnd', actor);
            actor.ai.fireProjectile('fx/projectile_enemy_shell.kfx', x, y, z, Angle.degToRad(45), true);
        }
        else
        {
            Snd.play('sounds/shaders/missile_launch.ksnd', actor);
            actor.ai.fireProjectile('fx/projectile_scatter_blast.kfx', x, y, z, Angle.degToRad(45), true);
        }
    },
    
    action_023 : function()
    {
    }
});
