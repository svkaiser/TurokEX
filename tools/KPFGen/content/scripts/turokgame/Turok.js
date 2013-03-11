//-----------------------------------------------------------------------------
//
// Turok.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

// components
Sys.runScript('scripts/turokgame/ComponentTurokPlayer.js');
Sys.runScript('scripts/turokgame/TurokPickup.js');
Sys.runScript('scripts/turokgame/TurokPickupLifeForce.js');

// features
Sys.runScript('scripts/turokgame/TurokHud.js');

// weapons
Sys.runScript('scripts/turokgame/AlienRifle.js');
Sys.runScript('scripts/turokgame/AutoShotgun.js');
Sys.runScript('scripts/turokgame/Bow.js');
Sys.runScript('scripts/turokgame/Chronoscepter.js');
Sys.runScript('scripts/turokgame/FusionCannon.js');
Sys.runScript('scripts/turokgame/GrenadeLauncher.js');
Sys.runScript('scripts/turokgame/Knife.js');
Sys.runScript('scripts/turokgame/Minigun.js');
Sys.runScript('scripts/turokgame/MissileLauncher.js');
Sys.runScript('scripts/turokgame/ParticleAccelerator.js');
Sys.runScript('scripts/turokgame/Pistol.js');
Sys.runScript('scripts/turokgame/PulseRifle.js');
Sys.runScript('scripts/turokgame/Rifle.js');
Sys.runScript('scripts/turokgame/Shotgun.js');

// packet events
Sys.runScript('scripts/turokgame/PacketEventRequestWeapon.js');
Sys.runScript('scripts/turokgame/PacketEventChangeWeapon.js');

Game.event_GameInitialized = function()
{
    PacketManager.add(PacketEventRequestWeapon);
    PacketManager.add(PacketEventChangeWeapon);
    
    Game.giveAll = function()
    {
        var svcl = arguments[1];
        
        if(svcl.state == SVC_STATE_INGAME)
        {
            var tPlayer = svcl.controller.owner.components.ComponentTurokPlayer;
            
            if(tPlayer == undefined)
                return;
                
            for(var i = 0; i < tPlayer.weapons.length; i++)
                tPlayer.weapons[i].bOwned = true;
                
            Sys.print('Lots of Goodies!');
        }
    }
    
    Server.addMessageEvent(Game.giveAll, 'giveAll');
}
