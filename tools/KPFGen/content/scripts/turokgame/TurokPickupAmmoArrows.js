//-----------------------------------------------------------------------------
//
// TurokPickupAmmoArrows.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokPickupAmmoArrows = class.extendStatic(TurokPickupAmmo);

class.properties(TurokPickupAmmoArrows,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    message : "5 tek arrows",
    amount : 5,
    id : AM_ARROWS,
    pickupSnd : 'sounds/shaders/generic_6_arrow_pickup_.ksnd'
});
