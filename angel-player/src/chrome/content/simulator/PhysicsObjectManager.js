/*
Manages physics objects in the world
    push adds an object to the list, assigns unique id to each object
    render adjusts the mesh of each object to match its physic object
    getState writes the state to a data structure, returns it
    runState reads from that state data structure, sets all object to such state, sets frame to that state

    todo:
        make getState read robot port values, make runState set such things
*/
function PhysicsObjectManager()
{
    this.array = []; //objects currently in world

    this.objects = {}; //objects that existed at some point; to allow access after removing

    this.ids = 0;
}

PhysicsObjectManager.prototype.length = function()
{
    return this.array.length;
};

PhysicsObjectManager.prototype.push = function(obj)
{
    this.array.push(obj);

    obj.physicsId = this.ids;
    this.ids += 1;

    this.objects[obj.physicsId] = obj;
};

PhysicsObjectManager.prototype.iterator = function()
{
    return Iterator(this.array);
};

PhysicsObjectManager.prototype.render = function()
{
    var origin, rotation, transform = new Ammo.btTransform();

    var arr = this.array;
    var it = Iterator(arr, true);
    for(key in it)
    {
        arr[key].getMotionState().getWorldTransform( transform ); // Retrieve box position & rotation from Ammo

        // Update position
        origin = transform.getOrigin();
        arr[key].mesh.position.x = origin.x();
        arr[key].mesh.position.y = origin.y();
        arr[key].mesh.position.z = origin.z();
         
        // Update rotation
        rotation = transform.getRotation();
        arr[key].mesh.quaternion.x = rotation.x();
        arr[key].mesh.quaternion.y = rotation.y();
        arr[key].mesh.quaternion.z = rotation.z();
        arr[key].mesh.quaternion.w = rotation.w();
    }
};

PhysicsObjectManager.prototype.getState = function()
{
    var tempState = [],
        arr = this.array,
        it = Iterator(arr, true),
        obj,
        linVel,
        angVel,
        tr,
        temp_tr;
    for(key in it)
    {
        tr = new Ammo.btTransform();

        obj = arr[key];

        linVel = new Ammo.btVector3(obj.getLinearVelocity().x(), obj.getLinearVelocity().y(), obj.getLinearVelocity().z());
        angVel = new Ammo.btVector3(obj.getAngularVelocity().x(), obj.getAngularVelocity().y(), obj.getAngularVelocity().z());
        temp_tr = obj.getCenterOfMassTransform();
        tr.setOrigin(new Ammo.btVector3(temp_tr.getOrigin().x(), temp_tr.getOrigin().y(), temp_tr.getOrigin().z()));
        tr.setRotation(new Ammo.btQuaternion(temp_tr.getRotation().x(), temp_tr.getRotation().y(), temp_tr.getRotation().z(), temp_tr.getRotation().w()));

        tempState.push([obj.physicsId, tr, linVel, angVel]);
    }

    tempState.frame = FRAME;
    tempState.version = version;
    tempState.motors = master.saveMotors();

    return tempState;
};

PhysicsObjectManager.prototype.loadState = function(state)
{
    FRAME = state.frame;
    version = state.version;

    for(var i = 0; i < state.length; i++)
    {
        this.array[i].clearForces();
        this.array[i].setActivationState(1);
        this.array[i].setWorldTransform(state[i][1]);
        this.array[i].setLinearVelocity(state[i][2]);
        this.array[i].setAngularVelocity(state[i][3]);
    }

    master.loadMotors(state.motors);

    this.render();
};