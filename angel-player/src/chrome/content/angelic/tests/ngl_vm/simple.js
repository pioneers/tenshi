var root = process.argv[2];
var misc = require ( root + '/misc.js' );
var angelic = require ( root + '/main.js' );

function compile_and_run ( text ) {
  var vm = angelic.make ( );
  vm.add_library ( 'core', [
    vm.make_exfn ( 0, 'print_float', misc.print ),
    ] );
  vm.load_text ( text );
  vm.save_pkg ( 'x86_64', 'simple.ngl_pkg', misc.print );
  return vm;
  }
var code = '' +
'func = fn (n):\n' +
'    return 255 + n\n' +
'main = fn:\n' +
'    print_float(func(42))\n' +
'';

compile_and_run ( code );
