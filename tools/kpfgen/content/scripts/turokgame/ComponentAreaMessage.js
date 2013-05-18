//-----------------------------------------------------------------------------
//
// ComponentAreaMessage.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentAreaMessage = class.extendStatic(ComponentArea, function()
{
    this.message = new Array();
});

class.properties(ComponentAreaMessage,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    active      : true,
    message     : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onLocalTick : function(player)
    {
        if(player == undefined || player == null)
            return;
            
        if(!player.controller.onGround())
            return;
            
        if(!Array.isArray(this.message))
            return;
            
        var len = this.message.length;
            
        if(len >= 8 || len <= 0)
            return;
        
        for(var i = 0; i < len; i++)
        {
            var msg = this.message[(len-1)-i];
            var hud = player.playerHud;
            var hMsg = hud.messages[((7+hud.strCounter)-i)&7];
            
            if(hMsg.state == 0)
                hMsg.addText(msg);
            else
            {
                hMsg.curTime = 2.0;
                hMsg.text = msg;
            }
        }
    }
});
