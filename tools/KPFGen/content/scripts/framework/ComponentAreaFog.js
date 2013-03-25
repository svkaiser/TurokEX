//-----------------------------------------------------------------------------
//
// ComponentAreaFog.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/ComponentArea.js');

ComponentAreaFog = class.extendStatic(ComponentArea);

class.properties(ComponentAreaFog,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    fog_Color_r : 0,
    fog_Color_g : 0,
    fog_Color_b : 0,
    fog_Far     : 1024.0,
    active      : true,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onPreRender : function(instigator)
    {
        if(Sys.getCvar('r_fog') <= 0)
        {
            Render.clearViewPort(0.25, 0.25, 0.25);
            return;
        }
        
        const FOG_LERP_SPEED = 0.025;
        
        Render.fogColor[0] = Math.lerp(Render.fogColor[0], (this.fog_Color_r / 255.0), FOG_LERP_SPEED);
        Render.fogColor[1] = Math.lerp(Render.fogColor[1], (this.fog_Color_g / 255.0), FOG_LERP_SPEED);
        Render.fogColor[2] = Math.lerp(Render.fogColor[2], (this.fog_Color_b / 255.0), FOG_LERP_SPEED);
        Render.fogColor[3] = 1;
        
        Render.clearViewPort(
            Render.fogColor[0],
            Render.fogColor[1],
            Render.fogColor[2]);
        
        Render.fogFar = Math.lerp(Render.fogFar, this.fog_Far, FOG_LERP_SPEED);
        Render.fogNear = Render.fogFar * 0.5;
        
        GL.enable(GL.FOG);
        GL.fog(GL.FOG_COORD_SRC, GL.FRAGMENT_DEPTH);
        GL.fog(GL.FOG_COLOR, Render.fogColor);
        GL.fog(GL.FOG_START, Render.fogNear);
        GL.fog(GL.FOG_END, Render.fogFar);
    }
});
