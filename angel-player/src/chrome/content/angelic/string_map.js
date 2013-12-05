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

var misc = require ( './misc.js' );

//
// This module provides a simple string -> obj map.
//

var make = function make ( obj ) {
  var storage = {};

  if ( obj !== undefined ) {
    for ( var k in obj ) {
      storage[k + '$'] = obj[k];
      }
    }
  var table = {
    // Get the value, or set it to alternative.
    'get': function ( key, alternative ) {
      var val = storage[key + '$'];
      if ( val === undefined ) {
        this.set ( key, alternative );
        }
      return storage[key + '$'];
      },
    'set': function ( key, val ) {
      storage[key + '$'] = val;
      return this;
      },
    'has': function ( key ) {
      return key + '$' in storage;
      },
    'delete': function ( key ) {
      return delete storage [ key + '$' ];
      },
    // Update values in place using a function.
    'map': function ( func ) {
      var self = this;

      this.each ( function ( key, val ) {
        self.set ( key, func ( key, val ) );
        } );
      return this;
      },
    // Run a function on every key, value pair.
    'each': function ( func ) {
      var key;
      var val;

      for ( key in storage ) {
        val = storage[key];
        key = key.substr ( 0, key.length - 1 );
        func ( key, val );
        }
      return this;
      },
    'toString': function ( val_str ) {
      var out = '{';


      if ( val_str === undefined ) {
        val_str = function ( val ) {
          return '' + val;
          //return JSON.stringify ( val, null, '  ' );
          };
        }
      this.each ( function ( key, val ) {
        out += '' + key + ' : ' + val_str ( val ) + ', ';
        } );

      out += '}';
      return out;
      },
    };
  return misc.obj_or ( Object.create ( table ), {
    storage: storage,
    } );
  };

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
