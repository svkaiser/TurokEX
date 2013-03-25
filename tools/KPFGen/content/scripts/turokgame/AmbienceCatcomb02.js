//-----------------------------------------------------------------------------
//
// AmbienceCatcomb02.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceCatcomb02 = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceCatcomb02,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 5,
    sounds :
        [
            'sounds/shaders/generic_37_mettalic_moan_1.ksnd',  // 0
            'sounds/shaders/generic_38_mettalic_moan_2.ksnd',  // 1
            'sounds/shaders/generic_39_mettalic_moan_3.ksnd',  // 2
            'sounds/shaders/generic_40_mettalic_moan_4.ksnd',  // 3
            'sounds/shaders/generic_41_mettalic_moan_5.ksnd'  // 4
        ]
});
