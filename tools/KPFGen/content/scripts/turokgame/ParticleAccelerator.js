//-----------------------------------------------------------------------------
//
// ParticleAccelerator.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

ParticleAccelerator = class.extends(Weapon, function()
{
    this.modelfile      = "models/mdl668/mdl668.kmesh";
    this.model          = Sys.loadModel(this.modelfile);
    
    this.origin.x       = -170.667;
    this.origin.y       = -153.6003;
    this.origin.z       = -9.21548;
    
    this.anim_Idle      = Sys.loadAnimation(this.model, "anim00");
    this.anim_Walk      = Sys.loadAnimation(this.model, "anim01");
    this.anim_Run       = Sys.loadAnimation(this.model, "anim02");
    this.anim_Charge    = Sys.loadAnimation(this.model, "anim03");
    this.anim_Fire      = Sys.loadAnimation(this.model, "anim04");
    this.anim_SwapIn    = Sys.loadAnimation(this.model, "anim05");
    this.anim_SwapOut   = Sys.loadAnimation(this.model, "anim06");
    
    this.state          = WS_READY;
    this.spinRotation   = new Quaternion(0, 0, 1, 0);
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(ParticleAccelerator,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bCharging : false,
    spinSpeed : 0.0,
    spinAngle : 0.0,
    spinRotation : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    tick : function()
    {
        const ACCELERATOR_SPIN_SPEED = 3.0;
        
        if(this.state == WS_READY)
        {
            if(this.spinSpeed > ACCELERATOR_SPIN_SPEED)
                this.spinSpeed = Math.lerp(this.spinSpeed, ACCELERATOR_SPIN_SPEED, 0.075);
            else
                this.spinSpeed = ACCELERATOR_SPIN_SPEED;
        }
        else if(this.state == WS_FIRING)
            this.spinSpeed = Math.lerp(this.spinSpeed, 15.0, 0.008);
        
        this.spinAngle -= this.spinSpeed * Sys.deltatime();
            
        this.spinRotation.setRotation(this.spinAngle, 0, 1, 0);
        this.model.setNodeRotation(1, this.spinRotation);

        this.super.prototype.tick.bind(this)();
        
        if(this.state == WS_SWAPOUT || this.state == WS_HOLDSTER)
        {
            this.spinAngle = 0.0;
            this.spinSpeed = 0.0;
        }
    },
    
    checkAttack : function()
    {
        if(this.bCharging)
            return true;
        
        if(Client.localPlayer.command.getAction('+attack'))
        {
            this.animState.blendAnim(this.anim_Charge,
                this.playSpeed, 125.0, NRender.ANIM_LOOP);

            this.state = WS_FIRING;
            this.bCharging = true;
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
        
        if(Client.localPlayer.command.getActionHeldTime('+attack') > 0 && this.bCharging)
            return;
        else
        {
            this.bCharging = false;
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
