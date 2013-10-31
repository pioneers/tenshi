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
var type = require ( './type.js' );
var obj = require ( './obj.js' );
var string_map = require ( './string_map.js' );
var scope = require ( './scope.js' );

var root_lib = {
  name_type_idx: function ( name ) {
    return this.current_scope.get_type ( name );
    },
  idx_type: function ( idx ) {
    return this.types[idx];
    },
  name_obj_idx: function ( name ) {
    return this.current_scope.get_text ( name );
    },
  idx_obj: function ( type_idx, idx ) {
    return this.objs[type_idx][idx];
    },
  get_obj: function ( type_name, obj_name ) {
    var type_idx = this.name_type_idx ( type_name );
    var obj_idx = this.current_scope.get_text ( obj_name );
    return this.idx_obj ( type_idx, obj_idx.obj );
    },
  add_type: function ( name, a_type ) {
    var idx = this.next_type_idx;
    this.next_type_idx += 1;
    this.types[idx] = a_type;
    this.current_scope.set_type ( name, idx );
    return idx;
    },
  add_obj: function ( name, type_idx, a_obj ) {
    var objs = this.objs[type_idx];
    var idx = this.next_obj_idx[type_idx];
    if ( objs === undefined ) {
      objs = this.objs[type_idx] = [];
      idx = this.next_obj_idx[type_idx] = 0;
      }
    this.next_obj_idx[type_idx] = idx + 1;
    objs[idx] = a_obj;
    this.current_scope.set_text ( name, { type: type_idx, obj: idx } );
    },
  };

var make = function make ( ) {
  var root_scope = scope.make ( );
  return misc.obj_or ( Object.create ( root_lib ), {
    next_type_idx: 0,
    next_obj_idx: [],
    types: [],
    // Contains arrays of arrays of objs.
    // Indexed by type, then object idx.
    objs: [],
    relocations: [],
    root: root_scope,
    current_scope: root_scope,
    } );
  };

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
