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
    };
    else
    {
        this.light.mesh.original.material.color.setHex(this.offColor);
    };
};