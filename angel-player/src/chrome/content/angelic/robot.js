var misc = require ( './misc.js' );
var angelic = require ( './main.js' );

//
// This is the main Angelic module
// Currently, it contains some test code.
//

function compile_and_run ( text, robot ) {
  var vm = angelic.make ( );
  function set_motor ( port, val ) {
    robot.motors [ port ] = val;
    robot.version += 1;
    }
  function get_sensor ( port, val ) {
    return robot.sensors [ port ];
    }
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'set_motor', set_motor ),
    vm.make_exfn ( 2, 'get_sensor', get_sensor ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 1000, 0.01 ); 
  return vm;
  }

var to_parse = '' +
'fib = fn (n):\n' +
'    n = n - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    return b\n' +
//'recfib = fn (n):\n' +
//'    if n == 0:\n' +
//'        return 0\n' +
//'    if n == 1:\n' +
//'        return 1\n' +
//'    else:\n' +
//'        return recfib (n - 1) + recfib (n - 2)\n' +
'main = fn:\n' +
'    x = 50\n' +
'    set_motor (0, 100)\n' +
'    set_motor (1, 100)\n' +
'    print (x)\n' +
'    fib50 = fib (x)\n' +
//'    fib10 = recfib (10)\n' +
'    print (fib50)\n' +
//'    print (fib10)\n' +
'    set_motor (0, 20)\n' +
'    set_motor (1, 20)\n' +
'';

var robot = { motors: {}, sensors: {}, version: 0 };
compile_and_run ( to_parse, robot );
misc.print ( 'robot', robot );

exports.compile_and_run = compile_and_run;
