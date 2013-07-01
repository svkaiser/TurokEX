//-----------------------------------------------------------------------------
//
// ComponentAreaSurface.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

const SURFACE_DEFAULT   = 0;
const SURFACE_WATER     = 1;
const SURFACE_STONE     = 2;
const SURFACE_METAL     = 3;

ComponentAreaSurface = class.extendStatic(ComponentArea);

class.properties(ComponentAreaSurface,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active          : true,
    floorSurface    : SURFACE_DEFAULT,
    wallSurface     : SURFACE_DEFAULT
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
});
