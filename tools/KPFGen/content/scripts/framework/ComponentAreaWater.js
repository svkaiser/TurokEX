//-----------------------------------------------------------------------------
//
// ComponentAreaWater.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Sys.dependsOn('scripts/framework/ComponentArea.js');

const WL_INVALID        = 0;
const WL_OVER           = 1;
const WL_BETWEEN        = 2;
const WL_UNDER          = 3;

ComponentAreaWater = class.extendStatic(ComponentArea);

class.properties(ComponentAreaWater,
{
    waterYPlane : 0.0,
    tint_r      : 0,
    tint_g      : 18,
    tint_b      : 95,
    tint_a      : 160,
    active      : true,
    
    getWaterLevel : function(controller)
    {
        var p = controller.plane;
        
        if(p != null)
        {
            controller.waterheight = p.area.ComponentAreaWater.waterYPlane;
                
            if(((controller.origin.y - controller.center_y) -
                controller.plane.distance(controller.origin)) +
                (controller.waterheight - controller.origin.y) >= controller.center_y)
            {
                if(controller.origin.y + controller.center_y >= controller.waterheight)
                {
                    if(controller.origin.y < controller.waterheight)
                        return WL_BETWEEN;
                    else
                        return WL_OVER;
                }
                
                return WL_UNDER;
            }
        }
        
        return WL_INVALID;
    }
});
