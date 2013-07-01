//-----------------------------------------------------------------------------
//
// ShotTrace.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ShotTrace = class.define();

class.properties(ShotTrace,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    left        : 0.0,
    up          : 0.0,
    spray       : 0.0,
    hitFx       : "",
    hitSnd      : "",
    damageClass : Damage,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    shoot : function(actor, org_x, org_y, org_z, plane)
    {
        var r_x = this.left + (this.spray * Sys.cRand());
        var r_y = this.up   + (this.spray * Sys.cRand());
        
        var t = Physics.rayTrace(
            new Vector(org_x, org_y, org_z),
            Vector.applyRotation(new Vector(r_x, r_y, 1), actor.rotation),
            64, 64, plane, actor);
            
        if(t != null)
        {
            var fx = this.hitFx;
            var snd = this.hitSnd;
            var rot = actor.rotation;
            
            if(t.hitActor != null)
            {
                var hit = t.hitActor;
                
                this.damageClass.prototype.inflict(hit, actor);
                rot = hit.rotation;
                
                // TODO - TEMP
                if(hit.components)
                {
                    for(var i in hit.components)
                    {
                        var component = hit.components[i];
                        
                        if(component.bloodType !== undefined)
                        {
                            switch(component.bloodType)
                            {
                            case BLOOD_TYPE_HUMAN:
                                fx = "fx/blood_gush1.kfx";
                                snd = "sounds/shaders/bullet_impact_13.ksnd";
                                break;
                            case BLOOD_TYPE_ALIEN:
                                fx = "fx/fx_048.kfx";
                                snd = "sounds/shaders/bullet_impact_14.ksnd";
                                break;
                            }
                        }
                    }
                }
            }
            
            Sys.spawnFx(fx, null, t.hitVector, rot, t.hitPlane, null, snd);
        }
    }
});
