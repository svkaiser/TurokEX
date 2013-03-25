//-----------------------------------------------------------------------------
//
// Weapon.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const WS_DEACTIVATED    = 0;
const WS_READY          = 1;
const WS_SWAPOUT        = 2;
const WS_SWAPIN         = 3;
const WS_FIRING         = 4;
const WS_HOLDSTER       = 5;

Weapon = class.define(function()
{
    this.origin         = new Vector(0, 0, -275.456);
    this.scale          = new Vector();
    this.angles         = { yaw : 0, pitch : 0, roll : 0 };
    this.animState      = new AnimState();
});

class.properties(Weapon,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    origin          : null,
    scale           : null,
    angles          : null,
    animState       : null,
    anim_Idle       : null,
    anim_Walk       : null,
    anim_Run        : null,
    anim_Fire       : null,
    anim_SwapIn     : null,
    anim_SwapOut    : null,
    
    state           : WS_DEACTIVATED,
    playSpeed       : 4.0,
    thudOffset      : 0.0,
    bOwned          : false,
    readySound      : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    checkHoldster : function()
    {
        if(Client.localPlayer.controller.state == STATE_MOVE_CLIMB &&
            this.state != WS_HOLDSTER)
        {
            this.animState.blendAnim(this.anim_SwapOut,
                this.playSpeed, 4.0, NRender.ANIM_NOINTERRUPT);

            this.state = WS_HOLDSTER;

            return true;
        }

        return false;
    },
    
    change : function()
    {
        this.animState.blendAnim(this.anim_SwapOut,
            this.playSpeed, 4.0, NRender.ANIM_NOINTERRUPT);

        this.state = WS_SWAPOUT;
    },
    
    checkAttack : function()
    {
        if(Client.localPlayer.command.getAction('+attack'))
        {
            this.animState.blendAnim(this.anim_Fire,
                this.playSpeed, 4.0, NRender.ANIM_NOINTERRUPT);

            this.state = WS_FIRING;
            return true;
        }
        
        return false;
    },
    
    checkWeaponChange : function()
    {
        if(Client.localPlayer.command.getAction('+nextweap') &&
            !Client.localPlayer.command.getActionHeldTime('+nextweap'))
        {
            var tPlayer = Client.localPlayer.controller.owner.components.ComponentTurokPlayer;
            
            if(tPlayer == undefined)
                return;
            
            tPlayer.cycleNextWeapon();
            return true;
        }
        
        if(Client.localPlayer.command.getAction('+prevweap') &&
            !Client.localPlayer.command.getActionHeldTime('+prevweap'))
        {
            var tPlayer = Client.localPlayer.controller.owner.components.ComponentTurokPlayer;
            
            if(tPlayer == undefined)
                return;
            
            tPlayer.cyclePrevWeapon();
            return true;
        }
        
        return false;
    },
    
    readyAnim : function()
    {
        if(this.animState.flags & NRender.ANIM_BLEND)
            return;
        
        var d = Client.localPlayer.prediction.accel.unit2();
        
        if(d >= 1.35)
        {
            this.animState.blendAnim(this.anim_Run,
                this.playSpeed, 8.0, NRender.ANIM_LOOP);
        }
        else if(d >= 0.1)
        {
            this.animState.blendAnim(this.anim_Walk,
                this.playSpeed, 8.0, NRender.ANIM_LOOP);
        }
        else
        {
            this.animState.blendAnim(this.anim_Idle,
                this.playSpeed, 8.0, NRender.ANIM_LOOP);
        }
    },
    
    ready : function()
    {
        if(this.checkHoldster())
            return;
            
        if(this.checkAttack())
            return;
            
        if(this.checkWeaponChange())
            return;
            
        this.readyAnim();
    },
    
    fire : function()
    {
        if(this.checkHoldster())
        {
            this.animState.flags &= ~NRender.ANIM_LOOP;
            return;
        }
        
        if(this.animState.flags & NRender.ANIM_STOPPED)
        {
            this.readyAnim();
            this.state = WS_READY;
        }
    },
    
    holdster : function()
    {
        if(Client.localPlayer.controller.state != STATE_MOVE_CLIMB)
        {
            this.animState.setAnim(this.anim_SwapIn,
                this.playSpeed, NRender.ANIM_NOINTERRUPT);

            this.state = WS_SWAPIN;
        }
    },
    
    swapOut : function()
    {
        if(this.checkHoldster())
            return;
            
        if(this.checkWeaponChange())
            return;
            
        if(this.animState.flags & NRender.ANIM_STOPPED)
        {
            var tPlayer = Client.localPlayer.controller.owner.components.ComponentTurokPlayer;
            
            if(tPlayer == undefined)
                return;
                
            tPlayer.setNewWeapon();
            
            var newWpn = tPlayer.activeWeapon;
            
            Snd.play(newWpn.readySound);
            newWpn.animState.setAnim(newWpn.anim_SwapIn,
                newWpn.playSpeed, NRender.ANIM_NOINTERRUPT);

            newWpn.state = WS_SWAPIN;
        }
    },
    
    swapIn : function()
    {
        if(this.checkHoldster())
            return;
            
        if(this.checkWeaponChange())
            return;
            
        if(this.animState.flags & NRender.ANIM_STOPPED)
        {
            this.readyAnim();
            this.state = WS_READY;
        }
    },
    
    tick : function()
    {
        const WEAPONTURN_MAX        = 0.08;
        const WEAPONTURN_EPSILON    = 0.001;
    
        this.angles.yaw = (this.angles.yaw -
            (Client.localPlayer.command.mouse_x * 0.1)) * 0.9;
            
        if(this.angles.yaw >  WEAPONTURN_MAX) this.angles.yaw =  WEAPONTURN_MAX;
        if(this.angles.yaw < -WEAPONTURN_MAX) this.angles.yaw = -WEAPONTURN_MAX;
        if(this.angles.yaw <  WEAPONTURN_EPSILON &&
            this.angles.yaw > -WEAPONTURN_EPSILON)
        {
            this.angles.yaw = 0;
        }
        
        this.angles.pitch = (this.angles.pitch -
            (Client.localPlayer.command.mouse_y * 0.1)) * 0.9;
            
        if(this.angles.pitch >  WEAPONTURN_MAX) this.angles.pitch =  WEAPONTURN_MAX;
        if(this.angles.pitch < -WEAPONTURN_MAX) this.angles.pitch = -WEAPONTURN_MAX;
        if(this.angles.pitch <  WEAPONTURN_EPSILON &&
            this.angles.pitch > -WEAPONTURN_EPSILON)
        {
            this.angles.pitch = 0;
        }
        
        switch(this.state)
        {
            case WS_READY:
                this.ready();
                break;
                
            case WS_SWAPOUT:
                this.swapOut();
                break;
                
            case WS_SWAPIN:
                this.swapIn();
                break;
                
            case WS_FIRING:
                this.fire();
                break;
                
            case WS_HOLDSTER:
                this.holdster();
                break;
                
            default:
                break;
        }

        if(this.state != WS_DEACTIVATED)
            this.animState.update();
    },
    
    draw : function()
    {
        if(this.model == null)
            return;

        Matrix.setProjection(45, 32);
        Matrix.setModelView();
        
        var mtx_pos = new Matrix();
        var mtx_flip = new Matrix();
        
        mtx_flip.scale(-1, 1, 1);
        mtx_pos.transpose();
        
        var mtx_transform = Matrix.multiply(mtx_pos, mtx_flip);
        
        var yaw = new Quaternion(this.angles.yaw, 0, 0, 1);
        var pitch = new Quaternion(this.angles.pitch, 1, 0, 0);
        
        var pmove = Client.localPlayer.prediction;
        
        // lean weapon if strafing
        if(Client.localPlayer.controller.state != STATE_MOVE_SWIM)
        {
            var roll = new Quaternion(pmove.angles.roll*1.5, 0, 1, 0);
            this.model.setNodeRotation(0, roll);
        }
        
        var mtx_rot = new Matrix();
        mtx_rot.setRotation(Quaternion.multiply(pitch, yaw));
        mtx_transform.applyVector(this.origin);
        
        var mtx_final = Matrix.multiply(mtx_rot, mtx_transform);
        
        if(pmove.plane != null)
        {
            // add a little vertical force to weapon if jumping or falling
            offset = (pmove.origin.y - pmove.plane.distance(pmove.origin));
            
            var velocity = pmove.velocity.y * pmove.frametime;

            if(!(offset < 0.2) && (velocity < 0.2 || velocity > 0.2))
            {
                offset = velocity;
                if(velocity > 0)
                {
                    // cut back offset a little if jumping
                    offset *= 0.35;
                }
            }
            else
            {
                offset = 0;
            }
            
            this.thudOffset = Math.lerp(this.thudOffset, offset, 0.25);
            mtx_final.addTranslation(0, this.thudOffset, 0);
        }
        
        mtx_final.load();
        
        Render.drawModel(this.model, this.animState);
    }
});
