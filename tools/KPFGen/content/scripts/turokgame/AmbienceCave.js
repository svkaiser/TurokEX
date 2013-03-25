//-----------------------------------------------------------------------------
//
// AmbienceCave.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/turokgame/ComponentAreaAmbience.js');

AmbienceCave = class.extendStatic(ComponentAreaAmbience);

class.properties(AmbienceCave,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    randFactor : 15,
    sounds :
        [
            'sounds/shaders/generic_42_cave_wing_flutter_w__bat_1.ksnd',  // 0
            'sounds/shaders/generic_43_cave_wing_flutter_w__bat_2.ksnd',  // 1
            'sounds/shaders/generic_44_cave_wing_flutter_1.ksnd',  // 2
            'sounds/shaders/generic_45_cave_wing_flutter_2.ksnd',  // 3
            'sounds/shaders/generic_46_cave_wing_flutter_3.ksnd',  // 4
            'sounds/shaders/generic_59_cave_wing_flutter_4.ksnd',  // 5
            'sounds/shaders/generic_60_bat_screech_1.ksnd',  // 6
            'sounds/shaders/generic_61_bat_screech_2.ksnd',  // 7
            'sounds/shaders/ominous_cave_growl_1.ksnd',  // 8
            'sounds/shaders/ominous_cave_growl_2.ksnd',  // 9
            'sounds/shaders/ominous_cave_growl_1.ksnd',  // 10
            'sounds/shaders/ominous_cave_growl_2.ksnd'  // 11
        ]
});
