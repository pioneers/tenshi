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

//
// This is the main Angelic module
// Currently, it contains some test code.
//

function compile_and_run ( text ) {
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  var a_compiler = compiler.make ( );
  var a_executor = executor.make ( );
  var parse_tree;
  var lib;

  a_parser.setupScopes ( scopes );
  a_compiler.setupScopes ( scopes );

  parse_tree = a_parser.parse ( text );

  misc.print ( parse_tree );

  lib = a_compiler.compile ( parse_tree );

  a_executor.execute ( lib );
  }

var to_parse = '' +
'fn fib n:\n' +
'    n = n - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    print (n)\n' +
'fn main:\n' +
'    n = 50 - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    print (b)\n' +
'fn test ():\n' +
'    if 0: test = 1\n' +
'    test = 5\n';

compile_and_run ( to_parse );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['compile_and_run'];
var exported_objects = [ compile_and_run ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
