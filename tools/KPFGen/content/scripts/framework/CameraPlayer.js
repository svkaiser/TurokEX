//-----------------------------------------------------------------------------
//
// CameraPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Camera.js');

CameraPlayer = class.extends(Camera);

class.properties(CameraPlayer,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    viewoffset : 0.0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    tick : function()
    {
        var ctrl            = Client.localPlayer.controller;
        var pmove           = Client.localPlayer.prediction;
        
        if(this.viewoffset != ctrl.view_y)
            this.viewoffset = Math.lerp(this.viewoffset, ctrl.view_y, 0.125);
        
        this.origin.x       = pmove.origin.x;
        this.origin.y       = pmove.origin.y + this.viewoffset + ctrl.center_y;
        this.origin.z       = pmove.origin.z;
        
        this.angles.yaw     = pmove.angles.yaw;
        this.angles.pitch   = pmove.angles.pitch;
        this.angles.roll    = pmove.angles.roll;
    },
    
    draw : function()
    {
        const VIEWBOB_EPISILON      = 0.001;
        const VIEWBOB_MAXSWAY       = Angle.degToRad(22.5);
        const VIEWBOB_FREQX         = 0.02;
        const VIEWBOB_FREQY         = 0.01;
        const VIEWBOB_ANGLE         = 0.0218;
        const VIEWBOB_SWIMFREQX     = 0.00200;
        const VIEWBOB_SWIMFREQY     = 0.00125;
        const VIEWBOB_SWIMANGLE     = 0.025;
    
        var pmove = Client.localPlayer.prediction;
        var ctrl = Client.localPlayer.controller;
        var bob_x = 0;
        var bob_y = 0;
        
        if(ctrl.state == STATE_MOVE_WALK &&
            (this.origin.y + pmove.velocity.y) - pmove.plane.distance(this.origin) <
                (ctrl.view_y + ctrl.center_y)+1)
        {
            // calculate bobbing
            var d = Math.abs(pmove.accel.z * pmove.frametime) * 0.06;
            
            if(d > VIEWBOB_EPISILON)
            {
                if(d > VIEWBOB_MAXSWAY)
                    d = VIEWBOB_MAXSWAY;
                
                bob_x = Math.sin(Sys.time() * VIEWBOB_FREQX) * VIEWBOB_ANGLE * d;
                bob_y = Math.sin(Sys.time() * VIEWBOB_FREQY) * VIEWBOB_ANGLE * d;
            }
        }
        else if(ctrl.state == STATE_MOVE_SWIM)
        {
            bob_x = Math.sin(Sys.time() * VIEWBOB_SWIMFREQX) * VIEWBOB_SWIMANGLE;
            bob_y = Math.sin(Sys.time() * VIEWBOB_SWIMFREQY) * VIEWBOB_SWIMANGLE;
        }
        
        this.angles.yaw -= bob_y;
        this.angles.pitch += bob_x;
        
        this.super.prototype.draw.bind(this)();
    }
});
