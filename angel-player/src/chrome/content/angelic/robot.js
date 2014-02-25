var misc = require ( './misc.js' );
var angelic = require ( './main.js' );

//
// This is the main Angelic module
//

function compile_and_run ( text, robot ) {
  var vm = angelic.make ( );
  function set_motor ( port, val ) {
    robot.motors [ port ] = val;
    robot.version += 1;
    }
  function get_sensor ( port ) {
    return robot.sensors [ port ];
    }
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'set_motor', set_motor ),
    vm.make_exfn ( 2, 'get_sensor', get_sensor ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 100, 0.01 );
  return vm;
  }

exports.compile_and_run = compile_and_run;
exports.make = angelic.make;
