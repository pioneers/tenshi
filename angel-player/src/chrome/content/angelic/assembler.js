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

function next_stack ( obj ) {
  return obj.last_stack_id++;
  }

function remove_undefined ( array ) {
  return array.filter ( function ( val ) { return val !== undefined; } );
  }

function get_label_end ( rows, start_index ) {
  var end_label;
  for ( end_label = start_index + 1; end_label < rows.length; end_label++ ) {
    if ( rows[end_label].type === 'label_end' &&
         rows[end_label].val === rows[start_index].val ) {
      return end_label;
      }
    }
  if ( end_label === rows.length ) {
    for ( end_label = start_index + 1; end_label < rows.length; end_label++ ) {
      if ( rows[end_label].type === 'label' ) {
        return end_label;
        }
      }
    }
  return end_label;
  }

function cmd_to_asm ( val ) {
  var out = val[0];
  val[1].forEach ( function ( v ) {
    if ( v !== undefined ) {
      out += ' ' + v;
      }
    } );
  if ( val[1].length + val[2].length !== 0 ) {
    out += ' ->';
    }
  val[2].forEach ( function ( v ) {
    if ( v !== undefined ) {
      out += ' $(' + v + ')';
      }
    } );
  return out;
  }

function lookup_to_asm ( val ) {
  return '%(' + val + ')';
  }

function literal_to_asm ( val ) {
  var prefix = '';
  if ( val[0] === 'int' ) {
    prefix = '(i)';
    }
  if ( val[0] === 'uint' ) {
    prefix = '(u)';
    }
  if ( val[0] === 'float' ) {
    prefix = '(f)';
    }
  return prefix + ' ' + val[1];
  }

function rows_to_asm ( rows, indent, mindent ) {
  if ( mindent === undefined ) {
    mindent = '  ';
    }
  out = '';
  for ( var i = 0; i < rows.length; i++ ) {
    var row = rows[i];
    if ( row.type === 'label' ) {
      out += indent + row.val + ':\n';
      var end_label = get_label_end ( rows, i );
      misc.print ( rows.slice ( i + 1, end_label ) );
      out += rows_to_asm ( rows.slice ( i + 1, end_label ), indent + mindent, mindent );
      i = end_label;
      }
    if ( row.type === 'end_label' ) {
      throw 'Label ended without having begun.';
      }
    if ( row.type === 'cmd' ) {
      out += indent + cmd_to_asm ( row.val ) + '\n';
      }
    if ( row.type === 'li' ) {
      out += indent + cmd_to_asm ( [ 'liw', [ ], [ row.val[2] ] ] ) + '\n';
      if ( typeof row.val[1] === 'string' ) {
        out += indent + lookup_to_asm ( row.val[1] ) + '\n'; 
        }
      else {
        out += indent + literal_to_asm ( row.val ) + '\n';
        }
      }
    if ( row.type === 'lookup' ) {
      out += indent + lookup_to_asm ( row.val[1] ) + '\n'; 
      }
    if ( row.type === 'j' ) {
      out += indent + cmd_to_asm ( [ 'jw', [ ], [ ] ] ) + '\n';
      out += indent + row.val + '\n';
      }
    if ( row.type === 'bz' ) {
      out += indent + cmd_to_asm ( [ 'bzw', [ ], [ ] ] ) + '\n';
      out += indent + row.val + '\n';
      }
    }
  return out;
  }

function obj_to_asm ( obj, indent, mindent ) {
  return rows_to_asm ( obj.data, indent, mindent );
  }

function make ( ) {
  return misc.obj_or (
    {
      objs : string_map.make ( ),
      obj_stack : [ ],
      next_id: 0,
    },
    {
      begin_obj : function begin_obj ( name, id ) {
        if ( id === undefined ) {
          id = this.next_id + 1;
          ++this.next_id;
          }
        var o = obj.make ( name );
        this.objs.set ( name, o );
        this.obj_stack.push ( o );
        },
      get_obj : function get_obj ( ) {
        return this.obj_stack [ this.obj_stack.length - 1 ];
        },
      end_obj : function end_obj ( ) {
        this.obj_stack.pop ( );
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
      emit_literal : function emit_literal ( kind, val ) {
        return this.emit ( 'literal', [ kind, val ] );
        },
      emit_lookup : function emit_lookup ( name ) {
        return this.emit ( 'lookup', name );
        },
      emit_li : function emit_li ( kind, val, res ) {
        if ( res === undefined ) {
          res = [ next_stack ( this.get_obj ( ) ) ];
          }
        this.emit ( 'li', [ kind, val, res ] );
        return res;
        },
      emit_cmd : function emit_cmd ( cmd, args, res ) {
        if ( res === undefined ) {
          res = [ next_stack ( this.get_obj ( ) ) ];
          }
        if ( args === undefined ) {
          args = [ ];
          }
        this.emit ( 'cmd', [ args, res ] );
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
          out += obj_to_asm ( val, indent + mindent, mindent );
          } );
        return out;
        },
    } );
  }

exports.make = make;
