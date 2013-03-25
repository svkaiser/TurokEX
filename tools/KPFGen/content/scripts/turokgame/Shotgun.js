//-----------------------------------------------------------------------------
//
// Shotgun.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Shotgun = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl669/mdl669.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -170.667;
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

class.properties(Shotgun,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bReload : false,
    readySound : 'sounds/shaders/ready_shotgun.ksnd',
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkAttack : function()
    {
        if(this.super.prototype.checkAttack.bind(this)())
        {
            Snd.play('sounds/shaders/riot_shotgun_shot.ksnd');
            return true;
        }
        
        return false;
    },
    
    fire : function()
    {
        if(this.animState.playTime >= 0.5)
        {
            if(!this.bReload)
            {
                this.bReload = true;
                Snd.play('sounds/shaders/ready_shotgun.ksnd');
            }
        }
        else
            this.bReload = false;
        
        this.super.prototype.fire.bind(this)();
    }
});
