//-----------------------------------------------------------------------------
//
// Damage.js
// DESCRIPTION: Base class for all damage types
//
//-----------------------------------------------------------------------------

Damage = class.define();

class.properties(Damage,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    amount : 7,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onSuccess : function(target, instigator)
    {
    },
    
    inflict : function(target, instigator)
    {
        if(target == null)
            return;
            
        var ok = false;
        var amt;
        
        if(arguments.length == 3)
            amt = arguments[2];
        else
            amt = this.amount;
            
        for(var i in target.components)
        {
            var component = target.components[i];
            
            if(component.onDamage)
            {
                ok = true;
                component.onDamage(instigator, amt, this);
            }
                
            if(component.health)
            {
                component.health -= amt;
                if(component.health <= 0)
                {
                    if(component.onDeath)
                        component.onDeath(instigator, this);
                }
            }
        }
        
        if(ok == true)
            this.onSuccess(target, instigator);
    }
});

//-----------------------------------------------------------------------------
//
// DamageKnife
//
//-----------------------------------------------------------------------------

DamageKnife = class.extends(Damage);

class.properties(DamageKnife,
{
    amount : 15,
    bloodSlashes : [
        ['', '', ''],
        ['fx/fx_063.kfx', 'fx/fx_060.kfx', 'fx/fx_066.kfx'],
        ['fx/fx_064.kfx', 'fx/fx_061.kfx', 'fx/fx_067.kfx'],
        ['', '', '']
    ],
    
    knifeSlash : function(target, instigator, fx, x, y, z, offset)
    {
        var org = target.origin;
        var vec = Vector.applyRotation(
            new Vector(x, y, z), instigator.rotation);
        
        vec.x += org.x;
        vec.y += org.y;
        vec.z += org.z;
        
        var roll = Quaternion.multiply(new Quaternion(offset, 0, 0, 1), instigator.rotation);

        Sys.spawnFx(fx, target, vec, roll, null);
    },
    
    onSuccess : function(target, instigator)
    {
        Snd.play('sounds/shaders/tomahawk_impact_flesh.ksnd', target);
        
        var player = instigator.components.ComponentTurokPlayer;
        
        if(player === null || player === undefined)
            return;
            
        for(var i in target.components)
        {
            var component = target.components[i];
            
            if(component.bloodType)
            {
                switch(player.weapons[WP_KNIFE].swishType)
                {
                case 0:
                    this.knifeSlash(target, instigator, this.bloodSlashes[component.bloodType][0],
                        -80.0, 58.25, 122.0, 0.2);
                    break;
                case 1:
                    this.knifeSlash(target, instigator, this.bloodSlashes[component.bloodType][1],
                        71.5, 0.0, 121.5, -0.9);
                    break;
                case 2:
                    this.knifeSlash(target, instigator, this.bloodSlashes[component.bloodType][2],
                        -49.5, 122.0, 141.0, 0.7166);
                    break;
                }
            }
        }
    }
});

//-----------------------------------------------------------------------------
//
// DamagePistolShot
//
//-----------------------------------------------------------------------------

DamagePistolShot = class.extends(Damage);

class.properties(DamagePistolShot,
{
    amount : 7
});

//-----------------------------------------------------------------------------
//
// DamageEnemyPistolShot
//
//-----------------------------------------------------------------------------

DamageEnemyPistolShot = class.extends(Damage);

class.properties(DamageEnemyPistolShot,
{
    amount : 4
});

//-----------------------------------------------------------------------------
//
// DamageShotgunShot
//
//-----------------------------------------------------------------------------

DamageShotgunShot = class.extends(Damage);

class.properties(DamageShotgunShot,
{
    amount : 5
});

//-----------------------------------------------------------------------------
//
// DamageLava
//
//-----------------------------------------------------------------------------

DamageLava = class.extends(Damage);

class.properties(DamageLava,
{
    amount : 3
});

//-----------------------------------------------------------------------------
//
// DamageMelee
//
//-----------------------------------------------------------------------------

DamageMelee = class.extends(Damage);

class.properties(DamageMelee,
{
    amount : 5
});

//-----------------------------------------------------------------------------
//
// DamageVeryWimpyMelee
//
//-----------------------------------------------------------------------------

DamageVeryWimpyMelee = class.extends(DamageMelee);

class.properties(DamageVeryWimpyMelee,
{
    amount : 1
});

//-----------------------------------------------------------------------------
//
// DamageWimpyMelee
//
//-----------------------------------------------------------------------------

DamageWimpyMelee = class.extends(DamageMelee);

class.properties(DamageWimpyMelee,
{
    amount : 2
});

//-----------------------------------------------------------------------------
//
// DamageWeakMelee
//
//-----------------------------------------------------------------------------

DamageWeakMelee = class.extends(DamageMelee);

class.properties(DamageWeakMelee,
{
    amount : 3
});

//-----------------------------------------------------------------------------
//
// DamageStrongMelee
//
//-----------------------------------------------------------------------------

DamageStrongMelee = class.extends(DamageMelee);

class.properties(DamageStrongMelee,
{
    amount : 10
});

//-----------------------------------------------------------------------------
//
// DamageVeryStrongMelee
//
//-----------------------------------------------------------------------------

DamageVeryStrongMelee = class.extends(DamageMelee);

class.properties(DamageVeryStrongMelee,
{
    amount : 15
});

//-----------------------------------------------------------------------------
//
// DamageHeavyMelee
//
//-----------------------------------------------------------------------------

DamageHeavyMelee = class.extends(DamageMelee);

class.properties(DamageHeavyMelee,
{
    amount : 20
});

//-----------------------------------------------------------------------------
//
// DamageVeryHeavyMelee
//
//-----------------------------------------------------------------------------

DamageVeryHeavyMelee = class.extends(DamageMelee);

class.properties(DamageVeryHeavyMelee,
{
    amount : 30
});

//-----------------------------------------------------------------------------
//
// DamageFatalMelee
//
//-----------------------------------------------------------------------------

DamageFatalMelee = class.extends(DamageMelee);

class.properties(DamageFatalMelee,
{
    amount : 40
});

//-----------------------------------------------------------------------------
//
// DamageVeryFatalMelee
//
//-----------------------------------------------------------------------------

DamageVeryFatalMelee = class.extends(DamageMelee);

class.properties(DamageVeryFatalMelee,
{
    amount : 50
});

//-----------------------------------------------------------------------------
//
// DamageDeathBlow
//
//-----------------------------------------------------------------------------

DamageDeathBlow = class.extends(DamageMelee);

class.properties(DamageDeathBlow,
{
    amount : 100
});

//-----------------------------------------------------------------------------
//
// DamageFleshMelee
//
//-----------------------------------------------------------------------------

DamageFleshMelee = class.extends(DamageMelee);

class.properties(DamageFleshMelee,
{
    amount : 5,
    
    onSuccess : function(target, instigator)
    {
        Snd.play('sounds/shaders/tomahawk_impact_flesh.ksnd', target);
    }
});

//-----------------------------------------------------------------------------
//
// DamageVeryWimpyFleshMelee
//
//-----------------------------------------------------------------------------

DamageVeryWimpyFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageVeryWimpyFleshMelee,
{
    amount : 1
});

//-----------------------------------------------------------------------------
//
// DamageWimpyFleshMelee
//
//-----------------------------------------------------------------------------

DamageWimpyFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageWimpyFleshMelee,
{
    amount : 2
});

//-----------------------------------------------------------------------------
//
// DamageWeakFleshMelee
//
//-----------------------------------------------------------------------------

DamageWeakFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageWeakFleshMelee,
{
    amount : 3
});

//-----------------------------------------------------------------------------
//
// DamageStrongFleshMelee
//
//-----------------------------------------------------------------------------

DamageStrongFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageStrongFleshMelee,
{
    amount : 10
});

//-----------------------------------------------------------------------------
//
// DamageVeryStrongFleshMelee
//
//-----------------------------------------------------------------------------

DamageVeryStrongFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageVeryStrongFleshMelee,
{
    amount : 15
});

//-----------------------------------------------------------------------------
//
// DamageHeavyFleshMelee
//
//-----------------------------------------------------------------------------

DamageHeavyFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageHeavyFleshMelee,
{
    amount : 20
});

//-----------------------------------------------------------------------------
//
// DamageVeryHeavyFleshMelee
//
//-----------------------------------------------------------------------------

DamageVeryHeavyFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageVeryHeavyFleshMelee,
{
    amount : 30
});

//-----------------------------------------------------------------------------
//
// DamageFatalFleshMelee
//
//-----------------------------------------------------------------------------

DamageFatalFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageFatalFleshMelee,
{
    amount : 40
});

//-----------------------------------------------------------------------------
//
// DamageVeryFatalFleshMelee
//
//-----------------------------------------------------------------------------

DamageVeryFatalFleshMelee = class.extends(DamageFleshMelee);

class.properties(DamageVeryFatalFleshMelee,
{
    amount : 50
});

//-----------------------------------------------------------------------------
//
// DamageFleshDeathBlow
//
//-----------------------------------------------------------------------------

DamageFleshDeathBlow = class.extends(DamageFleshMelee);

class.properties(DamageFleshDeathBlow,
{
    amount : 100
});

//-----------------------------------------------------------------------------
//
// DamageBluntMelee
//
//-----------------------------------------------------------------------------

DamageBluntMelee = class.extends(DamageMelee);

class.properties(DamageBluntMelee,
{
    amount : 5,
    
    onSuccess : function(target, instigator)
    {
        Snd.play('sounds/shaders/generic_81_kick_impact.ksnd', target);
    }
});

//-----------------------------------------------------------------------------
//
// DamageVeryWimpyBluntMelee
//
//-----------------------------------------------------------------------------

DamageVeryWimpyBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageVeryWimpyBluntMelee,
{
    amount : 1
});

//-----------------------------------------------------------------------------
//
// DamageWimpyBluntMelee
//
//-----------------------------------------------------------------------------

DamageWimpyBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageWimpyBluntMelee,
{
    amount : 2
});

//-----------------------------------------------------------------------------
//
// DamageWeakBluntMelee
//
//-----------------------------------------------------------------------------

DamageWeakBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageWeakBluntMelee,
{
    amount : 3
});

//-----------------------------------------------------------------------------
//
// DamageStrongBluntMelee
//
//-----------------------------------------------------------------------------

DamageStrongBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageStrongBluntMelee,
{
    amount : 10
});

//-----------------------------------------------------------------------------
//
// DamageVeryStrongBluntMelee
//
//-----------------------------------------------------------------------------

DamageVeryStrongBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageVeryStrongBluntMelee,
{
    amount : 15
});

//-----------------------------------------------------------------------------
//
// DamageHeavyBluntMelee
//
//-----------------------------------------------------------------------------

DamageHeavyBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageHeavyBluntMelee,
{
    amount : 20
});

//-----------------------------------------------------------------------------
//
// DamageVeryHeavyBluntMelee
//
//-----------------------------------------------------------------------------

DamageVeryHeavyBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageVeryHeavyBluntMelee,
{
    amount : 30
});

//-----------------------------------------------------------------------------
//
// DamageFatalBluntMelee
//
//-----------------------------------------------------------------------------

DamageFatalBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageFatalBluntMelee,
{
    amount : 40
});

//-----------------------------------------------------------------------------
//
// DamageVeryFatalBluntMelee
//
//-----------------------------------------------------------------------------

DamageVeryFatalBluntMelee = class.extends(DamageBluntMelee);

class.properties(DamageVeryFatalBluntMelee,
{
    amount : 50
});

//-----------------------------------------------------------------------------
//
// DamageBluntDeathBlow
//
//-----------------------------------------------------------------------------

DamageBluntDeathBlow = class.extends(DamageBluntMelee);

class.properties(DamageBluntDeathBlow,
{
    amount : 100
});
