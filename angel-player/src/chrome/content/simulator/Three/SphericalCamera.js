/*
manages the camera in a sphere around a central position
    updatePosition is automatically called
    moveX moves camera around in a circle on the xz plane (at level y) (left and right)
    moveY moves camera around up and down the sphere
    moveZ moves camera radius in and out
*/
function SphericalCamera(camera, radius, theta, phi)
{
    this.camera = camera;
    this.radius = radius;
    this.theta = theta;
    this.phi = phi;
}

SphericalCamera.prototype.updatePosition = function()
{
    //moves camera in a sphere around central position, calculates angles to do so
    this.camera.position.y = this.radius*Math.sin(this.phi) + centralPosition.y;

    var y = this.camera.position.y - centralPosition.y;

    var tempRadius = Math.sqrt(this.radius*this.radius - y*y);

    camera.position.x = tempRadius*Math.cos(this.theta) + centralPosition.x;
    camera.position.z = tempRadius*Math.sin(this.theta) + centralPosition.z;

    this.camera.lookAt(centralPosition);
};

SphericalCamera.prototype.moveX = function(speed)
{
    this.theta += speed;

    this.updatePosition();
};

SphericalCamera.prototype.moveY = function(speed)
{
    this.phi += speed;

    this.updatePosition();
};

SphericalCamera.prototype.moveZ = function(speed)
{
    this.radius += speed;

    this.updatePosition();
};