//-----------------------------------------------------------------------------
//
// Console.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/FontArial.js');

Console = new (class.define(function()
{
    //------------------------------------------------------------------------
    // CONSTANTS
    //------------------------------------------------------------------------
    
    const CON_MAX_HISTORY   = 16;
    
    const CON_STATE_DOWN    = 0;
    const CON_STATE_UP      = 1;
    
    const CON_STICKY_TIME   = 500;
    const CON_BLINK_TIME    = 350;
    
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    // scroll back
    this.scrollBackPos      = 0;
    this.scrollBackStr      = new Array();
    
    // history
    this.historyTop         = 0;
    this.historyCur         = -1;
    this.history            = new Array(CON_MAX_HISTORY);
    
    // typing
    this.typeStr            = "";
    this.typeStrPos         = 0;
    
    // special key checks
    this.bShiftDown         = false;
    this.bCtrlDown          = false;
    
    this.state              = CON_STATE_UP;
    
    this.blinkTime          = 0;
    
    // sticky keys
    this.keyHeld            = false;
    this.lastKeyPressed     = 0;
    this.timePressed        = 0;
    
    // drawing
    this.canvas             = new Canvas();
    this.bShowPrompt        = true;
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    this.setInputText = function(text)
    {
        this.typeStr = text;
    }
    
    this.resetInputText = function()
    {
        this.typeStr = "";
        this.typeStrPos = 0;
    }
    
    this.clearOutput = function()
    {
        this.scrollBackStr.splice(0, this.scrollBackStr.length);
        this.scrollBackStr.length = 0;
    }
    
    this.clearConsole = function()
    {
        Console.resetInputText();
        Console.clearOutput();
    }
    
    this.outputTextLine = function(text)
    {
        var size = Sys.getCvar('con_bufferSize');
        
        if(this.scrollBackStr.length > size)
            this.scrollBackStr.splice(0, this.scrollBackStr.length - size);
        
        this.scrollBackStr.push(text);
    }
    
    Sys.event_OutputText = function(text)
    {
        var curText;
        var strLength = 0;
        var lineLength = 0;
        
        curText = text;
        strLength = text.length;
        
        while(strLength > 0)
        {
            lineLength = curText.indexOf("\n");
            
            if(lineLength == -1)
                lineLength = strLength;
                
            var outText = curText.substr(0, lineLength);
            
            if(arguments[1])
                outText = '<color=' + arguments[1] + '>' + outText + '</color>';
                
            Console.outputTextLine(outText);
            
            curText = curText.substr(lineLength + 1, curText.length);
            strLength -= (lineLength + 1);
        }
    }
    
    this.lineScroll = function(dir)
    {
        if(dir)
        {
            if(this.scrollBackPos < this.scrollBackStr.length)
                this.scrollBackPos++;
        }
        else
        {
            if(this.scrollBackPos > 0)
                this.scrollBackPos--;
        }
    }
    
    this.backSpace = function()
    {
        if(this.typeStr.length > 0)
        {
            var trim = this.typeStr;
            
            this.setInputText(this.typeStr.substr(0, this.typeStrPos-1));
            
            if(this.typeStrPos < trim.length)
            {
                trim = trim.substr(this.typeStrPos, trim.length);
                this.setInputText(this.typeStr.concat(trim));
            }
            
            this.typeStrPos--;
            
            if(this.typeStrPos < 0)
                this.typeStrPos = 0;
        }
    }
    
    this.deleteChar = function()
    {
        if(this.typeStr.length > 0 && this.typeStrPos < this.typeStr.length)
        {
            var trim = this.typeStr;
            
            this.setInputText(this.typeStr.substr(0, this.typeStrPos));            
            trim = trim.substr(this.typeStrPos+1, trim.length);
            this.setInputText(this.typeStr.concat(trim));
        }
    }
    
    this.shiftHeld = function(c)
    {
        return (c == Input.K_RSHIFT || c == Input.K_LSHIFT);
    }
    
    this.parseKey = function(c)
    {
        switch(c)
        {
            case Input.K_BACKSPACE:
                this.backSpace();
                return;
        
            case Input.K_DELETE:
                this.deleteChar();
                return;
                
            case Input.K_LEFT:
                this.moveTypePos(0);
                return;
                
            case Input.K_RIGHT:
                this.moveTypePos(1);
                return;
                
            case Input.K_PAGEUP:
                this.lineScroll(1);
                return;
                    
            case Input.K_PAGEDOWN:
                this.lineScroll(0);
                return;
                
        }
        
        if(c >= 8 && c < 256)
        {
            var trim = this.typeStr;
            
            if(this.bShiftDown)
                c = Input.key(c);
                
            this.setInputText(this.typeStr.substr(0, this.typeStrPos));
            this.setInputText(this.typeStr.concat(String.fromCharCode(c)));
            
            if(this.typeStrPos < this.typeStr.length)
            {
                trim = trim.substr(this.typeStrPos, trim.length);
                this.setInputText(this.typeStr.concat(trim));
            }
            
            this.typeStrPos++;
        }
    }
    
    this.moveTypePos = function(dir)
    {
        if(dir)
        {
            this.typeStrPos++;
            if(this.typeStrPos > this.typeStr.length)
                this.typeStrPos = this.typeStr.length;
        }
        else
        {
            this.typeStrPos--;
            if(this.typeStrPos < 0)
                this.typeStrPos = 0;
        }
    }
    
    this.addCommandToHistory = function(string)
    {
        if(this.historyTop < CON_MAX_HISTORY)
        {
            this.history.unshift(string);
            this.historyTop++;
        }
        else
        {
            this.history.pop();
            this.history.unshift(string);
        }
    }
    
    this.lookupHistory = function(up)
    {
        if(this.historyTop <= 0)
            return;
        
        if(up)
        {
            if(this.historyCur > 0)
                this.historyCur--;
        }
        else
        {
            if(this.historyCur < (this.historyTop-1))
                this.historyCur++;
        }
        
        if(this.historyCur < 0)
            this.historyCur = 0;
        
        this.resetInputText();
        this.setInputText(this.history[this.historyCur]);
        this.typeStrPos = this.typeStr.length;
    }
    
    this.parseInput = function()
    {
        if(this.typeStr.length > 0)
        {
            var msg = '<color=192,192,192>  >>' + this.typeStr + '</color>';
            
            this.addCommandToHistory(this.typeStr);
            this.outputTextLine(msg);
            
            Sys.callCmd(this.typeStr);
            
            this.resetInputText();
            this.historyCur = (this.historyTop - 1);
        }
    }
    
    this.checkShift = function(ev)
    {
        var c = ev.data1;
        
        if(this.shiftHeld(c))
        {
            if(ev.type == EV_KEYDOWN)
                this.bShiftDown = true;
            else if(ev.type == EV_KEYUP)
                this.bShiftDown = false;
        }
    }
    
    this.checkStickyKeys = function(ev)
    {
        if(this.shiftHeld(ev.data1) || ev.data1 == Input.K_RETURN ||
            ev.data1 == Input.K_TAB)
            return;
        
        this.lastKeyPressed = ev.data1;
        
        if(ev.type == EV_KEYDOWN && this.keyHeld == false)
        {
            this.keyHeld = true;
            this.timePressed = Sys.ms();
        }
        else
        {
            this.keyHeld = false;
            this.timePressed = 0;
        }
    }
    
    this.stickyKeyTick = function()
    {
        if(this.keyHeld && ((Sys.ms() - this.timePressed) >= CON_STICKY_TIME))
            this.parseKey(this.lastKeyPressed);
    }
    
    this.updateBlink = function()
    {
        if(this.blinkTime < Sys.time())
        {
            this.bShowPrompt = !this.bShowPrompt;
            this.blinkTime = Sys.time() + CON_BLINK_TIME;
        }
    }
    
    this.tick = function()
    {
        if(this.state == CON_STATE_UP)
            return;
            
        this.stickyKeyTick();
        this.updateBlink();
    }
    
    this.draw = function()
    {
        if(this.state != EV_KEYDOWN)
            return;
        
        var viewport = Render.viewport;
        var w = viewport.width;
        var h = viewport.height * 0.6875;
        
        GL.setBlend(1);
        
        // background
        this.canvas.setDrawColor(4, 8, 16);
        this.canvas.setDrawAlpha(192);
        this.canvas.drawTile('textures/white.tga', 0, 0, w, h);
        
        // prompt text
        this.canvas.setDrawColor(255, 255, 255);
        this.canvas.setFont(FontArial);
        this.canvas.setDrawAlpha(255);
        this.canvas.drawString('> ', 0, h-15);
        
        if(this.bShowPrompt)
        {
            this.canvas.drawString('_', 16 +
                FontArial.stringWidth(this.typeStr, 1.0, this.typeStrPos), h-15);
        }
        
        if(this.typeStr.length > 0)
            this.canvas.drawString(this.typeStr, 16, h-15, true);
            
        if(this.scrollBackStr.length > 0)
        {
            var scy = h-34;
            
            for(var i = this.scrollBackStr.length-(this.scrollBackPos)-1; i >= 0; i--)
            {
                if(scy < 0)
                    break;
                
                this.canvas.drawString(this.scrollBackStr[i], 0, scy);
                scy -= 16;
            }
        }
        
        GL.setBlend(0);
        
        // typing borders
        this.canvas.setDrawColor(0, 128, 255);
        this.canvas.setDrawAlpha(255);
        this.canvas.drawTile('textures/white.tga', 0, h-17, w, 1);
        this.canvas.drawTile('textures/white.tga', 0, h, w, 1);
    }
    
    this.processInput = function(ev)
    {
        if(ev.type == EV_MOUSEDOWN ||
            ev.type == EV_MOUSEUP ||
            ev.type == EV_MOUSE)
        {
            return false;
        }
        
        if(ev.type == EV_MOUSEWHEEL && this.state == CON_STATE_DOWN)
        {
            switch(ev.data1)
            {
                case Input.BUTTON_WHEELUP:
                    this.lineScroll(1);
                    break;
                    
                case Input.BUTTON_WHEELDOWN:
                    this.lineScroll(0);
                    break;
            }
            
            return true;
        }
        
        this.checkShift(ev);
        this.checkStickyKeys(ev);
        
        var c = ev.data1;
        
        switch(this.state)
        {
            case CON_STATE_DOWN:
                if(ev.type == EV_KEYDOWN)
                {
                    switch(c)
                    {
                        case Input.K_BACKQUOTE:
                            this.state = CON_STATE_UP;
                            return true;
                            
                        case Input.K_RETURN:
                            this.parseInput();
                            return true;
                            
                        case Input.K_UP:
                            this.lookupHistory(true);
                            return true;
                            
                        case Input.K_DOWN:
                            this.lookupHistory(false);
                            return true;
                            
                        case Input.K_TAB:
                            this.parseKey(Input.K_SPACE);
                            this.parseKey(Input.K_SPACE);
                            this.parseKey(Input.K_SPACE);
                            this.parseKey(Input.K_SPACE);
                            return true;
                            
                        default:
                            this.parseKey(c);
                            return true;
                    }
                    
                    return false;
                }
                break;
                
            case CON_STATE_UP:
                if(ev.type == EV_KEYDOWN)
                {
                    switch(c)
                    {
                        case Input.K_BACKQUOTE:
                            this.state = CON_STATE_DOWN;
                            return true;
                    }
                    
                    return false;
                }
                break;
                
            default:
                return false;
        }
        
        return false;
    }
    
    //------------------------------------------------------------------------
    // INITIALIZATION
    //------------------------------------------------------------------------

    Sys.addCvar('con_bufferSize', '64');
    Sys.addCommand('clear', this.clearConsole);
    
    this.config(
        'history',
        'historyCur'
        );
}));
