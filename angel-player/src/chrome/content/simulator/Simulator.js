/*
creates a simulator, with canvas size width and height, in domElement
    initScene initalizes THREE renderer
    initPhysics intializes AMMO physics world
    initMouseCameraControls binds mouse events to camera controls
    loadTestMap is a placeholder for loading maps
    render loops the simulator
*/

function Simulator(domElement, master, mapId)
{
    this.robots = [];
    this.robot = null;
    this.domElement = domElement;
    this.saveMan = new SaveManager();
    this.master = master||(new MasterObject()); // for interaction with VM
    this.physicsObjects = new PhysicsObjectManager();
    this.version = 0;
    this.targetFrame = -1;
    this.currentFrame = 0;

    this.mouseEvent = function() {};

    this.width = domElement.offsetWidth;
    this.height = domElement.offsetHeight;

    this.sprite = [];

    this.loadMap(mapId||"testMap");

    var self = this;

    window.onresize = function(event)
    {
        self.width = self.domElement.offsetWidth;
        self.height = self.domElement.offsetHeight;
        self.renderer.setSize(self.width, self.height);
        self.camera.aspect = self.width/self.height;
        self.camera.width = self.width;
        self.camera.height = self.height;
        self.camera.updateProjectionMatrix();
    };
}

// creates a mesh at location, for testing purposes
// if box, args are width, height, depth
// if cylinder, arg3 is ignored, 1 is radius, 2 is height
Simulator.prototype.setTestSprite = function(x, y, z, index, color, type, arg1, arg2, arg3, quat)
{
    if(this.sprite[index])
    {
        this.removeTestSprite(index);
    }
    this.sprite[index] = this.createTestSprite(color, type, arg1, arg2, arg3);
    this.sprite[index].position.set(x, y, z);
    if(quat)
        this.sprite[index].quaternion.set(quat.x, quat.y, quat.z, quat.w);
    this.sprite[index].opacity = 0.2;
};

Simulator.prototype.createTestSprite = function(color, type, arg1, arg2, arg3)
{
    printOut(type);
    switch(type)
    {
        case "BOX":
            return createBoxMesh(arg1, arg2, arg3, color, this.scene);
        case "CYLINDER":
            return createCylMesh(arg1, arg2, color, this.scene);
        default:
            printOut(type);
    }
};

Simulator.prototype.removeTestSprite = function(index)
{
    if(this.sprite[index])
    {
        this.scene.remove(this.sprite[index]);
        this.sprite[index] = null;
    }
};

Simulator.prototype.initScene = function(width, height, fov, drawDistance, cameraPosition, backgroundColor)
{
    this.scene = new THREE.Scene();

    this.ambientLight = new THREE.AmbientLight(0x090909, 0.1);
        this.scene.add(this.ambientLight);

    this.directionalLight = new THREE.DirectionalLight(0xffffff, 0.9);
        this.directionalLight.position.set(1,1,1).normalize();
        this.scene.add(this.directionalLight);
        printOut(this.directionalLight.intensity);

    this.camera = new THREE.PerspectiveCamera(fov, width/height, 1, drawDistance);
        this.camera.width = width;
        this.camera.height = height;

    this.centralPosition = this.scene.position;

    this.cameraController = new PovCamera(this.centralPosition, this.camera,
                                          cameraPosition[0], cameraPosition[1], cameraPosition[2]);
        this.cameraController.updatePosition();
        this.camera.lookAt(this.centralPosition);

    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setSize(width, height);
    this.renderer.setClearColor(backgroundColor, 1);

    this.domElement.appendChild(this.renderer.domElement);
};

Simulator.prototype.initPhysics = function(gravity)
{
    // the following lines were borrowed from a tutorial
    // simulator team is not sure how it works
    var collisionConfiguration = new Ammo.btDefaultCollisionConfiguration();
    var dispatcher = new Ammo.btCollisionDispatcher(collisionConfiguration);
    var overlappingPairCache = new Ammo.btDbvtBroadphase();
    var solver = new Ammo.btSequentialImpulseConstraintSolver();
    this.physicsWorld = new Ammo.btDiscreteDynamicsWorld(dispatcher,
                                                         overlappingPairCache,
                                                         solver,
                                                         collisionConfiguration);
    this.physicsWorld.setGravity(new Ammo.btVector3(gravity[0], gravity[1], gravity[2]));
};

Simulator.prototype.initMouseCameraControls = function()
{
    var self = this;
    self.domElement.onmousedown = function(evt)
    {
        var mouseX = evt.pageX;
        var mouseY = evt.pageY;
        self.mouseEvent(evt.pageX - self.domElement.offsetLeft, evt.pageY - self.domElement.offsetTop);

        self.domElement.onmousemove = function(evt)
        {
            self.cameraController.moveX((evt.pageX - mouseX)*0.001);
            self.cameraController.moveY((evt.pageY - mouseY)*0.001);
            mouseX = evt.pageX;
            mouseY = evt.pageY;

            self.mouseEvent(evt.pageX - self.domElement.offsetLeft, evt.pageY - self.domElement.offsetTop);
        };

        self.domElement.onmouseout = function(evt)
        {
            self.domElement.onmousemove = null;
        };
    };

    self.domElement.onmouseup = function(evt)
    {
           self.domElement.onmousemove = null;
           self.domElement.onmouseout = null;
    };

    self.domElement.addEventListener("DOMMouseScroll", function(evt)
    {
        self.cameraController.moveZ(-evt.wheelDelta||-evt.detail);
    });
};

Simulator.prototype.loadMap = function(mapId)
{
    var fReader = new FileManager();

    var map = fReader.getMapJson(mapId);
    var cam = map.camera;

    var temp = null;

    this.initScene(this.width,this.height,cam.fov,cam.drawDistance,cam.position, parseInt(map.backgroundColor, 16));
    this.initPhysics(map.gravity);
    this.initMouseCameraControls();

    for(var i = 0; i < map.objects.length; i++)
    {
        temp = map.objects[i];

        if(temp.type == "BOX")
        {
            this.createBox(temp.width, temp.height, temp.depth, temp.mass, parseInt(temp.color, 16),
                           temp.position[0], temp.position[1], temp.position[2]).setFriction(temp.friction);
        }
        // TODO(ericnguyen): do cylinders later
    }


    for(i = 0; i < map.robot.length; i++)
    {
        temp = map.robot[i];

        this.loadRobot(fReader.getRobotJson(temp.id), temp.position);
    }

    this.robot = this.robots[0];
};

Simulator.prototype.loadRobot = function(robotJson, position)
{
    var data = robotJson;
    var robot = new Robot(this, this.master);
        robot.initChassi();
        // TODO(ericnguyen): modify master objects to allow working different robots
        // TODO(ericnguyen): the robot constructor takes 3 arguments, so this
        //                   implicitly sets the "id" argument to undefined
    var temp = null;

    for(var i = 0; i < data.chassi.length; i++)
    {
        temp = data.chassi[i];

        if(temp.type == "BOX")
        {
            robot.addShape(new Ammo.btBoxShape(new Ammo.btVector3(temp.size[0]/2, temp.size[1]/2, temp.size[2]/2)),
                           new Ammo.btTransform(new Ammo.btQuaternion(temp.rot[0], temp.rot[1], temp.rot[2]),
                                                new Ammo.btVector3(temp.offset[0], temp.offset[1], temp.offset[2])),
                           temp.mass,
                           createBoxMesh(temp.size[0], temp.size[1], temp.size[2], parseInt(temp.color, 16), this.scene));

        }
    }

    robot.finishChassi(position[0],position[1],position[2]);

    for(i = 0; i < data.motors.length; i++)
    {
        temp = data.motors[i];
        var motor = new Motor(this, temp.wheelRad, temp.wheelHeight, temp.wheelMass, parseInt(temp.wheelColor, 16),
                              temp.offset[0] + position[0], temp.offset[1] + position[1], temp.offset[2] + position[2]);

        robot.addMotor(i, motor, motor.wheel,
                       new Ammo.btVector3(temp.chassiLoc[0], temp.chassiLoc[1], temp.chassiLoc[2]),
                       new Ammo.btVector3(temp.objectLoc[0], temp.objectLoc[1], temp.objectLoc[2]),
                       new Ammo.btVector3(temp.chassiAxis[0], temp.chassiAxis[1], temp.chassiAxis[2]),
                       new Ammo.btVector3(temp.objectAxis[0], temp.objectAxis[1], temp.objectAxis[2]));
    }

    for(i = 0; i < data.sensors.length; i++)
    {
        temp = data.sensors[i];

        if(temp.type == "RF")
        {
            var rf = new Rangefinder(this, temp.size[0], temp.size[1], temp.size[2],
                                    temp.mass, temp.offset[0], temp.offset[1], temp.offset[2],
                                    temp.maxDistance);

            robot.addSensor(i, rf, rf.physicsObject,
                            new Ammo.btVector3(temp.chassiLoc[0], temp.chassiLoc[1], temp.chassiLoc[2]),
                            new Ammo.btVector3(temp.objectLoc[0], temp.objectLoc[1], temp.objectLoc[2]),
                            new Ammo.btVector3(temp.chassiAxis[0], temp.chassiAxis[1], temp.chassiAxis[2]),
                            new Ammo.btVector3(temp.objectAxis[0], temp.objectAxis[1], temp.objectAxis[2]));
        }

        // TODO(ericnguyen): allow loading of other types of sensors
    }

    this.robots.push(robot);
};

Simulator.prototype.render = function()
{
    if(this.saveMan.shouldSave(this.currentFrame))
        this.saveMan.storeState(this.physicsObjects.getState(this)); // deals with saving

    if(this.master.isPaused);
    else if(this.master.version < this.version) // Checks for reversing
    {
        // TODO(ericnguyen): refactor this
        this.saveMan.runVersion(this, this.master.version);
        this.targetFrame = this.master.frame;
        this.targetFrame = -1;
        this.master.version = this.version;
    }
    else if(this.currentFrame < this.targetFrame) // refines reversing
    {
        this.saveMan.runFrame(this, this.targetFrame);
        this.targetFrame = -1;
    }
    else
    {
        // if the master's version was updated, read motor ports
        if(this.master.version > this.version)
        {
            this.robot.updateMotors();
            this.version = this.master.version;
        }
        else
        {
            this.version += 1;
            this.master.version = this.version;
        }

        for(var botIndex in this.robots)
        {
            this.robots[botIndex].simulate(); // check version, update motors as necessary, write new sensor to master
            this.robots[botIndex].updateSensors(); // writes sensory data to master object
        }

        this.physicsWorld.stepSimulation( 1 / 60, 5 ); // tells ammo.js to apply physics
        this.physicsObjects.render(); // update meshes of moving objects
        this.renderer.render(this.scene, this.camera); // tells three.js to render scene
        this.currentFrame += 1; // update frame
        this.version = this.master.version; // syncs versions
        this.master.frame = this.currentFrame; //synchs frames
    }

    requestAnimationFrame(this.render.bind(this)); // repeat the render function
};

Simulator.prototype.createBox = function(width, height, depth, mass, color, iniX, iniY, iniZ)
{
    var box = createBoxPhysics(width, height, depth, mass, iniX, iniY, iniZ, this.physicsWorld);
    var mesh = createBoxMesh(width, height, depth, color, this.scene);
    box.mesh = mesh;
    mesh.position.set(iniX, iniY, iniZ);

    // transforming 0 mass object glitches physics
    if(mass > 0)
        this.physicsObjects.push(box);

    return box;
};

Simulator.prototype.createCylinder = function(radius, height, mass, color, iniX, iniY, iniZ)
{
    var cyl = createCylinderPhysics(radius, height, mass, iniX, iniY, iniZ, this.physicsWorld);
    var mesh = createCylMesh(radius, height, color, this.scene);

    cyl.mesh = mesh;
    mesh.position.set(iniX, iniY, iniZ);

    // transforming 0 mass object glitches physics
    if(mass > 0)
        this.physicsObjects.push(cyl);

    return cyl;
};

Simulator.prototype.removeObject = function(object)
{
    this.physicsObjects.remove(object);
    this.physicsWorld.removeRigidBody(object);
    this.scene.remove(object.mesh);
};
