//-----------------------------------------------------------------------------
//
// TurokAIAlien.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokAIAlien = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIAlien,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 60,
    meleeRange      : 102.4,
    runRange        : 136.53,
    bloodType       : BLOOD_TYPE_ALIEN,
    rangeDistance   : 1024.0,
    bCanMelee       : true,
    bCanRangeAttack : true,
    rangedAttacks   : (FLAG_ATTACK_RANGE1|FLAG_ATTACK_RANGE2),
    lookNode        : 12,
    lookYawAxis_x   : -1.0,
    lookYawAxis_y   : 0,
    lookYawAxis_z   : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/light_jungle_footfall.ksnd', this.parent.owner);
    },
    
    fireWeapon : function()
    {
        var actor = this.parent.owner; 
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        Snd.play('sounds/shaders/tek_weapon_1.ksnd', actor);
        actor.ai.fireProjectile('fx/projectile_enemy_tekshot.kfx', x, y, z, Angle.degToRad(45), true);
    },
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/alien_death.ksnd', this.parent.owner);
    },
    
    violentDeathScream : function()
    {
        Snd.play('sounds/shaders/alien_violent_death.ksnd', this.parent.owner);
    },
    
    gibDeath : function(gibs, x, y, z)
    {
        var actor = this.parent.owner; 
        
        var plane = Plane.fromIndex(actor.plane);
        var pos = actor.getLocalVector(x, y, z);
        
        Physics.tryMove(actor, actor.origin, pos, plane);
        var gib = Level.spawnActor(gibs, pos.x, pos.y, pos.z,
            actor.yaw, actor.pitch, plane);
        
        gib.bRotor = true;
        gib.mass = 1500;
        gib.friction = 0.5;
        gib.bounceDamp = 0.35;
        
        var velocity = new Vector(
            (Sys.cRand() - 0) * 0.3 + 0,
            (Sys.cRand() - 1) * 0.3 + 1,
            (Sys.cRand() - 0) * 0.3 + 0);
            
        velocity.normalize();
        velocity.scale((-Sys.rand(2) + 3) * 10.24);
        velocity.scale(15.0);
        
        gib.velocity = velocity;
        gib.rotorSpeed = 1.0 + (velocity.unit3() / 1500);
        velocity.normalize();
        var axis = Vector.cross(plane.normal, velocity);
        axis.normalize();
        
        gib.rotorVector = axis;
    },
    
    action_399 : function()
    {
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        this.gibDeath('gibs_alien01', x, y, z);
    },
    
    action_400 : function()
    {
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        this.gibDeath('gibs_alien02', x, y, z);
    },
    
    action_401 : function()
    {
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        this.gibDeath('gibs_alien03', x, y, z);
    },
    
    action_402 : function()
    {
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        
        this.gibDeath('gibs_alien04', x, y, z);
    },
    
    action_407 : function()
    {
        this.parent.owner.bHidden = true;
        this.parent.owner.bCollision = false;
    },
    
    action_000 : function()
    {
    },
    
    action_023 : function()
    {
    }
});
