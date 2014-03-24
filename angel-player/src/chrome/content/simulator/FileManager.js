function genBaseFile()
{
    return Components.classes["@mozilla.org/file/directory_service;1"].
           getService(Components.interfaces.nsIProperties).
           get("CurProcD", Components.interfaces.nsIFile);
}

// done because append modifies the object itself
function copyFile(base)
{
    var file  = Components.classes["@mozilla.org/file/local;1"]
            .createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath(base.path);
    return file;
}

// returns the appended file instead of undefined
function append(base, text)
{
    var file = copyFile(base);
        file.append(text);
    return file;
}

function FileManager()
{
    this.localFilePath  = genBaseFile();

    this.pathToSimulator = genBaseFile();
        this.pathToSimulator.append("chrome");
        this.pathToSimulator.append("content");
        this.pathToSimulator.append("simulator");
    this.pathToMaps = copyFile(this.pathToSimulator);
        this.pathToMaps.append("Data");
        this.pathToMaps.append("Maps");
    this.pathToRobots = copyFile(this.pathToSimulator);
        this.pathToRobots.append("Data");
        this.pathToRobots.append("Robots");
}

FileManager.prototype.getRobotJson = function(robotId)
{
    return JSON.parse(this.getFileData(append(this.pathToRobots, robotId).path));
};

FileManager.prototype.getMapJson = function(mapId)
{
    return JSON.parse(this.getFileData(append(this.pathToMaps, mapId).path));
};

FileManager.prototype.getMapIds = function()
{
    return this.getDirectoryFiles(this.pathToMaps.path);
};

FileManager.prototype.getRobotIds = function()
{
    return this.getDirectoryFiles(this.pathToRobots.path);
};

FileManager.prototype.saveRobot = function(robotObj, robotName)
{
    this.writeJsonObjToFile(append(this.pathToRobots, robotName).path, robotObj);
};

FileManager.prototype.saveMap = function(mapObj, mapName)
{
    this.writeJsonObjToFile(append(this.pathToMaps, mapName).path, mapObj);
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

    try
    {
        file.initWithPath(filePath);
    }
    catch(e)
    {
        printOut(e);
        printOut(filePath);
    }
    if ( file.exists() === false )
    {
        printOut("File doesn't exist.");
        return "File does not exist";
    }

    fstream.init(file, 0x01, 4, null);
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

    this.writeToFile(this.localFilePath.path + "\\test.txt", "Testing");
};