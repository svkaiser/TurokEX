//-----------------------------------------------------------------------------
//
// Minigun.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Minigun = class.extends(Weapon, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.modelfile      = "models/mdl661/mdl661.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -163.84032;
    this.origin.y       = -163.84032;
    this.origin.z       = -5.80214;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim04");
    
    this.state          = WS_READY;
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});
