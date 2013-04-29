//-----------------------------------------------------------------------------
//
// Pistol.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Pistol = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl663/mdl663.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -160.42698;
    this.origin.y       = -184.32036-16;
    this.origin.z       = -16.04216;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim05");
    
    this.state          = WS_READY;
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(Pistol,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    readySound : 'sounds/shaders/ready_pistol.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(ClientPlayer.command.getAction('+attack'))
        {
            this.spawnFx('fx/muzzle_pistol.kfx', -6.656, -3.2, 15.696);
            this.spawnFx('fx/bulletshell.kfx', -10.24, -10.24, 15.648);
            
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 6.0, NRender.ANIM_NOINTERRUPT);
            
            ClientPlayer.component.aPistolAttack();
            this.state = WS_FIRING;
            return true;
        }
        
        return false;
    }
});
