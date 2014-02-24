function MasterObject()
{
    this.motors = {};
    this.sensors = {};
    this.version = 0;
    this.isPaused = 0;
    this.frame = 0;
}

MasterObject.prototype.saveMotors = function()
{
    var motors = {};
    motors.version = this.version;

    for(var robot in this.motors)
    {
        motors[robot] = {};
        for(var motor in this.motors[robot])
        {
            motors[robot][motor] = this.motors[robot][motor];
        }
    }

    return motors;
};

MasterObject.prototype.loadMotors = function(state, vrsn)
{
    for(var robot in state)
    {
        for(var motor in state[robot])
        {
            this.motors[robot][motor] = state[robot][motor];
        }
    }

    this.version = vrsn;
};