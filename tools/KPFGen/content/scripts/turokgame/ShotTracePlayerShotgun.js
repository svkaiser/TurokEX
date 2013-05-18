//-----------------------------------------------------------------------------
//
// ShotTracePlayerShotgun.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ShotTracePlayerShotgun = class.extends(ShotTrace);

class.properties(ShotTracePlayerShotgun,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    left        : 0.0,
    up          : 0.0,
    spray       : 0.09,
    hitFx       : "fx/fx_011.kfx",
    hitSnd      : "sounds/shaders/bullet_impact_3.ksnd",
    damageClass : DamageShotgunShot
});
