//-----------------------------------------------------------------------------
//
// TurokString.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

TurokString = new (class.define(function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.text_r1 = 180;
    this.text_r2 = 77;
    this.text_g1 = 180;
    this.text_g2 = 63;
    this.text_b1 = 124;
    this.text_b2 = 42;
    
    this.numbers = [
        'hud/h_num9.tga',
        'hud/h_num8.tga',
        'hud/h_num7.tga',
        'hud/h_num6.tga',
        'hud/h_num5.tga',
        'hud/h_num4.tga',
        'hud/h_num3.tga',
        'hud/h_num2.tga',
        'hud/h_num1.tga',
        'hud/h_num0.tga'
    ];
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.setStringColor = function(r1, g1, b1, r2, g2, b2)
    {
        this.text_r1 = r1;
        this.text_g1 = g1;
        this.text_b1 = b1;
        this.text_r2 = r2;
        this.text_g2 = g2;
        this.text_b2 = b2;
    }
    
    this.drawText = function(canvas, text, x, y, bCenter)
    {
        if(typeof text !== 'string')
            return;
            
        canvas.setFont(FontHud);
        
        canvas.setVertexColor(0, this.text_r1>>2, this.text_g1>>2, this.text_b1>>2);
        canvas.setVertexColor(1, this.text_r1>>2, this.text_g1>>2, this.text_b1>>2);
        canvas.setVertexColor(2, this.text_r2>>2, this.text_g2>>2, this.text_b2>>2);
        canvas.setVertexColor(3, this.text_r2>>2, this.text_g2>>2, this.text_b2>>2);
     
        var offset = 6 * canvas.scale;
        if(offset <= 0)
            offset = 1;
        
        canvas.drawFixedString(text.toUpperCase(), x+offset, y+offset, bCenter);
        
        canvas.setVertexColor(0, this.text_r1, this.text_g1, this.text_b1);
        canvas.setVertexColor(1, this.text_r1, this.text_g1, this.text_b1);
        canvas.setVertexColor(2, this.text_r2, this.text_g2, this.text_b2);
        canvas.setVertexColor(3, this.text_r2, this.text_g2, this.text_b2);
        
        canvas.drawFixedString(text.toUpperCase(), x, y, bCenter);
    }
    
    this.drawNumber = function(canvas, value, x, y)
    {
        if(typeof value !== 'number')
            return;
        
        var str = value.toString();
        var pos = x;
        
        for(var i = 0; i < str.length; i++)
        {
            var ch = str.charAt(i);
            
            if(ch === '-')
            {
                canvas.drawFixedTile('hud/h_minus.tga', pos, y);
                pos += 9;
                continue;
            }
            
            var n = 57 - ch.charCodeAt();
            
            if(n > 9 || n < 0)
                continue;
                
            canvas.drawFixedTile(this.numbers[n], pos, y);
            pos += 9;
        }
    }
}));
