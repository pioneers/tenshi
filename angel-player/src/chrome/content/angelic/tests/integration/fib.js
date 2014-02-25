var root = process.argv[2];
var misc = require ( root + '/misc.js' );
var angelic = require ( root + '/main.js' );

var fib50 = 0;

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print', misc.print ),
    vm.make_exfn ( 1, 'setFib50', function ( val ) {
      fib50 = val;
      } ),
    ] );
  vm.load_text ( text );
  vm.start_main ( );
  vm.set_evaluation_rate ( 10000, 0.01 );
  return vm;
  }

var code = '' +
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
'main = fn:\n' +
'    x = 50\n' +
'    print (x)\n' +
'    fib50 = fib (x)\n' +
'    print (fib50)\n' +
'    setFib50 (fib50)\n' +
'';

compile_and_run ( code );

setTimeout ( function ( ) {
  if ( fib50 == 12586269025 ) {
    process.exit ( 0 );
    }
  else {
    process.exit ( 1 );
    }
  }, 1 );
