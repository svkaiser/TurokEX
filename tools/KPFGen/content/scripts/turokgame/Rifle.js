//-----------------------------------------------------------------------------
//
// Rifle.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Rifle = class.extends(Weapon, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.modelfile      = "models/mdl665/mdl665.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -204.8004;
    this.origin.z       = -19.4555;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim05");
    
    this.state          = WS_READY;
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});
