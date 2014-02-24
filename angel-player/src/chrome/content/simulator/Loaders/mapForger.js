//Unfinished, only pushed because Robert keeps asking me
function robotCreator()
{
  this.shapeDict = {};
    this.shapeDict[0] = Ammo.btBoxShape;
    this.shapeDict[1] = Ammo.btSphereShape;
    this.shapeDict[2] = Ammo.btCylinderShape;
  this.shapeArgDict = {};
    this.shapeArgDict[0] = Ammo.btVector3;
    this.shapeArgDict[1] = Ammo.btScaler;
    this.shapeArgDict[2] = Ammo.btVector3;
  this.meshDict = {};
    this.meshDict[0] = createBoxMesh;
    this.meshDict[2] = createCylMesh;
  this.sensorDict = {};
    this.sensorDict[0] = Rangefinder;
    this.sensorDict[1] = Infrared;
    this.sensorDict[2] = Touch;
}

// replace with json reader
robotCreator.prototype.getDataFromXml = function(xmlLink)
{
  var xmlDoc = loadXMLDoc(xmlLink);
  var data = {};
    data.shapes = [];
    data.motors = [];
    data.sensors = [];
  var shapesXML = xmlDoc.getElementsByTagName("shape");
  var motorsXML = xmlDoc.getElementsByTagName("motor");
  var sensorsXML = xmlDoc.getElementsByTagName("sensor");

  for(var i = 0; i < shapesXML.length; i++)
  {
    var shape = {};
    var temp = shapesXML[i].firstChild;
    var arr;

    shape.type = temp.firstChild.nodeValue;
    temp = temp.nextSibling;
    
    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    shape.typeArgArg = arr;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    shape.quaternion = arr;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    shape.vector = arr;
    temp = temp.nextSibling;

    shape.mass = temp.firstChild.nodeValue;
    temp = temp.nextSibling;

    shape.meshType = temp.firstChild.nodeValue;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    shape.meshArgs = arr;

    data.shapes.push(shape);
  }

  for(var i = 0; i < motorsXML.length; i++)
  {
    var motor = {};
    var temp = motorsXML[i].firstChild;
    var arr = [];

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    motor.args = arr;
    temp = temp.nextSibling;

    motor.port = temp.firstChild.nodeValue;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    motor.chassiLoc = arr;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    motor.motorLoc = arr;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    motor.chassiAxis = arr;
    temp = temp.nextSibling;

    arr = [];
    for(var j = 0; j < temp.childNodes.length; j++)
    {
      arr[j] = temp.childNodes[j].firstChild.nodeValue;
    }
    motor.motorAxis= arr;
    
    data.motors.push(motor);
  }

  return data;
}

robotCreator.prototype.loadRobot = function(data, iniX, iniY, iniZ)
{
    var robot = new Robot();

    robot.initChassi();
    for(var i = 0; i < data.shapes.length; i++)
    {
      var curShape = data.shapes[i];
      var shape = applyToConstructor(this.shapeDict[curShape.type],
                  applyToConstructor(this.shapeArgDict[curShape.type],
                                     curShape.typeArgArg));
      robot.addShape(shape,
                     applyToConstructor(Ammo.btTransform,
                          [applyToConstructor(Ammo.btQuaternion, curShape.quaternion),
                           applyToConstructor(Ammo.btVector3, curShape.vector)]),
                     curShape.mass,
                     applyToConstructor(this.meshDict[curShape.meshType],
                                        curShape.meshArgs);
    }
    robot.finishChassi(iniX, iniY, iniZ);

    for(var i = 0; i < data.motors.length; i++)
    {
      var curMotorData = data.motors[i];
      var tempMotor = applyToConstructor(Motor, curMotorData.args);
      robot.addMotor(curMotorData.port,
                     tempMotor,
                     tempMotor.motor,
                     applyToConstructor(Ammo.btVector3, curMotorData.chassiLoc),
                     applyToConstructor(Ammo.btVector3, curMotorData.motorLoc),
                     applyToConstructor(Ammo.btVector3, curMotorData.chassiAxis),
                     applyToConstructor(Ammo.btVector3, curMotorData.motorAxis));
    }

    for(var i = 0; i < data.sensors.length; i++)
    {
      var curSensorData = data.sensors[i];
      var tempSensor = applyToConstructor(this.sensorDict[curSensorData.type], curSensorData.args);
      robot.addSensor(curSensor.port,
                      tempSensor,
                      tempSensor.physicsObject,
                      applyToConstructor(Ammo.btVector3, curSensorData.chassiLoc),
                      applyToConstructor(Ammo.btVector3, curSensorData.sensorLoc),
                      applyToConstructor(Ammo.btVector3, curSensorData.chassiAxis),
                      applyToConstructor(Ammo.btVector3, curSensorData.sensorAxis));
    }

    return robot;
}