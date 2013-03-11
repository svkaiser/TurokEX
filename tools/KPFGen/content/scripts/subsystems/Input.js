//-----------------------------------------------------------------------------
//
// Input.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Input = class.extendNative(NInput, function()
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    this.shiftkey = new Array(this.NUMKEYS);
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.keyToAscii = function(s)
    {
        return s.charCodeAt();
    }
    
    this.key = function(c)
    {
        return this.shiftkey[c];
    }
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------
    
    for(var c = 0; c < this.NUMKEYS; c++)
        this.shiftkey[c] = c;

    this.shiftkey[this.keyToAscii('1')]     = this.keyToAscii('!');
    this.shiftkey[this.keyToAscii('2')]     = this.keyToAscii('@');
    this.shiftkey[this.keyToAscii('3')]     = this.keyToAscii('#');
    this.shiftkey[this.keyToAscii('4')]     = this.keyToAscii('$');
    this.shiftkey[this.keyToAscii('5')]     = this.keyToAscii('%');
    this.shiftkey[this.keyToAscii('6')]     = this.keyToAscii('^');
    this.shiftkey[this.keyToAscii('7')]     = this.keyToAscii('&');
    this.shiftkey[this.keyToAscii('8')]     = this.keyToAscii('*');
    this.shiftkey[this.keyToAscii('9')]     = this.keyToAscii('(');
    this.shiftkey[this.keyToAscii('0')]     = this.keyToAscii(')');
    this.shiftkey[this.keyToAscii('-')]     = this.keyToAscii('_');
    this.shiftkey[this.keyToAscii('=')]     = this.keyToAscii('+');
    this.shiftkey[this.keyToAscii('[')]     = this.keyToAscii('{');
    this.shiftkey[this.keyToAscii(']')]     = this.keyToAscii('}');
    this.shiftkey[this.keyToAscii('\\')]    = this.keyToAscii('|');
    this.shiftkey[this.keyToAscii(';')]     = this.keyToAscii(':');
    this.shiftkey[this.keyToAscii('\'')]    = this.keyToAscii('"');
    this.shiftkey[this.keyToAscii(',')]     = this.keyToAscii('<');
    this.shiftkey[this.keyToAscii('.')]     = this.keyToAscii('>');
    this.shiftkey[this.keyToAscii('/')]     = this.keyToAscii('?');
    this.shiftkey[this.keyToAscii('`')]     = this.keyToAscii('~');
    
    for(var c = this.keyToAscii('a'); c <= this.keyToAscii('z'); c++)
    {
        var ch = c - this.keyToAscii('a');
        this.shiftkey[c] = this.keyToAscii('A') + ch;
    }
    
    this.add(0, '+attack');
    this.add(0, '-attack');
    this.add(1, '+forward');
    this.add(1, '-forward');
    this.add(2, '+back');
    this.add(2, '-back');
    this.add(3, '+left');
    this.add(3, '-left');
    this.add(4, '+right');
    this.add(4, '-right');
    this.add(5, '+strafeleft');
    this.add(5, '-strafeleft');
    this.add(6, '+straferight');
    this.add(6, '-straferight');
    this.add(7, '+run');
    this.add(7, '-run');
    this.add(8, '+jump');
    this.add(8, '-jump');
    this.add(11, '+nextweap');
    this.add(11, '-nextweap');
    this.add(12, '+prevweap');
    this.add(12, '-prevweap');
});
