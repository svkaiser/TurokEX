var KScript_Chains = [];    // script chain array

// base script function
function KScript_Behavior()
{
    this.getScriptChains = function() { return KScript_Chains.length; }
    this.useOrtho = true;
    this.broadcastMessage = function(method) { KScript_Run(method); }
};

KScript_Behavior.prototype.OnStart = function() { };    // called before client/server updates
KScript_Behavior.prototype.OnUpdate = function() { };   // called at every frame
KScript_Behavior.prototype.OnRender = function() { };   // called at every draw frame
KScript_Behavior.prototype.OnOrtho = function() { };    // called on ortho mode

// script class initializer
function KScript(script)
{
    var _construct = function()
        {
            if(script.OnConstruct)
            {
                script.OnConstruct();
                delete script.OnConstruct;
            }
        };

    _construct.prototype = new KScript_Behavior();
    _construct.base = KScript_Behavior.prototype;
    
    for(var p in script)
        _construct.prototype[p] = script[p];

    // add this instance to the array
    KScript_Chains.push(new _construct);

    return _construct;
}

var KScript_Run = function(method)
{
    for(var i = 0; i < KScript_Chains.length; i++)
    {
        var chain = KScript_Chains[i];
        if(chain instanceof KScript_Behavior)
        {
            if(chain[method])
                chain[method]();
        }
    }
}    