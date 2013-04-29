//-----------------------------------------------------------------------------
//
// TurokPickupAmmoExpShells.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmoExpShells = class.extendStatic(TurokPickupAmmo);

class.properties(TurokPickupAmmoExpShells,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    message : "explosive shells",
    amount : 4,
    id : AM_EXPSHELLS,
    pickupSnd : "sounds/shaders/generic_2_shell_pickup.ksnd"
});
