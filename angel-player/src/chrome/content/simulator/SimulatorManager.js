function SimulatorManager()
{

}

SimulatorManager.prototype.runSimulation = function(masterObj, mapId, width, height, element)
{
    masterObj = masterObj||(new MasterObject());
    mapId = mapId||'default';

    return new Simulator(width, height, element, masterObj);
};