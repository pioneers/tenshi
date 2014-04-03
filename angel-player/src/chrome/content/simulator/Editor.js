function Editor(document, classSelectorElem, objSelectorElem,
                addingInfoElem, curSelectionElem,
                simElem, master, mapId)
{
    this.simulator = new Simulator(simElem, master, mapId);

    this.selector = true; // raycast for selection flag
    this.toggleSelection(); // initializes function for selection

    this.selections = {};
        this.selections[""] = [""];
        this.selections.Motors = ["Custom", "Motor 01"];
        this.selections.Sensors = ["Infrared", "Rangefinder", "Touch Sensor"];
        this.selections.Chassi = ["Box", "Cylinder", "Sphere"];

    this.args = {};
        this.args[""] = [""];
        this.args.Box = ["width", "height", "depth", "mass", "color", "iniX", "iniY", "iniZ"];
        this.args.Cylinder = ["radius", "height", "mass", "color", "iniX", "iniY", "iniZ"];

        this.args.Infrared = ["width", "length", "height", "mass", "iniX", "iniY", "iniZ"];
        this.args.Rangefinder = ["width", "length", "height", "mass", "iniX", "iniY", "iniZ"];
        this.args["Touch Sensor"] = ["width", "length", "height", "mass", "maxRange", "iniX", "iniY", "iniZ"];

    this.constr = {};
        this.constr[""] = null;
        this.constr.Box = this.simulator.createBox;
        this.constr.Cylinder = this.simulator.createCylinder;

    this.curSelection = null;

    this.document = document;
    this.classSelectorElem = classSelectorElem;
    this.objSelectorElem = objSelectorElem;
    this.addingInfoElem = addingInfoElem;
    this.curSelectionElem = curSelectionElem;

    this.curSelectionFuncs = [""];
}

// wrapped because needs reference to editor
Editor.prototype.wrappedSelector = function()
{
    var self = this;

    var func = function(x, y)
    {
        var point = self.simulator
                    .cameraController
                    .calculatePoint(x - self.simulator.width/2,
                                    y - self.simulator.height/2);

        // uncomment to check accuracy of clicking, near plane
        /*
        self.simulator.setTestSprite(point[0],
                                     point[1],
                                     point[2],
                                     0,
                                     0xff00ff,
                                     "BOX",
                                     .2, .2, .2);*/

        var originVector = new Ammo.btVector3(self.simulator.camera.position.x,
                                              self.simulator.camera.position.y,
                                              self.simulator.camera.position.z);

        var diffx = point[0] - self.simulator.camera.position.x,
            diffy = point[1] - self.simulator.camera.position.y,
            diffz = point[2] - self.simulator.camera.position.z;

        // raycast tests line segment, 1000 is distance to check for selection
        var posx = self.simulator.camera.position.x + 1000*diffx,
            posy = self.simulator.camera.position.y + 1000*diffy,
            posz = self.simulator.camera.position.z + 1000*diffz;

        var posVector = new Ammo.btVector3(posx, posy, posz);

        // uncomment to check accuracy of clicking, far plane
        /*
        self.simulator.setTestSprite(diffx*10 + self.simulator.camera.position.x,
                                     diffy*10 + self.simulator.camera.position.y,
                                     diffz*10 + self.simulator.camera.position.z,
                                     1,
                                     0xaa0000,
                                     "BOX",
                                     .2, .2, .2);*/

        var raycast = new Ammo.ClosestRayResultCallback(originVector, posVector);

        self.simulator.physicsWorld.rayTest(originVector, posVector, raycast);
        if(raycast.hasHit)
        {
            self.curSelection = raycast.get_m_collisionObject().getCollisionShape().parent;
            printOut(self.curSelection);
            self.simulator.loopEvent = self.wrappedHighlightSelection();
        }
    };
    return func;
};

Editor.prototype.wrappedHighlightSelection = function()
{
    var self = this;

    var func = function()
    {
        if(self.curSelection !== undefined && self.curSelection !== null)
        {
            var mesh = self.curSelection.mesh;
            self.simulator.setTestSprite(mesh.position.x,
                                         mesh.position.y,
                                         mesh.position.z,
                                         3,
                                         0x000088,
                                         mesh.type,
                                         (mesh.width ? mesh.width : mesh.radius) + 1,
                                         mesh.height + 1,
                                         (mesh.depth ? mesh.depth : 0) + 1,
                                         mesh.quaternion);
        }
        else
        {
            self.simulator.removeTestSprite(3);
        }
    };

    return func;
};

Editor.prototype.toggleSelection = function()
{
    if(this.selector)
    {
        this.simulator.mouseEvent = function(x,y) {};
    }
    else
    {
        this.simulator.mouseEvent = this.wrappedSelector();
    }

    this.selector = !this.selector;
};

Editor.prototype.setMode = function(mode)
{
    var editor = this;
    var genTranslateFunc = function(vector)
    {
        return function() { translateObject(editor.curSelection, vector); };
    };

    var genRotateFunc = function(vector)
    {
        return function() { rotateObjectEuler(editor.curSelection, vector); };
    };

    var genResizeFunc = function(vector)
    {
        return function() { editor.resizeObj(vector); };
    };

    switch(mode)
    {
        case "MOVE":
            this.curSelectionFuncs =
            [
                genTranslateFunc(new Ammo.btVector3(0, 0, 1)),
                genTranslateFunc(new Ammo.btVector3(0, 0, -1)),
                genTranslateFunc(new Ammo.btVector3(1, 0, 0)),
                genTranslateFunc(new Ammo.btVector3(-1, 0, 0)),
                genTranslateFunc(new Ammo.btVector3(0, 1, 0)),
                genTranslateFunc(new Ammo.btVector3(0, -1, 0))
            ];
            break;
        case "ROTATE":
            this.curSelectionFuncs =
            [
                genRotateFunc(new Ammo.btVector3(0, 0, 0.1)),
                genRotateFunc(new Ammo.btVector3(0, 0, -0.1)),
                genRotateFunc(new Ammo.btVector3(0.1, 0, 0)),
                genRotateFunc(new Ammo.btVector3(-0.1, 0, 0)),
                genRotateFunc(new Ammo.btVector3(0, 0.1, 0)),
                genRotateFunc(new Ammo.btVector3(0, -0.1, 0))
            ];
            break;
        case "RESIZE":
            this.curSelectionFuncs =
            [
                genResizeFunc(new Ammo.btVector3(0, 0, 0.1)),
                genResizeFunc(new Ammo.btVector3(0, 0, -0.1)),
                genResizeFunc(new Ammo.btVector3(0.1, 0, 0)),
                genResizeFunc(new Ammo.btVector3(-0.1, 0, 0)),
                genResizeFunc(new Ammo.btVector3(0, 0.1, 0)),
                genResizeFunc(new Ammo.btVector3(0, -0.1, 0))
            ];
            break;
        default:
            printOut("No such option.");
    }
};

Editor.prototype.resizeObj = function(vector)
{
    var object = this.curSelection,
        transformation = this.curSelection.getCenterOfMassTransform(),
        args = this.curSelection.args,
        type = this.curSelection.name;

    switch(type)
    {
        case "Box":
        {
            args[0] += vector.x();
            args[1] += vector.y();
            args[2] += vector.z();

            args[7] = -9999;
            // moves object out of way so we can rotate it freely then move it back
            // TODO(ericnguyen): feed rotation params into constructor
        }
            break;
        case "Cylinder":
        {
            args[0] += vector.z();
            args[1] += vector.x() + vector.y();
            // TODO(ericnguyen): elongate cylinders instead of resize radius

            args[6] = -9999;
        }
    }

    this.removeObj();
    this.createObj(args, type);

    transformObject(this.curSelection, transformation);
};

Editor.prototype.removeObj = function()
{
   this.simulator.removeObject(this.curSelection);
};

Editor.prototype.manipObj = function(id)
{
    this.curSelectionFuncs[id]();
};

Editor.prototype.addObj = function()
{
    var obj = this.objSelectorElem.value;

    var argArr = [];
    var temp = this.addingInfoElem.firstChild;
    while(temp)
    {
        if(temp.type)
            argArr.push(parseFloat(temp.value)||parseInt(temp.value, 16));
        temp = temp.nextSibling;
    }

    this.createObj(argArr, obj);
};

Editor.prototype.createObj = function(argsArray, name)
{
    printOut(name);
    this.curSelection = this.constr[name].apply(this.simulator, argsArray);
    this.curSelection.args = argsArray;
    this.curSelection.argNames = this.args[name];
    this.curSelection.name = name;
    this.updateCurSelection();
};

Editor.prototype.updateCurSelection = function()
{
    var target = this.curSelectionElem;
        target.innerHTML = "";
    var tempTxt, inp;

    tempTxt = document.createTextNode(this.curSelection.name);
    target.appendChild(tempTxt);
    target.appendChild(document.createElement("br"));
    for(var i = 0; i < this.curSelection.args.length; i++)
    {
        tempTxt = document.createTextNode(this.curSelection.argNames[i] + " : ");
        inp = document.createElement("input");
            inp.type = "text";
            inp.name = this.curSelection.argNames[i];
            inp.className = "inputField";
            inp.value = this.curSelection.args[i];
        target.appendChild(tempTxt);
        target.appendChild(inp);
        target.appendChild(document.createElement("br"));
    }
};

Editor.prototype.updateObjSelection = function()
{
    var objSel = this.objSelectorElem;
        objSel.innerHTML = "";

    var key = this.classSelectorElem.value;
    var tempOpt;

    for(var i = 0; i < this.selections[key].length; i++)
    {
        tempOpt = new Option(this.selections[key][i], this.selections[key][i]);
        objSel.options[objSel.length] = tempOpt;
    }
};

Editor.prototype.updateAttrSelection = function()
{
    var objAttr = this.addingInfoElem;
        objAttr.innerHTML = "";

    var key = this.objSelectorElem.value;
    var txt, inp;

    for(var i = 0; i < this.args[key].length; i++)
    {
        txt = document.createTextNode(this.args[key][i] + " : ");
        inp = document.createElement("input");
            inp.className = "inputField";
            inp.type = "text";
            inp.name = this.args[key][i];
        objAttr.appendChild(txt);
        objAttr.appendChild(inp);
        objAttr.appendChild(document.createElement("br"));
    }
};