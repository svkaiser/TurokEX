//-----------------------------------------------------------------------------
//
// GrenadeLauncher.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

GrenadeLauncher = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl650/mdl650.kmesh";
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

class.properties(GrenadeLauncher,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    readySound : 'sounds/shaders/ready_grenade_launcher.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(this.super.prototype.checkAttack.bind(this)())
        {
            Snd.play('sounds/shaders/grenade_launch.ksnd');
            
            // TODO
            var actor = ClientPlayer.actor;

            var mtx = Matrix.fromQuaternion(actor.rotation);
            mtx.addTranslation(
                ClientPlayer.camera.origin.x,
                ClientPlayer.camera.origin.y,
                ClientPlayer.camera.origin.z);

            this.spawnFx('fx/projectile_grenade.kfx', -18.432, -5.12, -25.6);  
            this.spawnFx('fx/muzzle_grenade_launcher.kfx', -10.35, -2.048, 18.432);
                
            return true;
        }
        
        return false;
    }
});
