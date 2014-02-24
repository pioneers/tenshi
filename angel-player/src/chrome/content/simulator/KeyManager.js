DOWN = 0;
HOLD = 1;
UP = 2;

function KeyManager(doc)
{
    var self = this;
    self.state = {};
    self.funcs = [{}, {}, {}];

    // embedded because need reference to KeyManger, not document
    // TODO(ericnguyen): find some way to do this better
    self.updateDown = function(e)
    {
        var key = e.which || e.keyCode; // some browsers use different
        printOut(key);

        self.state[key] = 1;

        self.actOnFuncs(DOWN, key);
    };

    self.updateUp = function(e)
    {
        var key = e.which || e.keyCode; // some browsers use different

        self.state[key] = 0;

        self.actOnFuncs(UP, key);
    };

    doc.onkeydown = self.updateDown;
    doc.onkeyup = self.updateUp;
}

KeyManager.prototype.updateHold = function()
{
    for(var key in this.state)
    {
        if(this.state[key])
        {
            this.actOnFuncs(HOLD, key);
        }
    }
};

KeyManager.prototype.bindFunc = function(type, key, func)
{
    if(this.funcs[type][key] !== undefined)
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
    if(this.funcs[type][key] !== undefined)
    {
        for(var i = 0; i < this.funcs[type][key].length; i++)
        {
            this.funcs[type][key][i]();
        }
    }
};