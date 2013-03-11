//-----------------------------------------------------------------------------
//
// ComponentTurokPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentTurokPlayer = class.extendStatic(ComponentPlayerStart, function()
{
    ComponentPlayerStart.prototype.constructor.bind(this)();
    
    this.numWeapons = 0;
    this.weapons    = new Array();
    
    this.weapons[this.numWeapons++] = new Knife();
    this.weapons[this.numWeapons++] = new Bow();
    this.weapons[this.numWeapons++] = new Pistol();
    this.weapons[this.numWeapons++] = new Shotgun();
    this.weapons[this.numWeapons++] = new AutoShotgun();
    this.weapons[this.numWeapons++] = new Rifle();
    this.weapons[this.numWeapons++] = new PulseRifle();
    this.weapons[this.numWeapons++] = new GrenadeLauncher();
    this.weapons[this.numWeapons++] = new Minigun();
    this.weapons[this.numWeapons++] = new AlienRifle();
    this.weapons[this.numWeapons++] = new MissileLauncher();
    this.weapons[this.numWeapons++] = new ParticleAccelerator();
    this.weapons[this.numWeapons++] = new FusionCannon();
    this.weapons[this.numWeapons++] = new Chronoscepter();
    
    this.activeWeapon = this.weapons[0];
});

class.properties(ComponentTurokPlayer,
{
    //------------------------------------------------------------------------
    // OBJECTS
    //------------------------------------------------------------------------
    
    ammo                :
    {
        arrows          : 30,
        tekArrows       : 0,
        clips           : 0,
        shells          : 0,
        expShells       : 0,
        mini            : 0,
        grenades        : 0,
        rockets         : 0,
        cells           : 0,
        fusion          : 0,
        special         : 0
    },
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    hudClass            : "TurokHud",
    lives               : 2,
    lifeForces          : 0,
    health              : 100,
    armor               : 0,
    activeWeapon        : null,
    pendingWeapon       : -1,
    activeWeaponID      : 0,
    bHasArmor           : false,
    bHasBackpack        : false,
    weapons             : null,
    numWeapons          : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    start : function()
    {
        ComponentPlayerStart.prototype.start.bind(this)();
    },
    
    //------------------------------------------------------------------------
    // WEAPON FUNCTIONS
    //------------------------------------------------------------------------
    
    cycleNextWeapon : function()
    {
        var nextWpn = this.activeWeaponID + 1;
        if(nextWpn >= this.numWeapons)
            nextWpn = 0;
            
        while(nextWpn != this.activeWeaponID)
        {
            if(this.weapons[nextWpn].bOwned)
            {
                PacketManager.send(PacketEventRequestWeapon, Client.peer, nextWpn);
                break;
            }
            
            if(++nextWpn >= this.numWeapons)
                nextWpn = 0;
        }
    },
    
    cyclePrevWeapon : function()
    {
        var nextWpn = this.activeWeaponID - 1;
        if(nextWpn < 0)
            nextWpn = (this.numWeapons - 1);
            
        while(nextWpn != this.activeWeaponID)
        {
            if(this.weapons[nextWpn].bOwned)
            {
                PacketManager.send(PacketEventRequestWeapon, Client.peer, nextWpn);
                break;
            }
            
            if(--nextWpn < 0)
                nextWpn = (this.numWeapons - 1);
        }
    },
    
    setNewWeapon : function(id)
    {
        this.activeWeapon = this.weapons[this.pendingWeapon];
        this.activeWeaponID = this.pendingWeapon;
        this.pendingWeapon = -1;
    },
    
    changeWeapon : function(id)
    {
        if(id < 0 || id >= this.numWeapons)
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
    }
});
