//-----------------------------------------------------------------------------
//
// Ammo.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Ammo = class.define(function()
{
    this.config(
        'amount',
        'max'
        );
});

class.properties(Ammo,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    initial     : 0,
    max         : 0,
    maxBackpack : 0,
    amount      : 0,
    hudIcon     : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    give : function(player, amt)
    {
        if(player.bHasBackpack)
        {
            if(this.amount == this.maxBackpack)
                return false;
        }
        else
        {
            if(this.amount == this.max)
                return false;
        }
        
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
        
        return true;
    },
    
    use : function(amt)
    {
        this.amount -= amt;
        if(this.amount < 0)
            this.amount = 0;
    }
});
