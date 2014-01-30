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
  var text = '';
  var modules = string_map.make ( );

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
    misc.print ( map );
    mod.exports.load_text ( map );
    modules.set ( modname, mod );
    };
  vm.load_text = function ( t ) {
    text = t;
    };
  vm.start_main = function ( ) {
    a_parser.setupScopes ( scopes );
    a_compiler.setupScopes ( scopes );
    a_recurser.setupScopes ( scopes );
    a_inferencer.setupScopes ( scopes );
    a_analyzer.setupScopes ( scopes );

    parse_tree = a_parser.parse ( text );

    a_analyzer.analyze ( parse_tree, modules );

    a_inferencer.infer ( parse_tree );

    a_compiler.compile_objs ( a_analyzer.all_objects );

    a_library.build_all_objects ( a_analyzer.all_objects );
    a_executor.run_code ( a_analyzer.map.get_text ( 'main' ).code );
    };
  vm.set_evaluation_rate = function ( steps, period ) {
    //TODO(kzentner): Implement this function, instead of start_main blocking.
    };
  return vm;
  }

exports.make = make;
