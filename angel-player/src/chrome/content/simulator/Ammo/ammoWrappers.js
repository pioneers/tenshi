/*/============================
Ammo.js wrappers
============================/*/

// TODO(ericnguyen): adjust friction/damping to reflect reality

// creates a box physic, returns box
function createBoxPhysics(width, height, depth, mass, iniX, iniY, iniZ, physicsWorld)
{
    var nMass = mass;
    var startTransform = new Ammo.btTransform();
    startTransform.setIdentity();
    startTransform.setOrigin(new Ammo.btVector3(iniX, iniY, iniZ)); // Set initial position

    var localInertia = new Ammo.btVector3(0, 0, 0); // physics fills this later

    var boxShape = new Ammo.btBoxShape(new Ammo.btVector3(width/2, height/2, depth/2));
    boxShape.calculateLocalInertia(nMass, localInertia);

    var motionState = new Ammo.btDefaultMotionState(startTransform);
    var rbInfo = new Ammo.btRigidBodyConstructionInfo(mass, motionState, boxShape, localInertia);
        rbInfo.set_m_linearDamping(.5); // friction seems to be off, no damping = infinite sliding
        rbInfo.set_m_angularDamping(.2);
    var boxAmmo = new Ammo.btRigidBody(rbInfo);

    if(physicsWorld)
        physicsWorld.addRigidBody(boxAmmo);

    boxAmmo.getCollisionShape().parent = boxAmmo;

    return boxAmmo;
}

// creates a cylinder physics, returns it
function createCylinderPhysics(radius, height, mass, iniX, iniY, iniZ, physicsWorld)
{
    var nMass = mass;
    var startTransform = new Ammo.btTransform();
    startTransform.setIdentity();
    startTransform.setOrigin(new Ammo.btVector3(iniX, iniY, iniZ)); // Set initial position

    var localInertia = new Ammo.btVector3(0, 0, 0);

    var cylShape = new Ammo.btCylinderShape(new Ammo.btVector3(radius, height/2, radius));
    cylShape.calculateLocalInertia(nMass, localInertia);

    var motionState = new Ammo.btDefaultMotionState(startTransform);
    var rbInfo = new Ammo.btRigidBodyConstructionInfo(nMass, motionState, cylShape, localInertia);
        rbInfo.set_m_angularDamping(.5);
        rbInfo.set_m_linearDamping(.5);
    var cylAmmo = new Ammo.btRigidBody(rbInfo);

    if(physicsWorld)
        physicsWorld.addRigidBody(cylAmmo);

    return cylAmmo;
}