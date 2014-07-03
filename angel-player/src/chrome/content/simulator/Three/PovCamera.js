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

PovCamera.prototype.calculatePlanes = function(near, fov, width, height)
{
    var vx = this.centralPosition.x - this.camera.position.x,
        vy = this.centralPosition.y - this.camera.position.y,
        vz = this.centralPosition.z - this.camera.position.z;

    // scales vectors to be near plane
        vx *= (near/100);
        vy *= (near/100);
        vz *= (near/100);

    // [a, b, c, d] in form ax + by + cz = d
    this.plane = [vx, vy, vz, (vx*(this.centralPosition.x - vx) +
                               vy*(this.centralPosition.y - vy) +
                               vz*(this.centralPosition.z - vz))];

    // finds normal on xz plane
    var tempx = vz,
        tempz = -vx,
        mag = Math.sqrt(tempz*tempz + tempx*tempx);

    // normalizes the vector
    this.xvector = [tempx/mag, 0, tempz/mag];

    // cross perpendicular and horizontal vector yields vertical vector
    this.yvector = [(this.plane[1]*this.xvector[2]),
                    this.plane[2]*this.xvector[0] - this.plane[0]*this.xvector[2],
                    -(this.plane[1]*this.xvector[0])];

    // normalizes vector
    mag = Math.sqrt(this.yvector[0]*this.yvector[0] +
                    this.yvector[1]*this.yvector[1] +
                    this.yvector[2]*this.yvector[2]);

    this.yvector = [this.yvector[0]/mag,
                    this.yvector[1]/mag,
                    this.yvector[2]/mag];

    // fov is y direction, calculates how much unit vector
    // takes up half the near plane
    // 0.83 is some random constant that makes it all accurate, discovered empirically
    // TODO(ericnguyen): find out why 0.83 makes everything work right
    this.heightFactor = 0.83*Math.tan(fov*Math.PI/180)/height;
    this.widthFactor = this.heightFactor;
};

// returns point in real space of point on projected plane
PovCamera.prototype.calculatePoint = function(width, height)
{
    this.calculatePlanes(1, this.camera.fov, this.camera.width, this.camera.height);

    // takes focus point 1 unit away from camera in facing direction
    // adds calculated xvector*width + yvector*height
    // returns point in space, for raytracing
    var  calculated =
         [this.camera.position.x + this.plane[0] -
            width*this.xvector[0]*this.widthFactor -
            height*this.yvector[0]*this.heightFactor,
          this.camera.position.y + this.plane[1] -
            height*this.yvector[1]*this.heightFactor,
          this.camera.position.z + this.plane[2] -
            width*this.xvector[2]*this.widthFactor -
            height*this.yvector[2]*this.heightFactor];

    return calculated;
};

PovCamera.prototype.updatePosition = function()
{
    this.centralPosition.y = this.radius*Math.sin(this.phi) + this.camera.position.y;
    // moves the position to look at around a sphere surrounding the camera
    var y = this.centralPosition.y - this.camera.position.y;

    var tempRadius = Math.sqrt(this.radius*this.radius - y*y);

    this.centralPosition.x = tempRadius*Math.cos(this.theta) + this.camera.position.x;
    this.centralPosition.z = tempRadius*Math.sin(this.theta) + this.camera.position.z;

    this.calculatePlanes(1, this.camera.fov, this.camera.width, this.camera.height);
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

exports.PovCamera = PovCamera;
