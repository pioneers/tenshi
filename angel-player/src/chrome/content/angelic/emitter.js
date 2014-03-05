var misc = require ( './misc.js' );
var string_map = require ( './string_map.js' );
var obj = require ( './obj.js' );

//
// This module is used by the compiler to emit low level code.
// That code in turn will be converted into a runnable format by another
// module.
// The purposes of this intermediate form are to:
//   - Separate lookup from the compilation pass more cleanly.
//   - Ensure that immediates / literals are output correctly.
//   - Abstract out the selection of particular jump / branch / load opcodes.
//   - Provide a readable output format for debugging the compiler
//

function next_label ( obj ) {
  return obj.last_label_id++;
  }

function make ( ) {
  return misc.obj_or ( {
      objs : string_map.make ( ),
      obj_stack : [ ],
      next_id: 0,
      block_stack : [ ],
    }, {
      begin_obj : function begin_obj ( name, id ) {
        if ( id === undefined ) {
          id = this.next_id + 1;
          ++this.next_id;
          }
        var o = obj.make ( name, id );
        this.objs.set ( name, o );
        this.obj_stack.push ( o );
        this.begin_block ( );
        },
      get_obj : function get_obj ( ) {
        return this.obj_stack [ this.obj_stack.length - 1 ];
        },
      end_obj : function end_obj ( ) {
        this.end_block ( );
        this.obj_stack.pop ( );
        },
      begin_block : function begin_block ( ) {
        this.block_stack.push ( string_map.make ( ) );
        this.emit ( 'block_begin', '' );
        return '';
        },
      end_block : function end_block ( ) {
        var block = this.block_stack.pop ( );
        var vars = [];
        var movement = 0;
        block.each ( function ( name, _ ) {
          --movement;
          vars.push ( name );
          } );
        if ( movement !== 0 ) {
          this.emit_cmd ( 'stack_1', [ movement ] );
          }
        this.emit ( 'block_end', vars );
        },
      get_block : function get_block ( ) {
        return this.block_stack [ this.block_stack.length - 1 ];
        },
      add_var : function add_var ( name ) {
        this.emit ( 'var', name );
        this.get_block ( ).set ( name, true );
        },
      has_var : function has_var ( name ) {
        for ( var b in this.block_stack ) {
          var block = this.block_stack[b];
          if ( block.has ( name ) ) {
            return true;
            }
          }
        return false;
        },
      make_row : function ( type, val ) {
        return { type : type, val : val };
        },
      emit : function emit ( type, val ) {
        var row = this.make_row ( type, val );
        this.get_obj ( ).data.push ( row );
        return row;
        },
      emit_label : function emit_label ( name ) {
        if ( name === undefined ) {
          name = '.' + next_label ( this.get_obj ( ) );
          }
        if ( this.get_obj ( ).labels.has ( name ) ) {
          throw 'Duplicate label ' + name + '!';
          }
        this.emit ( 'label', name );
        this.get_obj ( ).labels.set ( name, this.get_obj ( ).data.length - 1 );
        return name;
        },
      emit_label_end : function emit_label_end ( name ) {
        this.emit ( 'label_end', name );
        return name;
        },
      reserve_label : function reserve_label ( name ) {
        if ( name === undefined ) {
          name = '.' + next_label ( this.get_obj ( ) );
          }
        this.make_row ( 'label', name );
        return name;
        },
      emit_literal : function emit_literal ( fixup_kind, val ) {
        return this.emit ( 'literal', [ fixup_kind, val ] );
        },
      emit_lookup : function emit_lookup ( name ) {
        return this.emit ( 'lookup', name );
        },
      emit_li : function emit_li ( fixup_kind, val ) {
        if ( val === undefined ) {
          throw 'Load immediate argument should not be undefined.';
          }
        this.emit ( 'li', [ fixup_kind, val ] );
        },
      emit_cmd : function emit_cmd ( cmd, args ) {
        var row = [];
        row.push ( cmd );
        if ( args === undefined ) {
          row.push ( [ ] );
          }
        else {
          if ( typeof args !== 'object' ) {
            throw 'Command arguments should be an object not ' + typeof args;
            }
          row.push ( args );
          }
        this.emit ( 'cmd', row );
        },
      emit_result : function ( res ) {
        for ( var r in res ) {
          this.add_var ( res[r] );
          }
        this.emit ( 'res', res );
        return res;
        },
      emit_bz : function emit_bz ( label ) {
        return this.emit ( 'bz', label );
        },
      emit_j : function emit_j ( label ) {
        return this.emit ( 'j', label );
        },
      to_asm : function to_asm ( indent, mindent ) {
        if ( indent === undefined ) {
          indent = '';
          }
        if ( mindent === undefined ) {
          mindent = '  ';
          }
        out = '';
        this.objs.each ( function ( key, val ) {
          out += indent + key + ':\n';
          out += val.to_asm ( indent + mindent, mindent );
          } );
        return out;
        },
    } );
  }

exports.make = make;
