//-----------------------------------------------------------------------------
//
// LocalPlayer.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Player.js');
Sys.dependsOn('scripts/framework/CameraPlayer.js');
Sys.dependsOn('scripts/framework/Console.js');

const EV_KEYDOWN                = 0;
const EV_KEYUP                  = 1;
const EV_MOUSE                  = 2;
const EV_MOUSEDOWN              = 3;
const EV_MOUSEUP                = 4;
const EV_MOUSEWHEEL             = 5;
const EV_GAMEPAD                = 6;

LocalPlayer = class.extends(Player, function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.console        = Console;
    this.viewCamera     = new CameraPlayer();
    this.peer           = NClient.peer;
    this.latency        = new Array(NETBACKUPS);
    this.move_diff      = new Vector();
    this.oldMoves       = new Array(NETBACKUPS);
    this.oldCommands    = new Array(NETBACKUPS);
    
    for(var i = 0; i < NETBACKUPS; i++)
    {
        this.oldMoves[i] = new Vector();
        this.oldCommands[i] = new Command();
    }
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    // TODO - temporarily called by native code
    this.processInput = function(ev)
    {
        if(this.console.processInput(ev))
            return;
        
        switch(ev.type)
        {
        case EV_MOUSE:
            this.controller.mouseMove(ev.data2, ev.data3);
            break;

        case EV_KEYDOWN:
            Input.keyPress(ev.data1, false);
            break;

        case EV_KEYUP:
            Input.keyPress(ev.data1, true);
            break;

        case EV_MOUSEDOWN:
            Input.keyPress(ev.data1, false);
            break;

        case EV_MOUSEUP:
            Input.keyPress(ev.data1, true);
            break;
        }
    }
    
    this.buildCommand = function()
    {
        var command = this.command;
        var inputlist = Input.getActions();
        
        command.clearButtons();
        
        for(var i = 0; i < inputlist.length; i++)
        {
            if(inputlist[i])
            {
                command.buttons[i] = true;
            }
            else
                command.buttons[i] = false;
                
            if(command.buttons[i] == true)
            {
                if(command.heldtime[i] < 255)
                    command.heldtime[i]++;
                else
                    command.heldtime[i] = 255;
            }
            else
                command.heldtime[i] = 0;
        }
        
        if(this.controller)
            this.controller.updateCommandAngles(command);
        
        command.timestamp = Sys.time();
        command.frametime = Sys.deltatime();
    }
    
    this.tick = function()
    {
        this.console.tick();
        
        if(this.controller == null)
            return;
        
        this.prediction.clientMove(this);
        this.viewCamera.tick();
        
        for(var component in this.controller.plane.area)
            this.controller.plane.area[component].onLocalTick();
    }
});
