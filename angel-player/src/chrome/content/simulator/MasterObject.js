function MasterObject()
{
    this.motors = {};
    this.sensors = {};
    this.version = 0;
    this.isPaused = 0;
}

MasterObject.prototype.saveMotors = function()
{
    var motors = {};
    motors.version = this.version;

    for(var motor in this.motors)
    {
        motors[motor] = this.motors[motor];
    }

    return motors;
};

MasterObject.prototype.loadMotors = function(state)
{
    for(var motor in state)
    {
        this.motors[motor] = state[motor];
    }

    this.version = state.version;
};