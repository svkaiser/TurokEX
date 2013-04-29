//-----------------------------------------------------------------------------
//
// TurokPickupAmmoCharges.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmoCharges = class.extendStatic(TurokPickupAmmo);

class.properties(TurokPickupAmmoCharges,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    message : "fusion charge",
    amount : 1,
    id : AM_CHARGES,
    pickupSnd : 'sounds/shaders/generic_3_energy_pickup.ksnd'
});
