//-----------------------------------------------------------------------------
//
// ComponentGibs.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentGibs = class.extendStatic(Component);

class.properties(ComponentGibs,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active : true,
    lifeTime : 150,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTick : function()
    {
        var actor = this.parent.owner;
        
        if(this.lifeTime-- <= 0)
        {
            GameActor.remove(actor);
            return;
        }
        
        if(!(this.lifeTime & 7) && actor.velocity.unit3() >= 0.1)
        {
            Sys.spawnFx('fx/blood_gush1.kfx', null, actor.origin,
                actor.rotation, Plane.fromIndex(actor.plane));
        }
            
        if(actor.onGround() && actor.velocity.y > 1.0)
        {
            if(actor.velocity.y <= 204.8)
                Snd.play('sounds/shaders/generic_238.ksnd', actor);
            else if(actor.velocity.y <= 102.4)
                Snd.play('sounds/shaders/generic_239.ksnd', actor);
            else
                Snd.play('sounds/shaders/generic_240.ksnd', actor);
        }
    },
});
