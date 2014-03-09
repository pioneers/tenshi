var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var parser = require ( './parser.js' );
var misc = require ( './misc.js' );
var compiler = require ( './compiler.js' );
var executor = require ( './executor.js' );
var inferencer = require ( './inferencer.js' );
var library = require ( './library.js' );
var analyzer = require ( './analyze.js' );
var recurser = require ( './recurser.js' );
var module = require ( './module.js' );
var emitter = require ( './emitter.js' );
var assemble = require ( './assemble.js' );
var packager = require ( './packager.js' );
var binary = require ( './binary.js' );

//
// This is the main Angelic module
// Currently, it contains some test code.
//

function make ( ) {
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  var a_compiler = compiler.make ( );
  var a_executor = executor.make ( );
  var a_inferencer = inferencer.make ( );
  var a_library = library.make ( );
  var a_analyzer = analyzer.make ( );
  var a_recurser = recurser.make ( );
  var a_emitter = emitter.make ( );
  var a_assemble = assemble.make ( a_emitter );
  var a_packager = packager.make ( );
  var text = '';
  var modules = string_map.make ( );
  var pkg = null;

  var evaluation_steps = 0;
  var evaluation_period = 0;
  var vm = { };
  vm.make_exfn = function ( id, name, val ) {
    return { name: name, patch_class: 'external', object_id: id, obj: val };
    };
  vm.add_library = function ( modname, objs ) {
    // TODO(kzentner): Remove this restriction.
    misc.assert ( modname === 'core' );

    var mod = module.make ( modname );
    var map = string_map.make ( );
    objs.forEach ( function ( o ) {
      map.set ( o.name, { canonical_value: o, location: 'external' } );
      } );
    //misc.print ( map );
    mod.exports.load_text ( map );
    modules.set ( modname, mod );
    };
  vm.load_text = function ( t ) {
    text = t;

    a_parser.setupScopes ( scopes );
    a_compiler.setupScopes ( scopes );
    a_recurser.setupScopes ( scopes );
    a_inferencer.setupScopes ( scopes );
    a_analyzer.setupScopes ( scopes );
    a_assemble.setupScopes ( scopes );

    parse_tree = a_parser.parse ( text );

    a_analyzer.analyze ( parse_tree, modules );

    a_inferencer.infer ( parse_tree );

    a_compiler.compile_objs ( a_analyzer.all_objects );

    a_assemble.assemble_objs ( a_analyzer.all_objects );

    a_library.build_all_objects ( a_analyzer.all_objects );
    };
  vm.save_pkg = function ( target_type, filename, ecallback ) {
    var pkg = a_packager.create_pkg ( a_emitter.objs, modules, target_type );
    binary.write_buffer ( pkg, filename, ecallback );
    };
  vm.get_pkg = function ( target_type ) {
    return a_packager.create_pkg ( a_emitter.objs, modules, target_type );
    };
  vm.start_main = function ( ) {

    a_library.build_all_objects ( a_analyzer.all_objects );

    a_executor.func = a_analyzer.map.get_text ( 'main' ).code;
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
    a_executor.run_code ( null, evaluation_steps );
    setTimeout ( tick_vm, evaluation_period );
    }
  return vm;
  }

exports.make = make;
