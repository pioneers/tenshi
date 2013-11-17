/*
All infrareds are a single box : it shoots a ray out at its <0, 0, 1> direction
    run shoots a ray and sets its value to the color of the object it hits
*/
function Infrared(width, length, height, mass, iniX, iniY, iniZ)
{
    this.infrared = createBox(width, length, height, mass, 0x000000, iniX, iniY, iniZ);
    this.physicsObject = this.infrared;
}

Infrared.prototype.getVal = function()
{
    return this.value;
};

Infrared.prototype.run = function()
{
    var tr = new Ammo.btTransform();
    this.infrared.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btQuaternion(0, 0, 1, 0);

    var nPos = rotateV3ByQuat(upQ, rot);

    nPos = new Ammo.btVector3(pos.x() + 1000*nPos.x(), pos.y() + 1000*nPos.y(), pos.z() + 1000*nPos.z());

    this.raycast = new Ammo.ClosestRayResultCallback(pos, nPos);

    scene.world.rayTest(pos, nPos, this.raycast);
    if(this.raycast.hasHit)
    {
        var obj = this.raycast.get_m_collisionObject().getCollisionShape();

        try
        {
            this.value = obj.parent.mesh.material.color.getHex();
        }
        catch(err)
        {
            console.log(String(err));
        }
    }
};