//-----------------------------------------------------------------------------
//
// AmbienceVillage.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceVillage = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceVillage,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 8,
    sounds :
        [
            'sounds/shaders/generic_36_bird_call_19.ksnd',  // 0
            'sounds/shaders/thunder_roll_1.ksnd',  // 1
            'sounds/shaders/bird_call_2.ksnd',  // 2
            'sounds/shaders/thunder_roll_4.ksnd',  // 3
            'sounds/shaders/monkey_chip_scream_3.ksnd',  // 4
            'sounds/shaders/thunder_roll_2.ksnd',  // 5
            'sounds/shaders/generic_55_monkey_chirp_13.ksnd',  // 6
            'sounds/shaders/generic_30_bird_call_13.ksnd',  // 7
            'sounds/shaders/generic_31_bird_call_14.ksnd',  // 8
            'sounds/shaders/thunder_roll_3.ksnd',  // 9
            'sounds/shaders/monkey_chip_scream_3.ksnd',  // 10
            'sounds/shaders/monkey_chip_scream_4.ksnd',  // 11
            'sounds/shaders/monkey_chip_scream_4.ksnd',  // 12
            'sounds/shaders/generic_54_monkey_chirp_12.ksnd',  // 13
            'sounds/shaders/generic_28_bird_call_11.ksnd',  // 14
            'sounds/shaders/generic_51_monkey_chirp_9.ksnd',  // 15
            'sounds/shaders/generic_56_monkey_chirp_14.ksnd',  // 16
            'sounds/shaders/generic_55_monkey_chirp_13.ksnd',  // 17
            'sounds/shaders/generic_49_monkey_chirp_7.ksnd',  // 18
            'sounds/shaders/generic_50_monkey_chirp_8.ksnd',  // 19
            'sounds/shaders/thunder_roll_1.ksnd',  // 20
            'sounds/shaders/thunder_roll_2.ksnd',  // 21
            'sounds/shaders/thunder_roll_3.ksnd',  // 22
            'sounds/shaders/thunder_roll_4.ksnd'  // 23
        ]
});
