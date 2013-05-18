//-----------------------------------------------------------------------------
//
// ProjectileSpawner.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ProjectileSpawner = class.define();

class.properties(ProjectileSpawner,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    x               : 0.0,
    y               : 0.0,
    z               : 0.0,
    damage          : 0,
    speed           : 1024,
    mass            : 0,
    friction        : 0.0,
    airFriction     : 0.0,
    bounce          : 0.0,
    type            : "",
    bTouchActors    : true,
    bTouchSurface   : true,
    lifeTime        : 0.0,
    damageClass     : Damage,
    impactFX        : "",
    impactSnd       : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    spawn : function(actor, origin)
    {
        var vec = Vector.applyRotation(new Vector(this.x, this.y, this.z), actor.rotation);
        vec.x += origin.x;
        vec.y += origin.y + actor.centerHeight + actor.viewHeight;
        vec.z += origin.z;
        
        var plane = Plane.fromIndex(actor.plane);
        var moveOk = Physics.tryMove(actor, origin, vec, plane);

        var proj = Level.spawnActor(this.type, vec.x, vec.y, vec.z, actor.yaw, actor.pitch, plane);
        proj.owner = actor;
        
        var component = proj.components.ComponentProjectile;
        var ctrl = proj.components.ComponentProjectile.controller;
        
        component.damage        = this.damage;
        component.speed         = this.speed;
        component.mass          = this.mass;
        component.friction      = this.friction;
        component.airFriction   = this.airFriction;
        component.bTouchActors  = this.bTouchActors;
        component.bTouchSurface = this.bTouchSurface;
        component.lifeTime      = this.lifeTime;
        component.damageClass   = this.damageClass;
        component.impactFX      = this.impactFX;
        component.impactSnd     = this.impactSnd;
        component.sourceActor   = actor;
        
        if(moveOk == false)
        {
            component.destroy(origin);
            return null;
        }
        
        // optional axis vector
        if(arguments.length == 3 && (arguments[2] instanceof Vector))
            ctrl.velocity.copy(Vector.applyRotation(arguments[2], actor.rotation));
        else
            ctrl.velocity.copy(ctrl.forward);
        
        ctrl.velocity.scale(component.speed);
        
        return proj;
    }
});

//-----------------------------------------------------------------------------
//
// ProjectileSpawnerPulse.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ProjectileSpawnerPulse = class.extends(ProjectileSpawner);

class.properties(ProjectileSpawnerPulse,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    x               : -16.384,
    y               : -12.8,
    z               : 25.6,
    damage          : 7,
    speed           : 3840,
    mass            : 0,
    friction        : 0.0,
    airFriction     : 0.0,
    bounce          : 0.0,
    type            : "projectile_pulse",
    impactFX        : "fx/projectile_pulseshot_impact.kfx",
    impactSnd       : "sounds/shaders/generic_195.ksnd",
    bTouchActors    : true,
    bTouchSurface   : true,
    lifeTime        : 50.0,
});

//-----------------------------------------------------------------------------
//
// ProjectileSpawnerGrenade.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ProjectileSpawnerGrenade = class.extends(ProjectileSpawner);

class.properties(ProjectileSpawnerGrenade,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    x               : -18.432,
    y               : -5.12,
    z               : 25.6,
    damage          : 7,
    speed           : 768,
    mass            : 460,
    friction        : 0.149414,
    airFriction     : 0.0,
    bounce          : 0.597656,
    type            : "projectile_pulse",
    impactFX        : "fx/projectile_grenade_explosion.kfx",
    impactSnd       : "sounds/shaders/explosion_2.ksnd",
    bTouchActors    : true,
    bTouchSurface   : false,
    lifeTime        : 20.0,
});

