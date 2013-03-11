//-----------------------------------------------------------------------------
//
// Bow.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Bow = class.extends(Weapon, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.bOwned         = true;
    
    this.modelfile      = "models/mdl644/mdl644.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -133.12026;
    this.origin.y       = -150.18696;
    this.origin.z       = -12.62882;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim06");
    
    this.state          = WS_READY;
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});
