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

var module = require ( './module.js' );
var type = require ( './type.js' );
var fn = require ( './fn.js' );

function make ( ) {
  var core = module.make ( );
  var print = fn.make ( );
  print.external = true;
  core.exports.set_type ( 'number', type.make ( 'number' ) );
  core.exports.set_type ( 'bool', type.make ( 'bool' ) );
  core.exports.set_text ( 'print', print );
  core.objects.push ( print );
  return core;
  }

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
