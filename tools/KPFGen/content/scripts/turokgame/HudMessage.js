//-----------------------------------------------------------------------------
//
// HudMessage.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

HudMessage = class.define();

class.properties(HudMessage,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    text    : "",
    alpha   : 0,
    scale   : 1.0,
    time    : 15.0,
    curTime : 0.0,
    state   : 0,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    addText : function(str)
    {
        this.text = str;
        this.state = 1;
        this.alpha = 0;
        this.scale = 1.0;
        this.time = 15.0;
        this.curTime = 0.0;
    },
    
    update : function()
    {
        var state = this.state;
        
        if(state <= 0)
            return;
        
        if(state == 3)
        {
            var a = 255.0 - this.alpha * 0.01232;
            this.scale -= (Math.sin(a) * 0.33333334);
            
            if(this.scale <= 0.02)
                this.scale = 0.02;
                
            this.alpha -= 24;
            if(this.alpha - 24 <= 0)
            {
                this.alpha = 0;
                this.state = 0;
            }
        }
        else if(state == 2)
        {
            this.curTime -= (15 * Sys.deltatime());
            this.scale = 1.0;
            
            if(this.curTime <= 0)
                this.state = 3;
        }
        else if(state == 1)
        {
            this.alpha += 31;
            if(this.alpha >= 255)
            {
                this.curTime = this.time;
                this.state = 2;
                this.alpha = 255;
                this.scale = Math.sin(this.alpha * 0.00616);
            }
        }
    }
});
