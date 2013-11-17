/*
All touchs are a single box : it shoots a ray out at its <0, 0, 1> direction
    run shoots a ray and sets its value to the distance
*/
function TouchSensor(width, length, height, mass, maxRange, iniX, iniY, iniZ)
{
    this.touch = createBox(width, length, height, mass, 0x000000, iniX, iniY, iniZ);
    this.physicsObject = this.touch;
    this.maxRange = maxRange;
}

TouchSensor.prototype.getVal = function()
{
    return this.value;
};

TouchSensor.prototype.run = function()
{
    var tr = new Ammo.btTransform();
    this.touch.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btQuaternion(0, 0, 1, 0);

    var nPos = rotateV3ByQuat(upQ, rot);

    var distance = function(a, b)
    {
        return Math.sqrt((a.x() - b.x())*(a.x() - b.x()) +
                        (a.y() - b.y())*(a.y() - b.y()) +
                        (a.z() - b.z())*(a.z() - b.z()));
    };

    nPos = new Ammo.btVector3(pos.x() + this.maxRange*nPos.x(), pos.y() + this.maxRange*nPos.y(), pos.z() + this.maxRange*nPos.z());

    this.raycast = new Ammo.ClosestRayResultCallback(pos, nPos);

    scene.world.rayTest(pos, nPos, this.raycast);
    if(this.raycast.hasHit)
    {
        var end = this.raycast.get_m_hitPointWorld();

        this.value = (distance(pos, end) < this.maxRange);
    }
    else
    {
        this.value = false;
    }
};