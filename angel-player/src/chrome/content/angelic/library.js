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
var fn = require ( './fn.js' );
var exfn = require ( './exfn.js' );


var root_lib = {
  build_all_objects: function ( objs ) {
    var o;
    var obj;
    for ( o in objs ) {
      obj = objs[o];
      if ( obj.patch_class === 'fn' ) {
        obj.header = fn.make ( obj.code );
        }
      else {
        throw 'Could not handle object.';
        }
      }
    for ( o in objs ) {
      obj = objs[o];
      if ( obj.patch_class === 'fn' ) {
        this.process_fn ( obj );
        }
      else {
        throw 'Could not handle object.';
        }
      }
    return this.objs;
    },

  process_fn: function process_fn ( obj ) {
    var lookups = obj.lookups;
    var code = obj.code;
    for ( var l in lookups ) {
      var lookup = lookups[l];
      //misc.print ( lookup.target.variable.canonical_value.code );
      var target = lookup.target.variable;
      var header;
      if ( target.location === 'external' ) {
        header = exfn.make ( target.canonical_value.obj );
        }
      else {
        header = lookup.target.variable.canonical_value.header;
        }
      code[lookup.index] = header;
      }
    this.objs.push ( this.header );
    }
  };

var make = function make ( ) {
  return misc.obj_or ( Object.create ( root_lib ), {
    next_type_idx: 0,
    next_obj_idx: [],
    types: [],
    // Contains arrays of arrays of objs.
    // Indexed by type, then object idx.
    objs: [],
    relocations: [],
    } );
  };

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
