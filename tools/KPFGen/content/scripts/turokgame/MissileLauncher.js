//-----------------------------------------------------------------------------
//
// MissileLauncher.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

MissileLauncher = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl666/mdl666.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -153.6003;
    this.origin.z       = -9.21548;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim05");
    
    this.state          = WS_READY;
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(MissileLauncher,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    readySound : 'sounds/shaders/ready_missile_launcher.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(this.super.prototype.checkAttack.bind(this)())
        {
            this.spawnFx('fx/projectile_rocket.kfx', -4.096, -14.336, -25.6);
            Snd.play('sounds/shaders/missile_launch.ksnd');
            Snd.play('sounds/shaders/reload_missile_launcher.ksnd');
            return true;
        }
        
        return false;
    }
});
