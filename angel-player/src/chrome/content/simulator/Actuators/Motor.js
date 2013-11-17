/*
All motors consist of a box and a cylinder shape
    getBox gets the motor collidable shape
    getWheel gets the wheel collidable shape
    run reads the value stored, and runs the motor
*/
function Motor(boxX, boxY, boxZ, boxMass, boxColor, wheelRad, wheelHeight, wheelMass, wheelColor, iniX, iniY, iniZ)
{
    this.motor = createBox(boxX, boxY, boxZ, boxMass, boxColor, iniX, iniY + boxY, iniZ - boxZ);
    this.wheel = createCylinder(wheelRad, wheelHeight, wheelMass, wheelColor, iniX, iniY - wheelHeight, iniZ + wheelRad);
    this.value = 0;

    this.constraint = new Ammo.btHingeConstraint(this.motor, this.wheel,
                                                 new Ammo.btVector3(0, 0, boxZ/2),
                                                 new Ammo.btVector3(0, -wheelHeight/2, 0),
                                                 new Ammo.btVector3(0, 0, 1),
                                                 new Ammo.btVector3(0, 1, 0));
    scene.world.addConstraint(this.constraint); // TODO(ericnguyen): not use globals

    return this;
}

Motor.prototype.getBox = function()
{
    return this.motor;
};

Motor.prototype.getWheel = function()
{
    return this.wheel;
};

Motor.prototype.getVal = function()
{
    return this.value;
};

Motor.prototype.setVal = function(val)
{
    this.value = val;
};

Motor.prototype.run = function()
{
    var value = this.value;
    var tr = new Ammo.btTransform();
    this.wheel.getMotionState().getWorldTransform(tr);

    var pos = tr.getOrigin();
    var rot = tr.getRotation();
    var upQ = new Ammo.btVector3(0, 1, 0);

    var newQ = rotateV3ByQuat(upQ, rot);
        newQ = new Ammo.btVector3(value*newQ.x(), value*newQ.y(), value*newQ.z());

    this.wheel.setAngularVelocity(newQ);
};