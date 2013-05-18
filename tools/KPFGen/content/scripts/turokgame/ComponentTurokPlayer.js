//-----------------------------------------------------------------------------
//
// ComponentTurokPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

// weapons
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
//const WP_ACCELERATOR        = 11;
//const WP_FUSIONCANNON       = 12;
//const WP_CHRONOSCEPTER      = 13;
const WP_NUMWEAPONS         = 11;//= 14;

// ammo
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

// camera view
const ANGLE_MAXPITCH        = Angle.degToRad(90);
const VIEWBOB_EPISILON      = 0.001;
const VIEWBOB_MAXSWAY       = Angle.degToRad(22.5);
const VIEWBOB_FREQX         = 0.02;
const VIEWBOB_FREQY         = 0.01;
const VIEWBOB_ANGLE         = 0.0218;
const VIEWBOB_SWIMFREQX     = 0.00200;
const VIEWBOB_SWIMFREQY     = 0.00125;
const VIEWBOB_SWIMANGLE     = 0.025;

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
});

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
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
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
        this.controller.beginMovement();
        this.controller.updateWorldState(worldState);
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
    
    traceAttack : function(shotTraceClass, x, y, z)
    {
        shotTraceClass.prototype.shoot(
            this.parent.owner,
            this.controller.origin.x + x,
            this.controller.origin.y + y,
            this.controller.origin.z + z,
            this.controller.plane);
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
    
    aPulseAttack : function()
    {
        var self = this.parent.owner;
        var proj = ProjectileSpawnerPulse.prototype.spawn(self, self.origin);
        
        if(proj == null)
            return;
        
        Snd.play('sounds/shaders/machine_gun_shot_2.ksnd', self);
        Sys.spawnFx('fx/projectile_pulseball.kfx', proj, proj.origin,
            proj.rotation, Plane.fromIndex(self.plane), null, null);
            
        this.recoilPitch = -0.0288;
    },
    
    aGrenadeAttack : function()
    {
        var self = this.parent.owner;
        var proj = ProjectileSpawnerGrenade.prototype.spawn(self, self.origin);
        
        if(proj == null)
            return;
        
        Snd.play('sounds/shaders/grenade_launch.ksnd', self);
        Sys.spawnFx('fx/projectile_grenade.kfx', proj, proj.origin,
            proj.rotation, Plane.fromIndex(self.plane), null, null, true);
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
            this.clearViewPort(0.25, 0.25, 0.25);
            return;
        }
        
        for(var component in this.controller.plane.area)
            this.controller.plane.area[component].onPreRender();
    },
    
    onRender : function()
    {
        this.playerHud.canvas.drawActor(this.activeWeapon.parent.owner);
    },
    
    onPostRender : function()
    {
        this.playerHud.onDraw();
    },
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    start : function()
    {
        ComponentPlayerStart.prototype.start.bind(this)();
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
        var camera = ClientPlayer.camera;
        
        if(camera == null || camera == undefined)
            return;
        
        var origin = camera.origin;
        
        if(camera.viewHeight != actor.viewHeight)
        {
            if(actor.viewHeight - camera.viewHeight <= 0.01)
                camera.viewHeight = actor.viewHeight;
            else
                camera.viewHeight = Math.lerp(camera.viewHeight, actor.viewHeight, 0.125);
        }
        
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
        
        for(var component in this.controller.plane.area)
            this.controller.plane.area[component].onLocalTick(this);
            
        this.activeWeapon.onLocalTick();
        
        this.recoilPitch = Math.lerp(this.recoilPitch, 0, 0.125);
    }
});
