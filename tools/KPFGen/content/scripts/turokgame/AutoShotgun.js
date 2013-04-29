//-----------------------------------------------------------------------------
//
// AutoShotgun.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

AutoShotgun = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl642/mdl642.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -177.49368;
    this.origin.z       = -2.3888;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim03");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim05");
    
    this.state          = WS_READY;
    this.spinRotation   = new Quaternion(0, 1, 0, 0);
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(AutoShotgun,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bReload : false,
    spinAngle : 0.0,
    spinRotation : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    tick : function()
    {
        if(this.spinAngle > 0)
            this.spinAngle -= 0.03;
        else
            this.spinAngle = 0;
        
        this.spinRotation.setRotation(this.spinAngle, 1, 0, 0);
        this.model.setNodeRotation(1, this.spinRotation);

        this.super.prototype.tick.bind(this)();
    },
    
    checkAttack : function()
    {
        if(ClientPlayer.command.getAction('+attack'))
        {
            this.spawnFx('fx/muzzle_shotgun.kfx', -6.144, -1.9456, 15.696);
            this.spawnFx('fx/shotshell.kfx', -14.336, -12.288, 20.736);
            ClientPlayer.component.aAutoShotgunAttack();
            
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 6.0, NRender.ANIM_NOINTERRUPT);

            this.bReload = false;
            this.state = WS_FIRING;
            this.spinAngle = Angle.degToRad(36);
            return true;
        }
        
        return false;
    },
    
    fire : function()
    {
        if(this.animState.playTime >= 0.0125)
        {
            if(!this.bReload)
            {
                this.bReload = true;
                Snd.play('sounds/shaders/reload_auto_shotgun.ksnd');
            }
        }
        else
            this.bReload = false;
        
        this.super.prototype.fire.bind(this)();
    }
});
