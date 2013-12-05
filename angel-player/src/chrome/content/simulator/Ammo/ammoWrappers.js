/*/============================
Ammo.js wrappers
============================/*/

// initalizes physics, sets gravity to 0, -10, 0 (-m/s^2)
function initPhysics()
{
    var collisionConfiguration = new Ammo.btDefaultCollisionConfiguration();
    var dispatcher = new Ammo.btCollisionDispatcher(collisionConfiguration);
    var overlappingPairCache = new Ammo.btDbvtBroadphase();
    var solver = new Ammo.btSequentialImpulseConstraintSolver();
    scene.world = new Ammo.btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    scene.world.setGravity(new Ammo.btVector3(0, -10, 0));
}

// creates a box physic, returns box
function createBoxPhysics(width, height, depth, mass, iniX, iniY, iniZ)
{
    var nMass = mass;
    var startTransform = new Ammo.btTransform();
    startTransform.setIdentity();
    startTransform.setOrigin(new Ammo.btVector3(iniX, iniY, iniZ)); // Set initial position

    var localInertia = new Ammo.btVector3(0, 0, 0);

    var boxShape = new Ammo.btBoxShape(new Ammo.btVector3(width/2, height/2, depth/2));
    boxShape.calculateLocalInertia(nMass, localInertia);

    var motionState = new Ammo.btDefaultMotionState(startTransform);
    var rbInfo = new Ammo.btRigidBodyConstructionInfo(mass, motionState, boxShape, localInertia);
    var boxAmmo = new Ammo.btRigidBody(rbInfo);
    scene.world.addRigidBody(boxAmmo);

    boxAmmo.getCollisionShape().parent = boxAmmo;

    return boxAmmo;
}

// creates a cylinder physics, returns it
function createCylinderPhysics(radius, height, mass, iniX, iniY, iniZ)
{
    var nMass = mass;
    var startTransform = new Ammo.btTransform();
    startTransform.setIdentity();
    startTransform.setOrigin(new Ammo.btVector3(iniX, iniY, iniZ)); // Set initial position

    var localInertia = new Ammo.btVector3(0, 0, 0);
    //localInertia is initialized to zero because physics fills it in later

    var cylShape = new Ammo.btCylinderShape(new Ammo.btVector3(radius, height/2, radius));
    cylShape.calculateLocalInertia(nMass, localInertia);

    var motionState = new Ammo.btDefaultMotionState(startTransform);
    var rbInfo = new Ammo.btRigidBodyConstructionInfo(nMass, motionState, cylShape, localInertia);
    var cylAmmo = new Ammo.btRigidBody(rbInfo);
    scene.world.addRigidBody(cylAmmo);

    return cylAmmo;
}

// creates a cylinder physic attached to a mesh, specified by size
function createCylinder(radius, height, mass, color, iniX, iniY, iniZ)
{
    cyl = createCylinderPhysics(radius, height, mass, iniX, iniY, iniZ);
    mesh = makeMappedMesh(createCylMesh(radius, height, color, 1), 15, iniX, iniY, iniZ);

    cyl.mesh = mesh;
    mesh.position.set(iniX, iniY, iniZ);

    physicsObjects.push(cyl);

    return cyl;

}

// creates a box physic attached to a mesh, specified by size
function createBox(width, height, depth, mass, color, iniX, iniY, iniZ, dontAddPhysics)
{
    box = createBoxPhysics(width, height, depth, mass, iniX, iniY, iniZ);
    mesh = makeMappedMesh(createBoxMesh(width, height, depth, color), 5, iniX, iniY, iniZ);
    box.mesh = mesh;
    mesh.position.set(iniX, iniY, iniZ);

    if(!dontAddPhysics) physicsObjects.push(box);

    return box;
}