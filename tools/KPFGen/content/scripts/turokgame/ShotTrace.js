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
            Sys.spawnFx(this.hitFx, null, t.hitVector,
                actor.rotation, t.hitPlane, null, this.hitSnd);
                
            if(t.hitActor != null)
                this.damageClass.prototype.inflict(t.hitActor, actor);
        }
    }
});
