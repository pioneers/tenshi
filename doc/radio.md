radio.js API

==============

radio.js exports two things:

  * A constructor called `Radio`
  * A global variable called `debug`.

`debug` should only be set when the module is first loaded, before `Radio` is
called. It defaults to `false` and causes large amounts of radio internal state
and events to be logged to the console.

The typical way of using `radio.js` is to construct a `Radio` object.
The following methods are available.

  * All methods of an `EventEmitter`, since `Radio` inherits from `EventEmitter`.
   * Please see [the documentation](https://github.com/Wolfy87/EventEmitter/blob/master/docs/guide.md)
     for more details. However, the expected uses are repeated below.
   * `on`, which is used to register (semi) permanent callbacks.
      `on` takes two arguments, an `event`, and a `callback`. `callback` will be
      called with a single argument, according to `event`, as described in the `send`
      documentation.
  * The `Radio` constructor, which takes two optional parameters:
   * `address`, which should be a 16 character hexadecimal string representing the robot's XBee address.
   * `serport`, which should be a [serial port object.](https://github.com/voodootikigod/node-serialport)
  * `isConnected`, which returns `true` if the radio is connected to a robot.
   * Currently, no heart-beats are used, so the radio may have since disconnected from the radio.
  * `connectXBee`, which takes the same arguments as the constructor, but requires them, unlike the constructor.
    It should only be called if the XBee has not already been connected.
  * `disconnectXBee`, which takes no arguments and disconnects the XBee. It can
    be called even if the XBee was never connected.
  * `close`, which deletes internal `Radio` state, and should be called when
    the radio will never be used again.
  * `send`, which takes one mandatory argument, and one optional argument.
   * `data`, which should be something to send, depending on the second argument.
   * `type`, which defaults to `object`.
  * Valid types to send / receive are:
   * Any object which would reasonably be encoded in (UB)JSON, with `object` or `fast`.
   * Any string, with `string`.
   * A blob of code, either in compiled code or source form, with `code`.
   * Some binary data as described in , with `config`. Typpo should probably be used for configuring this.
