//-----------------------------------------------------------------------------
//
// TurokAIGrunt.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// ANIMATION ID CONSTANTS
//------------------------------------------------------------------------

const AI_ANIM_GRUNT_MELEE1                  = 2700;
const AI_ANIM_GRUNT_MELEE2                  = 2701;
const AI_ANIM_GRUNT_MELEE3                  = 2702;

const AI_ANIM_GRUNT_TWOHANDED_MELEE1        = 19;
const AI_ANIM_GRUNT_TWOHANDED_MELEE2        = 20;
const AI_ANIM_GRUNT_TWOHANDED_MELEE3        = 21;
const AI_ANIM_GRUNT_TWOHANDED_MELEE4        = 22;

const AI_ANIM_GRUNT_DEATH_TWOHANDED_STAND   = 143;
const AI_ANIM_GRUNT_DEATH_TWOHANDED_VIOLENT = 144;
const AI_ANIM_GRUNT_DEATH_TWOHANDED_RUNNING = 145;

const AI_ANIM_GRUNT_TWOHANDED_STAND         = 129;
const AI_ANIM_GRUNT_TWOHANDED_WALK          = 130;
const AI_ANIM_GRUNT_TWOHANDED_RUN           = 131;

const AI_ANIM_GRUNT_TWOHANDED_TURN_L_STAND  = 132;
const AI_ANIM_GRUNT_TWOHANDED_TURN_R_STAND  = 133;
const AI_ANIM_GRUNT_TWOHANDED_TURN_B_STAND  = 134;

const AI_ANIM_GRUNT_TWOHANDED_TURN_L_WALK   = 135;
const AI_ANIM_GRUNT_TWOHANDED_TURN_R_WALK   = 136;
const AI_ANIM_GRUNT_TWOHANDED_TURN_B_WALK   = 137;

const AI_ANIM_GRUNT_TWOHANDED_TURN_L_RUN    = 138;
const AI_ANIM_GRUNT_TWOHANDED_TURN_R_RUN    = 139;
const AI_ANIM_GRUNT_TWOHANDED_TURN_B_RUN    = 140;

//------------------------------------------------------------------------
// MELEE ANIMATION OBJECT
//------------------------------------------------------------------------

var g_AnimGruntMelee =
{
    animSets : [
        AI_ANIM_GRUNT_MELEE1,
        AI_ANIM_GRUNT_MELEE2,
        AI_ANIM_GRUNT_MELEE3
    ],
    
    weightSets : [10, 10, 10],
    enabled : [true, true, true]
};

//------------------------------------------------------------------------
// TWO HANDED DEATH ANIMATION OBJECT
//------------------------------------------------------------------------

var g_AnimDeathTwoHanded =
{
    animSets : [
        AI_ANIM_GRUNT_DEATH_TWOHANDED_STAND,
        AI_ANIM_GRUNT_DEATH_TWOHANDED_VIOLENT,
        AI_ANIM_GRUNT_DEATH_TWOHANDED_RUNNING
    ],
    
    weightSets : [18, 3, 8],
    enabled : [true, true, true]
};

//------------------------------------------------------------------------
// CLASS
//------------------------------------------------------------------------

TurokAIGrunt = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIGrunt,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 14,
    meleeRange      : 81.92,
    turnDirection   : 0,
    bloodType       : BLOOD_TYPE_HUMAN,
    rangeDistance   : 1024.0,
    runRange        : 138.24,
    bCanMelee       : true,
    bCanRangeAttack : false,
    bTwoHanded      : false,
    attackThreshold : 0,
    extendedRadius  : 2.5,
    rangedAttacks   : 0,
    lookNode        : 19,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    deathScream : function()
    {
        var actor = this.parent.owner;
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
    },
    
    injuryScream : function()
    {
        var actor = this.parent.owner;
        var rnd = Sys.rand(3);
        
        switch(rnd)
        {
            case 0:
                Snd.play('sounds/shaders/human_effort_injury_grunt_3.ksnd', actor);
                break;
            case 1:
                Snd.play('sounds/shaders/human_effort_injury_grunt_2.ksnd', actor);
                break;
            case 2:
                Snd.play('sounds/shaders/human_effort_injury_grunt_1.ksnd', actor);
                break;
        }
    },
    
    violentDeathScream : function(actor)
    {
        var rnd = Sys.rand(3);
        
        switch(rnd)
        {
            case 0:
                Snd.play('sounds/shaders/human_violent_death_3.ksnd', actor);
                break;
            case 1:
                Snd.play('sounds/shaders/human_violent_death_2.ksnd', actor);
                break;
            case 2:
                Snd.play('sounds/shaders/human_violent_death_1.ksnd', actor);
                break;
        }
    },
    
    bloodSpurt : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/blood_spurt1.kfx', x, y, z);
    },
    
    throwGrenade : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/fx_215.kfx', x, y, z);
    },
    
    fireDart : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.ai.fireProjectile('fx/fx_294.kfx', x, y, z, Angle.degToRad(45), true);
    },
    
    action_305 : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/fx_247.kfx', x, y, z);
    },
    
    demonProjectile : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.spawnFX('fx/fx_370.kfx', x, y, z);
    },
    
    shamanProjectile : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.ai.fireProjectile('fx/projectile_shaman.kfx', x, y, z, Angle.degToRad(45), true);
    },
    
    fireWeapon : function()
    {
        var actor = this.parent.owner; 
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        Snd.play('sounds/shaders/generic_8_enemy_human_gunfire.ksnd', actor);
        actor.ai.fireProjectile('fx/projectile_enemy_bullet.kfx', x, y, z, Angle.degToRad(45), true);
    },
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/light_jungle_footfall.ksnd', this.parent.owner);
    },
    
    //------------------------------------------------------------------------
    // ANIMATION TWEEN FUNCTIONS
    //------------------------------------------------------------------------
    
    idle : function(actor)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_STAND : AI_ANIM_STANDING;
        
        this.state = AI_STATE_STANDING;
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    walk : function(actor)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_WALK : AI_ANIM_WALKING;
        
        this.state = AI_STATE_WALKING;
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    run : function(actor)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_RUN : AI_ANIM_RUNNING;
        
        this.state = AI_STATE_RUNNING;
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP |
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandBack : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_B_STAND : AI_ANIM_TURN_B_STAND;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkBack : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_B_WALK : AI_ANIM_TURN_B_WALK;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunBack : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_B_RUN : AI_ANIM_TURN_B_RUN;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandRight : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_R_STAND : AI_ANIM_TURN_R_STAND;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkRight : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_R_WALK : AI_ANIM_TURN_R_WALK;

        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunRight : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_R_RUN : AI_ANIM_TURN_R_RUN;

        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandLeft : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_L_STAND : AI_ANIM_TURN_L_STAND;

        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkLeft : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_L_WALK : AI_ANIM_TURN_L_WALK;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunLeft : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_TURN_L_RUN : AI_ANIM_TURN_L_RUN;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_ROOTMOTION);
    },
    
    turnStandFront : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_STAND : AI_ANIM_STANDING;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
    },
    
    turnWalkFront : function(actor, angles)
    {
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_WALK : AI_ANIM_WALKING;
        
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
    },
    
    turnRunFront : function(actor, angles)
    {
        var angle10 = Angle.degToRad(4);
        var time = 0;
        var ai = actor.ai;
        
        if(!ai.bSeeTarget || ai.target == null)
            return;

        if(!(angles <= angle10 && angles >= -angle10))
        {
            time = (2 - (Vector.length3(actor.origin,
                ai.target.origin) / 1024.0));
        }
        if(time < 0)
            time = 0;
        
        var turnSpeed = 5.05 + (time * 4);
        var speed = 4.0 - time;
                    
        var anim = this.bTwoHanded ? AI_ANIM_GRUNT_TWOHANDED_RUN : AI_ANIM_RUNNING;
                    
        actor.blendAnim(anim, 4.0, 8.0,
            NRender.ANIM_LOOP | 
            NRender.ANIM_ROOTMOTION);
        actor.animState.frame = speed;
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        if(this.bTwoHanded)
            this.deathAnimObj = g_AnimDeathTwoHanded;
        else
            this.meleeAnimObj = g_AnimGruntMelee;
            
        ComponentTurokAI.prototype.onReady.bind(this)();
        
        var actor = this.parent.owner;
        
        switch(actor.modelVariant)
        {
        // soldier variants
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_01.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_01.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_01.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_01.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_01.tga");
            // arms
            actor.setTexture(10, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(11, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(12, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(13, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(14, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(15, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(16, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(17, 0, 0, "textures/tex0833_01.tga");
            actor.setTexture(10, 1, 1, "textures/tex0833_01.tga");
            actor.setTexture(14, 1, 1, "textures/tex0833_01.tga");
            break;
            
        // poacher variants
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_05.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_05.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_05.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_05.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_05.tga");
            // legs
            actor.setTexture(1, 1, 0, "textures/tex0710_02.tga");
            actor.setTexture(3, 1, 0, "textures/tex0710_02.tga");
            actor.setTexture(4, 1, 0, "textures/tex0710_02.tga");
            actor.setTexture(5, 1, 0, "textures/tex0710_02.tga");
            actor.setTexture(7, 1, 0, "textures/tex0710_02.tga");
            actor.setTexture(8, 1, 0, "textures/tex0710_02.tga");
            // shin
            actor.setTexture(2, 2, 0, "textures/tex0710_02.tga");
            actor.setTexture(6, 2, 0, "textures/tex0710_02.tga");
            break;
            
        // tribal variants
        case 14:
            // hands
            actor.setTexture(17, 8, 0, "textures/tex0833_02.tga");
        case 15:
            // hands
            actor.setTexture(17, 9, 0, "textures/tex0833_02.tga");
        case 16:
            // hands
            actor.setTexture(17, 10, 0, "textures/tex0833_02.tga");
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_02.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_02.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_02.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_02.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_02.tga");
            // arms
            actor.setTexture(10, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(11, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(12, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(13, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(14, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(15, 0, 0, "textures/tex0833_02.tga");
            actor.setTexture(16, 0, 0, "textures/tex0833_02.tga");
            // neck
            actor.setTexture(18, 0, 0, "textures/tex0835_01.tga");
            break;
            
        // shaman
        case 17:
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_03.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_03.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_03.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_03.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_03.tga");
            // neck
            actor.setTexture(18, 0, 0, "textures/tex0835_02.tga");
            // legs
            actor.setTexture(1, 1, 0, "textures/tex0710_01.tga");
            actor.setTexture(3, 1, 0, "textures/tex0710_01.tga");
            actor.setTexture(4, 1, 0, "textures/tex0710_01.tga");
            actor.setTexture(5, 1, 0, "textures/tex0710_01.tga");
            actor.setTexture(7, 1, 0, "textures/tex0710_01.tga");
            actor.setTexture(8, 1, 0, "textures/tex0710_01.tga");
            // shin
            actor.setTexture(2, 2, 0, "textures/tex0710_01.tga");
            actor.setTexture(6, 2, 0, "textures/tex0710_01.tga");
            // hands
            actor.setTexture(17, 11, 4, "textures/tex0833_03.tga");
            // arms
            actor.setTexture(10, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(11, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(12, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(13, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(14, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(15, 0, 0, "textures/tex0833_03.tga");
            actor.setTexture(16, 0, 0, "textures/tex0833_03.tga");
            break;
            
        // cyborg guard
        case 18:
            // legs
            actor.setTexture(1, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(3, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(4, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(5, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(7, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(8, 0, 0, "textures/tex0831_01.tga");
            // shin
            actor.setTexture(2, 0, 0, "textures/tex0831_01.tga");
            actor.setTexture(6, 0, 0, "textures/tex0831_01.tga");
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_04.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_04.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_04.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_04.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_04.tga");
            // arms
            actor.setTexture(10, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(11, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(12, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(13, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(14, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(15, 0, 0, "textures/tex0833_04.tga");
            actor.setTexture(16, 0, 0, "textures/tex0833_04.tga");
            // neck
            actor.setTexture(18, 0, 0, "textures/tex0835_03.tga");
            break;
            
        // demon variant
        case 21:
            // hands
            actor.setTexture(17, 1, 0, "textures/tex0833_06.tga");
        case 22:
            // hands
            actor.setTexture(17, 3, 0, "textures/tex0833_06.tga");
        case 23:
            // hands
            actor.setTexture(13, 2, 0, "textures/tex0833_06.tga");
            actor.setTexture(17, 15, 0, "textures/tex0833_06.tga");
            // legs
            actor.setTexture(1, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(3, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(4, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(5, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(7, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(8, 0, 0, "textures/tex0831_02.tga");
            // shin
            actor.setTexture(2, 0, 0, "textures/tex0831_02.tga");
            actor.setTexture(6, 0, 0, "textures/tex0831_02.tga");
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_06.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_06.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_06.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_06.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_06.tga");
            // arms
            actor.setTexture(10, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(11, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(12, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(13, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(14, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(15, 0, 0, "textures/tex0833_06.tga");
            actor.setTexture(16, 0, 0, "textures/tex0833_06.tga");
            // neck
            actor.setTexture(18, 0, 0, "textures/tex0835_04.tga");
            break;
            
        // demon lord
        case 24:
            // legs
            actor.setTexture(1, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(3, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(4, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(5, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(7, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(8, 0, 0, "textures/tex0831_03.tga");
            // shin
            actor.setTexture(2, 0, 0, "textures/tex0831_03.tga");
            actor.setTexture(6, 0, 0, "textures/tex0831_03.tga");
            // neck
            actor.setTexture(18, 0, 0, "textures/tex0835_05.tga");
            // chest
            actor.setTexture(0, 0, 0, "textures/tex0829_07.tga");
            actor.setTexture(9, 0, 0, "textures/tex0829_07.tga");
            actor.setTexture(0, 0, 1, "textures/tex0830_07.tga");
            actor.setTexture(9, 0, 1, "textures/tex0830_07.tga");
            actor.setTexture(9, 0, 2, "textures/tex0832_07.tga");
            // arms
            actor.setTexture(10, 3, 0, "textures/tex0833_07.tga");
            actor.setTexture(11, 2, 0, "textures/tex0833_07.tga");
            actor.setTexture(12, 2, 0, "textures/tex0833_07.tga");
            actor.setTexture(13, 2, 0, "textures/tex0833_07.tga");
            actor.setTexture(14, 3, 0, "textures/tex0833_07.tga");
            actor.setTexture(15, 2, 0, "textures/tex0833_07.tga");
            actor.setTexture(16, 2, 0, "textures/tex0833_07.tga");
            // hands
            actor.setTexture(13, 2, 0, "textures/tex0833_07.tga");
            actor.setTexture(17, 16, 4, "textures/tex0833_07.tga");
            break;
        }
    }
});
