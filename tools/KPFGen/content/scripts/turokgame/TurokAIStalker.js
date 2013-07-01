//-----------------------------------------------------------------------------
//
// TurokAIStalker.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokAIStalker = class.extendStatic(ComponentTurokAI);

class.properties(TurokAIStalker,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state           : AI_STATE_STANDING,
    health          : 20,
    meleeRange      : 95.56,
    rangeDistance   : 256.0,
    runRange        : 138.24,
    turnDirection   : 0,
    bloodType       : BLOOD_TYPE_HUMAN,
    bCanMelee       : true,
    bCanRangeAttack : false,
    rangedAttacks   : 0,
    extendedRadius  : 1.5,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    deathScream : function()
    {
        Snd.play('sounds/shaders/leaper_death.ksnd', this.parent.owner);
    },
    
    violentDeathScream : function()
    {
        Snd.play('sounds/shaders/leaper_violent_death.ksnd', this.parent.owner);
    },
    
    //------------------------------------------------------------------------
    // AI EVENTS
    //------------------------------------------------------------------------
    
    onWake : function()
    {
        var actor = this.parent.owner;
        
        if(!actor.ai.bDisabled)
            return;
        
        this.sightThreshold = 100;
        this.bTurning = false;
        
        this.run(actor);
    }
});
