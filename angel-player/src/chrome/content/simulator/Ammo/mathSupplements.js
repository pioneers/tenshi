// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

const window = require('tenshi/common/window')();
let {Ammo} = window;

function strVector(v)
{
    return "<" + v.x() + ", " + v.y() + ", " + v.z() + ">";
}

function strQuat(q)
{
    return "<" + q.x() + ", " + q.y() + ", " + q.z() + ", " + q.w() + ">";
}

// Replaces rotation of transform, for use of instant rotation
function replaceQuaternionOfTransform(base, quat)
{
  var tr = new Ammo.btTransform();
      tr.setRotation(quat);
      tr.setOrigin(new Ammo.btVector3(base.getOrigin().x(),
                                      base.getOrigin().y(),
                                      base.getOrigin().z()));

  return tr;
}

// Replaces position of transform, for use of teleports
function replacePositionOfTransform(base, vector)
{
  var tr = new Ammo.btTransform();
      tr.setRotation(new Ammo.btQuaternion(base.getRotation().x(),
                                           base.getRotation().y(),
                                           base.getRotation().z(),
                                           base.getRotation().w()));
      tr.setOrigin(vector);

  return tr;

}

// Returns a transformation with an added position vector
function translateTransform(base, vector)
{
  var tr = new Ammo.btTransform();
      tr.setRotation(new Ammo.btQuaternion(base.getRotation().x(),
                                           base.getRotation().y(),
                                           base.getRotation().z(),
                                           base.getRotation().w()));
      tr.setOrigin(new Ammo.btVector3(base.getOrigin().x() + vector.x(),
                                      base.getOrigin().y() + vector.y(),
                                      base.getOrigin().z() + vector.z()));

  return tr;
}

// Returns a transformation with an added rotation quaternion
function rotateTransform(base, quat)
{
  var tr = new Ammo.btTransform();
      tr.setRotation(multQuats(quat, base.getRotation()));
      tr.setOrigin(new Ammo.btVector3(base.getOrigin().x(),
                                      base.getOrigin().y(),
                                      base.getOrigin().z()));

  return tr;
}

function eulerToQuat(roll, pitch, yaw)
{
  var x, y, z, w;
  var c1, c2, c3,
      s1, s2, s3;

  c1 = Math.cos(roll/2);
  c2 = Math.cos(pitch/2);
  c3 = Math.cos(yaw/2);

  s1 = Math.sin(roll/2);
  s2 = Math.sin(pitch/2);
  s3 = Math.sin(yaw/2);

  x = s1*s2*c3 + c1*c2*s3;
  y = s1*c2*c3 + c1*s2*s3;
  z = c1*s2*c3 - s1*c2*s3;
  w = c1*c2*c3 - s1*s2*s3;

  return new Ammo.btQuaternion(x, y, z, w);
}

function quatToEuler(q)
{
  var test = q.x()*q.y() + q.z()*q.w();
  if(test > 0.499)
  {
    return new Ammo.btVector3(2*Math.atan2(q.x(), q.w()), Math.PI/2, 0);
  }
  else if(test < -0.499)
  {
    return new Ammo.btVector3(-2*Math.atan2(q.x(), q.w()), -Math.PI/2, 0);
  }

  var sol = new Ammo.btVector3(
            Math.atan2(2*q.y()*q.w() - 2*q.x()*q.z(),
                       1 - 2*q.y()*q.y() - 2*q.z()*q.z()),
            Math.sin(2*q.x()*q.y() + 2*q.z()*q.w()),
            Math.atan2(2*q.x()*q.w() - 2*q.y()*q.z(),
                       1 - 2*q.x()*q.x() - 2*q.z()*q.z()));
  return sol;
}

// Multiplies quaternions; built in library's multiply is broken
function multQuats(a, b)
{
    var c = new Ammo.btQuaternion(a.x()*b.x() - a.y()*b.y() - a.z()*b.z() - a.w()*b.w(),
      a.x()*b.y() + a.y()*b.x() - a.z()*b.w() + a.w()*b.z(),
      a.x()*b.z() + a.y()*b.w() + a.z()*b.x() - a.w()*b.y(),
      a.x()*b.w() - a.y()*b.z() + a.z()*b.y() + a.w()*b.x());
    return c;

}

function addVector(a, b)
{
  return new Ammo.btVector3(a.x() + b.x(), a.y() + b.y(), a.z() + b.z());
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

exports.rotateV3ByQuat = rotateV3ByQuat;
exports.distance = distance;
