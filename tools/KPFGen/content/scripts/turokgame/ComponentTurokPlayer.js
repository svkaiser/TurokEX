//-----------------------------------------------------------------------------
//
// ComponentTurokPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const WP_KNIFE          = 0;
const WP_BOW            = 1;
const WP_PISTOL         = 2;
const WP_SHOTGUN        = 3;
const WP_AUTOSHOTGUN    = 4;
const WP_RIFLE          = 5;
const WP_PULSERIFLE     = 6;
const WP_GRENADE_L      = 7;
const WP_MINIGUN        = 8;
const WP_ALIENRIFLE     = 9;
const WP_ROCKET_L       = 10;
const WP_ACCELERATOR    = 11;
const WP_FUSIONCANNON   = 12;
const WP_CHRONOSCEPTER  = 13;
const WP_NUMWEAPONS     = 14;

const AM_ARROWS         = 0;
const AM_TEKARROWS      = 1;
const AM_CLIPS          = 2;
const AM_SHELLS         = 3;
const AM_EXPSHELLS      = 4;
const AM_MINIAMMO       = 5;
const AM_GRENADES       = 6;
const AM_ROCKETS        = 7;
const AM_CELLS          = 8;
const AM_CHARGES        = 9;
const AM_SPECIAL        = 10;
const AM_NUMAMMO        = 11;

ComponentTurokPlayer = class.extendStatic(ComponentPlayerStart, function()
{
    ComponentPlayerStart.prototype.constructor.bind(this)();
    
    // construct weapons
    this.weapons                    = new Array(WP_NUMWEAPONS);
    this.weapons[WP_KNIFE]          = new Knife();
    this.weapons[WP_BOW]            = new Bow();
    this.weapons[WP_PISTOL]         = new Pistol();
    this.weapons[WP_SHOTGUN]        = new Shotgun();
    this.weapons[WP_AUTOSHOTGUN]    = new AutoShotgun();
    this.weapons[WP_RIFLE]          = new Rifle();
    this.weapons[WP_PULSERIFLE]     = new PulseRifle();
    this.weapons[WP_GRENADE_L]      = new GrenadeLauncher();
    this.weapons[WP_MINIGUN]        = new Minigun();
    this.weapons[WP_ALIENRIFLE]     = new AlienRifle();
    this.weapons[WP_ROCKET_L]       = new MissileLauncher();
    this.weapons[WP_ACCELERATOR]    = new ParticleAccelerator();
    this.weapons[WP_FUSIONCANNON]   = new FusionCannon();
    this.weapons[WP_CHRONOSCEPTER]  = new Chronoscepter();
    
    // construct ammo
    this.ammo                       = new Array(AM_NUMAMMO);
    
    // set defaults
    this.resetVars();
});

class.properties(ComponentTurokPlayer,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    controller          : "ControllerPlayer",
    hudClass            : "TurokHud",
    lives               : 2,
    lifeForces          : 0,
    health              : 100,
    armor               : 0,
    activeWeapon        : null,
    pendingWeapon       : -1,
    activeWeaponID      : WP_KNIFE,
    bHasArmor           : false,
    bHasBackpack        : false,
    weapons             : null,
    ammo                : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    start : function()
    {
        ComponentPlayerStart.prototype.start.bind(this)();
    },
    
    resetVars : function()
    {
        // set default ammo
        this.ammo[AM_ARROWS]    = 30;
        this.ammo[AM_TEKARROWS] = 0;
        this.ammo[AM_CLIPS]     = 0;
        this.ammo[AM_SHELLS]    = 0;
        this.ammo[AM_EXPSHELLS] = 0;
        this.ammo[AM_MINIAMMO]  = 0;
        this.ammo[AM_GRENADES]  = 0;
        this.ammo[AM_ROCKETS]   = 0;
        this.ammo[AM_CELLS]     = 0;
        this.ammo[AM_CHARGES]   = 0;
        this.ammo[AM_SPECIAL]   = 0;
        
        // set default weapon
        this.activeWeapon       = this.weapons[WP_KNIFE];
        
        this.bHasArmor          = false;
        this.bHasBackpack       = false;
        this.lives              = 2;
        this.lifeForces         = 0;
        this.health             = 100;
        this.armor              = 0;
        this.pendingWeapon      = -1;
        this.activeWeaponID     = WP_KNIFE;
        
        for(var i = (WP_BOW+1); i < WP_NUMWEAPONS; i++)
            this.weapons[i].bOwned = false;
    },
    
    //------------------------------------------------------------------------
    // WEAPON FUNCTIONS
    //------------------------------------------------------------------------
    
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
                
                PacketManager.send(PacketEventRequestWeapon, Client.peer, nextWpn);
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
                
                PacketManager.send(PacketEventRequestWeapon, Client.peer, nextWpn);
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
        
        this.activeWeapon = this.weapons[this.pendingWeapon];
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
            PacketManager.send(PacketEventRequestWeapon, Client.peer, id);
        
        weapon.bOwned = true;
        return true;
    }
});
