function MasterObject()
{
    this.robotMotors = {}; // stores motor vals of all robots
    this.robotSensors = {}; // stores sensor vals of all robots
    this.version = 0;
    this.isPaused = 0;
    this.frame = 0;
}

MasterObject.prototype.saveMotors = function()
{
    var robotMotors = {};
    robotMotors.version = this.version;

    for(var robot in this.robotMotors)
    {
        robotMotors[robot] = {};
        for(var motor in this.robotMotors[robot])
        {
            robotMotors[robot][motor] = this.robotMotors[robot][motor];
        }
    }

    return robotMotors;
};

MasterObject.prototype.loadMotors = function(state, vrsn)
{
    for(var robot in state)
    {
        for(var motor in state[robot])
        {
            this.robotMotors[robot][motor] = state[robot][motor];
        }
    }

    this.version = vrsn;
};