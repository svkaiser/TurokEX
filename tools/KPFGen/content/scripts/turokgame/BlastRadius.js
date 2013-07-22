//-----------------------------------------------------------------------------
//
// BlastRadius.js
//
// DESCRIPTION: Base class for all explosion functionality
//
//-----------------------------------------------------------------------------

BlastRadius = class.define();

class.properties(BlastRadius,
{   
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    explode : function(instigator, x, y, z, plane, radius, maxDamage,
        knockBack, bReduceDmg)
    {
        radius = radius * 10.24;
        
        var list = Level.getActorsInRadius(radius, x, y, z,
            Plane.fromIndex(plane), 9, true);
            
        if(list === null)
            return;
        
        var r = radius * radius;
        
        // declare all vars here instead of in the loop to avoid thrashing the GC
        var aobj;
        var dist;
        var origin;
        var ax;
        var ay;
        var az;
        var dmg;
        var force;
        var fx;
        var fy;
        var fz;
        var d;
        
        for(var i = 0; i < list.length; i++)
        {
            aobj = list[i];
            
            // don't damage self
            if(!bReduceDmg && GameActor.compare(aobj, instigator))
                continue;
            
            origin = aobj.origin;
            ax = origin.x - x;
            ay = (origin.y + aobj.height) - y;
            az = origin.z - z;
            
            dist = (ax*ax+ay*ay+az*az);
            
            force = (r < dist) ? 1 : Math.sqrt((r - dist) / r);
            dmg = force * maxDamage;
            
            // reduce damage on self
            if(bReduceDmg && GameActor.compare(aobj, instigator))
                dmg *= 0.25;
            
            Damage.prototype.inflict(aobj, instigator, Math.round(dmg));
            
            if(aobj.classFlags & 8)
                continue;
            
            // blast actors into the air on death!
            for(var j in aobj.components)
            {
                var component = aobj.components[j];
                
                if(component.health !== undefined && component.health <= 0)
                {
                    d = Math.sqrt(dist);
                    
                    if(d == 0.0)
                        continue;
                    
                    d = 1.0 / d;
                    
                    force = force * 50.0 * knockBack;
                    fx = (ax*d) * force;
                    fy = (Sys.rand(3) + 1) * 1 - force * -0.85;
                    fz = (az*d) * force;
                    
                    var vel = aobj.velocity;
                    vel.x = fx * 15.0;
                    vel.y = fy * 15.0;
                    vel.z = fz * 15.0;
                    aobj.velocity = vel;
                }
            }
        }
    }
});
