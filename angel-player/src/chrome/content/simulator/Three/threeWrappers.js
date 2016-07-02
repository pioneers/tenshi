// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

/*/============================
Three.js wrappers
============================/*/

const window = require('tenshi/common/window')();
let {THREE} = window;

// creates a cylinder mesh
function createCylMesh(radius, height, mColor, scene, op)
{
    height = height||1;
    radius = radius||1;
    mColor = mColor === undefined ? 0xffffff : mColor;

    var geom = new THREE.CylinderGeometry(radius, radius, height, 30);
    var material = new THREE.MeshLambertMaterial({ color: mColor, transparent: true, opacity: op||1});
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
function createBoxMesh(width, height, depth, mColor, scene, op)
{
    width = width||1;
    height = height||1;
    depth = depth||1;
    mColor = mColor === undefined ? 0xffffff : mColor;

    var geom = new THREE.CubeGeometry(width, height, depth);
    var material = new THREE.MeshLambertMaterial({ color: mColor, transparent: true, opacity: op||1});
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

exports.createBoxMesh = createBoxMesh;
exports.createCylMesh = createCylMesh;
