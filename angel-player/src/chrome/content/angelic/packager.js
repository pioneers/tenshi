var buffer = require ( 'buffer' );
var misc = require ( './misc.js' );

var BUNCH_SIZE = 4;

var root = {
  make_patch : function make_patch ( obj, rows ) {
    // TODO(kzentner): Actually create a patch here.
    var buf = new buffer.Buffer ( rows.length * BUNCH_SIZE );
    // This makes binary diffs cleaner.
    buf.fill ( 0 );
    var fixups = [];
    return {
      buf : buf,
      fixups : fixups,
      offset : 0,
      id : obj.id,
      };
    },
  process_obj : function process_obj ( name, obj ) {
    var data = obj.data;

    // TODO(kzentner): Add passes here.

    var patch = this.make_patch ( obj, data );
    this.patches.push ( patch );
    },
  output_size : function output_size ( ) {
    // TODO(kzentner): Compute the size.
    var size = 0;
    return size;
    },
  create_pkg : function create_pkg ( map, modules, target_type ) {
    var self = this;
    this.target_type = target_type;

    map.each ( function ( name, obj ) {
      self.process_obj ( name, obj );
      } );

    var out_size = this.output_size ( );

    this.buffer = new buffer.Buffer ( out_size );
    // This makes binary diffs cleaner.
    this.buffer.fill ( 0 );

    this.write ( );

    return this.buffer;
    },
  write : function write ( ) {
    // TODO(kzentner): Implement these methods.
    // this.write_header ( );
    // this.write_patches ( );
    // this.write_fixups ( );
    },
  };

function make ( ) {
  return misc.obj_or ( {
    patches: [],
    fixups: [],
    buffer: null,
    buffer_idx: 0,
    offset: 0,
    },
    root );
  }

exports.make = make;
