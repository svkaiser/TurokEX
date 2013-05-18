//-----------------------------------------------------------------------------
//
// ComponentPillarMover.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

ComponentPillarMover = class.extendStatic(Component);

class.properties(ComponentPillarMover,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    moveSpeed       : 8,
    moveAmount      : -1,
    diffHeight      : 0.0,
    destHeight      : 0.0,
    time            : 0.0,
    bMove           : false,
    distance        : 0.0,
    moveSound       : "",
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    //------------------------------------------------------------------------
    // EVENTS
    //------------------------------------------------------------------------
    
    onReady : function()
    {
    },
    
    onTrigger : function(instigator, args)
    {
        if(this.moveSpeed == 0)
            return;
        
        var actor = this.parent.owner;
        
        Snd.play('sounds/shaders/' + this.moveSound + '.ksnd', actor);
        
        this.bMove = true;
        this.moveSpeed = 1.0 / this.moveSpeed;
        this.destHeight = actor.origin.y;
        this.diffHeight = (5 * this.moveAmount) * -10.24 + actor.origin.y;
        this.time = 1.0;
        
        var plane = Plane.fromIndex(actor.plane);
        this.distance = plane.distance(actor.origin);
    },
    
    onTick : function()
    {  
        if(this.bMove == false)
            return;
            
        var actor = this.parent.owner;
        
        var y = (Math.cos((1.0 - this.time) * Math.PI) - 1.0) * 0.5 *
            (this.diffHeight - this.destHeight) + this.destHeight;
        
        this.time -= (actor.timeStamp * this.moveSpeed);
        
        if(this.time <= 0)
        {
            this.bMove = false
            return;
        }
        
        var height = y - this.destHeight + this.distance;
        
        Level.changeFloorHeight(actor.plane, height);
        
        var newOrg = new Vector(actor.origin.x, y, actor.origin.z);
        actor.origin = newOrg;
        actor.updateTransform();
    }
});
