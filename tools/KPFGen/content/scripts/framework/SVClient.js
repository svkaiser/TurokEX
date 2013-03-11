//-----------------------------------------------------------------------------
//
// SVClient.js
// DESCRIPTION: Server-side client controller that
// may contain non-persistent player/client data. Used
// to communicate data between server and client
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/Player.js');

const SVC_STATE_INACTIVE    = 0;
const SVC_STATE_ACTIVE      = 1;
const SVC_STATE_INGAME      = 2;

SVClient = class.extends(Player);

class.properties(SVClient,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    state : SVC_STATE_INACTIVE,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    create : function(ev)
    {
        this.state = SVC_STATE_ACTIVE;
        this.peer = ev.peer;
        this.clientID = ev.peer.connectID;
        
        this.resetNetSequence();
    }
});
