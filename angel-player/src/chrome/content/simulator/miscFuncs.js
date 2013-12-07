function printMethods(obj)
{
    var prop;
    for(prop in obj)
    {
        console.log("Foo has property " + prop);
    }
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
    var testShape = new Ammo.btBoxShape(new Ammo.btVector3(1, 1, 5));
    var cylShape = new Ammo.btCylinderShape(new Ammo.btVector3(6, 1, 6));
    robot.initChassi();
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(5, 10, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 0)),
       100,
       createBoxMesh(10, 20, 2, 0xaa00aa, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(5, 5, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 2)),
       100,
       createBoxMesh(10, 10, 2, 0xbb00bb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(4, 4, 2 + 5 + 1)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(4, -4, 2 + 5 + 1)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-4, 4, 2 + 5 + 1)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-4, -4, 2 + 5 + 1)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(5, 6, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 2 + 5 + 5 + 1 + 1)),
       10,
       createBoxMesh(10, 12, 2, 0x0055555, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(3, 3, 2 + 5 + 5 + 1 + 1 + 1 + 5)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(3, -3, 2 + 5 + 5 + 1 + 1 + 1 + 5)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-3, 3, 2 + 5 + 5 + 1 + 1 + 1 + 5)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(testShape,
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(-3, -3, 2 + 5 + 5 + 1 + 1 + 1 + 5)),
       10,
       createBoxMesh(2, 2, 10, 0xbbbbbb, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(4, 4, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 2 + 5 + 5 + 1 + 1 + 1 + 10 + 1)),
       10,
       createBoxMesh(8, 8, 2, 0x664477, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(3, 3, 1)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 2 + 5 + 5 + 1 + 1 + 1 + 10 + 2 + 1)),
       10,
       createBoxMesh(6, 6, 2, 0x593366, simulator.scene));
    robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(2, 20, 2)),
       new Ammo.btTransform(new Ammo.btQuaternion(0, 0, 0, 1),
       new Ammo.btVector3(0, 0, 8)),
       10,
       createBoxMesh(4, 40, 4, 0x110011, simulator.scene));
    robot.finishChassi(0, 0, 100);

    var x = [1, -1, 1, -1];
    var y = [1, 1, -1, -1];
    for(var i = 0; i < 4; i++)
    {
        temp_motor = new Motor(simulator, 2, 2, 2, 100, 0xff99ff, 3, 2, 100, 0xaabbff, x[i]*5, y[i]*7, 100);
        robot.addMotor(i, temp_motor, temp_motor.motor,
           new Ammo.btVector3(x[i]*5, y[i]*7, 0),
           new Ammo.btVector3(0, 0, -1),
           new Ammo.btVector3(x[i]*1, 0, 0),
           new Ammo.btVector3(0, 0, 1));

    }

    temp_rf = new Rangefinder(simulator, 1, 1, 1, 1, 0, 100, 0, 150);
    robot.addSensor(0, temp_rf, temp_rf.physicsObject,
        new Ammo.btVector3(0, 10, 0),
        new Ammo.btVector3(0, 0, -0.5),
        new Ammo.btVector3(0, 1, 0),
        new Ammo.btVector3(0, 0, 1));
    return robot;
}