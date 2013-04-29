//-----------------------------------------------------------------------------
//
// PulseRifle.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

PulseRifle = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl655/mdl655.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -170.667;
    this.origin.z       = -9.21548;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim04");
    
    this.state          = WS_READY;
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(PulseRifle,
{
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(ClientPlayer.command.getAction('+attack'))
        {
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 4.0, NRender.ANIM_LOOP);
                
            this.animState.playTime = 0.08;

            this.state = WS_FIRING;
            return true;
        }
        
        return false;
    },
    
    fire : function()
    {
        if(this.checkHoldster())
        {
            this.animState.flags &= ~NRender.ANIM_LOOP;
            return;
        }
        
        if(this.animState.playTime >= 0.16)
        {
            this.animState.playTime -= 0.16;
            this.spawnFx('fx/projectile_pulseshot.kfx', -20.48, -12.8, -96);
            Snd.play('sounds/shaders/machine_gun_shot_2.ksnd');
        }
        
        if(!ClientPlayer.command.getAction('+attack'))
        {
            this.readyAnim();
            this.state = WS_READY;
        }
    }
});
