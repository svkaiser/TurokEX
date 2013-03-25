//-----------------------------------------------------------------------------
//
// AmbienceSwamp.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceSwamp = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceSwamp,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 30,
    sounds :
        [
            'sounds/shaders/bird_call_4.ksnd',  // 0
            'sounds/shaders/bird_call_5.ksnd',  // 1
            'sounds/shaders/bird_call_6.ksnd',  // 2
            'sounds/shaders/bird_call_7.ksnd',  // 3
            'sounds/shaders/generic_26_bird_call_9.ksnd',  // 4
            'sounds/shaders/generic_29_bird_call_12.ksnd',  // 5
            'sounds/shaders/generic_30_bird_call_13.ksnd',  // 6
            'sounds/shaders/generic_32_bird_call_15.ksnd',  // 7
            'sounds/shaders/generic_35_bird_call_18.ksnd',  // 8
            'sounds/shaders/generic_31_bird_call_14.ksnd',  // 9
            'sounds/shaders/generic_33_bird_call_16.ksnd',  // 10
            'sounds/shaders/generic_34_bird_call_17.ksnd',  // 11
            'sounds/shaders/cricket_chirp.ksnd',  // 12
            'sounds/shaders/cicaada_chirp.ksnd',  // 13
            'sounds/shaders/locust_chirp.ksnd',  // 14
            'sounds/shaders/cicaada_chirp.ksnd',  // 15
            'sounds/shaders/generic_54_monkey_chirp_12.ksnd',  // 16
            'sounds/shaders/generic_56_monkey_chirp_14.ksnd',  // 17
            'sounds/shaders/monkey_chip_scream_1.ksnd',  // 18
            'sounds/shaders/monkey_chip_scream_2.ksnd',  // 19
            'sounds/shaders/monkey_chip_scream_3.ksnd',  // 20
            'sounds/shaders/generic_53_monkey_chirp_11.ksnd',  // 21
            'sounds/shaders/generic_57_cricket_chirp_2.ksnd',  // 22
            'sounds/shaders/generic_58_locust_chirp_2.ksnd'  // 23
        ]
});
