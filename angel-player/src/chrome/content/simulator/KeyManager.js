DOWN = 0
HOLD = 1
UP = 2

function KeyManager()
{
	this.state = {};
	this.funcs = [{}, {}, {}];
	document.onkeydown = this.updateDown;
	document.onkeyup = this.updateUp;
}

KeyManager.prototype.updateDown = function(e)
{
	var key = e.which || e.keyCode; //some browsers use different

	this.state[key] = 1;

	this.actOnFuncs(DOWN, key);
};

KeyManager.prototype.updateUp = function(e)
{
	var key = e.which || e.keyCode; //some browsers use different

	this.state[key] = 0;

	this.actOnFuncs(UP, key);
};

KeyManager.prototype.updateHold = function()
{
	for(key in this.state)
	{
		if(this.state[key])
		{
			this.actOnFuncs(HOLD, key);
		}
	}
};

KeyManager.prototype.bindFunc = function(type, key, func)
{
	if(this.funcs[type][key])
	{
		this.funcs[type][key].push(func);
	}
	else
	{
		this.funcs[type][key] = [func];
	}
};

KeyManager.prototype.actOnFuncs = function(type, key)
{
	if(this.func[type][key])
	{
		for(var i = 0; i = this.func[type][key].length; i++)
		{
			this.func[type][key][i]();
		}
	}
};