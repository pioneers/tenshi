// Import Machinery used to make xulrunner and node.js act the same.
var require = require;
var export_vals;

if ( require === undefined ) {
  export_vals = function ( names, vals ) {
    };
  require = function ( filename ) {
    var module = {};
    Components.utils.import ( 'chrome://angel-player/content/angelic/' + filename, module );
    return module;
    };
  }
else {
  export_vals = function ( names, vals ) {
    var n;

    for ( n in names ) {
      exports[names[n]] = vals[n];
      }
    };
  }
// End Import Machinery

var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var parser = require ( './parser.js' );
var misc = require ( './misc.js' );
var compiler = require ( './compiler.js' );
var executor = require ( './executor.js' );
var inferencer = require ( './inferencer.js' );
var library = require ( './library.js' );
var analyzer = require ( './analyze.js' );

//
// This is the main Angelic module
// Currently, it contains some test code.
//

function compile_and_run ( text ) {
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  var a_compiler = compiler.make ( );
  var a_executor = executor.make ( );
  var a_inferencer = inferencer.make ( );
  var a_library = library.make ( );
  var a_analyzer = analyzer.make ( );
  var parse_tree;
  var lib;
  var all_objs;

  a_parser.setupScopes ( scopes );
  a_compiler.setupScopes ( scopes );
  a_inferencer.setupScopes ( scopes );
  a_analyzer.setupScopes ( scopes );

  parse_tree = a_parser.parse ( text );
  //a_inferencer.infer ( parse_tree );

  a_analyzer.analyze ( parse_tree );

  a_compiler.compile_objs ( a_analyzer.all_objects );
  a_library.build_all_objects ( a_analyzer.all_objects );
  //misc.print ( a_analyzer.map );
  a_executor.run_code ( a_analyzer.map.get_text ( 'main' ).code );

  //misc.print ( parse_tree );

  //lib = a_compiler.compile ( parse_tree );

  //a_executor.execute ( lib );
  }

//var to_parse = '' +
//'main = fn:\n' +
//'    print(1)\n' +
//'';

var to_parse = '' +
'main = fn:\n' +
'    x = 50 - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while x != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        x = x - 1\n' +
'    print (b)\n' +
'';

//compile_and_run ( to_parse );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['compile_and_run'];
var exported_objects = [ compile_and_run ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
