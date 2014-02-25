var root = process.argv[2];
var misc = require ( root + '/misc.js' );
var angelic = require ( root + '/main.js' );

var worked = 0;

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'set_worked', function ( val ) {
      worked = val;
      } ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 10000, 0.01 );
  return vm;
  }

var code = '' +
'\n' +
'\n' +
'test = fn:\n' +
'    print(1)\n' +
'\n' +
'\n' +
'main = fn:\n' +
'    set_worked(1)\n' +
'';

compile_and_run ( code );

setTimeout ( function ( ) {
  if ( worked == 1 ) {
    process.exit ( 0 );
    }
  else {
    process.exit ( 1 );
    }
  }, 1 );
