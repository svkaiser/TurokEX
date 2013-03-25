//-----------------------------------------------------------------------------
//
// Bow.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Bow = class.extends(Weapon, function()
{
    this.bOwned         = true;
    
    this.modelfile      = "models/mdl644/mdl644.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -133.12026;
    this.origin.y       = -150.18696;
    this.origin.z       = -12.62882;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Aim       = Sys.loadAnimation(this.model, "anim03");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim06");
    
    this.state          = WS_READY;
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(Bow,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bAiming : false,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(this.bAiming)
            return true;
        
        if(Client.localPlayer.command.getAction('+attack'))
        {
            Snd.play('sounds/shaders/bow_stretch.ksnd');
            this.animState.blendAnim(this.anim_Aim,
                this.playSpeed, 18.0, NRender.ANIM_LOOP);

            this.state = WS_FIRING;
            this.bAiming = true;
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
        
        if(Client.localPlayer.command.getActionHeldTime('+attack') > 0 && this.bAiming)
            return;
        else
        {
            if(this.bAiming == true)
                Snd.play('sounds/shaders/bow_twang.ksnd');
            
            this.bAiming = false;
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 4.0, NRender.ANIM_NOINTERRUPT);
        }
        
        if(this.animState.flags & NRender.ANIM_STOPPED)
        {
            this.readyAnim();
            this.state = WS_READY;
        }
    }
});
