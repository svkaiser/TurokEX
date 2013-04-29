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
        if(ClientPlayer.component.controller.state == STATE_MOVE_CLIMB &&
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
        if(ClientPlayer.command.getAction('+attack'))
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
        if(ClientPlayer.command.getAction('+nextweap') &&
            ClientPlayer.command.getActionHeldTime('+nextweap') == 0)
        {
            ClientPlayer.component.cycleNextWeapon();
            return true;
        }
        
        if(ClientPlayer.command.getAction('+prevweap') &&
            ClientPlayer.command.getActionHeldTime('+prevweap') == 0)
        {
            ClientPlayer.component.cyclePrevWeapon();
            return true;
        }
        
        return false;
    },
    
    readyAnim : function()
    {
        if(this.animState.flags & NRender.ANIM_BLEND)
            return;
            
        if(Sys.getCvar('g_weaponbobbing') > 0)
        {
            this.animState.blendAnim(this.anim_Idle,
                this.playSpeed, 8.0, NRender.ANIM_LOOP);
            return;
        }
        
        var d = ClientPlayer.worldState.accel.unit2();
        
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
        if(ClientPlayer.component.controller.state != STATE_MOVE_CLIMB)
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
            ClientPlayer.component.setNewWeapon();
            
            var newWpn = ClientPlayer.component.activeWeapon;
            
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
            (ClientPlayer.command.mouse_x * 0.00175)) * 0.9;
            
        if(this.angles.yaw >  WEAPONTURN_MAX) this.angles.yaw =  WEAPONTURN_MAX;
        if(this.angles.yaw < -WEAPONTURN_MAX) this.angles.yaw = -WEAPONTURN_MAX;
        if(this.angles.yaw <  WEAPONTURN_EPSILON &&
            this.angles.yaw > -WEAPONTURN_EPSILON)
        {
            this.angles.yaw = 0;
        }
        
        this.angles.pitch = (this.angles.pitch -
            (ClientPlayer.command.mouse_y * 0.00175)) * 0.9;
            
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
        
        var wstate = ClientPlayer.worldState;
        var controller = ClientPlayer.component.controller;
        
        // lean weapon if strafing
        if(controller != null && controller.state != STATE_MOVE_SWIM)
        {
            var roll = new Quaternion(wstate.roll*1.5, 0, 1, 0);
            this.model.setNodeRotation(0, roll);
        }
        
        var mtx_rot = new Matrix();
        mtx_rot.setRotation(Quaternion.multiply(pitch, yaw));
        mtx_transform.applyVector(this.origin);
        
        var mtx_final = Matrix.multiply(mtx_rot, mtx_transform);
        
        if(wstate.plane != null)
        {
            // add a little vertical force to weapon if jumping or falling
            offset = (wstate.origin.y - wstate.plane.distance(wstate.origin));
            
            var velocity = wstate.velocity.y * wstate.frameTime;

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
            
            if(Sys.getCvar('g_weaponbobbing') > 0)
            {
                var bob_xz = 0;
                var bob_y = 0;
                
                var actor = ClientPlayer.actor;
            
                if(controller.state == STATE_MOVE_WALK &&
                    (wstate.origin.y + wstate.velocity.y) - wstate.plane.distance(wstate.origin) <
                    (actor.viewHeight + actor.centerHeight)+1)
                {
                    const WEPBOB_EPISILON  = 0.001;
                    const WEPBOB_MAXSWAY   = Angle.degToRad(22.5);
                    const WEPBOB_FREQ      = 0.007;
                    const WEPBOB_FREQY     = 0.014;
                    const WEPBOB_ANGLE     = 8;
                    
                    var d = Math.abs(wstate.accel.z * wstate.frameTime) * 0.06;
                    
                    if(d > WEPBOB_EPISILON)
                    {
                        if(d > WEPBOB_MAXSWAY)
                            d = WEPBOB_MAXSWAY;
                        
                        bob_xz = Math.sin(Sys.time() * WEPBOB_FREQ) * WEPBOB_ANGLE * d;
                        bob_y = Math.sin(Sys.time() * WEPBOB_FREQY) * WEPBOB_ANGLE * d;
                    }
                }
                
                mtx_final.addTranslation(bob_xz, this.thudOffset + bob_y, bob_xz);
            }
            else
                mtx_final.addTranslation(0, this.thudOffset, 0);
        }
        
        mtx_final.load();
        
        Render.drawModel(this.model, this.animState);
    },
    
    spawnFx : function(fx, x, y, z)
    {
        var actor = ClientPlayer.actor;

        var mtx = Matrix.fromQuaternion(actor.rotation);
        mtx.addTranslation(
            ClientPlayer.camera.origin.x,
            ClientPlayer.camera.origin.y,
            ClientPlayer.camera.origin.z);

        Sys.spawnFx(fx, actor,
            Vector.toWorld(new Vector(x, y, z), mtx),
            actor.rotation,
            Plane.fromIndex(actor.plane));
    }
});
