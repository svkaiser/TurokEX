//-----------------------------------------------------------------------------
//
// Rifle.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Weapon.js');

Rifle = class.extends(Weapon, function()
{
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
    
    this.animState.setAnim(this.anim_Idle, this.playSpeed, NRender.ANIM_LOOP);
});

class.properties(Rifle,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    bShotsFired : [false, false, false],
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    fire : function()
    {
        if(this.checkHoldster())
        {
            this.animState.flags &= ~NRender.ANIM_LOOP;
            return;
        }
        
        if(this.animState.playTime >= 0.0)
        {
            if(!this.bShotsFired[0])
            {
                this.bShotsFired[0] = true;
                Snd.play('sounds/shaders/rifle_shot.ksnd');
            }
        }
        
        if(this.animState.playTime >= 0.1)
        {
            if(!this.bShotsFired[1])
            {
                this.bShotsFired[1] = true;
                Snd.play('sounds/shaders/rifle_shot.ksnd');
            }
        }
        
        if(this.animState.playTime >= 0.2)
        {
            if(!this.bShotsFired[2])
            {
                this.bShotsFired[2] = true;
                Snd.play('sounds/shaders/rifle_shot.ksnd');
            }
        }
        
        if(this.animState.flags & NRender.ANIM_STOPPED)
        {
            this.readyAnim();
            this.state = WS_READY;
            
            this.bShotsFired[0] = false;
            this.bShotsFired[1] = false;
            this.bShotsFired[2] = false;
        }
    }
});
