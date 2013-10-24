var require = require;
var export_vals;

if ( require === undefined ) {
  export_vals = function ( names, vals ) {
    };
  require = function ( filename ) {
    return Components.utils.import ( 'chrome://angel-player/content/angelic/' + filename );
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

var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var parser = require ( './parser.js' );
var misc = require ( './misc.js' );

//
// This is the main Angelic module
// Currently, it contains some test code.
//

function compile_and_run ( text ) {
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  var parse_tree;

  a_parser.setupScopes ( scopes );

  parse_tree = a_parser.parse ( text );

  misc.print ( parse_tree );

  // More functionality added here later.
  }

var to_parse = '' +
'fn fib n:\n' +
'    a = 1\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    return n\n' +
'fn main:\n' +
'    fib ( 100 )\n' +
'fn test ():\n' +
'    test = 5\n' +
'    if 0: test = 1\n' +
'    else: text = 2\n';

compile_and_run ( to_parse );

var EXPORTED_SYMBOLS = ['compile_and_run'];
var exported_objects = [ compile_and_run ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
