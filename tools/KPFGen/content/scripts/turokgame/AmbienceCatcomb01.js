//-----------------------------------------------------------------------------
//
// AmbienceCatcomb01.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceCatcomb01 = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceCatcomb01,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 13,
    sounds :
        [
            'sounds/shaders/ominous_cave_growl_1.ksnd',  // 0
            'sounds/shaders/ominous_cave_growl_2.ksnd',  // 1
            'sounds/shaders/ghostly_catacomb_moan_1.ksnd',  // 2
            'sounds/shaders/generic_62_painful_moan_4.ksnd',  // 3
            'sounds/shaders/generic_63_catacomb_growl_1.ksnd',  // 4
            'sounds/shaders/generic_64_catacomb_growl_2.ksnd',  // 5
            'sounds/shaders/wind_blow_5.ksnd',  // 6
            'sounds/shaders/wind_blow_4.ksnd',  // 7
            'sounds/shaders/wind_blow_5.ksnd',  // 8
            'sounds/shaders/wind_blow_4.ksnd',  // 9
            'sounds/shaders/wind_blow_5.ksnd',  // 10
            'sounds/shaders/wind_blow_4.ksnd',  // 11
            'sounds/shaders/wind_blow_3.ksnd',  // 12
            'sounds/shaders/wind_blow_4.ksnd',  // 13
            'sounds/shaders/wind_blow_3.ksnd',  // 14
            'sounds/shaders/wind_blow_4.ksnd',  // 15
            'sounds/shaders/generic_63_catacomb_growl_1.ksnd',  // 16
            'sounds/shaders/generic_64_catacomb_growl_2.ksnd'  // 17
        ]
});
