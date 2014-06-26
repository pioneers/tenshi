// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

const { printOut } = require('tenshi/simulator/miscFuncs');

var DOWN = 0;
var HOLD = 1;
var UP = 2;

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

exports.KeyManager = KeyManager;
exports.DOWN = DOWN;
exports.HOLD = HOLD;
exports.UP = UP;
