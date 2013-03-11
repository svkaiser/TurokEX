//-----------------------------------------------------------------------------
//
// Camera.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Camera = class.define(function()
{
    this.origin     = new Vector();
    this.angles     = { yaw : 0, pitch : 0, roll : 0 };
    this.yaw        = new Quaternion(0, 0, 1, 0);
    this.pitch      = new Quaternion(0, 1, 0, 0);
    this.roll       = new Quaternion(0, 0, 0, 1);
    this.matrix     = new Matrix();
});

class.properties(Camera,
{
    draw : function()
    {
        var angles = this.angles;
        
        Matrix.setProjection(Sys.getCvar('cl_fov'), 0.1);
        Matrix.setModelView();
        
        // setup angles
        this.yaw.setRotation(-angles.yaw + Math.PI, 0, 1, 0);
        this.pitch.setRotation(angles.pitch, 1, 0, 0);
        this.roll.setRotation(angles.roll, 0, Math.sin(angles.pitch), Math.cos(angles.pitch));
        
        // setup matrix
        this.matrix.setRotation(Quaternion.multiply(
            Quaternion.multiply(this.yaw, this.roll), this.pitch));
            
        var pos = Vector.toWorld(this.origin, this.matrix);

        // set and load matrix
        this.matrix.addTranslation(-pos.x, -pos.y, -pos.z);
        this.matrix.load();
    }
});
