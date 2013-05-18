//-----------------------------------------------------------------------------
//
// AnimalBoar.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

AnimalBoar = class.extendStatic(TurokAIAnimal, function()
{
    TurokAIAnimal.prototype.constructor.bind(this)();
});

class.properties(AnimalBoar,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    activeDistance  : 716.8,
    health          : 30,
    fleeLoopFrame   : 23,
});
