//-----------------------------------------------------------------------------
//
// ComponentAreaFog.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentAreaFog = class.extendStatic(Component);

class.properties(ComponentAreaFog,
{
    fog_Color_r : 0,
    fog_Color_g : 0,
    fog_Color_b : 0,
    fog_Far     : 1024.0,
    active      : true
});
