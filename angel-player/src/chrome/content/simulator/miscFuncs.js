function printMethods(obj)
{
    var prop;
    for(prop in obj)
    {
        printOut("Foo has property " + prop);
    }
}

function printOut(string)
{
  Components.utils.reportError(string);
  console.log(string);
}

// TODO(ericnguyen): fix bug with saving wrong frame (right version, wrong frame)
function saveFrame(simulator)
{
    simulator.targetVersion = simulator.master.version;
    simulator.targetFrame = simulator.master.frame;
    console.log("saved to version " + simulator.master.version);
    console.log("saved to frame " + simulator.master.frame);
}

// TODO(ericnguyen): fix bug with jumping to wrong frame
function loadFrame(simulator)
{
    console.log("from ver. " + simulator.master.version + " to " + simulator.targetVersion);
    simulator.master.version = simulator.targetVersion;
}

// creates a test car
// TODO(ericnguyen): create save/load format for robots
function createCar3(simulator)
{
    var robot = new Robot(simulator, simulator.master);
    var testShape = new Ammo.btBoxShape(new Ammo.btVector3(1, 5, 1));
    var cylShape = new Ammo.btCylinderShape(new Ammo.btVector3(6, 1, 6));
    robot.initChassi();
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(5, 1, 10)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 0)),
       100,
       createBoxMesh(10, 2, 20, 0xaa00aa, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 3, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 3, 8.5)),
       10,
       createBoxMesh(2, 6, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 4, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 4, 6)),
       10,
       createBoxMesh(2, 8, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 5, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 5, 3)),
       10,
       createBoxMesh(2, 10, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 6, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 6, 0)),
       10,
       createBoxMesh(2, 12, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 5, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 5, -3)),
       10,
       createBoxMesh(2, 10, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 4, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 4, -6)),
       10,
       createBoxMesh(2, 8, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 3, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 2 + 3, -8.5)),
       10,
       createBoxMesh(2, 6, 2, 0x000044, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(2, 2, 5)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(5, 2 + 2, 0)),
       10,
       createBoxMesh(4, 4, 10, 0x222222, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(2, 2, 5)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-5, 2 + 2, 0)),
       10,
       createBoxMesh(4, 4, 10, 0x222222, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 1, 3)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-5, 2 + 2, 5 + 3)),
       10,
       createBoxMesh(2, 2, 6, 0x666666, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(1, 1, 3)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(5, 2 + 2, 5 + 3)),
       10,
       createBoxMesh(2, 2, 6, 0x666666, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(0.5, 0.5, 5)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(5, 2 + 2, 5 + 6 + 5)),
       10,
       createBoxMesh(1, 1, 10, 0x999999, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(0.5, 0.5, 5)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-5, 2 + 2, 5 + 6 + 5)),
       10,
       createBoxMesh(1, 1, 10, 0x999999, simulator.scene));
    robot.finishChassi(0, 100, 0);

    var x = [1, -1, 1, -1];
    var y = [1, 1, -1, -1];
    for(var i = 0; i < 4; i++)
    {
        temp_motor = new Motor(simulator, 2, 2, 2, 100, 0xff99ff, 3, 2, 100, 0xaabbff, x[i]*5, 50, y[i]*7);
        robot.addMotor(i, temp_motor, temp_motor.motor,
           new Ammo.btVector3(x[i]*5, 0, y[i]*7),
           new Ammo.btVector3(0, 0, -1),
           new Ammo.btVector3(x[i]*1, 0, 0),
           new Ammo.btVector3(0, 0, 1));

    }

    temp_rf = new Rangefinder(simulator, 1, 1, 1, 1, 0, 100, 0, 150);
    robot.addSensor(0, temp_rf, temp_rf.physicsObject,
        new Ammo.btVector3(0, 0, 10),
        new Ammo.btVector3(0, 0, -0.5),
        new Ammo.btVector3(0, 0, 1),
        new Ammo.btVector3(0, 0, 1));
    return robot;
}