//-----------------------------------------------------------------------------
//
// Ticcmd.js
// DESCRIPTION: Input commands. Used by both local client and server-clients
//
//-----------------------------------------------------------------------------

const MAXACTIONS = 256; // TODO - hardcoded limit. do not change

/*Command = class.define(function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.angle_x    = 0;
    this.angle_y    = 0;
    this.mouse_x    = 0;
    this.mouse_y    = 0;
    this.timestamp  = 0.0;
    this.frametime  = 0.0;
    this.buttons    = new Array(MAXACTIONS);
    this.heldtime   = new Array(MAXACTIONS);
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.getAction = function(name)
    {
        return (this.buttons[Input.getActionID(name)] == true);
    }
    
    this.getActionHeldTime = function(name)
    {
        return this.heldtime[Input.getActionID(name)] - 1;
    }
    
    this.clearButtons = function()
    {
        for(var i = 0; i < MAXACTIONS; i++)
            this.buttons[i] = false;
    }
});*/
