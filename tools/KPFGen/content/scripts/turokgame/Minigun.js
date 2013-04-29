//-----------------------------------------------------------------------------
//
// Minigun.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Minigun = class.extends(Weapon, function()
{
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
    this.spinRotation   = new Quaternion(0, 0, 1, 0);
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(Minigun,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    spinSpeed : 0.0,
    spinAngle : 0.0,
    spinRotation : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    tick : function()
    {
        if(this.state == WS_FIRING)
            this.spinSpeed = Math.lerp(this.spinSpeed, 40.0, 0.35);
        else
            this.spinSpeed = Math.lerp(this.spinSpeed, 0.0, 0.025);
        
        this.spinAngle += this.spinSpeed * Sys.deltatime();
        this.spinRotation.setRotation(this.spinAngle, 0, 1, 0);
        this.model.setNodeRotation(1, this.spinRotation);

        this.super.prototype.tick.bind(this)();
    },
    
    checkAttack : function()
    {
        if(ClientPlayer.command.getAction('+attack'))
        {
            Snd.play('sounds/shaders/mini_gun_whir.ksnd', ClientPlayer.actor);
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 4.0, NRender.ANIM_LOOP);

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
        
        if(this.animState.playTime >= 0.11)
        {
            this.animState.playTime -= 0.11;
            // TODO
            this.spawnFx('fx/fx_037.kfx', -4.608, -3.1744, 14.848);
            this.spawnFx('fx/bulletshell.kfx', -10.24, -10.24, 13.82);
            Snd.play('sounds/shaders/mini_gun_shot.ksnd');
        }
        
        if(!ClientPlayer.command.getAction('+attack'))
        {
            Snd.play('sounds/shaders/minigun_stop.ksnd');
            this.readyAnim();
            this.state = WS_READY;
        }
    }
});
