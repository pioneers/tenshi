/*
Manages saved states
    shouldSave returns if the current frame should be saved:
        ie, it hasn't been stored, and is not being replayed
    storeState saves the state, takes state given by physicsObjectManager
    tenCheck is internal and should not be called externally;
        rearranges saved states, so always by order of 10
    runFrame runs the frame number,
        looks inside storage for earliest frame that is under frame number,
        runs until it hits frame number
    printVersions prints data structure of saveQueue
        useful for debugging
*/
function SaveManager()
{
    this.saveQueues = [[]];
    this.savedFrames = 0;
}

SaveManager.prototype.printVersions = function()
{
    var temp_str;
    for(i = 0; i < this.saveQueues.length; i++)
    {
        temp_str = i + " :: ";
        for(j = 0; j < this.saveQueues[i].length; j++)
        {
            temp_str += this.saveQueues[i][j].version + " | ";
        }
        console.log(temp_str);
    }
};

SaveManager.prototype.shouldSave = function(frame)
{
    return ((frame%10 === 0) && frame >= this.savedFrames);
};

SaveManager.prototype.storeState = function(state)
{
    this.savedFrames += 10;
    this.saveQueues[0].push(state);
    if(this.saveQueues[0].length > 10) this.tenCheck();
};

SaveManager.prototype.tenCheck = function()
{
    for(var i = 0; i < this.saveQueues.length; i++)
    {
        if(this.saveQueues[i].length > 10)
        {
            if(i + 1 >= this.saveQueues.length)
            {
                this.saveQueues.push([]);
            }
            this.saveQueues[i + 1].push(this.saveQueues[i][0]);
            this.saveQueues[i] = [this.saveQueues[i][10]];
        }
        else break;
    }
};

SaveManager.prototype.runFrame = function(simulator, frame)
{
    for(i = 0; i < this.saveQueues.length && this.saveQueues[i][0].frame > frame; i++);

    var tempQ = this.saveQueues[i];
    var save = tempQ[0];

    for(i = 0; i < tempQ.length && tempQ[i].frame <= frame; i++)
    {
        save = tempQ[i];
    }

    simulator.physicsObjects.loadState(simulator, save);
    console.log(save.frame, frame);

    for(var curFrame = save.frame; curFrame < frame; curFrame++)
        simulator.physicsWorld.stepSimulation(1/60, 5);
};

SaveManager.prototype.runVersion = function(simulator, vrsn)
{
    for(i = 0; i < this.saveQueues.length && this.saveQueues[i][0].version > vrsn; i++);

    var tempQ = this.saveQueues[i];
    var save = tempQ[0];

    for(i = 0; i < tempQ.length && tempQ[i].version <= vrsn; i++)
    {
        save = tempQ[i];
    }

    simulator.physicsObjects.loadState(simulator, save);
};

exports.SaveManager = SaveManager;
