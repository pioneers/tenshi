/*/============================
Three.js wrappers
============================/*/

//initalizes the renderer, camera, etc
function initScene()
{
    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera(45, WIDTH/HEIGHT, 1, 1000);
        camera.position.set(0, 40, 200);
        camera.lookAt(scene.position);

    centralPosition = scene.position;

    cameraController = new PovCamera(camera, 30, 30, 30);
        cameraController.updatePosition();
        camera.lookAt(centralPosition);

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(WIDTH, HEIGHT);
    renderer.setClearColor(0xD4AF37, 1);

    document.getElementById("mainScreen").appendChild(renderer.domElement);
}

//creates a cylinder mesh
function createCylMesh(radius, height, mColor, dontAddScene)
{
    height = height||1;
    radius = radius||1;
    if(mColor == undefined)
        mColor = mColor||0xffffff;

    var geom = new THREE.CylinderGeometry(radius, radius, height, 30);
    var material = new THREE.MeshBasicMaterial({ color: mColor });
    var mesh = new THREE.Mesh(geom, material);

    mesh.receiveShadow = true;
    mesh.castShadow = true;

    if(!dontAddScene) scene.add(mesh);
    return mesh;

}

//creates a box mesh
function createBoxMesh(width, height, depth, mColor, dontAddScene)
{
    width = width||1;
    height = height||1;
    depth = depth||1;
    if(mColor == undefined)
        mColor = mColor||0xffffff;

    var geom = new THREE.CubeGeometry(width, height, depth);
    var material = new THREE.MeshBasicMaterial({ color: mColor });
    var mesh = new THREE.Mesh(geom, material);

    mesh.receiveShadow = true;
    mesh.castShadow = true;

    if(!dontAddScene) scene.add(mesh);
    return mesh;
}

/*Takes a mesh, and adds x,y, and z lines to allow easier understanding
    mesh is initial mesh
    len is length of each line
    iniX...iniZ is initial position of the mesh
*/

function makeMappedMesh(mesh, len, iniX, iniY, iniZ)
{
    var group = new THREE.Object3D();

    axisX = createBoxMesh(len, 0.5, 0.5, 0xff0000, 1);
    axisY = createBoxMesh(0.5, len, 0.5, 0x00ff00, 1);
    axisZ = createBoxMesh(0.5, 0.5, len, 0x0000ff, 1);

    axisX.position.x = len/2;
    axisY.position.y = len/2;
    axisZ.position.z = len/2;

    group.add(mesh);
    group.add(axisX);
    group.add(axisY);
    group.add(axisZ);

    scene.add(group);

    group.original = mesh;
    group.material = mesh.material;
    return group;
}

//Removes lines from a mapped mesh
function makeUnMappedMesh(mesh)
{
    return mesh.original;
}