//-----------------------------------------------------------------------------
//
// main.js
// DESCRIPTION: Main Script File
//
//-----------------------------------------------------------------------------

var Global = this;

Sys.runScript('scripts/base/Class.js');
Sys.runScript('scripts/base/JSON.js');
Sys.runScript('scripts/base/Object.js');
Sys.runScript('scripts/base/Function.js');
Sys.runScript('scripts/base/Math.js');
Sys.runScript('scripts/base/Array.js');

Sys.runScript('scripts/subsystems/Input.js');
Sys.runScript('scripts/subsystems/Client.js');
Sys.runScript('scripts/subsystems/Server.js');
Sys.runScript('scripts/subsystems/Render.js');
Sys.runScript('scripts/subsystems/Game.js');

// TEMP
Sys.runScript('scripts/framework/ComponentPlayerStart.js');
Sys.runScript('scripts/framework/ComponentAreaFog.js');
Sys.runScript('scripts/framework/ComponentAreaWater.js');
Sys.runScript('scripts/framework/ComponentAreaClimb.js');
Sys.runScript('scripts/framework/ComponentAreaCrawl.js');
Sys.runScript('scripts/framework/ComponentAreaSky.js');
Sys.runScript('scripts/framework/ComponentPickup.js');

Sys.runScript('scripts/turokgame/Turok.js');