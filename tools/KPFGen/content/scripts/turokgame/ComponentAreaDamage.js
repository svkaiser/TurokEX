//-----------------------------------------------------------------------------
//
// ComponentAreaDamage.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/Damage.js');

ComponentAreaDamage = class.extendStatic(ComponentArea);

class.properties(ComponentAreaDamage,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active      : true,
    damageTime  : 1.0,
    time        : 0.0,
    damageClass : DamageLava,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onLocalTick : function(player)
    {
        // TODO
        var ctrl = player.controller;
        
        if(ctrl == null || ctrl == undefined)
            return;
        
        if(!(ctrl instanceof ControllerPlayer))
            return;
            
        if(!ctrl.onGround())
            return;
        
        this.time += /*ctrl.owner.timeStamp*/Sys.deltatime();
        
        if(this.time >= this.damageTime)
        {
            this.time = 0.0;
            this.damageClass.prototype.inflict(ctrl.owner, null);
        }
    }
});
