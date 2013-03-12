//-----------------------------------------------------------------------------
//
// Knife.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Knife = class.extends(Weapon, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.bOwned         = true;
    
    this.modelfile      = "models/mdl653/mdl653.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -153.6003;
    this.origin.z       = -9.21548;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_FireAlt1  = Sys.loadAnimation(this.model, "anim04");
    this.anim_FireAlt2  = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim06");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim07");
    
    this.state          = WS_READY;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.checkAttack = function()
    {
        if(Client.localPlayer.command.getAction('+attack'))
        {
            var fireAnim;
            var r = Sys.rand(100);
            
            if(r <= 33)
                fireAnim = this.anim_Fire;
            else if(r <= 66)
                fireAnim = this.anim_FireAlt1;
            else
                fireAnim = this.anim_FireAlt2;
                
            this.animState.blendAnim(fireAnim,
                this.playSpeed, 4.0, NRender.ANIM_NOINTERRUPT);

            this.state = WS_FIRING;
            return true;
        }
        
        return false;
    }
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});
