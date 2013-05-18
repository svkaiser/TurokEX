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
    
    inflict : function(target, instigator)
    {
        if(target == null)
            return;
            
        for(var i in target.components)
        {
            var component = target.components[i];
            
            if(component.onDamage)
                component.onDamage(instigator, this.amount);
                
            if(component.health)
            {
                component.health -= this.amount;
                if(component.health <= 0)
                {
                    if(component.onDeath)
                        component.onDeath(instigator);
                }
            }
        }
    }
});

//-----------------------------------------------------------------------------
//
// DamagePistolShot.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

DamagePistolShot = class.extends(Damage);

class.properties(DamagePistolShot,
{
    amount : 7
});

//-----------------------------------------------------------------------------
//
// DamageShotgunShot.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

DamageShotgunShot = class.extends(Damage);

class.properties(DamageShotgunShot,
{
    amount : 5
});

//-----------------------------------------------------------------------------
//
// DamageLava.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

DamageLava = class.extends(Damage);

class.properties(DamageLava,
{
    amount : 3
});
