/*
manages the camera similar to a first person pov
    updatePosition is automatically called
    moveX turns camera left and right
    moveY turns camera up and down
    moveZ moves camera forward and back
    elevate changes y coord
    strafe moves in perpendicular position on xz plane
    forward moves forward in xz plane
*/
function PovCamera(centralPosition, camera, iniX, iniY, iniZ)
{

    this.camera = camera;
    this.camera.position.x = iniX;
    this.camera.position.y = iniY;
    this.camera.position.z = iniZ;
    this.radius = 100;
    // how far away focus point is

    this.centralPosition = centralPosition;

    var y = this.centralPosition.y - iniY;
    var tempRadius = Math.sqrt(this.radius*this.radius - y*y);

    this.theta = Math.acos((this.centralPosition.x - iniX)/tempRadius);
    this.phi = Math.asin((this.centralPosition.y - iniY)/this.radius);
    // angle measure how much camera is tilted
}

PovCamera.prototype.updatePosition = function()
{
    this.centralPosition.y = this.radius*Math.sin(this.phi) + this.camera.position.y;
    // moves the position to look at around a sphere surrounding the camera
    var y = this.centralPosition.y - this.camera.position.y;

    var tempRadius = Math.sqrt(this.radius*this.radius - y*y);

    this.centralPosition.x = tempRadius*Math.cos(this.theta) + this.camera.position.x;
    this.centralPosition.z = tempRadius*Math.sin(this.theta) + this.camera.position.z;
};

PovCamera.prototype.moveX = function(speed)
{
    this.theta -= speed;

    this.updatePosition();
    this.camera.lookAt(this.centralPosition);
};

PovCamera.prototype.moveY = function(speed)
{
    this.phi += speed;

    this.updatePosition();
    this.camera.lookAt(this.centralPosition);
};

// Finds unit vector in direction camera is facing
// Adds speed*vector to both focus point and camera
PovCamera.prototype.moveZ = function(speed)
{
    var x = Math.cos(this.theta),
        y = Math.sin(this.phi),
        z = Math.sin(this.theta);

    this.camera.position.x += speed*x;
    this.camera.position.y += speed*y;
    this.camera.position.z += speed*z;

    this.centralPosition.x += speed*x;
    this.centralPosition.y += speed*y;
    this.centralPosition.z += speed*z;

    this.updatePosition();
};

PovCamera.prototype.elevate = function(speed)
{
    this.camera.position.y += speed;
    this.centralPosition.y += speed;
};

// Finds unit vector in direction on xz plane
// Adds speed*vector to both focus point and camera
PovCamera.prototype.forward = function(speed)
{
    var x = Math.cos(this.theta),
        z = Math.sin(this.theta);

    this.camera.position.x += speed*x;
    this.centralPosition.x += speed*x;

    this.camera.position.z += speed*z;
    this.centralPosition.z += speed*z;

    this.updatePosition();
};

// Finds perpendicular unit vector in direction on xz plane
// Adds speed*vector to both focus point and camera
PovCamera.prototype.strafe = function(speed)
{
    var x = Math.cos(this.theta + Math.PI/2),
        z = Math.sin(this.theta + Math.PI/2);

    this.camera.position.x += speed*x;
    this.centralPosition.x += speed*x;

    this.camera.position.z += speed*z;
    this.centralPosition.z += speed*z;

    this.updatePosition();
};