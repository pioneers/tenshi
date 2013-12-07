/*
manages the camera similar to a first person pov
    updatePosition is automatically called
    moveX turns camera left and right
    moveY turns camera up and down
    moveZ moves camera forward and back
*/
function PovCamera(centralPosition, camera, iniX, iniY, iniZ)
{

    this.camera = camera;
    this.camera.position.x = iniX;
    this.camera.position.y = iniY;
    this.camera.position.z = iniZ;
    this.radius = 100;
    this.fixedRadius = 100;
    // implementation requires changing radius and fixed one
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

PovCamera.prototype.moveZ = function(speed)
{
    // moves look-at position to position along line between it and camera,
    // sets camera to that position
    // sets look-at position back to 100 away from camera, in original direction
    var iniX = this.centralPosition.x;
    var iniY = this.centralPosition.y;
    var iniZ = this.centralPosition.z;

    this.radius += speed;
    this.updatePosition();

    this.camera.position.x -= iniX - this.centralPosition.x;
    this.camera.position.y -= iniY - this.centralPosition.y;
    this.camera.position.z -= iniZ - this.centralPosition.z;

    this.radius = this.fixedRadius;
    this.updatePosition();
};