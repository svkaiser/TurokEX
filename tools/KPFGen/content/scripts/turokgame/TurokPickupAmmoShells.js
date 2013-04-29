//-----------------------------------------------------------------------------
//
// TurokPickupAmmoShells.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmoShells = class.extendStatic(TurokPickupAmmo);

class.properties(TurokPickupAmmoShells,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    message : "shotgun shells",
    amount : 5,
    id : AM_SHELLS,
    pickupSnd : "sounds/shaders/generic_2_shell_pickup.ksnd"
});
