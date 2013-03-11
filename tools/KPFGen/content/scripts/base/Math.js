//-----------------------------------------------------------------------------
//
// Math.js
// DESCRIPTION: Extensions to Javascript Math class
//
//-----------------------------------------------------------------------------

Math.lerp = function(cur, next, time)
{
    var t = (time * (60 * Sys.deltatime()));
    if(t > 1) return next;
    return (next - cur) * t + cur;
}

Math.qNormalize = function(rot)
{
    var d = Math.sqrt(rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w);

    if(d != 0.0)
    {
        rot.x = rot.x * 1.0 / d;
        rot.y = rot.y * 1.0 / d;
        rot.z = rot.z * 1.0 / d;
        rot.w = rot.w * 1.0 / d;
    }
}
