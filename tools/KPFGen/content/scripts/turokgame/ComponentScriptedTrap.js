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
