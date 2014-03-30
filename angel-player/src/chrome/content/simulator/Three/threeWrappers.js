/*/============================
Three.js wrappers
============================/*/

// creates a cylinder mesh
function createCylMesh(radius, height, mColor, scene)
{
    height = height||1;
    radius = radius||1;
    mColor = mColor === undefined ? 0xffffff : mColor;

    var geom = new THREE.CylinderGeometry(radius, radius, height, 30);
    var material = new THREE.MeshLambertMaterial({ color: mColor });
    var mesh = new THREE.Mesh(geom, material);

    mesh.receiveShadow = true;
    mesh.castShadow = true;
    mesh.type = "CYLINDER";

    mesh.radius = radius;
    mesh.height = height;

    if(scene) 
        scene.add(mesh);
    return mesh;

}

// creates a box mesh
function createBoxMesh(width, height, depth, mColor, scene)
{
    width = width||1;
    height = height||1;
    depth = depth||1;
    mColor = mColor === undefined ? 0xffffff : mColor;

    var geom = new THREE.CubeGeometry(width, height, depth);
    var material = new THREE.MeshLambertMaterial({ color: mColor });
    var mesh = new THREE.Mesh(geom, material);

    mesh.receiveShadow = true;
    mesh.castShadow = true;
    mesh.type = "BOX";

    mesh.width = width;
    mesh.height = height;
    mesh.depth = depth;

    if(scene)
        scene.add(mesh);
    return mesh;
}

/*Takes a mesh, and adds x,y, and z lines to allow easier understanding
    mesh is initial mesh
    len is length of each line
    iniX...iniZ is initial position of the mesh
*/

function makeMappedMesh(mesh, len, iniX, iniY, iniZ, scene)
{
    var group = new THREE.Object3D();

    axisX = createBoxMesh(len, 0.5, 0.5, 0xff0000, scene);
    axisY = createBoxMesh(0.5, len, 0.5, 0x00ff00, scene);
    axisZ = createBoxMesh(0.5, 0.5, len, 0x0000ff, scene);

    axisX.position.x = len/2;
    axisY.position.y = len/2;
    axisZ.position.z = len/2;

    group.add(mesh);
    group.add(axisX);
    group.add(axisY);
    group.add(axisZ);

    if(scene)
        scene.add(group);

    group.original = mesh;
    group.material = mesh.material;
    return group;
}

// Removes lines from a mapped mesh
function makeUnMappedMesh(mesh)
{
    return mesh.original;
}