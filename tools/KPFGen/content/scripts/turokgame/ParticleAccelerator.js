//-----------------------------------------------------------------------------
//
// ParticleAccelerator.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

ParticleAccelerator = class.extends(Weapon, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.modelfile      = "models/mdl668/mdl668.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -153.6003;
    this.origin.z       = -9.21548;
    
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
