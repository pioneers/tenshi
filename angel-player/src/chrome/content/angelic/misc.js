// Import Machinery used to make xulrunner and node.js act the same.
var require = require;
var export_vals;
var console = console;

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
  console = require ( 'console' );
  }
// End Import Machinery

//
// This module provides small helper functions.
//

// Combine two objects by (over)writing every field in b into a.
var obj_or = function obj_or ( a, b ) {
  var k;
  for ( k in b ) {
    a[k] = b[k];
    }
  return a;
  };

var print = function print () {
  var a;

  if ( console === undefined ) {
    for ( a in arguments ) {
      dump ( JSON.stringify ( arguments[a], null, '  ' ) );
      }
    }
  else {
    for ( a in arguments ) {
      console.log ( JSON.stringify ( arguments[a], null, '  ' ) );
      }
    }
  };

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['obj_or', 'print'];
var exported_objects = [ obj_or ,  print ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
