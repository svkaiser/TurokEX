//-----------------------------------------------------------------------------
//
// AmbienceLostLand.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceLostLand = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceLostLand,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 10,
    sounds :
        [
            'sounds/shaders/thunder_roll_1.ksnd',  // 0
            'sounds/shaders/thunder_roll_2.ksnd',  // 1
            'sounds/shaders/thunder_roll_3.ksnd',  // 2
            'sounds/shaders/thunder_roll_4.ksnd',  // 3
            'sounds/shaders/wind_blow_3.ksnd',  // 4
            'sounds/shaders/wind_blow_4.ksnd',  // 5
            'sounds/shaders/wind_blow_3.ksnd',  // 6
            'sounds/shaders/wind_blow_4.ksnd',  // 7
            'sounds/shaders/generic_183.ksnd',  // 8
            'sounds/shaders/generic_183.ksnd',  // 9
            'sounds/shaders/generic_242.ksnd',  // 10
            'sounds/shaders/generic_183.ksnd'  // 11
        ]
});
