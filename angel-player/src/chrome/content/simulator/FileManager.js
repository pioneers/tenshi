const { Cc, Ci, Cu } = require('chrome');
const { printOut } = require('tenshi/simulator/miscFuncs');
const G = require('tenshi/simulator/window_imports').globals;

function genBaseFile()
{
    return Cc["@mozilla.org/file/directory_service;1"].
           getService(Ci.nsIProperties).
           get("CurProcD", Ci.nsIFile);
}

// done because append modifies the object itself
function copyFile(base)
{
    var file  = Cc["@mozilla.org/file/local;1"]
            .createInstance(Ci.nsILocalFile);
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
    var file = Cc["@mozilla.org/file/local;1"]
                .createInstance(Ci.nsILocalFile);
        file.initWithPath(filePath);
    var entries = file.directoryEntries;
    var array = [];
    while(entries.hasMoreElements())
    {
      var entry = entries.getNext();
      entry.QueryInterface(Ci.nsIFile);
      array.push(entry.path.replace(filePath, ""));
    }

    return array;
};

FileManager.prototype.writeToFile = function(fileName, data)
{
    var file = Cc["@mozilla.org/file/local;1"].
           createInstance(Ci.nsILocalFile);
    file.initWithPath(fileName);

    Cu.import("resource://gre/modules/NetUtil.jsm");
    Cu.import("resource://gre/modules/FileUtils.jsm");

    var ostream = FileUtils.openSafeFileOutputStream(file);

    var converter = Cc["@mozilla.org/intl/scriptableunicodeconverter"].
                    createInstance(Ci.nsIScriptableUnicodeConverter);
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
    if (G.window.File && G.window.FileReader && G.window.FileList && G.window.Blob)
    {
        console.log("File reader exists.");
    }
    else
    {
        console.log("No file reader exists.");
    }

    var file = Cc["@mozilla.org/file/local;1"]
                   .createInstance(Ci.nsILocalFile);

    var fstream = Cc["@mozilla.org/network/file-input-stream;1"]
    .createInstance(Ci.nsIFileInputStream);
    var sstream = Cc["@mozilla.org/scriptableinputstream;1"]
    .createInstance(Ci.nsIScriptableInputStream);

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

exports.FileManager = FileManager;
