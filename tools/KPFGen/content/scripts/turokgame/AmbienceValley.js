//-----------------------------------------------------------------------------
//
// AmbienceValley.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceValley = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceValley,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 8,
    sounds :
        [
            'sounds/shaders/wind_blow_5.ksnd',  // 0
            'sounds/shaders/wind_blow_4.ksnd',  // 1
            'sounds/shaders/wind_blow_5.ksnd',  // 2
            'sounds/shaders/wind_blow_4.ksnd',  // 3
            'sounds/shaders/wind_blow_5.ksnd',  // 4
            'sounds/shaders/wind_blow_4.ksnd',  // 5
            'sounds/shaders/wind_blow_5.ksnd',  // 6
            'sounds/shaders/wind_blow_4.ksnd',  // 7
            'sounds/shaders/wind_blow_3.ksnd',  // 8
            'sounds/shaders/wind_blow_3.ksnd',  // 9
            'sounds/shaders/generic_242.ksnd',  // 10
            'sounds/shaders/generic_184.ksnd'  // 11
        ]
});
