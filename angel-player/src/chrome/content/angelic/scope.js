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
var string_map = require ( './string_map.js' );


//
// This module provides a scope abstraction. Scopes are capable of storing and
// looking up objects by "type" and by "text".  Note that arguments for both
// families of methods take strings as arguments. In other words, for the
// purposes of this module, "type" means a string representing a type. Types
// and text are stored as seperate tables, and there is no requirement that
// values have both a "text" and "type" key.
//
// Scopes also provide a heirarchical lookup mechanism, looking up entries that
// aren't found in their parent scopes automatically.
//

var root = {
  get_text: function ( key ) {
    return undefined;
    },
  get_type: function ( key ) {
    return undefined;
    },
  set_text: function ( key, val ) {
    },
  set_type: function ( key, val ) {
    },
  reset_text: function ( key, val ) {
    },
  reset_type: function ( key, val ) {
    },
  load_text: function ( table ) {
    return this;
    },
  load_type: function ( table ) {
    return this;
    },
  is_above: function ( scope ) {
    return true;
    },
  };

var make = function ( ) {
  // root is the sentinel parent scope.
  // All recursive methods should have stub implementations here.
  return function make_scope ( prev_scope ) {
    var text_table = string_map.make ( );
    var type_table = string_map.make ( );

    if ( prev_scope === undefined ) {
      prev_scope = root;
      }
    return {
      // Set <field> on an object looked up by text <key> to <val>
      field_text: function ( key, field, val ) {
        var obj = this.get_text ( key, {} );
        obj[field] = val;
        },
      // Set <field> on an object looked up by type <key> to <val>
      field_type: function ( key, field, val ) {
        var obj = this.get_type ( key, {} );
        obj[field] = val;
        },
      // Combine each element in a string_map with the object with
      // the same text key. Basically, this function maps
      // misc.obj_or.
      load_text: function ( table ) {
        var self = this;

        table.each ( function ( key, val ) {
          self.set_text ( key,
                          misc.obj_or ( self.get_text ( key ) || {},
                          val ) );
          } );
        return this;
        },
      // Combine each element in a string_map with the object with
      // the same type key. Basically, this function maps
      // misc.obj_or.
      load_type: function ( table ) {
        var self = this;

        table.each ( function ( key, val ) {
          self.set_type ( key,
                          misc.obj_or ( self.get_type ( key ) || {},
                          val ) );
          } );
        return this;
        },
      get_text: function ( key, alternative ) {
        if ( text_table.has ( key ) ) {
          return text_table.get ( key );
          }
        else if ( alternative !== undefined ) {
          text_table.set ( key, alternative );
          return alternative;
          }
        else {
          return prev_scope.get_text ( key, alternative );
          }
        },
      get_type: function ( key, alternative ) {
        if ( type_table.has ( key ) ) {
          return type_table.get ( key );
          }
        else if ( alternative !== undefined ) {
          type_table.set ( key, alternative );
          return alternative;
          }
        else {
          return prev_scope.get_type ( key, alternative );
          }
        },
      set_text: function ( key, val ) {
        text_table.set ( key, val );
        return this;
        },
      set_type: function ( key, val ) {
        type_table.set ( key, val );
        return this;
        },
      toString: function ( ) {
        return '{ \'text\':' + text_table.toString ( ) +
              ',\n\'type\':' + type_table.toString ( ) + ' }';
        },
      above: function ( ) {
        return prev_scope;
        },
      is_above: function ( possible_child ) {
        while ( possible_child !== this && possible_child !== root ) {
          possible_child = possible_child.above ( );
          }
        if ( possible_child === this ) {
          return true;
          }
        else if ( possible_child === root ) {
          return false;
          }
        },
      };
    };
  } ( );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
