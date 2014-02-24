function FileManager()
{
    this.localFilePath  = Components.classes["@mozilla.org/file/directory_service;1"].
           getService(Components.interfaces.nsIProperties).
           get("CurProcD", Components.interfaces.nsIFile).path;

    this.pathToSimulator = this.localFilePath + "\\chrome\\content\\simulator\\";
    this.pathToMaps = this.pathToSimulator + "Data\\Maps\\";
    this.pathToRobots = this.pathToSimulator + "Data\\Robots\\";
}

FileManager.prototype.getRobotJson = function(robotId)
{
    return JSON.parse(this.getFileData(this.pathToRobots + robotId));
};

FileManager.prototype.getMapJson = function(mapId)
{
    return JSON.parse(this.getFileData(this.pathToMaps + mapId));
};

FileManager.prototype.getMapIds = function()
{
    return this.getDirectoryFiles(this.pathToMaps);
};

FileManager.prototype.getRobotIds = function()
{
    return this.getDirectoryFiles(this.pathToRobots);
};

FileManager.prototype.saveRobot = function(robotObj, robotName)
{
    this.writeJsonObjToFile(this.pathToRobots + robotName, robotObj);
};

FileManager.prototype.saveMap = function(mapObj, mapName)
{
    this.writeJsonObjToFile(this.pathToMaps + mapName, mapObj);
};

FileManager.prototype.writeJsonObjToFile = function(fileName, data)
{
    this.writeToFile(fileName, JSON.stringify(data));
};

FileManager.prototype.getDirectoryFiles = function(filePath)
{
    var file = Components.classes["@mozilla.org/file/local;1"]
                .createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath(filePath);
    var entries = file.directoryEntries;
    var array = [];
    while(entries.hasMoreElements())
    {
      var entry = entries.getNext();
      entry.QueryInterface(Components.interfaces.nsIFile);
      array.push(entry.path.replace(filePath, ""));
    }

    return array;
};

FileManager.prototype.writeToFile = function(fileName, data)
{
    var file = Components.classes["@mozilla.org/file/local;1"].
           createInstance(Components.interfaces.nsILocalFile);
    file.initWithPath(fileName);

    Components.utils.import("resource://gre/modules/NetUtil.jsm");
    Components.utils.import("resource://gre/modules/FileUtils.jsm");

    var ostream = FileUtils.openSafeFileOutputStream(file);

    var converter = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].
                    createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
    converter.charset = "UTF-8";
    var istream = converter.convertToInputStream(data);

    NetUtil.asyncCopy(istream, ostream, function(status) {
      if (!Components.isSuccessCode(status)) {
        return;
      }
      else
      {
          printOut("Wrote to " + fileName);
      }
    });
};

FileManager.prototype.getFileData = function(filePath)
{
    if (window.File && window.FileReader && window.FileList && window.Blob) 
    {
        console.log("File reader exists.");
    }
    else 
    {
        console.log("No file reader exists.");
    }

    var file = Components.classes["@mozilla.org/file/local;1"]
                   .createInstance(Components.interfaces.nsILocalFile);

    var fstream = Components.classes["@mozilla.org/network/file-input-stream;1"]
    .createInstance(Components.interfaces.nsIFileInputStream);
    var sstream = Components.classes["@mozilla.org/scriptableinputstream;1"]
    .createInstance(Components.interfaces.nsIScriptableInputStream);

    file.initWithPath(filePath);
    if ( file.exists() == false )
    {
        printOut("File doesn't exist.");
        return "File does not exist";
    }

    fstream.init(file, 0x01, 00004, null);
    sstream.init(fstream);

    var output = sstream.read(sstream.available());
    sstream.close();
    fstream.close();

    return output;
};

FileManager.prototype.test = function()
{
    printOut(this.getRobotIds()[0]);
    printOut(this.getMapIds());

    this.writeToFile(this.localFilePath + "\\test.txt", "Testing");
};