//-----------------------------------------------------------------------------
//
// Game.js
//
// DESCRIPTION: User-data for the Game object class.
// Game class is referenced internally
//
//-----------------------------------------------------------------------------

const NUM_LEVELS        = 50;

const SKILL_EASY        = 0;
const SKILL_NORMAL      = 1;
const SKILL_HARD        = 2;
const SKILL_HARDCORE    = 3;

//------------------------------------------------------------------------
// VARS
//------------------------------------------------------------------------

Game.saveData = [];
Game.bAIDisabled = false;
Game.skill = SKILL_NORMAL;

//------------------------------------------------------------------------
// FUNCTIONS
//------------------------------------------------------------------------
 
Game.main = function()
{
    this.saveData.length = NUM_LEVELS;
    
    for(var i = 0; i < this.saveData.length; i++)
        this.saveData[i] = null;
    
    this.config('saveData', 'skill');
    
    // cvars
    Sys.addCvar('g_wpnautoswitch', '1');
    Sys.addCvar('g_displayplayerinfo', '0');
    Sys.addCvar('g_weaponbobbing', '0');
    Sys.addCvar('g_showcrosshairs', '0');
    
    // commands
    Sys.addCommand('disableai', this.disableAI);
    Sys.addCommand('automap', this.toggleAutomap);
    Sys.addCommand('altAmmo', this.swapAmmoTypes);
    
    // input
    Input.add(0, '+attack');
    Input.add(0, '-attack');
    Input.add(1, '+forward');
    Input.add(1, '-forward');
    Input.add(2, '+back');
    Input.add(2, '-back');
    Input.add(3, '+left');
    Input.add(3, '-left');
    Input.add(4, '+right');
    Input.add(4, '-right');
    Input.add(5, '+strafeleft');
    Input.add(5, '-strafeleft');
    Input.add(6, '+straferight');
    Input.add(6, '-straferight');
    Input.add(7, '+run');
    Input.add(7, '-run');
    Input.add(8, '+jump');
    Input.add(8, '-jump');
    Input.add(11, '+nextweap');
    Input.add(11, '-nextweap');
    Input.add(12, '+prevweap');
    Input.add(12, '-prevweap');
    
    //Level.changeMap(42);
}

Game.disableAI = function()
{
    var list = Level.getActors(0);
    
    for(var i = 0; i < list.length; i++)
    {
        var actor = list[i];
        
        if(!actor)
            continue;
        
        if(actor.ai)
        {
            if(this.bAIDisabled)
                actor.ai.bDisabled = false;
            else
                actor.ai.bDisabled = true;
        }
    }
    
    this.bAIDisabled ^= 1;
}

Game.toggleAutomap = function()
{
    ClientPlayer.component.bMapEnabled ^= 1;
}

Game.swapAmmoTypes = function()
{
    ClientPlayer.component.activeWeapon.switchAmmoType();
}

Game.onLevelLoad = function()
{
    /*var id = Level.mapID;
    
    if(id == 42)
    {
        var camera = ClientPlayer.camera;
        
        if(camera !== null && camera !== undefined && camera.owner)
        {
            camera.owner = null;
            camera.yaw = Angle.degToRad(0);
            camera.origin = new Vector(0.0, 40.96, -61.44);
            ClientPlayer.component.bHideHud = true;
            ClientPlayer.component.bLockControls = true;
        }
    }*/
        
    /*var list = Level.getActors(0);
    
    this.saveData[id] = new Array();
    
    for(var i = 0; i < list.length; i++)
    {
        if(list[i].name === "")
            continue;
        
        this.saveData[id][list[i].name] = false;
    }*/
}

Game.onLevelUnLoad = function()
{
    /*var id = Level.mapID;
    var file = Sys.newTextFile('MapSave' + id + '.txt');
    
    Sys.writeTextFile(file, '{\n');
    for(var i in this.saveData[id])
    {
        Sys.writeTextFile(file, '"' + i + '" : ' + this.saveData[id][i] + ',\n');
    }
    
    Sys.writeTextFile(file, '"dummy" : 0 \n}');
    Sys.closeTextFile(file);*/
}
