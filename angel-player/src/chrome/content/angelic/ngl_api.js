var misc = require ( './misc' );
var ngl_vm;

try {
  ngl_vm = require ( '../vendor-js/ngl_vm' );

  // TODO(kzentner): Throw an exception if any on functions failing.

  exports.module_new = ngl_vm.cwrap ( 'ngl_api_module_new', 'number', [ ] );
  exports.module_get_symbol = ngl_vm.cwrap ( 'ngl_api_module_get_symbol',
      'number', [ 'number', 'string' ] );
  exports.module_set_symbol = ngl_vm.cwrap ( 'ngl_api_module_set_symbol',
      'string', [ 'number', 'string', 'number', 'number' ] );

  exports.vm_new = ngl_vm.cwrap ( 'ngl_api_vm_new', 'number', [ ] );

  exports.vm_get_module = ngl_vm.cwrap ( 'ngl_api_vm_get_module',
      'number', [ 'number', 'string' ] );

  exports.vm_set_module = ngl_vm.cwrap ( 'ngl_api_vm_set_module',
      'string', [ 'number', 'string', 'number' ] );

  exports.vm_load_pkg = ngl_vm.cwrap ( 'ngl_api_vm_load_pkg',
      'string', [ 'number', 'number' ] );

  exports.vm_start_main = ngl_vm.cwrap ( 'ngl_api_vm_start_main',
      'string', [ 'number' ] );

  exports.vm_run = ngl_vm.cwrap ( 'ngl_api_vm_run',
      'string', [ 'number' ] );

  exports.vm_run_ticks = ngl_vm.cwrap ( 'ngl_api_vm_run_ticks',
      'string', [ 'number', 'number' ] );

  exports.buffer_to_ptr = function ( buf ) {
    var mem = ngl_vm._malloc ( buf.length );
    if ( mem === 0 ) {
      throw 'Could not allocate memory for buffer copy.';
      }
    for ( var i = 0; i < buf.length; i++ ) {
      ngl_vm.setValue ( mem + i, buf [ i ], 'i8' );
      }
    return mem;
    };

  exports.add_function = function ( fn ) {
    return ngl_vm.Runtime.addFunction ( fn );
    };

  exports.free = ngl_vm._free;
  }
catch ( _ ) {
  function warn ( ) {
    misc.print ( 'WARNING: ngl_vm could not be loaded.' );
    }

  exports.module_new = warn;
  exports.module_set_symbol = warn;
  exports.module_get_symbol = warn;

  exports.vm_new = warn;
  exports.vm_get_module = warn;
  exports.vm_set_module = warn;
  exports.vm_load_pkg = warn;
  exports.vm_start_main = warn;
  exports.vm_run = warn;
  exports.vm_run_ticks = warn;
  exports.buffer_to_ptr = warn;
  exports.add_function = warn;
  }
