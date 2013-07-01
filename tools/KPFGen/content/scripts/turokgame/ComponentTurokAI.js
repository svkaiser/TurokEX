//-----------------------------------------------------------------------------
//
// ComponentTurokAI.js
//
// DESCRIPTION: Base class AI controller for actors. Determines what animations to
// play, callbacks to animation frame actions, enemy behaviors etc
//
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// Blood type constants
//------------------------------------------------------------------------
    
const BLOOD_TYPE_NONE           = 0;
const BLOOD_TYPE_HUMAN          = 1;    // TODO - Rename to BLOOD_TYPE_MAMMAL
const BLOOD_TYPE_ALIEN          = 2;
const BLOOD_TYPE_MECH           = 3;

//------------------------------------------------------------------------
// AI State constants
//------------------------------------------------------------------------

const AI_STATE_STANDING         = 0;
const AI_STATE_WALKING          = 1;
const AI_STATE_RUNNING          = 2;
const AI_STATE_ATTACK_MELEE     = 3;
const AI_STATE_ATTACK_RANGE     = 4;
const AI_STATE_DEATH            = 5;
const AI_STATE_DROPPING         = 6;

//------------------------------------------------------------------------
// Animation ID constants. These are shared across all AI actors
//------------------------------------------------------------------------

const AI_ANIM_STANDING          = 0;
const AI_ANIM_WALKING           = 1;
const AI_ANIM_RUNNING           = 2;

const AI_ANIM_TURN_L_STAND      = 3;
const AI_ANIM_TURN_R_STAND      = 4;
const AI_ANIM_TURN_B_STAND      = 6;
const AI_ANIM_TURN_L_WALK       = 7;
const AI_ANIM_TURN_R_WALK       = 8;
const AI_ANIM_TURN_B_WALK       = 9;
const AI_ANIM_TURN_L_RUN        = 10;
const AI_ANIM_TURN_R_RUN        = 11;
const AI_ANIM_TURN_B_RUN        = 12;

const AI_ANIM_MELEE_GENERIC1    = 18;
const AI_ANIM_MELEE_GENERIC2    = 19;
const AI_ANIM_MELEE_GENERIC3    = 20;
const AI_ANIM_MELEE_GENERIC4    = 21;
const AI_ANIM_MELEE_GENERIC5    = 22;
const AI_ANIM_MELEE_GENERIC6    = 25;
const AI_ANIM_MELEE_GENERIC7    = 26;

const AI_ANIM_ATTACK_RANGE1     = 24;
const AI_ANIM_ATTACK_RANGE2     = 59;
const AI_ANIM_ATTACK_RANGE3     = 121;
const AI_ANIM_ATTACK_RANGE4     = 122;
const AI_ANIM_ATTACK_RANGE5     = 123;
const AI_ANIM_ATTACK_RANGE6     = 124;
const AI_ANIM_ATTACK_RANGE7     = 141;
const AI_ANIM_ATTACK_RANGE8     = 142;
const AI_ANIM_ATTACK_RANGE9     = 150;
const AI_ANIM_ATTACK_RANGE10    = 151;

const AI_ANIM_DEATH_KNOCKBACK1  = 30;
const AI_ANIM_DEATH_KNOCKBACK2  = 31;
const AI_ANIM_DEATH_KNOCKBACK3  = 32;
const AI_ANIM_DEATH_KNOCKBACK4  = 33;

const AI_ANIM_DEATH_STAND       = 34;
const AI_ANIM_DEATH_VIOLENT     = 35;
const AI_ANIM_DEATH_RUNNING     = 37;

const AI_ANIM_SPAWN_DROP        = 202;

//------------------------------------------------------------------------
// Range attack flags
//------------------------------------------------------------------------

const FLAG_ATTACK_RANGE1        = 1;
const FLAG_ATTACK_RANGE2        = 2;
const FLAG_ATTACK_RANGE3        = 4;
const FLAG_ATTACK_RANGE4        = 8;
const FLAG_ATTACK_RANGE5        = 16;
const FLAG_ATTACK_RANGE6        = 32;
const FLAG_ATTACK_RANGE7        = 64;
const FLAG_ATTACK_RANGE8        = 128;
const FLAG_ATTACK_RANGE9        = 256;
const FLAG_ATTACK_RANGE10       = 512;

//------------------------------------------------------------------------
// Range attack animation object
//------------------------------------------------------------------------

var g_AnimRangeAttacks =
{
    animSets : [
        AI_ANIM_ATTACK_RANGE1,
        AI_ANIM_ATTACK_RANGE2,
        AI_ANIM_ATTACK_RANGE3,
        AI_ANIM_ATTACK_RANGE4,
        AI_ANIM_ATTACK_RANGE5,
        AI_ANIM_ATTACK_RANGE6,
        AI_ANIM_ATTACK_RANGE7,
        AI_ANIM_ATTACK_RANGE8,
        AI_ANIM_ATTACK_RANGE9,
        AI_ANIM_ATTACK_RANGE10
    ],
    
    weightSets : [95, 75, 20, 5, 95, 50, 95, 50, 100, 50],
    enabled : [true, true, true, true, true, true, true, true, true, true]
};

//------------------------------------------------------------------------
// Generic death animation object
//------------------------------------------------------------------------

var g_AnimDeath =
{
    animSets : [
        AI_ANIM_DEATH_STAND,
        AI_ANIM_DEATH_VIOLENT,
        AI_ANIM_DEATH_RUNNING
    ],
    
    weightSets : [18, 3, 8],
    enabled : [true, true, true]
};

//------------------------------------------------------------------------
// Generic melee animation object
//------------------------------------------------------------------------

var g_AnimMelee =
{
    animSets : [
        AI_ANIM_MELEE_GENERIC1,
        AI_ANIM_MELEE_GENERIC2,
        AI_ANIM_MELEE_GENERIC3,
        AI_ANIM_MELEE_GENERIC4,
        AI_ANIM_MELEE_GENERIC5,
        AI_ANIM_MELEE_GENERIC6,
        AI_ANIM_MELEE_GENERIC7
    ],
    
    weightSets : [10, 10, 10, 8, 8, 6, 6],
    enabled : [true, true, true, true, true, true, true]
};

//------------------------------------------------------------------------
// CLASS
//------------------------------------------------------------------------

ComponentTurokAI = class.extendStatic(Component);

class.properties(ComponentTurokAI,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active          : true,                 // component enabled or disabled?
    state           : AI_STATE_STANDING,    // determines current behavior for the AI
    targetID        : -1,                   // id of a actor to trigger
    health          : 0,                    // determines how much damage to take before dying
    bloodType       : BLOOD_TYPE_NONE,      // used to determine what type of blood to spawn
    meleeRange      : 0.0,                  // how far should the AI do a melee attack?
    runRange        : 184.25,               // how far should the AI start running after its target?
    rangeDistance   : 1024.0,               // how far should the AI do long range attacks?
    bCanMelee       : true,                 // if enabled, AI will try to reach its target
    bCanRangeAttack : false,                // if enabled, AI will try to do long range attacks
    bAttacking      : false,                // AI is currently attacking its target
    attackThreshold : 0,                    // how long before attacking again
    sightThreshold  : 0,                    // determines when to stop chasing if lost sight of target
    extendedRadius  : 1.5,                  // radius used for checking obsticles in addition to actor's main radius
    bTurning        : false,                // AI is playing a turning animation
    turnAnim        : 0,                    // animation ID of the current turning animation
    rangedAttacks   : 0,                    // range attack flags
    lookNode        : 0,                    // node ID for the AI's head
    lookYawAxis_x   : 0,
    lookYawAxis_y   : 1,
    lookYawAxis_z   : 0,
    meleeAnimObj    : g_AnimMelee,
    deathAnimObj    : g_AnimDeath,
    
    //------------------------------------------------------------------------
    // MELEE DAMAGE FUNCTIONS
    //------------------------------------------------------------------------
    
    wimpyMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageWimpyMelee);
    },
    
    weakMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageWeakMelee);
    },
    
    melee : function()
    {
        this.meleeDamage(this.parent.owner, DamageMelee);
    },
    
    strongMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageStrongMelee);
    },
    
    bluntMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageBluntMelee);
    },
    
    fleshMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageFleshMelee);
    },
    
    weakFleshMelee : function()
    {
        this.meleeDamage(this.parent.owner, DamageWeakFleshMelee);
    },
    
    meleeDamage : function(actor, damageClass)
    {
        var ai = actor.ai;
        var target = ai.target;
        var an = ai.yawToTarget();
        
        if(an <= 0.78 && an >= -0.78)
        {
            if(ai.getTargetDistance() <= (this.meleeRange * 1.024))
                damageClass.prototype.inflict(target, actor);
        }
    },
    
    //------------------------------------------------------------------------
    // GENERIC DAMAGE
    //------------------------------------------------------------------------
    
    customDamage : function()
    {
        var actor = this.parent.owner;
        var target = arguments[0];
        var amt = arguments[1];
        
        if(target == null)
            return;
        
        Damage.prototype.inflict(target, actor, amt);
    },
    
    weakExplosion : function()
    {
        var radius = arguments[1];
        var origin = arguments[2];
        var plane = arguments[4];
        
        BlastRadius.prototype.explode(this.parent.owner, origin.x, origin.y, origin.z,
            plane, radius, 10.0, 1.0, false);
    },
    
    //------------------------------------------------------------------------
    // SPECIAL EFFECTS
    //------------------------------------------------------------------------
    
    // random footstep fx/sfx
    footstepPuff : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/steppuff1.kfx', x, y, z);
    },
    
    action_103 : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/fx_051.kfx', x, y, z);
    },
    
    //------------------------------------------------------------------------
    // ANIMATION PICKER
    // Randomly picks an animation index by the weight value
    // animSetObj must be setup like g_AnimDeath or g_AnimMelee
    //------------------------------------------------------------------------
    
    animPicker : function(actor, animSetObj, total)
    {
        if(total <= 0)
            return animSetObj.animSets[0];
        
        // TODO - warn if arrays in animSetObj exceed 16
        var rndArray = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
        var randomSum = 0;
        var count = 0;
        var i;
        
        // go through and validate all animation IDs in the object
        for(i = 0; i < total; i++)
        {
            if(!animSetObj.enabled[i] ||
                !actor.checkAnimID(animSetObj.animSets[i]))
                continue;
            
            rndArray[count++] = randomSum;
            randomSum += animSetObj.weightSets[i];
        }
        
        var r = Sys.rand(randomSum);
        var animIdx = total - 1;
        
        count = count - 1;
        
        for(i = (total-1); i >= 0; i--)
        {
            if(!animSetObj.enabled[i] ||
                !actor.checkAnimID(animSetObj.animSets[i]))
            {
                // ignore disabled indexes
                animIdx--;
                continue;
            }
                    
            if(rndArray[count] > r)
            {
                animIdx--;
                count--;
                if(animIdx < 0 || count < 0)
                    break;
                
                continue;
            }
            break;
        }

        return animSetObj.animSets[animIdx];
    },
    
    //------------------------------------------------------------------------
    // RANGE ATTACK
    //------------------------------------------------------------------------
    
    tryRangeAttack : function(actor, ai)
    {
        var targ = ai.target;
        var dist = ai.getTargetDistance();
        var cr = 100;
        
        if(!this.bCanMelee)
            cr = 200;
        
        // attack more aggressively when close to target
        if(Sys.rand(cr) <= Math.round(dist * 100.0 / this.rangeDistance))
        {
            this.attackThreshold = 300;
            this.state = AI_STATE_STANDING;
            this.bAttacking = false;
            return;
        }
        
        var an = ai.yawToTarget();
        var inView = (an <= 0.5235 && an >= -0.5235);
        
        if(inView && ai.canSeeTarget())
        {
            if(dist <= this.rangeDistance && !this.bAttacking &&
                this.rangedAttacks != 0)
            {
                var f = this.rangedAttacks;
                
                for(var i = 0; i < 10; i++)
                    g_AnimRangeAttacks.enabled[i] = false;
                
                if(f & FLAG_ATTACK_RANGE1)
                    g_AnimRangeAttacks.enabled[0] = true;
                    
                if(f & FLAG_ATTACK_RANGE2)
                    g_AnimRangeAttacks.enabled[1] = true;
                    
                if(f & FLAG_ATTACK_RANGE3)
                    g_AnimRangeAttacks.enabled[2] = true;
                    
                if(f & FLAG_ATTACK_RANGE4)
                    g_AnimRangeAttacks.enabled[3] = true;
                    
                if(f & FLAG_ATTACK_RANGE5)
                    g_AnimRangeAttacks.enabled[4] = true;
                    
                if(f & FLAG_ATTACK_RANGE6)
                    g_AnimRangeAttacks.enabled[5] = true;
                    
                if(f & FLAG_ATTACK_RANGE7)
                    g_AnimRangeAttacks.enabled[6] = true;
                    
                if(f & FLAG_ATTACK_RANGE8)
                    g_AnimRangeAttacks.enabled[7] = true;
                    
                if(f & FLAG_ATTACK_RANGE9)
                    g_AnimRangeAttacks.enabled[8] = true;
                    
                if(f & FLAG_ATTACK_RANGE10)
                    g_AnimRangeAttacks.enabled[9] = true;
                
                anim = this.animPicker(actor, g_AnimRangeAttacks, 10);
                
                if(anim != -1)
                {
                    this.bAttacking = true;
                    this.state = AI_STATE_ATTACK_RANGE;
                    
                    actor.ai.setIdealYaw(actor.yaw + ai.yawToTarget(), 4.096);
                    actor.blendAnim(anim, 4.0, 8.0, NRender.ANIM_ROOTMOTION);
                }
            }
        }
        else if(this.bAttacking)
        {
            this.state = AI_STATE_STANDING;
            this.bAttacking = false;
        }
        
        // limit field of view
        if(!inView)
        {
            if(an > 0)
            {
                if(an <= 0.872)
                    an = 0.873;
            }
            else
            {
                if(an >= -0.872)
                    an = -0.873;
            }
            
            this.turn(actor, an);
        }
    },
    
    //------------------------------------------------------------------------
    // MELEE ATTACK
    //------------------------------------------------------------------------
    
    meleeAttack : function(actor, ai)
    {
        return this.animPicker(actor, this.meleeAnimObj, 6);
    },
    
    //------------------------------------------------------------------------
    // DEATH FUNCTIONS
    //------------------------------------------------------------------------
    
    deathAnim : function(actor)
    {
        // optional animation
        if(this.state == AI_STATE_RUNNING && !this.bAttacking)
            this.deathAnimObj.enabled[2] = true;
        else
            this.deathAnimObj.enabled[2] = false;

        return this.animPicker(actor, this.deathAnimObj, 3);
    },
    
    //------------------------------------------------------------------------
    // ANIMATION TWEEN FUNCTIONS
    //------------------------------------------------------------------------
    
    idle : function(actor)
    {
        this.state = AI_STATE_STANDING;
        actor.blendAnim(AI_ANIM_STANDING, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    walk : function(actor)
    {
        this.state = AI_STATE_WALKING;
        actor.blendAnim(AI_ANIM_WALKING, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    run : function(actor)
    {
        this.state = AI_STATE_RUNNING;
        actor.blendAnim(AI_ANIM_RUNNING, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_WALK, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunBack : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_B_RUN, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandRight : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_R_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkRight : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_R_WALK, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunRight : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_R_RUN, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandLeft : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_L_STAND, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkLeft : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_L_WALK, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunLeft : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_TURN_L_RUN, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandFront : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_STANDING, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkFront : function(actor, angles)
    {
        actor.blendAnim(AI_ANIM_WALKING, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
    },
    
    // run more aggressively while turning and while close to its target
    turnRunFront : function(actor, angles)
    {
        var angle10 = Angle.degToRad(4);
        var time = 0;
        var ai = actor.ai;
        var allowTurn = false;
        
        if(!ai.bSeeTarget || ai.target == null)
            return;

        if(!(angles <= angle10 && angles >= -angle10))
        {
            allowTurn = true;
            time = (2 - (Vector.length3(actor.origin,
                ai.target.origin) / 1024.0));
        }
        if(time < 0)
            time = 0;
        
        var turnSpeed = 5.05 + (time * 4);
        var speed = 4.0 - time;
                    
        actor.blendAnim(AI_ANIM_RUNNING, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
        actor.animState.frame = speed;
    },
    
    //------------------------------------------------------------------------
    // TURNING
    //------------------------------------------------------------------------
    
    turn : function(actor, angles)
    {
        if(this.health <= 0)
            return;
        
        // runners make sharper turns
        var angle45 = Angle.degToRad(this.state == AI_STATE_STANDING ? 50 : 70);
        var angle135 = Angle.degToRad(150);
        
        var ai = actor.ai;
        
        // simply face at target if not enough angles to do turning animations
        if(!this.bTurning && angles <= angle45 && angles >= -angle45)
        {
            if(ai.thinkTime <= 0.0)
                return;
                
            var an = angles;
                
            if(an < 0)
                an = -an;
            
            ai.bLookAtTarget = true;
            ai.setIdealYaw(actor.yaw + angles, (180.0 * (1.0 / ai.thinkTime)) * an);
            
            if(this.bAttacking)
                return;
            
            switch(this.state)
            {
            case AI_STATE_STANDING:
                this.turnStandFront(actor, angles);
                break;
                
            case AI_STATE_WALKING:
                this.turnWalkFront(actor, angles);
                break;
                
            case AI_STATE_RUNNING:
                this.turnRunFront(actor, angles);
                break;
            }
            return;
        }
        
        // don't interrupt attacks
        if(this.bAttacking || this.bTurning)
            return;
            
        if(ai.bSeeTarget && this.bCanMelee)
        {
            var dist = ai.getTargetDistance();
                        
            if(dist > this.runRange)
                this.state = AI_STATE_RUNNING;
            else if(dist > this.meleeRange)
                this.state = AI_STATE_WALKING;
            else
                this.state = AI_STATE_STANDING;
        }
        
        var curAnim = actor.animState.animID;
        
        // don't try turning if we're already blending an animation
        if(!(actor.animState.flags & NRender.ANIM_BLEND))
        {
            var anim;
            var speed;
            var turnSpeed;
            
            this.turnDirection = 0;
            
            if(angles < -angle135)
            {
                this.bTurning = true;
                ai.bTurning = false;
                ai.bLookAtTarget = false;
                
                switch(this.state)
                {
                case AI_STATE_STANDING:
                    this.turnStandBack(actor, angles);
                    break;
                    
                case AI_STATE_RUNNING:
                    this.turnRunBack(actor, angles);
                    break;
                    
                case AI_STATE_WALKING:
                    this.turnWalkBack(actor, angles);
                    break;
                }
                
                if(curAnim != actor.animState.animID)
                    this.turnAnim = actor.animState.animID;
            }
            else if(angles > angle45 || angles < -angle45)
            {
                this.bTurning = true;
                ai.bTurning = false;
                ai.bLookAtTarget = false;
                
                if(angles < 0)
                    this.turnDirection = 1;
                
                if(this.turnDirection == 0)
                {
                    switch(this.state)
                    {
                    case AI_STATE_STANDING:
                        this.turnStandRight(actor, angles);
                        break;
                        
                    case AI_STATE_RUNNING:
                        this.turnRunRight(actor, angles);
                        break;
                        
                    case AI_STATE_WALKING:
                        this.turnWalkRight(actor, angles);
                        break;
                    }
                }
                else
                {
                    switch(this.state)
                    {
                    case AI_STATE_STANDING:
                        this.turnStandLeft(actor, angles);
                        break;
                        
                    case AI_STATE_RUNNING:
                        this.turnRunLeft(actor, angles);
                        break;
                        
                    case AI_STATE_WALKING:
                        this.turnWalkLeft(actor, angles);
                        break;
                    }
                }
                
                if(curAnim != actor.animState.animID)
                    this.turnAnim = actor.animState.animID;
            }
            else
            {
                this.bTurning = false;
                
                switch(this.state)
                {
                case AI_STATE_STANDING:
                    this.turnStandFront(actor, angles);
                    break;
                    
                case AI_STATE_WALKING:
                    this.turnWalkFront(actor, angles);
                    break;
                    
                case AI_STATE_RUNNING:
                    this.turnRunFront(actor, angles);
                    break;
                    
                default:
                    return;
                }
            }
        }
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    // called after actor has been initialized in level
    onReady : function()
    {
        var actor = this.parent.owner;
        
        actor.physics = Physics.PT_DEFAULT;
        actor.bNoDropOff = true;
        actor.ai.bFindPlayers = true;
        actor.ai.bAvoidWalls = true;
        actor.ai.bLookAtTarget = true;
        actor.ai.nodeHead = this.lookNode;
        actor.ai.headYawAxis = new Vector(this.lookYawAxis_x, this.lookYawAxis_y, this.lookYawAxis_z);
        actor.setAnim(AI_ANIM_STANDING, 4.0, NRender.ANIM_LOOP);
        
        if(this.rangeDistance < 1.0)
            this.rangeDistance = 1.0;
        
        if(actor.plane != -1)
        {
            var origin = actor.origin;
            var plane = Plane.fromIndex(actor.plane);
            
            if(origin.y - plane.distance(origin) > (1024.0 - 1.024))
            {
                actor.physics = Physics.PT_NONE;
                actor.ai.bDisabled = true;
                actor.ai.bFindPlayers = false;
                actor.bHidden = true;
                return;
            }
        }
    },
    
    onTrigger : function()
    {
        var actor = this.parent.owner;
        
        if(!actor.checkAnimID(AI_ANIM_SPAWN_DROP))
            return;
        
        // enable physics and unhide it
        actor.physics = Physics.PT_DEFAULT;
        actor.ai.bDisabled = false;
        actor.bHidden = false;
        
        this.state = AI_STATE_DROPPING;
        actor.setAnim(AI_ANIM_SPAWN_DROP, 4.0, 0);
    },
    
    onDamage : function(instigator, amount, dmgClass)
    {
    },
    
    onDeath : function(instigator)
    {
        var self = this.parent.owner;
        
        if(this.state == AI_STATE_DEATH)
            return;
        
        self.bCollision = false;
        
        var anim = AI_ANIM_DEATH_STAND;
        
        anim = this.deathAnim(self);
        
        self.blendAnim(anim, 4.0, 4.0,
            NRender.ANIM_NOINTERRUPT | NRender.ANIM_ROOTMOTION);
        
        this.state = AI_STATE_DEATH;
        this.bTurning = false;
        
        self.ai.clearTarget();
        self.ai.bTurning = false;
        self.ai.bDisabled = true;
        self.bNoDropOff = false;
        self.velocity.clear();
        
        if(!(this.targetID <= -1))
            Level.triggerActors(this.targetID, 0);
    },
    
    onTick : function()
    {
        var actor = this.parent.owner;
        
        if(actor.timeStamp >= 1 || actor.timeStamp <= 0)
            return;
        
        // check if the AI thinks its still turning but not really
        // playing any turning animation
        // TODO - I shouldn't have to do this...
        if(this.bTurning)
        {
            if(actor.animState.flags & NRender.ANIM_STOPPED ||
                actor.animState.animID != this.turnAnim)
            {
                this.bTurning = false;
            }
        }
    },
    
    //------------------------------------------------------------------------
    // AI EVENTS
    //------------------------------------------------------------------------
    
    // AI is actively seeking its target
    chaseThink : function(actor, ai)
    {
        if(this.attackThreshold > 0)
            this.attackThreshold -= 15;
        
        // handle melee and ranged attacks
        if(this.state != AI_STATE_DROPPING && !this.bTurning &&
            !this.bAttacking && this.health > 0)
        {
            if(this.bCanMelee && ai.getTargetDistance() <= this.meleeRange)
            {
                if(!this.bAttacking)
                {
                    var angle = ai.yawToTarget();
                    
                    // try to look at its target
                    if(!(angle <= 0.78 && angle >= -0.78))
                        this.turn(actor, angle);
                    else
                    {
                        var anim = this.meleeAttack(actor, ai);
                        
                        if(anim != -1)
                        {
                            this.bAttacking = true;
                            ai.bLookAtTarget = false;
                            actor.ai.setIdealYaw(actor.yaw + ai.yawToTarget(), 4.096);
                            this.state = AI_STATE_ATTACK_MELEE;
                            actor.blendAnim(anim, 4.0, 8.0, NRender.ANIM_ROOTMOTION);
                        }
                    }
                }
            }
            else if(this.bCanRangeAttack && this.attackThreshold <= 0) {
                this.tryRangeAttack(actor, ai);
            }
        }
        
        // handle states here
        switch(this.state)
        {
        ////////////////////////////////////////////////////
        // STANDING STATE
        ////////////////////////////////////////////////////
        case AI_STATE_STANDING:
            if(!this.bTurning && !this.bAttacking && this.bCanMelee)
            {
                var dist = ai.getTargetDistance();
                
                if(dist > this.runRange)
                    this.run(actor);
                else if(dist > this.meleeRange)
                    this.walk(actor);
            }
            if(ai.bSeeTarget)
                this.turn(actor, ai.yawToTarget());
            else
                this.turn(actor, ai.getBestAngleToTarget(this.extendedRadius));
            break;
        
        ////////////////////////////////////////////////////
        // CHASING STATE
        ////////////////////////////////////////////////////
        case AI_STATE_RUNNING:
            if(!ai.bSeeTarget && (this.sightThreshold-- <= 0))
            {
                // when AI gives up, go back to walking state
                this.bTurning = false;
                this.walk(actor);
            }
            if(!this.bTurning && !this.bAttacking)
            {
                var an = ai.yawToTarget();
    
                if(!(an <= 0.78 && an >= -0.78))
                {
                    var dist = ai.getTargetDistance();
                    
                    if(dist <= this.meleeRange)
                        this.idle(actor);
                    else if(dist <= this.runRange)
                        this.walk(actor);
                }
            }
            this.turn(actor, ai.getBestAngleToTarget(this.extendedRadius));
            break;
            
        ////////////////////////////////////////////////////
        // WALKING STATE
        ////////////////////////////////////////////////////
        case AI_STATE_WALKING:
            if(!this.bTurning && !this.bAttacking && this.sightThreshold > 0)
            {
                var dist = ai.getTargetDistance();
                
                if(dist > this.runRange)
                    this.run(actor);
                else if(dist <= this.meleeRange)
                    this.idle(actor);
            }
            
            if(!ai.bSeeTarget && this.sightThreshold <= 0 && Sys.rand(1000) >= 995)
            {
                // AI has forgotten about it's target. go back to idling
                ai.clearTarget();
                this.bTurning = false;
                this.bAttacking = false;
                this.idle(actor);
            }
            else
                this.turn(actor, ai.getBestAngleToTarget(this.extendedRadius));
            break;
            
        ////////////////////////////////////////////////////
        // MELEE STATE
        ////////////////////////////////////////////////////
        case AI_STATE_ATTACK_MELEE:
            if(actor.animState.flags & NRender.ANIM_STOPPED)
            {
                this.state = AI_STATE_STANDING;
                this.bAttacking = false;
                this.bTurning = false;
                return;
            }
            break;
            
        ////////////////////////////////////////////////////
        // RANGE STATE
        ////////////////////////////////////////////////////
        case AI_STATE_ATTACK_RANGE:
            if(actor.animState.flags & NRender.ANIM_STOPPED)
            {
                this.state = AI_STATE_STANDING;
                this.bAttacking = false;
                this.bTurning = false;
                this.tryRangeAttack(actor, ai);
                return;
            }
            if(!this.bCanMelee)
                this.turn(actor, ai.yawToTarget());
            break;
        }
    },
    
    // AI lost sight of its target and will begin to fumble around
    wanderThink : function(actor, ai)
    {
        // bump against walls. will ignore if it sees a target
        if(ai.bAvoidWalls && !this.bTurning)
        {
            if(!ai.checkPosition(this.extendedRadius, actor.yaw + 0.5236)) {
                this.turn(actor, -1.22);
            }
            else if(!ai.checkPosition(this.extendedRadius, actor.yaw - 0.5236)) {
                this.turn(actor, 1.22);
            }
        }
        
        if(!this.bTurning)
        {
            if(Sys.rand(100) <= 4)
                this.turn(actor, 1.22 + (Sys.fRand() * Math.PI));
            
            if(Sys.rand(1000) >= 998)
            {
                switch(this.state)
                {
                case AI_STATE_STANDING:
                    if(Sys.rand(100) <= 10)
                        this.walk(actor);
                    else
                        this.run(actor);
                    break;
                    
                case AI_STATE_WALKING:
                case AI_STATE_RUNNING:
                    this.idle(actor);
                    break;
                }
            }
        }
    },
    
    onThink : function()
    {
        var ai = this.parent.owner.ai;
        var actor = ai.owner;
        
        if(this.state == AI_STATE_DROPPING)
        {
            // don't do nothing until the drop animation has finished
            if(!(actor.animState.flags & NRender.ANIM_STOPPED))
                return;
                
            // wake up the AI
            this.sightThreshold = 100;
            this.run(actor);
            ai.bFindPlayers = true;
        }
        
        if(ai.bSeeTarget)
        {
            this.sightThreshold++;
            if(this.sightThreshold > 100)
                this.sightThreshold = 100;
        }
        else if(this.sightThreshold <= 50 && this.bAttacking)
            this.bAttacking = false;
        
        if(ai.target != null)
            this.chaseThink(actor, ai);
        else
            this.wanderThink(actor, ai);
        
        // try to break out of the turning animations when we're not actually turning
        // TODO - I shouldn't have to do this...
        if(!this.bTurning && actor.animState.flags & NRender.ANIM_STOPPED)
        {
            switch(this.state)
            {
            case AI_STATE_STANDING:
                this.idle(actor);
                break;
                
            case AI_STATE_WALKING:
                this.walk(actor);
                break;
                
            case AI_STATE_RUNNING:
                this.run(actor);
                break;
            }
        }
    },
    
    // callback when AI has spotted its target
    onTargetFound : function()
    {
        if(this.state == AI_STATE_DROPPING)
            return;
        
        if(this.bCanMelee)
            this.run(this.parent.owner);
        else
            this.idle(this.parent.owner);
        
        this.sightThreshold = 100;
    },
    
    // callback when AI is out of the player's range
    onSleep : function()
    {
        var self = this.parent.owner;
        
        if(!self.ai.bDisabled)
            return;
        
        this.idle(self);
        this.bTurning = false;
        this.bAttacking = false;
        
        // give up its target
        self.ai.clearTarget();
        self.ai.bTurning = false;
    },
    
    // callback when AI is within the player's range
    onWake : function()
    {
        var actor = this.parent.owner;
        
        if(!actor.ai.bDisabled)
            return;
        
        this.sightThreshold = 100;
        this.bTurning = false;
        
        if(this.bCanMelee && actor.ai.target != null)
            this.run(actor);
        else
            this.idle(actor);
    }
});
