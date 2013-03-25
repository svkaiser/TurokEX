//-----------------------------------------------------------------------------
//
// ComponentAreaWater.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/ComponentArea.js');

ComponentAreaWater = class.extendStatic(ComponentArea);

class.properties(ComponentAreaWater,
{
    waterYPlane : 0.0,
    tint_r : 0,
    tint_g : 18,
    tint_b : 95,
    tint_a : 160,
    active : true
});
