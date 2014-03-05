var buffer = require ( 'buffer' );
var misc = require ( './misc.js' );
var opcodes = require ( './opcodes.js' );

function make_row ( type, val ) {
  return { type : type, val : val };
  }

function expand_li ( rows ) {
  var out = [];
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'li' ) {
      out.push ( make_row ( 'cmd', [ 'li_w', [ ] ] ) );
      out.push ( make_row ( 'literal', row.val ) );
      }
    else {
      out.push ( row );
      }
    }
  return out;
  }

function expand_vars ( rows ) {
  var stack = [];
  var v;

  var out = [];
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'res' ) {
      stack = stack.slice ( 0, -row.val.length );
      for ( v in row.val ) {
        stack.push ( row.val [ v ] );
        }
      }
    else if ( row.type === 'cmd' ) {
      var args = row.val[1].slice();

      for ( v in args ) {
        // Replace only variables, not labels.
        if ( typeof args[v] === 'string' && args[v][0] !== '.' ) {
          args[v] = stack.length - stack.lastIndexOf ( args[v] ) - 1;
          }
        }

      var cmd = row.val[0];
      var movement = 0;
      if ( cmd === 'call_1' ) {
        movement = -row.val[1][0];
        }
      else if ( cmd === 'stack_1' ) {
        movement = row.val[1][0];
        }
      else {
        movement = opcodes.stack[opcodes.op[cmd].code];
        }
      if ( movement >= 0 ) {
        for ( var i = 0; i < movement; i++ ) {
          stack.push ( 0 );
          }
        }
      else {
        stack = stack.slice ( 0, stack.length + movement );
        }

      out.push ( make_row ( 'cmd', [ cmd, args ] ) );
      }
    else {
      out.push ( row );
      }
    }
  return out;
  }

function max_size_row ( row ) {
  if ( row.type === 'cmd' ) {
    return 1;
    }
  else if ( row.type === 'literal' ) {
    return 2;
    }
  else if ( row.type === 'bz' ) {
    return 2;
    }
  else if ( row.type === 'j' ) {
    return 2;
    }
  else {
    return 0;
    }
  }

function max_distance ( rows, start, label ) {
  var dist = 0;
  var i;
  var row;

  for ( i = start; i < rows.length; i++ ) {
    row = rows[i];
    if ( row.type === 'label' && row.val === label ) {
      return dist;
      }
    dist += max_size_row ( row );
    }

  // Didn't find it going forwards, search backwards.
  dist = 0;
  for ( i = start; i >= 0; i-- ) {
    row = rows[i];
    if ( row.type === 'label' && row.val === label ) {
      return dist;
      }
    dist += max_size_row ( row );
    }

  throw 'Could not find label ' + label;
  }

function expand_label_1 ( label ) {
  return [ label + '[0]' ];
  }

function expand_label_2 ( label ) {
  return [ label + '[0]', label + '[1]' ];
  }

function expand_label_3 ( label ) {
  return [ label + '[0]', label + '[1]', label + '[2]' ];
  }

function expand_bz_and_j ( rows ) {
  var out = [];
  for ( var r in rows ) {
    var row = rows[r];
    var label;
    var max_dist;
    if ( row.type === 'bz' ) {
      label = row.val;
      max_dist = max_distance ( rows, r, label );
      if ( max_dist < Math.pow ( 2, 7 ) ) {
        out.push ( make_row ( 'cmd', [ 'bz_1', expand_label_1 ( label ) ] ) );
        }
      else if ( max_dist < Math.pow ( 2, 15 ) ) {
        out.push ( make_row ( 'cmd', [ 'bz_2', expand_label_2 ( label ) ] ) );
        }
      else if ( max_dist < Math.pow ( 2, 23 ) ) {
        out.push ( make_row ( 'cmd', [ 'bz_3', expand_label_3 ( label ) ] ) );
        }
      else {
        out.push ( make_row ( 'cmd', [ 'bz_w', [ ] ] ) );
        out.push ( make_row ( 'literal', [ 'label', label ] ) );
        }
      }
    else if ( row.type === 'j' ) {
      label = row.val;
      max_dist = max_distance ( rows, r, label );
      if ( max_dist < Math.pow ( 2, 7 ) ) {
        out.push ( make_row ( 'cmd', [ 'j_1', expand_label_1 ( label ) ] ) );
        }
      else if ( max_dist < Math.pow ( 2, 15 ) ) {
        out.push ( make_row ( 'cmd', [ 'j_2', expand_label_2 ( label ) ] ) );
        }
      else if ( max_dist < Math.pow ( 2, 23 ) ) {
        out.push ( make_row ( 'cmd', [ 'j_3', expand_label_3 ( label ) ] ) );
        }
      else {
        out.push ( make_row ( 'cmd', [ 'j_w', [ ] ] ) );
        out.push ( make_row ( 'literal', [ 'label', label ] ) );
        }
      }
    else {
      out.push ( row );
      }
    }
  return out;
  }

var BUNCH_SIZE = 4;

function push_bunch ( out, index, bunch ) {
  for ( var i = bunch.length; i < BUNCH_SIZE; i++ ) {
    bunch[i] = 0;
    }
  out[index] = make_row ( 'bunch', bunch );
  }

function compress_opcodes ( rows ) {
  var out = [ ];
  var current_bunch = [ ];
  var bunch_count = 0;
  var bunch_out_index = 0;
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'label' ) {
      push_bunch ( out, bunch_out_index, current_bunch );
      // We output 1 bunch.
      ++bunch_count;

      // We output the label.
      out[bunch_count] = row;
      ++bunch_count;

      bunch_out_index = bunch_count;
      current_bunch = [ ];
      out.push ( row );
      }
    else if ( row.type === 'cmd' ) {
      var size = 1 + row.val[1].length;
      if ( current_bunch.length + size > BUNCH_SIZE ) {
        push_bunch ( out, bunch_out_index, current_bunch );
        ++bunch_count;
        bunch_out_index = bunch_count;
        current_bunch = [ ];
        }
      current_bunch.push ( row.val[0] );
      current_bunch = current_bunch.concat ( row.val[1] );
      }
    else if ( row.type === 'literal' ) {
      // We can generate much better code here if we don't output a bunch.
      // This allows multiple variables to be initialized using one bunch of
      // li_w's.
      out[ bunch_count + 1 ] = row;
      ++bunch_count;
      }
    }
  if ( current_bunch.length > 0 ) {
    push_bunch ( out, bunch_out_index, current_bunch );
    }
  return out;
  }

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

    data = expand_li ( data );
    data = expand_bz_and_j ( data );
    data = expand_vars ( data );
    data = compress_opcodes ( data );

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
