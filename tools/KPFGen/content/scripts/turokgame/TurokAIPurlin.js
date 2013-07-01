//-----------------------------------------------------------------------------
//
// TurokAIPurlin.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const AI_ANIM_PURLIN_SPAWN_DROP         = 205;

TurokAIPurlin = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIPurlin,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 150,
    meleeRange      : 184.32,
    turnDirection   : 0,
    bloodType       : BLOOD_TYPE_HUMAN,
    rangeDistance   : 384.0,
    runRange        : 208.0,
    bCanMelee       : true,
    bCanRangeAttack : true,
    rangedAttacks   : FLAG_ATTACK_RANGE3,
    extendedRadius  : 2.5,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    stomp : function()
    {
        Snd.play('sounds/shaders/generic_139.ksnd', this.parent.owner);
    },
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/hulk_footfall.ksnd', this.parent.owner);
    },
    
    swooshSound : function()
    {
        Snd.play('sounds/shaders/knife_swish_1.ksnd', this.parent.owner);
    },
    
    alertScream: function()
    {
        Snd.play('sounds/shaders/hulk_alert.ksnd', this.parent.owner);
    },
    
    injuryScream : function()
    {
        Snd.play('sounds/shaders/hulk_injury.ksnd', this.parent.owner);
    },
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/hulk_death.ksnd', this.parent.owner);
    },
    
    violentDeathScream : function()
    {
        Snd.play('sounds/shaders/hulk_violent_death.ksnd', this.parent.owner);
    },
    
    spawnShockwave : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/projectile_shockwave.kfx', x, y, z);
    },
    
    shockwaveExplosion : function()
    {
        Snd.play('sounds/shaders/explosion_2.ksnd', this.parent.owner);
    },
    
    action_023 : function()
    {
        var actor = this.parent.owner;
        var ai = actor.ai;
        
        ai.setIdealYaw(actor.yaw + ai.yawToTarget(), 4.096);
    },
    
    fireWeapon : function()
    {
        var actor = this.parent.owner;
        var ai = actor.ai;
            
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        ai.setIdealYaw(actor.yaw + actor.ai.yawToTarget(), 5);
        ai.fireProjectile('fx/projectile_flame.kfx', x, y, z, Angle.degToRad(45), true);
        Snd.play('sounds/shaders/explosion_1.ksnd', this.parent.owner);
    },
    
    shockwaveDamage : function()
    {
        var actor = this.parent.owner;
        var ai = actor.ai;
        var target = ai.target;
        
        if(target == null)
            return;
        
        if(target.plane == -1)
            return;
            
        var torg = target.origin;
        var aorg = actor.getLocalVector(arguments[1], arguments[2], arguments[3]);
        
        if(torg.y - Plane.fromIndex(target.plane).distance(torg) <= 0.512)
        {
            var pos_y = torg.y - aorg.y;
            
            if(pos_y < 0.0)
                pos_y = -pos_y;
                
            if(pos_y <= 30.72)
            {
                var radius = arguments[0] * 10.24;
                var dist = ai.getTargetDistance();
                
                if(dist < radius)
                {
                    radius = (radius * radius);
                    radius = (radius - (dist * dist)) / radius;
                    
                    Damage.prototype.inflict(target, actor,
                        Math.round(radius * 10.0));
                }
            }
        }
    },
    
    strongKnockBack : function()
    {
        var actor = this.parent.owner;
        var ai = actor.ai;
        var target = ai.target;
        
        if(target == null)
            return;
            
        var torg = target.origin;
        var aorg = actor.getLocalVector(arguments[1], arguments[2], arguments[3]);
        
        aorg.y += (target.viewHeight * 0.5);
        torg.sub(aorg);
        
        if(torg.unit3() <= (10.0 * 10.24) + actor.radius)
        {
            torg.normalize();
            torg.scale(1075.2);
            
            var velocity = target.velocity;
            var origin = target.origin;
            velocity.add(torg);
            velocity.y = 537.6;
            
            // TODO - avoid doing this
            origin.y += 1.0;
            target.origin = origin;
            target.velocity = velocity;
        }
    },
    
    //------------------------------------------------------------------------
    // ANIMATION TWEEN FUNCTIONS
    //------------------------------------------------------------------------
    
    turnRunBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    //------------------------------------------------------------------------
    // AI EVENTS
    //------------------------------------------------------------------------
    
    onTrigger : function()
    {
        var actor = this.parent.owner;
        
        actor.physics = Physics.PT_DEFAULT;
        actor.ai.bDisabled = false;
        actor.bHidden = false;
        
        this.state = AI_STATE_DROPPING;
        actor.setAnim(AI_ANIM_PURLIN_SPAWN_DROP, 4.0, 0);
    }
});
