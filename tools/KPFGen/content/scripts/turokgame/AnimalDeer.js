//-----------------------------------------------------------------------------
//
// AnimalDeer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

AnimalDeer = class.extendStatic(TurokAIAnimal, function()
{
    TurokAIAnimal.prototype.constructor.bind(this)();
});

class.properties(AnimalDeer,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    activeDistance  : 716.8,
    health          : 40,
    fleeLoopFrame   : 14,
});
