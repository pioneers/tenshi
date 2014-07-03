// This file is used to contain objects that are used by the CommonJS code
// but are imported into the webpage. The init() function in the main code
// for the simulator/editor needs to set this up.

// TODO(rqou): What happens when you try to load both the simulator and the
// editor at the same time?

exports.globals = {
    window: undefined,
    document: undefined,
    $: undefined,
    Ammo: undefined,
    THREE: undefined,
};
