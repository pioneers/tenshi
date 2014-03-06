function Editor(document, classSelectorElem, objSelectorElem,
				addingInfoElem, curSelectionElem,
				simElem, master, mapId)
{
	this.simulator = new Simulator(simElem, master, mapId);

	this.selections = {};
	    this.selections[""] = [""];
	    this.selections["Motors"] = ["Custom", "Motor 01"];
	    this.selections["Sensors"] = ["Infrared", "Rangefinder", "Touch Sensor"];
	    this.selections["Chassi"] = ["Box", "Cylinder", "Sphere"];

    this.args = {};
	  	this.args[""] = [""];
	    this.args["Box"] = ["width", "height", "depth", "mass", "color", "iniX", "iniY", "iniZ"];
	    this.args["Cylinder"] = ["radius", "height", "mass", "color", "iniX", "iniY", "iniZ"];

	    this.args["Infrared"] = ["width", "length", "height", "mass", "iniX", "iniY", "iniZ"];
	    this.args["Rangefinder"] = ["width", "length", "height", "mass", "iniX", "iniY", "iniZ"];
	    this.args["Touch Sensor"] = ["width", "length", "height", "mass", "maxRange", "iniX", "iniY", "iniZ"];

    this.constr = {};
	    this.constr[""] = null;
	    this.constr["Box"] = this.simulator.createBox;
	    this.constr["Cylinder"] = this.simulator.createCylinder;

	this.curSelection = null;

	this.document = document;
	this.classSelectorElem = classSelectorElem;
	this.objSelectorElem = objSelectorElem;
	this.addingInfoElem = addingInfoElem;
	this.curSelectionElem = curSelectionElem;
}

Editor.prototype.addObj = function()
{
    var obj = this.objSelectorElem.value;
    var constructr = this.constr[obj];

    var argArr = [];
    var temp = this.addingInfoElem.firstChild;
    while(temp)
    {
        if(temp.type)
            argArr.push(parseFloat(temp.value)||parseInt(temp.value));
        temp = temp.nextSibling;
    }

    this.curSelection = constructr.apply(this.simulator, argArr);
    this.curSelection.args = argArr;
    this.curSelection.argNames = this.args[obj];
    this.curSelection.name = obj;
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