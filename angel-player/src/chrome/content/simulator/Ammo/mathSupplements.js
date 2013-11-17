function strVector(v)
{
    return "<" + v.x() + ", " + v.y() + ", " + v.z() + ">";
}

function strQuat(q)
{
    return "<" + q.x() + ", " + q.y() + ", " + q.z() + ", " + q.z() + ">";
}

// Multiplies quaternions; built in library's multiply is broken
function multQuats(a, b)
{
    c = new Ammo.btQuaternion(a.x()*b.x() - a.y()*b.y() - a.z()*b.z() - a.w()*b.w(),
      a.x()*b.y() + a.y()*b.x() - a.z()*b.w() + a.w()*b.z(),
      a.x()*b.z() + a.y()*b.w() + a.z()*b.x() - a.w()*b.y(),
      a.x()*b.w() - a.y()*b.z() + a.z()*b.y() + a.w()*b.x());
    return c;

}

// Rotates a real vector by a quaternion transformation
function rotateV3ByQuat(vector, quat)
{
    var tempQuat = new Ammo.btQuaternion(vector.x(), vector.y(), vector.z(), 0);
    var quatConj = new Ammo.btQuaternion(-quat.x(), -quat.y(), -quat.z(), quat.w());

    return multQuats(multQuats(quat, tempQuat), quatConj);
}