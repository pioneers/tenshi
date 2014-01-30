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

exports.obj_or = obj_or;
exports.print = print;
exports.assert = assert;
