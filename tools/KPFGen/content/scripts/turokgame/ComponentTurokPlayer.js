//-----------------------------------------------------------------------------
//
// ComponentTurokPlayer.js
//
// DESCRIPTION: Main player component
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////
// WEAPON CONSTANTS
////////////////////////////////////////////////////
const WP_KNIFE              = 0;
const WP_BOW                = 1;
const WP_PISTOL             = 2;
const WP_SHOTGUN            = 3;
const WP_AUTOSHOTGUN        = 4;
const WP_RIFLE              = 5;
const WP_PULSERIFLE         = 6;
const WP_GRENADE_L          = 7;
const WP_MINIGUN            = 8;
const WP_ALIENRIFLE         = 9;
const WP_ROCKET_L           = 10;

// TODO - Finish remaining weapons

//const WP_ACCELERATOR        = 11;
//const WP_FUSIONCANNON       = 12;
//const WP_CHRONOSCEPTER      = 13;
const WP_NUMWEAPONS         = 11;//= 14;

////////////////////////////////////////////////////
// AMMO CONSTANTS
////////////////////////////////////////////////////
const AM_ARROWS             = 0;
const AM_TEKARROWS          = 1;
const AM_CLIPS              = 2;
const AM_SHELLS             = 3;
const AM_EXPSHELLS          = 4;
const AM_MINIAMMO           = 5;
const AM_GRENADES           = 6;
const AM_ROCKETS            = 7;
const AM_CELLS              = 8;
const AM_CHARGES            = 9;
const AM_SPECIAL            = 10;
const AM_NUMAMMO            = 11;

////////////////////////////////////////////////////
// CAMERA ANGLE CONSTANTS
////////////////////////////////////////////////////
const ANGLE_MAXPITCH        = Angle.degToRad(90);
const VIEWBOB_EPISILON      = 0.001;
const VIEWBOB_MAXSWAY       = Angle.degToRad(22.5);
const VIEWBOB_FREQX         = 0.02;
const VIEWBOB_FREQY         = 0.01;
const VIEWBOB_ANGLE         = 0.0218;
const VIEWBOB_SWIMFREQX     = 0.00200;
const VIEWBOB_SWIMFREQY     = 0.00125;
const VIEWBOB_SWIMANGLE     = 0.025;

////////////////////////////////////////////////////
// PLAYER CLASS
////////////////////////////////////////////////////

ComponentTurokPlayer = class.extendStatic(ComponentPlayerStart, function()
{
    ComponentPlayerStart.prototype.constructor.bind(this)();
    
    // construct weapons
    this.weapons = new Array(WP_NUMWEAPONS);
    
    this.weapons[WP_KNIFE]          = this.setupWeapon('WeaponKnife');
    this.weapons[WP_BOW]            = this.setupWeapon('WeaponBow');
    this.weapons[WP_PISTOL]         = this.setupWeapon('WeaponPistol');
    this.weapons[WP_SHOTGUN]        = this.setupWeapon('WeaponShotgun');
    this.weapons[WP_AUTOSHOTGUN]    = this.setupWeapon('WeaponAutoShotgun');
    this.weapons[WP_RIFLE]          = this.setupWeapon('WeaponRifle');
    this.weapons[WP_PULSERIFLE]     = this.setupWeapon('WeaponPulseRifle');
    this.weapons[WP_GRENADE_L]      = this.setupWeapon('WeaponGrenadeLauncher');
    this.weapons[WP_MINIGUN]        = this.setupWeapon('WeaponMiniGun');
    this.weapons[WP_ALIENRIFLE]     = this.setupWeapon('WeaponAlienRifle');
    this.weapons[WP_ROCKET_L]       = this.setupWeapon('WeaponRocketLauncher');
    
    // construct ammo
    this.ammo = new Array(AM_NUMAMMO);
    
    this.ammo[AM_ARROWS]        = new AmmoArrow();
    this.ammo[AM_TEKARROWS]     = new AmmoTekArrow();
    this.ammo[AM_CLIPS]         = new AmmoClip();
    this.ammo[AM_SHELLS]        = new AmmoShell();
    this.ammo[AM_EXPSHELLS]     = new AmmoExpShell();
    this.ammo[AM_MINIAMMO]      = new AmmoMini();
    this.ammo[AM_GRENADES]      = new AmmoGrenade();
    this.ammo[AM_ROCKETS]       = new AmmoRocket();
    this.ammo[AM_CELLS]         = new AmmoCell();
    this.ammo[AM_CHARGES]       = new AmmoCharge();
    this.ammo[AM_SPECIAL]       = new AmmoCell(); // TODO
    
    // set defaults
    this.resetVars();
    
    // create object when class is constructed
    this.teleportInfo = {
        x       : 0.0,
        y       : 0.0,
        z       : 0.0,
        yaw     : 0.0,
        map     : -1,
        plane   : -1
    };
    
    // configure properties for serialization
    this.config(
        'lives',
        'lifeForces',
        'ammo',
        'health',
        'armor',
        'bHasArmor',
        'bHasBackpack',
        'keys_level2',
        'keys_level3',
        'keys_level4',
        'keys_level5',
        'keys_level6',
        'keys_level7',
        'keys_level8',
        'activeWeaponID',
        'bTeleporting',
        'teleportInfo'
        );
        
    this.teleportInfo.config(
        'x',
        'y',
        'z',
        'yaw',
        'map',
        'plane'
        );
});

////////////////////////////////////////////////////
// PLAYER CLASS PROPERTIES
////////////////////////////////////////////////////

class.properties(ComponentTurokPlayer,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    controllerClass     : "ControllerPlayer",
    hudClass            : "TurokHud",
    lives               : 2,
    lifeForces          : 0,
    health              : 100,
    armor               : 0,
    mortalWound         : 100,
    activeWeapon        : null,
    pendingWeapon       : -1,
    activeWeaponID      : WP_KNIFE,
    bHasArmor           : false,
    bHasBackpack        : false,
    spiritTime          : 0.0,
    weapons             : null,
    ammo                : null,
    recoilPitch         : 0.0,
    keys_level2         : 0,
    keys_level3         : 0,
    keys_level4         : 0,
    keys_level5         : 0,
    keys_level6         : 0,
    keys_level7         : 0,
    keys_level8         : 0,
    bHideHud            : false,
    bLockControls       : false,
    cameraEvent         : null,
    bTeleporting        : false,
    teleportInfo        : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // called when a new game has been initialized
    resetVars : function()
    {
        // set default weapon
        this.activeWeapon           = this.weapons[WP_KNIFE];
        this.activeWeapon.bActive   = true;
        
        this.bHasArmor              = false;
        this.bHasBackpack           = false;
        this.lives                  = 2;
        this.lifeForces             = 0;
        this.health                 = 100;
        this.armor                  = 0;
        this.mortalWound            = 100;
        this.spiritTime             = 0.0;
        this.pendingWeapon          = -1;
        this.activeWeaponID         = WP_KNIFE;
        this.recoilPitch            = 0.0;
        this.bHideHud               = false;
        this.bLockControls          = false;
        this.bTeleporting           = false;
        this.cameraEvent            = null;
        
        this.weapons[WP_KNIFE].bOwned = true;
        this.weapons[WP_BOW].bOwned = true;
        
        for(var i = (WP_BOW+1); i < WP_NUMWEAPONS; i++)
        {
            this.weapons[i].bActive = false;
            this.weapons[i].bOwned = true;
        }
        
        for(var i = 0; i < AM_NUMAMMO; i++)
            this.ammo[i].amount = this.ammo[i].initial;
    },
    
    teleport : function()
    {
        if(this.teleportInfo.level >= 0 &&
            this.teleportInfo.level != Level.mapID)
        {
            Level.changeMap(this.teleportInfo.level);
            return;
        }
        
        var ws = ClientPlayer.worldState;
        
        ws.origin = new Vector(
            this.teleportInfo.x,
            this.teleportInfo.y,
            this.teleportInfo.z);
            
        ws.yaw = this.teleportInfo.yaw;
        ws.plane = Plane.fromIndex(this.teleportInfo.plane);
        
        Snd.play('sounds/shaders/warp_inout.ksnd', this.parent.owner);
        
        this.cameraEvent = CameraEventTeleport;
        
        this.cameraEvent.prototype.startEvent();
        this.cameraEvent.prototype.fadeIn();
        this.cameraEvent.prototype.stage = 3;
    },
    
    //------------------------------------------------------------------------
    // LEVEL KEYS
    //------------------------------------------------------------------------
    
    giveLevelKey : function(level, bits)
    {
        switch(level)
        {
        case 2:
            this.keys_level2 |= (1 << bits);
            break;
        case 3:
            this.keys_level3 |= (1 << bits);
            break;
        case 4:
            this.keys_level4 |= (1 << bits);
            break;
        case 5:
            this.keys_level5 |= (1 << bits);
            break;
        case 6:
            this.keys_level6 |= (1 << bits);
            break;
        case 7:
            this.keys_level7 |= (1 << bits);
            break;
        case 8:
            this.keys_level8 |= (1 << bits);
            break;
        default:
            break;
        }
    },
    
    checkLevelKey : function(map)
    {
        var num = 0;
        var hud = this.playerHud;
        
        switch(map)
        {
        case 4:
        case 5:
            num = 6;
            if(this.keys_level2 & 1) num--;
            if(this.keys_level2 & 2) num--;
            if(this.keys_level2 & 4) num--;
            if(this.keys_level3 & 1) num--;
            if(this.keys_level3 & 2) num--;
            if(this.keys_level3 & 4) num--;
            break;
        case 6:
        case 7:
            num = 3;
            if(this.keys_level4 & 1) num--;
            if(this.keys_level4 & 2) num--;
            if(this.keys_level5 & 4) num--;
            break;
        case 8:
        case 9:
            num = 3;
            if(this.keys_level5 & 1) num--;
            if(this.keys_level5 & 2) num--;
            if(this.keys_level4 & 4) num--;
            break;
        case 10:
        case 11:
            num = 3;
            if(this.keys_level6 & 1) num--;
            if(this.keys_level6 & 2) num--;
            if(this.keys_level8 & 1) num--;
            break;
        case 12:
        case 13:
            num = 3;
            if(this.keys_level8 & 2) num--;
            if(this.keys_level8 & 4) num--;
            if(this.keys_level6 & 4) num--;
            break;
        case 14:
        case 15:
        case 16:
            num = 3;
            if(this.keys_level7 & 1) num--;
            if(this.keys_level7 & 2) num--;
            if(this.keys_level7 & 4) num--;
            break;
        case 17:
        case 18:
        case 19:
            num = 2;
            if(this.keys_level8 & 8) num--;
            if(this.keys_level8 & 16) num--;
            break;
        }
        
        if(num > 1)
        {
            hud.notify(num + " keys to find");
            hud.notify("on this level");
        }
        else if(num == 1)
        {
            hud.notify("1 key to find");
            hud.notify("on this level");
        }
        else
        {
            hud.notify("all keys on");
            hud.notify("this level found");
        }
        
        hud.messages[(hud.strCounter-1) & 7].time = 60.0;
        hud.messages[(hud.strCounter-2) & 7].time = 60.0;
    },
    
    //------------------------------------------------------------------------
    // WEAPON FUNCTIONS
    //------------------------------------------------------------------------
    
    setupWeapon : function(weaponName)
    {
        var wpnClass = class.find(weaponName);
        
        if(wpnClass == null)
            return null;
        
        return wpnClass.prototype.spawn(this).components[weaponName];
    },
    
    cycleNextWeapon : function()
    {
        var nextWpn;

        if(this.pendingWeapon != -1)
            nextWpn = this.pendingWeapon + 1;
        else
            nextWpn = this.activeWeaponID + 1;
            
        if(nextWpn >= WP_NUMWEAPONS)
            nextWpn = 0;
            
        while(nextWpn != this.activeWeaponID)
        {
            if(this.weapons[nextWpn].bOwned)
            {
                if(this.pendingWeapon != -1)
                {
                    this.pendingWeapon = nextWpn;
                    return;
                }
                
                //PacketManager.send(PacketEventRequestWeapon, NClient.peer, nextWpn);
                this.changeWeapon(nextWpn);
                break;
            }
            
            if(++nextWpn >= WP_NUMWEAPONS)
                nextWpn = 0;
        }
    },
    
    cyclePrevWeapon : function()
    {
        var nextWpn;

        if(this.pendingWeapon != -1)
            nextWpn = this.pendingWeapon - 1;
        else
            nextWpn = this.activeWeaponID - 1;
        
        if(nextWpn < 0)
            nextWpn = (WP_NUMWEAPONS - 1);
            
        while(nextWpn != this.activeWeaponID)
        {
            if(this.weapons[nextWpn].bOwned)
            {
                if(this.pendingWeapon != -1)
                {
                    this.pendingWeapon = nextWpn;
                    return;
                }
                
                //PacketManager.send(PacketEventRequestWeapon, NClient.peer, nextWpn);
                this.changeWeapon(nextWpn);
                break;
            }
            
            if(--nextWpn < 0)
                nextWpn = (WP_NUMWEAPONS - 1);
        }
    },
    
    setNewWeapon : function()
    {
        if(this.pendingWeapon == -1)
            return;
        
        this.activeWeapon.bActive = false;
        this.activeWeapon = this.weapons[this.pendingWeapon];
        this.activeWeapon.bActive = true;
        this.activeWeaponID = this.pendingWeapon;
        this.pendingWeapon = -1;
    },
    
    changeWeapon : function(id)
    {
        if(id < 0 || id >= WP_NUMWEAPONS)
            return;
            
        var weapon = this.weapons[id];
        
        if(weapon == null)
            return;
        
        // TODO
        if(weapon.bOwned)
        {
            this.pendingWeapon = id;
            this.activeWeapon.change();
        }
    },
    
    giveWeapon : function(id)
    {
        if(id < 0 || id >= WP_NUMWEAPONS)
            return false;
            
        var weapon = this.weapons[id];
        
        if(weapon == null)
            return false;
        
        if(Sys.getCvar('g_wpnautoswitch') != 0 && weapon.bOwned == false)
            //PacketManager.send(PacketEventRequestWeapon, NClient.peer, id);
            this.changeWeapon(id);
        
        weapon.bOwned = true;
        return true;
    },
    
    //------------------------------------------------------------------------
    // GENERAL MOVEMENT
    //------------------------------------------------------------------------
    
    clampAngles : function(worldState, command)
    {
        var factor = 1.0;
        
        if(this.controller.state == STATE_MOVE_SWIM)
            factor = 0.35;
            
        worldState.yaw -= Angle.degToRad(command.mouse_x * factor);
        worldState.pitch -= Angle.degToRad(command.mouse_y * factor);
        
        if(worldState.pitch > ANGLE_MAXPITCH) worldState.pitch = ANGLE_MAXPITCH;
        if(worldState.pitch < -ANGLE_MAXPITCH) worldState.pitch = -ANGLE_MAXPITCH;
    },
    
    playerMovement : function(worldState, command)
    {
        this.controller.updateFromWorldState(worldState, command);
        
        if(!this.bLockControls)
            this.controller.beginMovement();

        this.controller.updateWorldState(worldState);
        
        if(this.bLockControls)
        {
            worldState.yaw = this.parent.owner.yaw;
            worldState.pitch = this.parent.owner.pitch;
        }
    },
    
    // TODO
    toggleNoClip : function()
    {
        this.controller.bNoClip ^= 1;
        
        if(this.controller.bNoClip == false)
        {
            this.controller.plane = Level.findPlane(this.controller.origin);
            this.controller.updateWorldState(ClientPlayer.worldState);
        }
    },
    
    //------------------------------------------------------------------------
    // ACTIONS
    //------------------------------------------------------------------------
    
    customDamage : function()
    {
        var actor = this.parent.owner;
        var target = arguments[0];
        
        if(target == null)
            return;
        
        if(GameActor.compare(target, actor))
            return;

        Damage.prototype.inflict(target, actor, arguments[1]);
    },
    
    traceAttack : function(shotTraceClass, x, y, z)
    {
        shotTraceClass.prototype.shoot(
            this.parent.owner,
            this.controller.origin.x + x,
            this.controller.origin.y + y,
            this.controller.origin.z + z,
            this.controller.plane);
    },
    
    aKnifeAttack : function()
    {
        var self = this.parent.owner;
        var t = Physics.rayTrace(
            new Vector(
                this.controller.origin.x,
                this.controller.origin.y + (self.centerHeight + self.viewHeight),
                this.controller.origin.z),
            Vector.applyRotation(new Vector(0, 0, 1), self.rotation),
            1, 64, this.controller.plane, self);
            
        if(t != null)
        {
            if(t.hitActor != null)
                DamageKnife.prototype.inflict(t.hitActor, self);
        }
    },
    
    // damage calculation may be a bit off. unsure about the true values
    arrowDamage : function()
    {
        var target = arguments[0];
        var dmg = arguments[1] + 5.0;
        var org = arguments[2];
        var vel = arguments[3];
        
        if(dmg == 0.0)
            return;
        
        Damage.prototype.inflict(target, this.parent.owner,
            (vel.unit3() * (1.0 / dmg)) * 20.0 * 0.001953125);
    },
    
    grenadeExplode : function()
    {
        var radius = arguments[1];
        var origin = arguments[2];
        var plane = arguments[4];
        
        BlastRadius.prototype.explode(this.parent.owner, origin.x, origin.y, origin.z,
            plane, radius, 100.0, 1.0, false);
    },
    
    action_042 : function()
    {
        var radius = arguments[1];
        var origin = arguments[2];
        var plane = arguments[4];
        
        BlastRadius.prototype.explode(this.parent.owner, origin.x, origin.y, origin.z,
            plane, radius, 50.0, 0.5, true);
    },
    
    action_124 : function()
    {
        var actor = this.parent.owner;
        var target = arguments[0];
        
        if(target == null)
            return;
        
        if(GameActor.compare(target, actor))
            return;

        Damage.prototype.inflict(target, actor, arguments[1]);
    },
    
    action_365 : function()
    {
        var radius = arguments[1];
        var origin = arguments[2];
        var plane = arguments[4];
        
        BlastRadius.prototype.explode(this.parent.owner, origin.x, origin.y, origin.z,
            plane, radius, 20.0, 1.0, true);
    },
    
    aPistolAttack : function()
    {
        Snd.play('sounds/shaders/pistol_shot.ksnd', this.parent.owner);
        this.traceAttack(ShotTracePlayerPistol, 0, 52, 0);
        this.recoilPitch = -0.0325;
    },
    
    aShotgunAttack : function()
    {
        Snd.play('sounds/shaders/riot_shotgun_shot.ksnd', this.parent.owner);
        
        for(var i = 0; i < 5; i++)
            this.traceAttack(ShotTracePlayerShotgun, 0, 52, 0);
        this.recoilPitch = -0.0408;
    },
    
    aAutoShotgunAttack : function()
    {
        Snd.play('sounds/shaders/auto_shotgun_shot.ksnd', this.parent.owner);
        
        for(var i = 0; i < 5; i++)
            this.traceAttack(ShotTracePlayerShotgun, 0, 52, 0);
        this.recoilPitch = -0.0408;
    },
    
    aRifleAttack : function()
    {
        Snd.play('sounds/shaders/rifle_shot.ksnd', this.parent.owner);
        this.traceAttack(ShotTracePlayerRifle, 0, 52, 0);
        this.recoilPitch = -0.0325;
    },
    
    aMinigunAttack : function()
    {
        Snd.play('sounds/shaders/pistol_shot.ksnd', this.parent.owner);
        this.traceAttack(ShotTracePlayerMinigun, 0, 52, 0);
        this.recoilPitch = -0.0325;
    },
    
    //------------------------------------------------------------------------
    // RENDER EVENTS
    //------------------------------------------------------------------------
    
    onPreRender : function()
    {
        var worldState = ClientPlayer.worldState;
        var plane = worldState.plane;
        
        if(plane == null || this.controller == null)
        {
            Render.clearViewPort(0.25, 0.25, 0.25);
            return;
        }
        
        for(var component in this.controller.plane.area)
            this.controller.plane.area[component].onPreRender();
            
        this.activeWeapon.parent.owner.bHidden = true;
    },
    
    onRender : function()
    {
        var wpn = this.activeWeapon.parent.owner;
        wpn.bHidden = false;
        
        if(this.bHideHud == false)
            this.playerHud.canvas.drawActor(wpn);
    },
    
    onPostRender : function()
    {
        if(this.bHideHud == false)
            this.playerHud.onDraw();
        
        if(this.cameraEvent != null)
            this.cameraEvent.prototype.draw();
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    start : function()
    {
        ComponentPlayerStart.prototype.start.bind(this)();
        
        if(this.activeWeaponID != WP_KNIFE)
            this.changeWeapon(this.activeWeaponID);
            
        if(this.bTeleporting)
        {
            this.teleport();
            this.bTeleporting = false;
        }
    },
    
    onReady : function()
    {
    },
    
    onDamage : function(instigator, amount)
    {
        var self = this.parent.owner;
        
        switch(Sys.rand(5) & 3)
        {
        case 0:
            Snd.play('sounds/shaders/generic_10_turok_injury_1.ksnd', self);
            break;
        case 1:
            Snd.play('sounds/shaders/generic_11_turok_injury_2.ksnd', self);
            break;
        case 2:
            Snd.play('sounds/shaders/generic_12_turok_injury_3.ksnd', self);
            break;
        case 3:
            Snd.play('sounds/shaders/generic_13_turok_injury_4.ksnd', self);
            break;
        }
        
        this.playerHud.flash(192, 0, 0);
        
        if(this.health > 15 && this.health - amount < 15)
            this.playerHud.notify('low health');
    },
    
    onTick : function()
    {
        for(var component in this.controller.plane.area)
            this.controller.plane.area[component].onTick(this);
    },
    
    onLocalTick : function()
    {
        if(NClient.state != NClient.STATE_INGAME)
            return;
            
        var actor = ClientPlayer.actor;
        var camera = ClientPlayer.camera;
        
        if(actor == null || actor == undefined)
            return;
        
        //
        // update and clamp angles
        //
        var command = ClientPlayer.command;
        var worldState = ClientPlayer.worldState;
        
        this.clampAngles(worldState, command);
        
        //
        // begin movement and update world state
        //
        this.controller.local = true;
        this.playerMovement(worldState, command);
        
        //
        // update camera
        //
        
        if(camera == null || camera == undefined)
            return;
        
        if(camera.viewHeight != actor.viewHeight)
        {
            if(actor.viewHeight - camera.viewHeight <= 0.01)
                camera.viewHeight = actor.viewHeight;
            else
                camera.viewHeight = Math.lerp(camera.viewHeight, actor.viewHeight, 0.125);
        }
        
        if(camera.owner != null && !this.bTeleporting)
        {
            var origin = camera.origin;
            
            origin.x = worldState.origin.x;
            origin.y = worldState.origin.y + actor.centerHeight + camera.viewHeight;
            origin.z = worldState.origin.z;
            
            camera.yaw      = worldState.yaw;
            camera.pitch    = worldState.pitch;
            camera.roll     = worldState.roll;
            camera.origin   = origin;
        
            //
            // calculate camera bobbing
            //
            var bob_x = 0;
            var bob_y = 0;
            
            switch(this.controller.state)
            {
                case STATE_MOVE_WALK:
                    if((worldState.origin.y + worldState.velocity.y) -
                    worldState.plane.distance(worldState.origin) <
                    (actor.viewHeight + actor.centerHeight)+1)
                    {
                        var d = Math.abs(worldState.accel.z * worldState.frameTime) * 0.06;
                        
                        if(d > VIEWBOB_EPISILON)
                        {
                            if(d > VIEWBOB_MAXSWAY)
                                d = VIEWBOB_MAXSWAY;
                            
                            bob_x = Math.sin(Sys.time() * VIEWBOB_FREQX) * VIEWBOB_ANGLE * d;
                            bob_y = Math.sin(Sys.time() * VIEWBOB_FREQY) * VIEWBOB_ANGLE * d;
                        }
                    }
                    break;
                    
                case STATE_MOVE_SWIM:
                    bob_x = Math.sin(Sys.time() * VIEWBOB_SWIMFREQX) * VIEWBOB_SWIMANGLE;
                    bob_y = Math.sin(Sys.time() * VIEWBOB_SWIMFREQY) * VIEWBOB_SWIMANGLE;
                    break;
                    
                default:
                    break;
            }
            
            camera.yaw -= bob_y;
            camera.pitch += bob_x + this.recoilPitch;
        }
        
        if(this.controller.plane != null)
        {
            for(var component in this.controller.plane.area)
                this.controller.plane.area[component].onLocalTick(this);
        }
        
        if(!this.bTeleporting)
        {
            this.activeWeapon.tick();
            this.recoilPitch = Math.lerp(this.recoilPitch, 0, 0.125);
        }
            
        if(this.cameraEvent != null)
        {
            this.cameraEvent.prototype.tick(this.playerHud);
            this.cameraEvent.prototype.runEvent(this, camera);
        }
    }
});
