//-----------------------------------------------------------------------------
//
// Turok.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

// components
Sys.runScript('scripts/turokgame/ComponentTurokPlayer.js');
Sys.runScript('scripts/turokgame/TurokPickup.js');
Sys.runScript('scripts/turokgame/ComponentAreaAmbience.js');
Sys.runScript('scripts/turokgame/TurokPickupLifeForce.js');
Sys.runScript('scripts/turokgame/TurokPickupWeapon.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponPistol.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponRifle.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponShotgun.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponRiotgun.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponGrenadeLauncher.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponMinigun.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponPulseRifle.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponAlienRifle.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponRocketLauncher.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponAccelerator.js');
Sys.runScript('scripts/turokgame/TurokPickupWeaponFusion.js');

// ambience
Sys.runScript('scripts/turokgame/AmbienceJungle.js');
Sys.runScript('scripts/turokgame/AmbienceSwamp.js');
Sys.runScript('scripts/turokgame/AmbienceCatcomb01.js');
Sys.runScript('scripts/turokgame/AmbienceCatcomb02.js');
Sys.runScript('scripts/turokgame/AmbienceCave.js');
Sys.runScript('scripts/turokgame/AmbienceRuins.js');
Sys.runScript('scripts/turokgame/AmbienceLostLand.js');
Sys.runScript('scripts/turokgame/AmbienceValley.js');
Sys.runScript('scripts/turokgame/AmbienceVillage.js');

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

// cvars
Sys.addCvar('g_wpnautoswitch', '1');

// input
Input.add(0, '+attack');
Input.add(0, '-attack');
Input.add(1, '+forward');
Input.add(1, '-forward');
Input.add(2, '+back');
Input.add(2, '-back');
Input.add(3, '+left');
Input.add(3, '-left');
Input.add(4, '+right');
Input.add(4, '-right');
Input.add(5, '+strafeleft');
Input.add(5, '-strafeleft');
Input.add(6, '+straferight');
Input.add(6, '-straferight');
Input.add(7, '+run');
Input.add(7, '-run');
Input.add(8, '+jump');
Input.add(8, '-jump');
Input.add(11, '+nextweap');
Input.add(11, '-nextweap');
Input.add(12, '+prevweap');
Input.add(12, '-prevweap');

// initialize game
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
    
    Server.addMessageEvent(Game.giveAll, 'giveall');
}
