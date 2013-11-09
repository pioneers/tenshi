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
var module = require ( './module.js' );
var misc = require ( './misc.js' );

var sscope_text_methods = string_map.make ( {
  'fn' : {
    analyze : function ( analyzer ) {
      var func = fn.make ( this.name, this.ast );
      analyzer.add_object ( func );
      this.body.analyze ( analyzer );
      },
    }
  } );

var sscope_type_methods = string_map.make ( {
  'block' : {
    analyze : function ( analyzer ) {
      for ( var idx in this.children ) {
        var child = this.children[idx];
        }
      }
    },
  } );

function setupScopes ( scopes ) {
  var escope = scopes.get ( 'expression', scope.make ( ) );
  var sscope = scopes.get ( 'statement', scope.make ( escope ) );
  }

function analyze ( ast ) {
  var root_module = module.make ( '' );
  root_module.ast = ast;
  return {
    '': root_module,
    };
  }

function make ( ) {
  return {
    analyze: analyze,
    setupScopes: setupScopes,
    };
  }

function test ( text ) {
  var parser = require ( './parser.js' );
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  a_parser.setupScopes ( scopes );
  parse_tree = a_parser.parse ( text );
  //misc.print ( parse_tree );

  var a_analyzer = make ( );
  var modules = a_analyzer.analyze ( parse_tree );
  misc.print ( modules );
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
'fn main():\n' +
'    x = 50 - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while x != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        x = x - 1\n' +
'    print (b)\n' +
'fn test ():\n' +
'    test = 0\n' +
'    if 0 != 0: test = 1\n';


test ( to_parse );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
