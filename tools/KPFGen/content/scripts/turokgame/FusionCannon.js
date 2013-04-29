//-----------------------------------------------------------------------------
//
// FusionCannon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

FusionCannon = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl662/mdl662.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = 238.9338;
    this.origin.z       = -43.34888;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim05");
    
    this.state          = WS_READY;
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(FusionCannon,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    readySound : 'sounds/shaders/ready_tek_weapon_2.ksnd',
    bFired : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(this.super.prototype.checkAttack.bind(this)())
        {
            Snd.play('sounds/shaders/tek_weapon_2.ksnd');
            return true;
        }
        
        return false;
    },
    
    fire : function()
    {
        if(this.animState.playTime >= 1.1333)
        {
            if(!this.bFired)
            {
                this.bFired = true;
                this.spawnFx('fx/projectile_fusionshot.kfx', -4.096, -14.336, -25.6);
            }
        }
        else
            this.bFired = false;
        
        this.super.prototype.fire.bind(this)();
    }
});
