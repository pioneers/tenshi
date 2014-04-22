var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var parser = require ( './parser.js' );
var misc = require ( './misc.js' );
var inferencer = require ( './inferencer.js' );
var analyzer = require ( './analyze.js' );
var recurser = require ( './recurser.js' );
var modul = require ( './modul.js' );
var emitter = require ( './emitter.js' );
var assemble = require ( './assemble.js' );
var packager = require ( './packager.js' );
var binary = require ( './binary.js' );
var factory = require ( './factory' );
var ngl_api = require ( './ngl_api' );
var buffer = require ( './buffer' );

//
// This is the main Angelic module
//

function make ( ) {
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  var a_inferencer = inferencer.make ( );
  var a_analyzer = analyzer.make ( );
  var a_recurser = recurser.make ( );
  var a_emitter = emitter.make ( );
  var a_assemble = assemble.make ( a_emitter );
  var a_packager = packager.make ( );
  var modules = string_map.make ( );
  var pkg = null;
  var common_defs_path = '../common_defs';

  var cvm = ngl_api.vm_new ( );

  var evaluation_steps = 0;
  var evaluation_period = 0;
  var vm = { };
  vm.make_exfn = function ( id, name, val ) {
    var fact = factory.make ( { }, 'js' );
    fact.load_type_file ( common_defs_path + '/ngl_types.yaml' );
    var core = ngl_api.vm_get_module ( cvm, 'core' );
    var type = ngl_api.module_get_symbol ( core, 'ngl_type_ngl_ex_func' );
    var thunk = ngl_api.add_function ( val );
    var func = fact.wrap ( fact.get_type ( 'ngl_ex_func' ), {
      header: {
        refc: 0,
        type: type
        },
      thunk: thunk,
      });
    buf = buffer.Buffer ( fact.get_size ( 'ngl_ex_func' ) );
    func.write ( buf );
    return {
      name: name,
      patch_class: 'external',
      object_id: id,
      ptr: ngl_api.buffer_to_ptr ( buf )
      };
    };
  vm.add_library = function ( modname, objs ) {
    // TODO(kzentner): Remove this restriction.
    misc.assert ( modname === 'core' );

    var cmod = ngl_api.module_new ( );
    for ( var i in objs ) {
      var o = objs[i];
      ngl_api.module_set_symbol ( cmod, o.name, o.object_id, o.ptr );
      }
    ngl_api.vm_set_module ( cvm, modname, cmod );

    var mod = modul.make ( modname );
    var map = string_map.make ( );
    objs.forEach ( function ( o ) {
      map.set ( o.name, { canonical_value: o, location: 'external' } );
      } );
    mod.exports.load_text ( map );
    modules.set ( modname, mod );
    };
  vm.set_common_defs_path = function ( path ) {
    common_defs_path = path;
    };
  vm.load_text = function ( text ) {
    a_parser.setupScopes ( scopes );
    a_recurser.setupScopes ( scopes );
    a_inferencer.setupScopes ( scopes );
    a_analyzer.setupScopes ( scopes );
    a_assemble.setupScopes ( scopes );

    parse_tree = a_parser.parse ( text );

    a_analyzer.analyze ( parse_tree, modules );

    a_inferencer.infer ( parse_tree );

    a_assemble.assemble_objs ( a_analyzer.all_objects );
    };
  vm.save_pkg = function ( target_type, filename, ecallback ) {
    var pkg = this.get_pkg ( target_type );
    binary.write_buffer ( pkg, filename, ecallback );
    };
  vm.get_pkg = function ( target_type ) {
    return a_packager.create_pkg ( a_emitter.objs, modules, target_type,
                                   common_defs_path );
    };
  vm.start_main = function ( ) {
    var pkg_ptr = ngl_api.buffer_to_ptr ( this.get_pkg ( 'js' ) );
    ngl_api.vm_load_pkg ( cvm, pkg_ptr );
    ngl_api.free ( pkg_ptr );
    ngl_api.vm_start_main ( cvm );
    };
  vm.set_evaluation_rate = function ( steps, period ) {
    evaluation_steps = steps;
    evaluation_period = period;
    clearTimeout ( tick_vm );
    if ( period !== 0 && steps !== 0 ) {
      setTimeout ( tick_vm, evaluation_period );
      }
    };
  function tick_vm ( ) {
    ngl_api.vm_run_ticks ( cvm, evaluation_steps );
    setTimeout ( tick_vm, evaluation_period );
    }
  return vm;
  }

exports.make = make;
