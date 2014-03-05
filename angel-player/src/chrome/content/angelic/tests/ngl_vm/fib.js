var root = process.argv[2];
var misc = require ( root + '/misc.js' );
var angelic = require ( root + '/main.js' );

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print_float', misc.print ),
    ] );
  vm.load_text ( text );
  vm.save_pkg ( 'x86_64', 'fib.ngl_pkg', misc.print );
  return vm;
  }

//var code = '' +
//'fib = fn (n):\n' +
//'    n = n - 1\n' +
//'    a = 0\n' +
//'    b = 1\n' +
//'    while n != 0:\n' +
//'        temp = a + b\n' +
//'        a = b\n' +
//'        b = temp\n' +
//'        n = n - 1\n' +
//'    return b\n' +
//'main = fn:\n' +
//'    x = 50\n' +
//'    print_float (x)\n' +
//'    fib50 = fib (x)\n' +
//'    print_float (fib50)\n' +
//'';

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
'    print_float (x)\n' +
'    n = x\n' +
'    n = n - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    fib50 = b\n' +
'    print_float (fib50)\n' +
'';

compile_and_run ( code );
