//-----------------------------------------------------------------------------
//
// ShotTracePlayerRifle.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ShotTracePlayerRifle = class.extends(ShotTrace);

class.properties(ShotTracePlayerRifle,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    left    : 0.0,
    up      : 0.0,
    spray   : 0.035,
    hitFx   : "fx/fx_011.kfx",
    hitSnd  : "sounds/shaders/bullet_impact_3.ksnd"
});
