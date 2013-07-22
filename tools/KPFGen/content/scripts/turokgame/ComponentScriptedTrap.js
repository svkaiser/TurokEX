//-----------------------------------------------------------------------------
//
// ComponentScriptedTrap.js
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const ANIM_TRAP_IDLE        = 2603;
const ANIM_TRAP_ACTIVATE    = 2604;

ComponentScriptedTrap = class.extendStatic(Component);

class.properties(ComponentScriptedTrap,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bTriggered : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // kinda sucks that I have to define the same functions twice. though it would probably
    // make more since to have the AI component inherit off of the scripted actor component
    dealDamage : function(dmgClass, x, y, z)
    {
        var self = this.parent.owner;
        var target = ClientPlayer.actor;
        var torg = target.origin;
        var aorg = self.getLocalVector(x, y, z);
        
        torg.y += target.viewHeight;
        torg.sub(aorg);
        
        if(torg.unit3() <= ((10.0 * 10.24) + target.radius) * 0.35)
            dmgClass.prototype.inflict(ClientPlayer.actor, self);
    },
    
    weakKnockBack : function()
    {
        var r = arguments[0] * 10.24;
        var actor = this.parent.owner;
        var aorg = actor.getLocalVector(arguments[1], arguments[2], arguments[3]);
        
        var list = Level.getActorsInRadius(r, aorg.x, aorg.y - actor.baseHeight * 0.5, aorg.z,
            Plane.fromIndex(actor.plane), 9, true);
            
        if(list == null)
            return;
            
        // declare all vars here instead of in the loop to avoid thrashing the GC
        var vx;
        var vy;
        var vz;
        var aobj;
        var origin;
        var vel;
        var j;
        var bDead;
        
        var velocity = new Vector(0, 0, 0);
        
        for(var i = 0; i < list.length; i++)
        {
            aobj = list[i];
            bDead = false;
            
            for(j in aobj.components)
            {
                var component = aobj.components[j];
                if(component.health && component.health <= 0)
                {
                    bDead = true;
                    break;
                }
            }
            
            // don't knock around dead things
            if(bDead)
                continue;
            
            origin = aobj.origin;
            
            vx = origin.x - aorg.x;
            vy = (origin.y + aobj.baseHeight * 0.5) - aorg.y;
            vz = origin.z - aorg.z;
            
            if(Math.sqrt(vx*vx+vy*vy+vz*vz) <= r + aobj.radius)
            {
                vel = aobj.velocity;
                
                // TODO - need to fix this
                // nudge player actors a few feet from the ground
                // so they'll be affected by the xz velocity
                if(aobj.classFlags & 8)
                {
                    origin.y += 1.0;
                    aobj.origin = origin;
                }
            
                velocity.x = vx;
                velocity.y = vy;
                velocity.z = vz;
                
                velocity.scale(13.824);
                velocity.add(vel);
                velocity.y = 307.2;
                
                aobj.velocity = velocity;
            }
        }
    },
    
    melee : function()
    {
        this.dealDamage(DamageMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    wimpyMelee : function()
    {
        this.dealDamage(DamageWimpyMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    weakMelee : function()
    {
        this.dealDamage(DamageWeakMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    strongMelee : function()
    {
        this.dealDamage(DamageStrongMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    veryHeavyMelee : function()
    {
        this.dealDamage(DamageVeryHeavyMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    bluntMelee : function()
    {
        this.dealDamage(DamageBluntMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    fleshMelee : function()
    {
        this.dealDamage(DamageFleshMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    weakFleshMelee : function()
    {
        this.dealDamage(DamageWeakFleshMelee,
            arguments[1], arguments[2], arguments[3]);
    },
    
    action_093 : function()
    {
        ClientPlayer.component.recoilPitch = arguments[0] * -0.017;
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
        var anim = ANIM_TRAP_IDLE;
        var self = this.parent.owner;
        
        if(!self.checkAnimID(anim))
            anim = ANIM_TRAP_ACTIVATE;
        
        self.setAnim(anim, 4.0, NRender.ANIM_LOOP);
    },
    
    onTrigger : function()
    {
        if(this.bTriggered)
            return;
        
        var actor = this.parent.owner;
        
        actor.setAnim(ANIM_TRAP_ACTIVATE, 4.0, NRender.ANIM_LOOP);
        this.bTriggered = true;
    }
});
