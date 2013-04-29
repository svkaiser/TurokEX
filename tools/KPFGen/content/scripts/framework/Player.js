//-----------------------------------------------------------------------------
//
// Player.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Ticcmd.js');
Sys.dependsOn('scripts/framework/PredictionMove.js');
Sys.dependsOn('scripts/framework/Controller.js');
Sys.dependsOn('scripts/framework/ControllerPlayer.js');

Player = class.define(function()
{
    //------------------------------------------------------------------------
    // OBJECTS
    //------------------------------------------------------------------------
    
    this.netsequence =
    {
        ingoing : 0,
        outgoing : 0,
        acks : 0
    };
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.name           = "Player";
    //this.command        = new Command();
    this.prediction     = new PredictionMove();
    this.controller     = null;
    this.clientID       = 0;
    this.peer           = null;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.resetNetSequence = function()
    {
        this.netsequence.ingoing = 0;
        this.netsequence.outgoing = 1;
    }
});
