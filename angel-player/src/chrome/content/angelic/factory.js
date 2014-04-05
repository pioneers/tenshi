var misc = require('./misc.js');
var yaml = require('../vendor-js/js-yaml/lib/js-yaml.js');
var fs = require('./fs.js');
var buffer = require ( './buffer.js' );
var Int64 = require ( '../vendor-js/Int64.js' );

//
// This is the JavaScript side of the Typpo system.
// It exposes the ability to create a factory object, which can then be used to
// create objects and serialize them to buffers.
//
// For more details, see commond_defs/types.yaml.
//

var kind_prototypes = { };

function get_buffer_method_base ( type, factory ) {
  var size = factory.get_size ( type );
  var prefix;
  if ( type.repr === 'integer' ) {
    // Calculate the number of bits from the number of bytes.
    prefix = 'Int' + ( size * 8 );
    }
  else if ( type.repr === 'unsigned' ) {
    prefix = 'UInt' + ( size * 8 );
    }
  else if ( type.repr === 'float' ) {
    if ( size === 4 ) {
      prefix = 'Float';
      }
    else if ( size === 8 ) {
      prefix = 'Double';
      }
    else {
      throw 'Unsupported size for floating point value!';
      }
    }
  var postfix = '';
  // If the size is 1, there's no prefix.
  if ( size !== 1 ) {
    if ( type.endian === 'big' ) {
      postfix = 'BE';
      }
    else {
      postfix = 'LE';
      }
    }
  return prefix + postfix;
  }

function rshift_32 ( val ) {
  // Javascript seems to ignore shifts >= 32 bits.
  // More precisely, any value becomes 0 when shifted by 32.
  return ( val / lshift_32 ( 1 ) );
  }

function lshift_32 ( val ) {
  // Javascript seems to ignore shifts >= 32 bits.
  // More precisely, any value becomes 0 when shifted by 32.
  return val * ( 1 << 16 ) * ( 1 << 16 );
  }

// Wrap a base type
kind_prototypes.base = {
  _is_64bit_int : function ( ) {
    return this.type.size === 8 && this.type.repr !== 'float';
    },
  init : function ( ) {
    this.val = null;
    },
  wrap : function wrap ( factory, type, val ) {
    var out = factory.create ( type );
    out.set ( val );
    return out;
    },
  unwrap : function unwrap ( ) {
    return this.val;
    },
  set : function ( val ) {
    // Allow setting to a wrapped type or an unwrapped type.
    if ( typeof val.val !== 'undefined' ) {
      val = val.val;
      }
    if ( val === null ) {
      val = 0;
      }
    if ( this._is_64bit_int ( ) ) {
      if ( val instanceof Int64 ) {
        this.val = new Int64 ( val.buffer );
        }
      else {
        this.val = new Int64 ( val );
        }
      }
    else {
      this.val = val;
      }
    },
  get : function ( ) {
    return this.val;
    },
  get_write_method : function get_write_method ( buffer ) {
    if ( typeof this.type.write_method === 'undefined' ) {
      if ( this._is_64bit_int ( ) ) {
        var endian = this.type.endian;
        this.type.write_method = function ( val, offset ) {
          var hi = val.buffer.readUInt32BE ( 0 );
          var lo = val.buffer.readUInt32BE ( 4 );
          if ( endian === 'big' ) {
            this.writeUInt32BE ( hi, offset );
            this.writeUInt32BE ( lo, offset + 4 );
            }
          else {
            this.writeUInt32LE ( lo, offset );
            this.writeUInt32LE ( hi, offset + 4 );
            }
          };
        }
      else {
        var method_name = 'write' + get_buffer_method_base ( this.type, this.factory );
        // Cache the method in the type for later use.
        this.type.write_method = buffer [ method_name ];
        }
      }
    return this.type.write_method;
    },
  get_read_method : function get_read_method ( buffer ) {
    if ( typeof this.type.read_method === 'undefined' ) {
      if ( this._is_64bit_int ( ) ) {
        var endian = this.type.endian;
        this.type.read_method = function ( offset ) {
          var lo, hi;
          var pair = [];
          if ( endian === 'big' ) {
            hi = this.readUInt32BE ( offset );
            lo = this.readUInt32BE ( offset + 4 );
            }
          else {
            lo = this.readUInt32LE ( offset );
            hi = this.readUInt32LE ( offset + 4 );
            }
          return new Int64 ( hi, lo );
          };
        }
      else {
        var method_name = 'read' + get_buffer_method_base ( this.type, this.factory );
        // Cache the method in the type for later use.
        this.type.read_method = buffer [ method_name ];
        }
      }
    return this.type.read_method;
    },
  write : function ( buffer ) {
    // Call the appropriate write method on the buffer.
    this.get_write_method ( buffer ).apply ( buffer, [ this.val, this.offset ] );
    },
  read : function ( buffer ) {
    this.val = this.get_read_method ( buffer ).apply ( buffer, [ this.offset ] );
    },
  set_offset : function set_offset ( offset ) {
    this.offset = offset;
    },
  };

// From this side, these two kinds act identically.
kind_prototypes.alien = kind_prototypes.base;

// Used by both union and struct kinds to get the type of a slot by name.
function get_slot_type ( factory, type, slot_name ) {
  if ( typeof slot_name !== 'string' ) {
    throw 'Need slot name!';
    }
  for ( var s in type.slots ) {
    var slot = type.slots [ s ];
    if ( slot.name === slot_name ) {
      var slot_type = factory.get_type ( slot.type );
      if ( typeof slot.endian !== 'undefined' &&
           typeof slot_type.endian === 'undefined' ) {
        slot_type = misc.shallow_copy ( slot_type );
        slot_type.endian = slot.endian;
        slot_type.write_method = undefined;
        }
      return slot_type;
      }
    }
  throw 'Slot ' + slot_name + ' does not exist in type ' + type.name;
  }

function round_up ( input, to_round ) {
  if ( to_round === 0 ) {
    return input;
    }
  return input + ( to_round - ( input % to_round ) ) % to_round;
  }

function unwrap_composite ( comp ) {
  var out = {};
  for ( var s in comp.type.slots ) {
    var slot = comp.type.slots [ s ];
    out [ slot.name ] = comp.get_slot ( slot.name ).unwrap ( );
    }
  return out;
  }

kind_prototypes.struct = {
  init : function ( ) {
    this.slot_values = { };
    },
  wrap : function ( factory, type, obj ) {
    // Wrap around an object which has fields names which are the same as the
    // struct.
    if ( obj.type === type ) {
      // If already wrapped, re-use the existing object.
      // TODO(kzentner): This has unusual semantics if the object modified. Do
      // we want this behavior?
      return obj;
      }
    else {
      var out = factory.create ( type );
      for ( var k in obj ) {
        // Set all available fields.
        out.set_slot ( k, obj [ k ] );
        }
      return out;
      }
    },
  unwrap : function ( ) {
    return unwrap_composite ( this );
    },
  set_slot : function ( slot_name, val ) {
    var type = get_slot_type ( this.factory, this.type, slot_name );
    this.slot_values [ slot_name ] = this.factory.wrap ( type, val );
    misc.assert ( this.slot_values [ slot_name ].type.endian === type.endian );
    this.recalculate_offsets ( );
    },
  get_slot : function ( slot_name ) {
    return this.slot_values [ slot_name ];
    },
  recalculate_offsets : function recalculate_offsets ( ) {
    // Cause all offsets of slots to be recalculated.
    this.set_offset ( this.offset );
    },
  set_offset : function set_offset ( offset ) {
    this.offset = offset;
    var relative = 0;
    for ( var s in this.type.slots ) {
      // Recalculate the offsets of all slots.
      var slot = this.type.slots [ s ];
      var val = this.slot_values [ slot.name ];
      var size = this.factory.get_size ( slot.type );
      if ( ! this.type.packed ) {
        // This assumes that types are self-aligned. Seems to be the case in GCC.
        // TODO(kzentner): Confirm that this is the case in all relevant compilers.
        relative = round_up ( relative, size );
        }
      if ( val !== undefined ) {
        val.set_offset ( offset + relative );
        }
      relative += size;
      }
    },
  write : function ( buffer ) {
    // Recursively write all of the slots.
    for ( var s in this.type.slots ) {
      var slot = this.type.slots [ s ];
      var val = this.slot_values [ slot.name ];
      if ( val === undefined ) {
        throw 'No value for field ' + slot.name;
        }
      val.write ( buffer );
      }
    },
  read : function ( buffer ) {
    var s;
    var slot;
    // Fill all the slots with new objects.
    for ( s in this.type.slots ) {
      slot = this.type.slots [ s ];
      this.set_slot ( slot.name, this.factory.create ( slot.type ) );
      }
    this.recalculate_offsets ( );

    // Recursively read all of the slots.
    for ( s in this.type.slots ) {
      slot = this.type.slots [ s ];
      var val = this.slot_values [ slot.name ];
      val.read ( buffer );
      }
    },
  };

kind_prototypes.union = {
  init : function ( ) {
    this.slot_values = { };
    this.last_set_slot = null;
    this.last_set_value = null;
    },
  wrap : function ( factory, type, obj ) {
    if ( obj.type === type ) {
      return obj;
      }
    else {
      var out = factory.create ( type );
      out.last_set_value = factory.wrap ( obj );
      return out;
      }
    },
  unwrap : function ( ) {
    return unwrap_composite ( this );
    },
  set_slot : function ( slot_name, val ) {
    // Record only the most recent assignment.
    var type = get_slot_type ( this.factory, this.type, slot_name );
    this.last_set_slot = slot_name;
    this.last_set_value = this.factory.wrap ( type, val );
    this.slot_values [ slot_name ] = this.last_set_value;
    },
  get_slot : function ( slot_name ) {
    if ( slot_name === this.last_set_slot ) {
      return this.last_set_value;
      }
    else if ( typeof this.slot_values [ slot_name ] !== 'undefined' ) {
      return this.slot_values [ slot_name ];
      }
    else {
      var buf = buffer.Buffer ( this.get_size ( ) );
      // Temporarily  set offset of slot.
      this.last_set_slot.set_offset ( 0 );
      this.last_set_slot.write ( buf );
      // Reset offset.
      this.set_offset ( this.offset );
      return this.factory.read ( buf, this.type.slots [ slot_name ].type );
      }
    return this.slot_values [ slot_name ];
    },
  write : function ( buffer ) {
    this.last_set_value.write ( buffer );
    },
  set_offset : function set_offset ( offset ) {
    this.offset = offset;
    if ( this.last_set_value ) {
      this.last_set_value.set_offset ( offset );
      }
    },
  read : function ( buffer ) {
    for ( var s in this.type.slots ) {
      var slot = this.type.slots [ s ];
      var type = get_slot_type ( this.factory, this.type, slot.name );
      var val = this.factory.create ( type );
      val.set_offset ( this.offset );
      val.read ( buffer );
      this.set_slot ( slot.name, val );
      }
    },
  };

function get_array_elem_type ( typename ) {
  var subtype = typename.split ( '[' ) [ 0 ].trim ( );
  return subtype;
  }

function get_array_length ( typename ) {
  var match = typename.match ( /\[([0-9]+)\]/ );
  if ( ! match ) {
    return undefined;
    }
  return parseInt ( match [ 1 ], 10 );
  }

kind_prototypes.array = {
  init : function ( ) {
    this.vals = [];
    this.base = this.factory.get_type ( get_array_elem_type ( this.type.name ) );
    },
  wrap : function ( factory, type, obj ) {
    // If already wrapped, re-use the existing object.
    // TODO(kzentner): This has unusual semantics if the object modified. Do
    // we want this behavior?
    if ( obj.type === type ) {
      return obj;
      }
    else {
      var out = factory.construct_object ( kind_prototypes.array, type );
      // If we can unwrap the object, unwrap it to get an array to work with.
      if ( typeof obj.unwrap !== 'undefined' ) {
        obj = obj.unwrap ( );
        }
      for ( var i = 0; i < obj.length; i++ ) {
        out.push ( obj [ i ] );
        }
      return out;
      }
    },
  unwrap : function ( ) {
    return this.vals.map ( function ( x ) { return x.unwrap ( ); } );
    },
  push : function ( val ) {
    this.vals.push ( this.factory.wrap ( this.base, val ) );
    },
  get_length : function ( ) {
    if ( this.length !== null ) {
      return this.length;
      }
    else if ( typeof this.type.length !== 'undefined' ) {
      return this.type.length;
      }
    else {
      return undefined;
      }
    },
  set_length : function ( length ) {
    this.length = length;
    },
  write : function ( buffer ) {
    var offset = this.offset;
    var length = this.get_length ( );
    if ( length !== undefined &&
         length !== this.vals.length ) {
      throw 'Incorrect array length when writing ' + this.type.name;
      }
    for ( var k in this.vals ) {
      this.vals [ k ].write ( buffer );
      }
    },
  recalculate_offsets : function recalculate_offsets ( ) {
    // Cause all offsets of slots to be recalculated.
    this.set_offset ( this.offset );
    },
  set_offset : function set_offset ( offset ) {
    var elem_size = this.factory.get_size ( this.base );
    this.offset = offset;
    for ( var k in this.vals ) {
      var val = this.vals [ k ];
      val.set_offset ( offset );
      offset += elem_size;
      }
    },
  read : function ( buffer ) {
    var offset = this.offset;
    var elem_size = this.factory.get_size ( this.base );
    while ( offset + elem_size <= buffer.length ) {
      var val = this.factory.create ( this.base );
      val.set_offset ( offset );
      val.read ( buffer );
      this.push ( val );
      offset += elem_size;
      }
    },
  };

function get_proto ( type ) {
  return kind_prototypes [ type.kind ];
  }

function is_pointer ( typename ) {
  return typename.trim().substr ( -1 ) === '*';
  }

function is_array ( typename ) {
  return typename.indexOf ( '[' ) !== -1;
  }

// This is the main interface to this module.
var factory_prototype = {
  construct_object : function construct_object ( proto, type ) {
    var out = misc.obj_or ( Object.create ( proto ), {
      factory : this,
      type : type,
      offset : 0,
      } );
    out.init ( );
    return out;
    },
  load_type_file : function ( filename ) {
    var types = yaml.safeLoad ( fs.readFileSync ( filename ).toString ( ) );
    this.load_types ( types );
    },
  get_type : function get_type ( typename ) {
    var type = this.types [ typename ];
    if ( type === undefined ) {
      if ( is_pointer ( typename ) ) {
        return {
          name : typename,
          kind : 'base',
          size : 'native',
          repr : 'unsigned',
          };
        }
      else if ( is_array ( typename ) ) {
        return {
          name : typename,
          kind : 'array',
          };
        }
      }
    return type;
    },
  get_native_size : function get_native_size ( ) {
      if ( this.target_type === 'ARM' || this.target_type ===  'js' ) {
        return 4;
        }
      else if ( this.target_type === 'x86_64' ) {
        return 8;
        }
      else {
        throw 'Unknown platform!';
        }
    },
  get_size : function get_size ( type ) {
    // TODO(kzentner): Correct alignment in all cases requires finding
    // the strictest aligned type in any embedded struct slot and then
    // rounding the struct size up to that amount.
    var size;
    var s;
    var actual_type = type;
    if ( typeof type === 'string' ) {
      if ( is_pointer ( type ) ) {
        return this.get_native_size ( );
        }
      else if ( type.substr ( -2 ) === '[]' ) {
        return 0;
        }
      else if ( type.match ( /\[([0-9]+)\]/ ) ) {
        // Handle arrays.
        // Extract size in "base [size]".
        return get_array_length ( type ) * this.get_size ( get_array_elem_type ( type ) );
        }
      actual_type = this.get_type ( type );
      }
    if ( actual_type === undefined ) {
      throw 'Could not get size of type ' + type;
      }
    else {
      type = actual_type;
      }
    if ( typeof type.size !== 'undefined' ) {
      size = type.size;
      if ( size === 'native' ) {
        return this.get_native_size ( );
        }
      else {
        return size;
        }
      }
    else if ( type.kind === 'struct' ) {
      var total = 0;
      for ( s in type.slots ) {
        size = this.get_size ( type.slots [ s ].type );
        if ( ! type.packed ) {
          // TODO(kzentner): Make this not assume that all slots are self-aligned.
          // In particular, structs are not aligned to their size, but to the
          // most strictly aligned base type among their slots. 
          // TODO(kzentner): Confirm that this is the case in all relevant
          // compilers.
          total = round_up ( total, size );
          }
        total += size;
        }
      return total;
      }
    else if ( type.kind === 'union' ) {
      var max = 0;
      for ( s in type.slots ) {
        var slot = type.slots [ s ];
        max = Math.max ( max, this.get_size ( slot.type ) );
        }
      return max;
      }
    else {
      throw 'Could not calculate size of kind ' + type.kind;
      }
    },
  create : function create ( type ) {
    if ( typeof type === 'string' ) {
      type = this.get_type ( type );
      }
    var prototype = get_proto ( type );
    return this.construct_object ( prototype, type );
    },
  load_types : function load_types ( type_list, discard_old ) {
    var type_map = this.types || {};
    if ( discard_old ) {
      type_map = {};
      }
    for ( var t in type_list ) {
      var type = type_list[t];
      type_map[type.name] = type;
      }
    this.types = type_map;
    },
  set_target_type : function set_target_type ( target_type ) {
    this.target_type = target_type;
    },
  wrap : function wrap ( type, val ) {
    misc.assert ( typeof type === 'object', 'type should be an object.' );
    var proto = get_proto ( type );
    return proto.wrap ( this, type, val );
    },
  get_const : function get_const ( name ) {
    return this.get_type ( name ).value;
    },
  read : function ( typename, buffer, offset ) {
    if ( offset === undefined ) {
      offset = 0;
      }
    var out = this.create ( typename );
    out.set_offset ( offset );
    out.read ( buffer );
    return out;
    },
  };

// Create a new factory.
function make ( types, target_type ) {
  var out = Object.create ( factory_prototype );
  out.target_type = target_type;
  out.load_types ( types );
  return out;
  }

exports.make = make;
