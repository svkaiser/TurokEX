//-----------------------------------------------------------------------------
//
// TurokPickupAmmoRockets.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmoRockets = class.extendStatic(TurokPickupAmmo);

class.properties(TurokPickupAmmoRockets,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    message : "4 rockets",
    amount : 4,
    id : AM_ROCKETS,
    pickupSnd : 'sounds/shaders/generic_7_rocket_pickup.ksnd'
});
