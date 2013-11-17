/*
All rangefinders are a single box : it shoots a ray out at its <0, 0, 1> direction
    run shoots a ray and sets its value to the distance
*/
function Rangefinder(width, length, height, mass, iniX, iniY, iniZ)
{
    this.rangefinder = createBox(width, length, height, mass, 0x000000, iniX, iniY, iniZ);
    this.physicsObject = this.rangefinder;
}

Rangefinder.prototype.getVal = function()
{
    return this.value;
};

Rangefinder.prototype.run = function()
{
    var tr = new Ammo.btTransform();
    this.rangefinder.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btQuaternion(0, 0, 1, 0);

    var nPos = rotateV3ByQuat(upQ, rot);

    var distance = function(a, b)
    {
        return Math.sqrt((a.x() - b.x())*(a.x() - b.x()) +
    (a.y() - b.y())*(a.y() - b.y()) +
    (a.z() - b.z())*(a.z() - b.z()));
    }

    nPos = new Ammo.btVector3(pos.x() + 1000*nPos.x(), pos.y() + 1000*nPos.y(), pos.z() + 1000*nPos.z());

    this.raycast = new Ammo.ClosestRayResultCallback(pos, nPos);

    scene.world.rayTest(pos, nPos, this.raycast);
    if(this.raycast.hasHit)
    {
        var end = this.raycast.get_m_hitPointWorld();

        this.value = distance(pos, end);
    }
};