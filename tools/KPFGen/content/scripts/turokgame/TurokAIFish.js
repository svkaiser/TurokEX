//-----------------------------------------------------------------------------
//
// TurokAIFish.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokAIFish = class.extendStatic(ComponentTurokAI, function()
{
    ComponentTurokAI.prototype.constructor.bind(this)();
});

class.properties(TurokAIFish,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    anim_Wander     : null,
    anim_Flop       : null,
    bFlopping       : false,
    moveTime        : 0,
    facingYaw       : 0.0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkInWater : function()
    {
        var plane = this.controller.plane;
        
        if(plane != null && plane.area != undefined)
        {
            var areaWater = plane.area.ComponentAreaWater;
            
            if(areaWater != undefined)
                return areaWater.getWaterLevel(this.controller);
        }
        
        return WL_INVALID;
    },
    
    start : function()
    {
    },
    
    onReady : function()
    {
        this.anim_Wander    = Sys.loadAnimation(this.parent.owner.model, "anim00");
        this.anim_Flop      = Sys.loadAnimation(this.parent.owner.model, "anim02");
        
        ComponentTurokAI.prototype.onReady.bind(this)();
    },
    
    onTick : function()
    {
        var actor = this.parent.owner;
        this.controller.updateFromActor(actor, this);
        
        var plane = this.controller.plane;
        
        if(plane != null)
        {
            // flop around
            if(this.checkInWater() <= WL_OVER)
            {
                if(this.controller.onGround())
                {
                    if(Sys.rand(100) > 95)
                    {
                        this.controller.velocity.x = -50 + (Sys.rand(100));
                        this.controller.velocity.y = 100 + (Sys.rand(100));
                        this.controller.velocity.z = -50 + (Sys.rand(100));
                    }
                }
                else if(this.bFlopping == true)
                {
                    this.controller.angles.yaw += 0.0872;
                }
                
                if(this.bFlopping == false)
                {
                    this.bFlopping = true;
                    actor.animState.blendAnim(this.anim_Flop,
                        2.0, 4.0, NRender.ANIM_NOINTERRUPT | NRender.ANIM_LOOP);
                }
            }
            else if(this.bFlopping == true)
            {
                this.bFlopping = false;
                actor.animState.blendAnim(this.anim_Wander,
                    2.0, 4.0, NRender.ANIM_NOINTERRUPT | NRender.ANIM_LOOP | NRender.ANIM_ROOTMOTION);
            }
            else // wander around
            {
                const HALF_PI = Angle.degToRad(90);
                var dir = Vector.applyRotation(actor.animState.rootMotion, actor.rotation);
                dir.scale(0.5);
                this.controller.velocity.add(dir);
                dir.normalize();
                
                var t = Physics.rayTrace(this.controller.origin,
                    dir, 1, 128, plane, actor);
                
                if(t != null && this.moveTime <= 0)
                {
                    var an = Angle.invertClampSum(this.controller.angles.yaw, t.hitNormal.toYaw());
                    this.moveTime = Sys.rand(80) + 40;
                    
                    // turn away from obstacle
                    if(an < 0)
                        this.facingYaw = Angle.clamp(this.controller.angles.yaw + HALF_PI);
                    else
                        this.facingYaw = Angle.clamp(this.controller.angles.yaw - HALF_PI);
                }
                else if(this.moveTime <= 0 && Sys.rand(100) >= 95)
                {
                    // pick a random direction to turn to
                    this.moveTime = Sys.rand(40) + 40;
                    this.facingYaw = Angle.clamp(actor.yaw + (-0.785 + (HALF_PI * Sys.cRand())));
                }
                
                // process turning
                if(this.moveTime > 0)
                {
                    this.moveTime--;
                    this.controller.angles.yaw = Angle.clamp(this.facingYaw -
                        this.controller.angles.yaw) * 0.017 + this.controller.angles.yaw;
                }
            }
        }
        
        if(this.controller.frametime < 1 && this.controller.frametime > 0)
        {
            var wl = this.checkInWater();
            var c = this.controller;
        
            if(wl != WL_UNDER)
                c.gravity(c.mass);
            
            if(wl == WL_UNDER && c.velocity.y != 0)
                applyVerticalFriction(0.125);
            
            c.beginMovement();
            
            if(c.onGround() || wl == WL_UNDER)
                c.applyFriction(c.friction);
        }
        
        this.controller.updateActor(actor);
        
        actor.animState.update();
    },
    
    onLocalTick : function()
    {
    }
});
