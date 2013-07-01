//-----------------------------------------------------------------------------
//
// TurokAIRaptor.js
//
// DESCRIPTION: Ratpor AI
//
//-----------------------------------------------------------------------------

TurokAIRaptor = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIRaptor,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 50,
    meleeRange      : 143.36,
    runRange        : 206.0,
    bloodType       : BLOOD_TYPE_HUMAN,
    rangeDistance   : 0.0,
    bCanMelee       : true,
    lookNode        : 3,
    lookYawAxis_x   : 0,
    lookYawAxis_y   : 0.2,
    lookYawAxis_z   : 1.0,
    
    //------------------------------------------------------------------------
    // ACTION CALLBACKS
    //------------------------------------------------------------------------
    
    footstepSound : function()
    {
        Snd.play('sounds/shaders/raptor_footfall.ksnd', this.parent.owner);
    },
    
    violentDeathScream : function()
    {
        Snd.play('sounds/shaders/raptor_violent_death.ksnd', this.parent.owner);
    },
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/raptor_death.ksnd', this.parent.owner);
    },
    
    fireRaptorMissile : function()
    {
        var actor = this.parent.owner;
        var x = arguments[1];
        var y = arguments[2];
        var z = arguments[3];
        actor.ai.fireProjectile('fx/fx_279.kfx', x, y, z, Angle.degToRad(45), true);
    }
});
