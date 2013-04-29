//-----------------------------------------------------------------------------
//
// Ammo.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Ammo = class.define();

class.properties(Ammo,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    initial     : 0,
    max         : 0,
    maxBackpack : 0,
    consumeAmt  : 1,
    amount      : 0,
    hudIcon     : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    give : function(player, amt)
    {
        this.amount += amt;
        
        if(player.bHasBackpack)
        {
            if(this.amount > this.maxBackpack)
                this.amount = this.maxBackpack;
        }
        else
        {
            if(this.amount > this.max)
                this.amount = this.max;
        }
    },
    
    use : function()
    {
        this.amount -= this.consumeAmt;
        if(this.amount < 0)
            this.amount = 0;
    }
});
