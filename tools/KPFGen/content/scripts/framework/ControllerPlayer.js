//-----------------------------------------------------------------------------
//
// ControllerPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const STATE_MOVE_NONE   = 0;
const STATE_MOVE_WALK   = 1;
const STATE_MOVE_AIR    = 2;
const STATE_MOVE_CLIMB  = 3;
const STATE_MOVE_SWIM   = 4;
const STATE_MOVE_NOCLIP = 5;

const WL_INVALID        = 0;
const WL_OVER           = 1;
const WL_BETWEEN        = 2;
const WL_UNDER          = 3;

ControllerPlayer = class.extends(Controller, function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    const CRAWL_FLOORHEIGHT         = 15.36;
    
    const ANGLE_MAXPITCH            = Angle.degToRad(90);
    
    const JUMP_VELOCITY             = 768;
    const JUMP_GROUNDEPISILON       = 0.512;
    const JUMP_SLOPEHEIGHTMIN       = 8;
    
    const WALK_STRAFEROLL_ANGLE     = 0.0835;
    const WALK_STRAFEROLL_SPEED     = 0.1;

    const SLOPESLIDE_DIST           = 10.24;
    const SLOPESLIDE_VELOCITY       = 8.0;
    const SLOPESLIDE_LERP           = 0.5;

    const MOVE_FRICTION             = 1.0;
    const MOVE_GRAVITY              = 2232;

    const WALKSPEED_F_FORWARD       = 384;
    const WALKSPEED_F_BACKWARD      = -292;
    const WALKSPEED_F_ACCEL         = 0.09;
    const WALKSPEED_F_DEAACCEL      = 0.5;

    const WALKSPEED_R_FORWARD       = 384;
    const WALKSPEED_R_BACKWARD      = -384;
    const WALKSPEED_R_ACCEL         = 0.25;
    const WALKSPEED_R_DEAACCEL      = 0.5;
    
    const AIRSPEED_F_FORWARD        = 384;
    const AIRSPEED_F_BACKWARD       = -292;
    const AIRSPEED_F_ACCEL          = 0.05;
    const AIRSPEED_F_DEAACCEL       = 0.25;

    const AIRSPEED_R_FORWARD        = 384;
    const AIRSPEED_R_BACKWARD       = -384;
    const AIRSPEED_R_ACCEL          = 0.05;
    const AIRSPEED_R_DEAACCEL       = 0.25;
    
    const SWIMROLL_MAXANGLE         = 0.33;
    const SWIMROLL_SPEED            = 0.05;

    const SWIM_FORWARDSPEED         = 245.76;
    const SWIM_BACKSPEED            = 153.5;
    const SWIM_SIDESPEED            = 245.76;
    const SWIM_UPSPEED              = 338;
    const SWIM_ACCELERATION         = 0.01953125;
    const SWIM_DEACCELERATION       = 0.05;
    const SWIM_UPDEACCELERATION     = 0.111;
    const SWIM_THRUSTSPEED          = 480;
    const SWIM_THRUSTDEACCELERATE   = 0.02;

    // TODO: the original game specifies '6.2832' but time is calculated differently here
    const SWIM_MOVETIME             = 512;
    
    const CLIMBSPEED_F_FORWARD      = 215;
    const CLIMBSPEED_F_BACKWARD     = 0;
    const CLIMBSPEED_F_ACCEL        = 0.0625;
    const CLIMBSPEED_F_DEAACCEL     = 0.5;
    
    const CLIMB_DIST                = 1;
    const CLIMB_FACEANGLE           = 40.0;
    const CLIMB_YAWLERP             = 0.084;
    const CLIMB_BOBANGLE            = 0.33;
    const CLIMB_BOBSPEED            = 0.025;
    const CLIMB_BOBWINDOWN          = 0.035;
    
    const CLIMB_LEAP_AMNT           = 353.28;

    // TODO: the original game specifies '11' but time is calculated differently here
    const CLIMB_MOVETIME            = 687;
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.state                      = STATE_MOVE_NONE;          // current move state
    this.command                    = null;                     // player's command object is copied here
    this.mouse_x                    = 0.0;
    this.mouse_y                    = 0.0;
    this.bCrawling                  = false;                    // moving at half the view height and speed
    this.bNoClip                    = false;                    // free movement/clipping
    this.bFlying                    = false;
    
    // TODO
    this.center_y                   = 0;
    this.view_y                     = 0;
    
    this.waterheight                = 0;                        // the Y-plane height of a water plane
    this.waterlevel                 = WL_INVALID;               // used to determine swim movement behavior
    this.bLerping                   = false;                    // pulled back to surface?
    this.bRollDir                   = true;                     // view roll bobbing for climbing
    
    //------------------------------------------------------------------------
    // OBJECTS
    //------------------------------------------------------------------------
    
    this.speed = [
    // STATE_MOVE_NONE
    {
        forward :
        {
            forwardspeed    : 0,
            backspeed       : 0,
            acceleration    : 0,
            deacceleration  : 0
        },
        
        right :
        {
            forwardspeed    : 0,
            backspeed       : 0,
            acceleration    : 0,
            deacceleration  : 0
        }
    },
    // STATE_MOVE_WALK
    {
        forward :
        {
            forwardspeed    : WALKSPEED_F_FORWARD,
            backspeed       : WALKSPEED_F_BACKWARD,
            acceleration    : WALKSPEED_F_ACCEL,
            deacceleration  : WALKSPEED_F_DEAACCEL
        },
        
        right :
        {
            forwardspeed    : WALKSPEED_R_FORWARD,
            backspeed       : WALKSPEED_R_BACKWARD,
            acceleration    : WALKSPEED_R_ACCEL,
            deacceleration  : WALKSPEED_R_DEAACCEL
        }
    },
    // STATE_MOVE_AIR
    {
        forward :
        {
            forwardspeed    : AIRSPEED_F_FORWARD,
            backspeed       : AIRSPEED_F_BACKWARD,
            acceleration    : AIRSPEED_F_ACCEL,
            deacceleration  : AIRSPEED_F_DEAACCEL
        },
        
        right :
        {
            forwardspeed    : AIRSPEED_R_FORWARD,
            backspeed       : AIRSPEED_R_BACKWARD,
            acceleration    : AIRSPEED_R_ACCEL,
            deacceleration  : AIRSPEED_R_DEAACCEL
        }
    },
    // STATE_MOVE_CLIMB
    {
        forward :
        {
            forwardspeed    : CLIMBSPEED_F_FORWARD,
            backspeed       : CLIMBSPEED_F_BACKWARD,
            acceleration    : CLIMBSPEED_F_ACCEL,
            deacceleration  : CLIMBSPEED_F_DEAACCEL
        },
        
        right :
        {
            forwardspeed    : 0,
            backspeed       : 0,
            acceleration    : 0,
            deacceleration  : 0
        }
    },
    // STATE_MOVE_SWIM
    {
        forward :
        {
            forwardspeed    : SWIM_FORWARDSPEED,
            backspeed       : -SWIM_BACKSPEED,
            acceleration    : SWIM_ACCELERATION,
            deacceleration  : SWIM_DEACCELERATION
        },
        
        right :
        {
            forwardspeed    : SWIM_SIDESPEED,
            backspeed       : -SWIM_SIDESPEED,
            acceleration    : SWIM_ACCELERATION,
            deacceleration  : SWIM_DEACCELERATION
        }
    },
    // STATE_MOVE_NOCLIP
    {
        forward :
        {
            forwardspeed    : 1024,
            backspeed       : -1024,
            acceleration    : 0.1,
            deacceleration  : 0.5
        },
        
        right :
        {
            forwardspeed    : 1024,
            backspeed       : -1024,
            acceleration    : 0.1,
            deacceleration  : 0.5
        }
    }];
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.mouseAccel_r = function(val)
    {
        var accel = Sys.getCvar('cl_macceleration');
        
        if(accel == 0.0)
            return val;
            
        if(val < 0)
            return this.mouseAccel_r(-val);
            
        return Math.pow(val, accel / 200.0 + 1.0);
    }
    
    // apply sensitivity to mouse movement
    this.mouseMove = function(x, y)
    {
        var amt = 128.0;
        
        if(this.state == STATE_MOVE_SWIM)
            amt = 256.0;
        
        this.mouse_x += (this.mouseAccel_r(x) * Sys.getCvar('cl_msensitivityx')) / amt;
        this.mouse_y += (this.mouseAccel_r(y) * Sys.getCvar('cl_msensitivityy')) / amt;
    }
    
    // update angles for player command and handle pitch clamping
    this.updateCommandAngles = function(command)
    {
        command.mouse_x = Angle.degToRad(this.mouse_x);
        command.mouse_y = Angle.degToRad(this.mouse_y);
        
        command.angle_x -= command.mouse_x;
        command.angle_y -= command.mouse_y;
        
        if(command.angle_y > ANGLE_MAXPITCH) command.angle_y = ANGLE_MAXPITCH;
        if(command.angle_y < -ANGLE_MAXPITCH) command.angle_y = -ANGLE_MAXPITCH;
        
        this.mouse_x = 0.0;
        this.mouse_y = 0.0;
    }
    
    // update controller values from prediction result
    this.updateFromPrediction = function(pmove)
    {
        this.origin         = pmove.origin;
        this.velocity       = pmove.velocity;
        this.accel          = pmove.accel;
        this.timestamp      = pmove.timestamp;
        this.frametime      = pmove.frametime;
        this.plane          = pmove.plane;
        this.angles.yaw     = pmove.angles.yaw;
        this.angles.pitch   = pmove.angles.pitch;
        this.angles.roll    = pmove.angles.roll;
    }
    
    this.jump = function()
    {
        if(this.bCrawling)
            return;
        
        this.velocity.y = JUMP_VELOCITY;
        
        // TODO - determine if client or server
        if(this == Client.localPlayer.controller)
            Snd.play('sounds/shaders/generic_21_turok_jump.ksnd');
    }
    
    this.checkCrawlSpace = function()
    {
        if(this.bCrawling)
        {
            var t = 80 * this.frametime;
            
            if(t != 0)
            {
                this.accel.x *= (1 / t);
                this.accel.z *= (1 / t);
            }
        }
    }
    
    this.getClimbDistance = function(plane)
    {
        var n = plane.normal;
        
        return (Vector.dot(this.origin, n) - 
                Vector.dot(plane.pt1, n));
    }
    
    // determines if player is facing the climbable surface
    this.facingPlane = function(plane)
    {
        // don't bother if we're behind it
        if(this.getClimbDistance(plane) < 0)
            return false;
        
        var angle = Angle.diff(Angle.clamp(plane.getYaw()),
            this.angles.yaw + Math.PI);
        
        if(angle < 0)
            angle = -angle;
        
        if(Angle.radToDeg(angle) > CLIMB_FACEANGLE)
            return false;
            
        return true;
    }
    
    this.findClimbablePlane = function()
    {
        if(this.plane == null)
            return null;
        
        // test the plane that we're standing in
        if(this.plane.area.ComponentAreaClimb)
            return this.plane;
            
        return null;
    }
    
    this.canClimb = function()
    {  
        var plane = this.findClimbablePlane();
            
        if(plane != null)
        {
            var y = this.origin.y + this.center_y;
            
            if(this.state != STATE_MOVE_CLIMB)
            {
                // too far from the plane? ignore if we're already climbing
                if(y > plane.pt1.y &&
                    y > plane.pt2.y &&
                    y > plane.pt3.y)
                {
                    return false;
                }
                
                var dist = this.getClimbDistance(this.plane);
                if(dist > 4.096 || dist <= -4.096)
                    return false;
            }
            
            this.velocity.clear();
            return true;
        }
            
        return false;
    }
    
    // always forces the player's yaw to face the surface
    this.turnAndFacePlane = function()
    {
        var yaw;
        
        yaw = Angle.clamp(Angle.diff(this.angles.yaw + Math.PI,
            this.plane.getYaw()));
            
        this.angles.yaw = -yaw * CLIMB_YAWLERP + this.angles.yaw;
        this.command.angle_x = this.angles.yaw;
    }
    
    this.lerpToPlane = function()
    {
        var plane = this.plane;
        var dist = this.getClimbDistance(plane);
        
        this.velocity.x = -(plane.normal.x * dist) * 40.96;
        this.velocity.z = -(plane.normal.z * dist) * 40.96;
    }
    
    this.canClimbLeap = function()
    {
        if(this.state != STATE_MOVE_CLIMB)
            return false;
        
        if(this.command.getAction('+jump') && !this.command.getActionHeldTime('+jump'))
        {
            this.jump();
            
            var an = this.angles.yaw;
            
            this.accel.x =  Math.sin(an) * CLIMB_LEAP_AMNT;
            this.accel.z =  Math.cos(an) * CLIMB_LEAP_AMNT;
            this.state = STATE_MOVE_AIR;
            
            return true;
        }
        
        return false;
    }
    
    this.checkWater = function()
    {
        var p = this.plane;
        
        if(p != null && p.area.ComponentAreaWater)
        {
            this.waterheight = p.area.ComponentAreaWater.waterYPlane;
            
            if(((this.origin.y - this.center_y) - this.plane.distance(this.origin)) +
                (this.waterheight - this.origin.y) >= this.center_y)
            {
                if(this.origin.y + this.center_y >= this.waterheight)
                {
                    if(this.origin.y < this.waterheight)
                        return WL_BETWEEN;
                    else
                        return WL_OVER;
                }
                
                return WL_UNDER;
            }
        }

        return WL_INVALID;
    }
    
    this.inWater = function()
    {
        var oldLevel = this.waterlevel;
        this.waterlevel = this.checkWater();
        
        if(oldLevel == WL_UNDER && this.waterlevel == WL_BETWEEN)
        {
            // TODO - determine if client or server
            if(this == Client.localPlayer.controller)
                Snd.play('sounds/shaders/generic_16_turok_small_water_gasp.ksnd');
        }
        
        if(this.waterlevel == WL_INVALID || this.waterlevel == WL_OVER)
            return false;
        
        if(this.waterlevel == WL_UNDER && oldLevel != WL_UNDER)
        {
            // TODO - determine if client or server
            if(this == Client.localPlayer.controller)
                Snd.play('sounds/shaders/water_splash_1.ksnd');
        }
        
        if(this.state != STATE_MOVE_SWIM)
        {
            this.accel.y = this.velocity.y;
            return true;
        }
        
        return true;
    }
    
    // basic jumping routine checks
    this.canJump = function()
    {
        var plane = this.plane;
        var origin = this.origin;
        
        if(this.state == STATE_MOVE_AIR)
        {
            if(origin.y - plane.distance(origin) > JUMP_GROUNDEPISILON)
                return true;
            else
                return false;
        }
        
        if(this.command.getAction('+jump') && !this.command.getActionHeldTime('+jump'))
        {
            var velocity = this.velocity;
            
            // can't jump if standing on a steep slope
            if(plane.isAWall() && origin.y - plane.distance(origin) <= JUMP_SLOPEHEIGHTMIN)
                return false;
            
            // must be standing on ground or walking off a ledge
            if(origin.y - plane.distance(origin) <= JUMP_GROUNDEPISILON ||
                (velocity.y < 0 && velocity.y > -JUMP_VELOCITY))
            {
                this.jump();
                return true;
            }
        }
        
        return false;
    }
    
    // rolls the player's view when strafing
    this.rollView = function()
    {
        if(this.command.getAction('+strafeleft'))
        {
            this.angles.roll = Math.lerp(this.angles.roll,
                -WALK_STRAFEROLL_ANGLE, WALK_STRAFEROLL_SPEED);
        }
        else if(this.command.getAction('+straferight'))
        {
            this.angles.roll = Math.lerp(this.angles.roll,
                WALK_STRAFEROLL_ANGLE, WALK_STRAFEROLL_SPEED);
        }
        else
        {
            this.angles.roll = Math.lerp(this.angles.roll,
                0, WALK_STRAFEROLL_SPEED);
        }
    }
    
    // roll the player's view when strafing or turning while underwater
    this.swimRoll = function()
    {
        var roll = 0;
        
        if(this.waterlevel == WL_UNDER)
        {
            var t = 60 * this.frametime;
            
            if(t != 0)
                roll = this.command.mouse_x * 12 * 1 / t;
        
            if(roll > SWIMROLL_MAXANGLE)
                roll = SWIMROLL_MAXANGLE;
                
            if(roll < -SWIMROLL_MAXANGLE)
                roll = -SWIMROLL_MAXANGLE;
        }
        
        if(this.command.getAction('+strafeleft'))
            roll += -(SWIMROLL_MAXANGLE / 2);
        else if(this.command.getAction('+straferight'))
            roll += (SWIMROLL_MAXANGLE / 2);
            
        this.angles.roll = Math.lerp(this.angles.roll,
            roll, SWIMROLL_SPEED);
    }
    
    this.checkThrust = function()
    {
        if((this.command.getAction('+forward') && !this.command.getActionHeldTime('+forward')) &&
            this.movetime < this.timestamp)
        {
            this.movetime = SWIM_MOVETIME + this.timestamp;
            this.accel.z = SWIM_THRUSTSPEED;
            
            // TODO - determine if client or server
            if(this == Client.localPlayer.controller)
            {
                if(this.waterlevel == WL_UNDER)
                    Snd.play('sounds/shaders/underwater_swim_2.ksnd');
                else
                    Snd.play('sounds/shaders/water_splash_2.ksnd');
            }
        }
    }
    
    this.lerpToSurface = function()
    {
        this.bLerping = false;
        
        if(this.waterlevel == WL_UNDER && this.accel.y > 0)
        {
            var viewy = this.origin.y + this.center_y + 
                this.view_y;
            
            if(viewy > this.waterheight)
            {
                this.bLerping = true;
                
                this.origin.y =
                    this.lerp(this.origin.y, this.waterheight, 0.1);
                
                if(this.origin.y >= (this.waterheight - this.center_y))
                {
                    this.accel.y = 0;
                    this.velocity.y = 0;
                }
            }
        }
    }
    
    this.getSinkHeight = function()
    {
        var p = this.plane;
        
        if(p == null)
            return 0.0;
            
        var origin = this.origin;
        var dist = this.plane.distance(origin);
        
        if(origin.y - dist <= 0.512)
            return 0.0;
            
        return dist;
    }
    
    // slowly drift towards the bottom, but will remain floating slightly
    // above the floor/terrain
    this.sink = function()
    {
        if(this.bLerping)
            return;
            
        if(this.frametime == 0.0)
            return;
        
        if(this.waterlevel == WL_UNDER &&
            (this.velocity.y < 0.125 && this.velocity.y > -0.125))
        {
            var sink = this.origin.y - this.getSinkHeight();
            
            if(sink * 0.125 >= 2.0)
                sink = sink * 0.125;
            else
                sink = 2.0;
            
            this.velocity.y =
                this.lerp(this.velocity.y, (sink * 4) / this.frametime, -0.0035);
        }
    }
    
    this.slideOnSlope = function()
    {
        var plane = this.plane;
        var origin = this.origin;
        var velocity = this.velocity;
        
        if(plane.isAWall() && origin.y - plane.distance(origin) <= SLOPESLIDE_DIST)
        {
            velocity.y = this.lerp(velocity.y, -SLOPESLIDE_VELOCITY, SLOPESLIDE_LERP);
        }
    }
    
    this.noClipMove = function()
    {
        this.setDirection(
            this.angles.pitch,
            this.angles.yaw,
            0);
            
        this.accelerate(this.speed[STATE_MOVE_NOCLIP]);
        this.deAccelY(0.5);
        this.origin.x += (this.velocity.x * this.frametime);
        this.origin.y += (this.velocity.y * this.frametime);
        this.origin.z += (this.velocity.z * this.frametime);
        this.velocity.clear();
    }
    
    this.flyMove = function()
    {
        this.setDirection(
            this.angles.pitch,
            this.angles.yaw,
            0);
            
        this.accelerate(this.speed[STATE_MOVE_NOCLIP]);
        this.deAccelY(0.5);
        this.super.prototype.beginMovement.bind(this)();
        this.velocity.clear();
    }
    
    this.walkMove = function()
    {
        this.setDirection(
            0,
            this.angles.yaw,
            0);
            
        this.accelerate(this.speed[STATE_MOVE_WALK]);
        this.rollView();
        this.slideOnSlope();
        this.checkCrawlSpace();
        this.super.prototype.beginMovement.bind(this)();
        this.gravity(MOVE_GRAVITY);
        this.applyFriction(MOVE_FRICTION);
        
        var pl = this.plane;
        var dist = pl.distance(this.origin);
        
        if(!pl.isAWall() && (this.origin.y - dist) < 10.24)
            this.origin.y = dist;
    }
    
    this.airMove = function()
    {
        this.setDirection(
            0,
            this.angles.yaw,
            0);
            
        this.accelerate(this.speed[STATE_MOVE_AIR]);
        this.super.prototype.beginMovement.bind(this)();
        this.gravity(MOVE_GRAVITY);
        this.applyFriction(MOVE_FRICTION);
    }
    
    this.climbMove = function()
    {
        this.turnAndFacePlane();
        
        // move along plane normal
        this.setDirection(
            0,
            this.plane.normal.toYaw(),
            0);
        
        if(this.command.getAction('+forward') && this.movetime < this.timestamp)
        {
            this.movetime = CLIMB_MOVETIME + this.timestamp;
            this.accel.clear();
            this.velocity.clear();
            
            if(this.bRollDir == true)
            {
                // TODO - determine if client or server
                if(this == Client.localPlayer.controller)
                    Snd.play('sounds/shaders/generic_23_turok_climb_1.ksnd');
                    
                this.bRollDir = false;
            }
            else
            {
                // TODO - determine if client or server
                if(this == Client.localPlayer.controller)
                    Snd.play('sounds/shaders/generic_24_turok_climb_2.ksnd');
                    
                this.bRollDir = true;
            }
        }
        
        var speed = this.speed[STATE_MOVE_CLIMB];
        
        if(this.movetime >= this.timestamp)
        {
            this.accelZ(speed.forward.forwardspeed, speed.forward.acceleration);
            this.accelY(speed.forward.forwardspeed, speed.forward.acceleration);
            
            if(this.bRollDir == true)
            {
                this.angles.roll = Math.lerp(this.angles.roll,
                    CLIMB_BOBANGLE, CLIMB_BOBSPEED);
            }
            else
            {
                this.angles.roll = Math.lerp(this.angles.roll,
                    -CLIMB_BOBANGLE, CLIMB_BOBSPEED);
            }
        }
        else
        {
            this.deAccelZ(speed.forward.deacceleration);
            this.deAccelY(speed.forward.deacceleration);
            
            // wind down the view roll
            this.angles.roll = Math.lerp(this.angles.roll, 0, CLIMB_BOBWINDOWN);
        }
        
        this.velocity.add(Vector.gScale(this.forward, this.accel.z));
        this.velocity.y = this.accel.y;
        
        this.lerpToPlane();
        Physics.move(this);
            
        this.applyFriction(MOVE_FRICTION);
        this.velocity.y = 0;
    }
    
    this.swimMove = function()
    {
        var pitch = this.angles.pitch;
        
        if(this.waterlevel == WL_BETWEEN)
        {
            if(pitch < Angle.degToRad(45))
                pitch = 0;
        }
        
        this.setDirection(
            pitch,
            this.angles.yaw,
            0);
            
        this.swimRoll();
        this.checkThrust();
        
        if(this.movetime >= this.timestamp)
        {
            // wind down thrusting speed
            this.deAccelZ(SWIM_THRUSTDEACCELERATE);
            this.velocity.add(Vector.gScale(this.forward, this.accel.z));
        }
        else
            this.accelerate(this.speed[STATE_MOVE_SWIM]);
            
        if(this.waterlevel == WL_UNDER)
        {
            if(this.command.getAction('+jump'))
                this.accelY(SWIM_UPSPEED, SWIM_ACCELERATION);
            else
                this.deAccelY(SWIM_UPDEACCELERATION);
                
            this.velocity.y += this.accel.y;
        }
        else
        {
            if(this.command.getAction('+jump') && !this.command.getActionHeldTime('+jump'))
                this.jump();
        }
        
        if(this.accel.unit3() >= 3)
        {
            if(!(Sys.ticks() % 100))
            {
                // TODO - determine if client or server
                if(this == Client.localPlayer.controller)
                {
                    if(this.waterlevel == WL_UNDER)
                        Snd.play('sounds/shaders/underwater_swim_1.ksnd');
                    else
                        Snd.play('sounds/shaders/water_splash_3.ksnd');
                }
            }
        }
        
        if(this.waterlevel == WL_BETWEEN)
        { 
            if(this.velocity.y < 0 && this.velocity.y > -250.0)
            {
                this.accel.y = 0;
                this.velocity.y = 0;
            }
        }
        
        this.lerpToSurface();
        this.sink();
        this.super.prototype.beginMovement.bind(this)();
        
        this.applyFriction(MOVE_FRICTION);
        
        if(this.waterlevel == WL_UNDER)
            this.velocity.y = 0;
    }
    
    this.accelerate = function(properties)
    {
        var fwd = properties.forward;
        var rgt = properties.right;
        
        // accel Z axis
        if(this.command.getAction('+forward'))
            this.accelZ(fwd.forwardspeed, fwd.acceleration);
        else if(this.command.getAction('+back'))
            this.accelZ(fwd.backspeed, fwd.acceleration);
        else
            this.deAccelZ(fwd.deacceleration);
        
        // accel X axis
        if(this.command.getAction('+strafeleft'))
            this.accelX(rgt.forwardspeed, rgt.acceleration);
        else if(this.command.getAction('+straferight'))
            this.accelX(rgt.backspeed, rgt.acceleration);
        else
            this.deAccelX(rgt.deacceleration);
        
        //
        // apply acceleration to velocity
        //
        this.velocity.add(Vector.gScale(this.forward, this.accel.z));
        this.velocity.add(Vector.gScale(this.right, this.accel.x));
    }
});

class.properties(ControllerPlayer,
{
    beginMovement : function()
    {
        if(this.frametime >= 1)
            return;
        
        if(this.bNoClip == true)
        {
            this.noClipMove();
            return;
        }
        
        if(this.bFlying == true)
        {
            this.flyMove();
            return;
        }
        
        var area = this.plane.area;
        
        if(this.inWater())
        {
            this.state = STATE_MOVE_SWIM;
        }
        else if(this.canClimb() && !this.canClimbLeap())
        {
            this.state = STATE_MOVE_CLIMB;
        }
        else if(this.canJump())
        {
            this.state = STATE_MOVE_AIR;
        }
        else
        {
            this.state = STATE_MOVE_WALK;
        }
        
        switch(this.state)
        {
            case STATE_MOVE_WALK:
                this.walkMove();
                break;
                
            case STATE_MOVE_AIR:
                this.airMove();
                break;
                
            case STATE_MOVE_CLIMB:
                this.climbMove();
                break;
                
            case STATE_MOVE_SWIM:
                this.swimMove();
                break;
        }
        
        if(area != this.plane.area)
        {
            var component;
            
            for(component in area)
                area[component].onExit(this);
            
            for(component in this.plane.area)
                this.plane.area[component].onEnter(this);
        }
    },
    
    // play 'oof' sound and thump the view height if the player
    // hit the floor hard enough
    hitFloor : function()
    {
        const FALL_THRESHOLD_THUMP      = 240;
        const FALL_THRESHOLD_STOPSPEED  = 480;
        const FALL_THUMPFACTOR          = 0.75;
    
        var y = this.velocity.y * this.frametime;
        
        if(this.plane != null && !this.plane.area.ComponentAreaClimb)
            this.super.prototype.hitFloor.bind(this)();
            
        var localController = Client.localPlayer.controller;
        
        if(this.origin.y - this.plane.distance(this.origin) <= 0.1024 &&
            y <= -(1024 * this.frametime))
        {
            // TODO - determine if client or server
            if(this == localController)
                Snd.play('sounds/shaders/generic_22_turok_land.ksnd');
        }
        
        if(this.velocity.y == 0 && y <= -(FALL_THRESHOLD_THUMP * this.frametime))
        {
            if(y <= -(FALL_THRESHOLD_STOPSPEED * this.frametime))
                this.accel.clear();
            
            if(this == localController)
                Client.localPlayer.viewCamera.viewoffset =
                    (localController.owner.viewHeight * 0.5);
        }
    }
});
