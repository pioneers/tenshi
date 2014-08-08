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

/*
All LEDs are a single box
    run takes value, turns it to either offColor or onColor, depending on true/false
*/
function LED(offColor, onColor, width, length, height, mass, iniX, iniY, iniZ)
{
    this.light = createBox(width, length, height, mass, offColor, iniX, iniY, iniZ);
    this.offColor = offColor;
    this.onColor = onColor;
    this.value = 0;
}

LED.prototype.setVal = function(value)
{
    this.value = value;
};

LED.prototype.getVal = function(value)
{
    return this.value;
};

LED.prototype.run = function()
{
    if(this.value)
    {
        this.light.mesh.original.material.color.setHex(this.onColor);
    }
    else
    {
        this.light.mesh.original.material.color.setHex(this.offColor);
    }
};