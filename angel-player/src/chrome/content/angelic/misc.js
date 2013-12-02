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
      var util = require ( 'util' );
      //console.log ( JSON.stringify ( arguments[a], null, '  ' ) );
      console.log ( util.inspect ( arguments[a], { colors: true, depth: null } ) );
      }
    }
  };

// This exists because there is no built in assert in JavaScript.
function assert ( thing, reason ) {
  if ( ! thing ) {
    throw 'ERROR: ' + reason;
    }
  }


// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['obj_or', 'print', 'assert' ];
var exported_objects = [ obj_or ,  print ,  assert  ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
