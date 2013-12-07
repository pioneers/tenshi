/*
creates a simulator, with canvas size width and height, in domElement
    initScene initalizes THREE renderer
    initPhysics intializes AMMO physics world
    initMouseCameraControls binds mouse events to camera controls
    loadTestMap is a placeholder for loading maps
    render loops the simulator
*/

function Simulator(width, height, domElement)
{
    this.domElement = domElement;
    this.saveMan = new SaveManager();
    this.master = new MasterObject(); // for interaction with VM
    this.physicsObjects = new PhysicsObjectManager();
    this.version = 0;
    this.targetFrame = -1;
    this.currentFrame = 0;

    this.initScene(width, height);
    this.initPhysics();
    this.initMouseCameraControls();
    this.loadTestMap();

    this.robot = createCar3(this); // createCar3 is placeholder for loading car model
}

Simulator.prototype.initScene = function(width, height)
{
    this.scene = new THREE.Scene();

    // 45 is fov, 1000 is draw distance
    this.camera = new THREE.PerspectiveCamera(45, width/height, 1, 1000);
        this.camera.position.set(0, 40, 200);
        this.camera.lookAt(this.scene.position);

    this.centralPosition = this.scene.position;

    this.cameraController = new PovCamera(this.centralPosition, this.camera, 30, 30, 30);
        this.cameraController.updatePosition();
        this.camera.lookAt(this.centralPosition);

    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setSize(width, height);
    this.renderer.setClearColor(0xD4AF37, 1);

    this.domElement.appendChild(this.renderer.domElement);
};

Simulator.prototype.initPhysics = function()
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
    this.physicsWorld.setGravity(new Ammo.btVector3(0, -50, 0));
};

Simulator.prototype.initMouseCameraControls = function()
{
	var self = this;
    self.domElement.onmousedown = function(evt)
    {
        mouseX = evt.pageX;
        mouseY = evt.pageY;
        self.domElement.onmousemove = function(evt)
        {
            self.cameraController.moveX((evt.pageX - mouseX)*0.001);
            self.cameraController.moveY((evt.pageY - mouseY)*0.001);
            mouseX = evt.pageX;
            mouseY = evt.pageY;
        };
    };

    self.domElement.onmouseup = function(evt)
    {
           self.domElement.onmousemove = null;
    };

    self.domElement.addEventListener("DOMMouseScroll", function(evt)
    {
        self.cameraController.moveZ(-evt.wheelDelta||-evt.detail);
    });
};

// TODO(ericnguyen): create map format to read/save
Simulator.prototype.loadTestMap = function()
{
    this.createBox(10, 800, 600, 0, 0x0f0f0f, -300, 400 - 40, 0);
    this.createBox(10, 800, 600, 0, 0x0f0f0f, 300, 400 - 40, 0);
    this.createBox(600, 800, 10, 0, 0x0f0f0f, 0, 400 - 40, 300);
    this.createBox(600, 800, 10, 0, 0x0f0f0f, 0, 400 - 40, -300);
    this.createBox(1200, 10, 1200, 0, 0xffffff, 0, 800 - 40, 0);
    this.createBox(1200, 10, 1200, 0, 0x0000ff, 0, -40, 0).setFriction(1);

    this.createBox(3, 3, 3, 27, 0x990000, 0, 50, 0);
    this.createBox(3, 3, 3, 27, 0x990000, 0, 50, 0);
    this.createBox(3, 3, 3, 27, 0x990000, 0, 50, 0);
    this.createBox(3, 3, 3, 27, 0x990000, 0, 50, 0);
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

        this.robot.simulate(); // check version, update motors as necessary, write new sensor to master
        this.robot.updateSensors(); // writes sensory data to master object
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
    var mesh = makeMappedMesh(createCylMesh(radius, height, color, this.scene), 15, iniX, iniY, iniZ, this.scene);

    cyl.mesh = mesh;
    mesh.position.set(iniX, iniY, iniZ);

    // transforming 0 mass object glitches physics
    if(mass > 0)
        this.physicsObjects.push(cyl);

    return cyl;
};