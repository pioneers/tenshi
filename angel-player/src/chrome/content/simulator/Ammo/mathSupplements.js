function strVector(v)
{
    return "<" + v.x() + ", " + v.y() + ", " + v.z() + ">";
}

function strQuat(q)
{
    return "<" + q.x() + ", " + q.y() + ", " + q.z() + ", " + q.w() + ">";
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

function toUnitVector(vector)
{
  var magnitude = Math.sqrt(vector.x()*vector.x() +
                            vector.y()*vector.y() +
                            vector.z()*vector.z());
  return new Ammo.btVector3(vector.x()/magnitude, vector.y()/magnitude, vector.z()/magnitude);
}

function scaleVector(scale, vector)
{
  return new Ammo.btVector3(vector.x()*scale, vector.y()*scale, vector.z()*scale);
}

// a and b are btVector3s representing points
function distance(a, b)
{
  return Math.sqrt((a.x() - b.x())*(a.x() - b.x()) +
                   (a.y() - b.y())*(a.y() - b.y()) +
                   (a.z() - b.z())*(a.z() - b.z()));
}