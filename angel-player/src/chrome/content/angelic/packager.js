// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

var buffer = require ( './buffer.js' );
var fixup_kinds = require ( './fixup_kinds.js' );
var misc = require ( './misc.js' );
var opcodes = require ( './opcodes.js' );
var factory = require ( '../common/factory.js' );

// These values are from ngl_package.h
var size_uint32_t = 4;
var size_pkg_header = size_uint32_t * 4;
var size_fixup_table_header = size_uint32_t;
var size_fixup = size_uint32_t * 3;
var size_patch_table_header = size_uint32_t;
var size_patch_header = size_uint32_t * 5;
var size_bytecode_header = size_uint32_t * 2;
var size_obj_header = size_uint32_t;

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
  // This function works by simulating the operations the virtual machine will
  // actually perform, and then extracts the stack location of variables from
  // that simulation.

  // This is more robust, and actually produces the correct result in all
  // currently used cases, compared to counting temporary values, which assumes
  // variables are not created while there are temporary values.

  // TODO(kzentner): Add some checks that simulated stack is the same before
  // and after a branch or jump. Of course, this condition should already be
  // true in correct code output, but a check here would help debugging.

  // This is the simulated stack. It will contain variable names, and 0 (which
  // is used to represent temporary values).
  var stack = [];
  var v;
  var TEMP = 0;

  var out = [];
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'res' ) {
      // The top values on the stack have been declared to be a variable.
      var to_remove = stack.slice ( -row.val.length );
      for ( var k in to_remove ) {
        misc.assert ( to_remove[k] === TEMP,
                      'Only temporary values should become variables.' );
        }

      // Replace the top row.val.length temporaries with variables.
      stack = stack.slice ( 0, -row.val.length );
      for ( v in row.val ) {
        stack.push ( row.val [ v ] );
        }
      }
    else if ( row.type === 'cmd' ) {
      var args = row.val[1].slice();

      // Expand the variables.
      for ( v in args ) {
        // Replace only variables, not labels.
        if ( typeof args[v] === 'string' && args[v][0] !== '.' ) {
          args[v] = stack.length - stack.lastIndexOf ( args[v] ) - 1;
          }
        }

      // Calculate the stack movement caused by this opcode. If it's positive,
      // add that many temporaries to the top of the stack.
      var cmd = row.val[0];
      var movement = 0;
      // The call_1 and stack_1 opcodes have variable stack movement, depending
      // on their arguments.
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
          stack.push ( TEMP );
          }
        }
      else {
        stack = stack.slice ( 0, stack.length + movement );
        }

      out.push ( make_row ( 'cmd', [ cmd, args ] ) );
      }
    else {
      // Propagate all other unmatched rows.
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
  // The number of rows which have been output. Note that this count includes
  // literals and labels, even though labels don't take up actual space.
  var row_count = 0;

  // The address where current_bunch should be written to. The reason this is
  // not just row_count + 1 is that we want to be able to output a bunch of
  // commands after having output their word-sized argument.
  // In particular, we would like to be able to put more commands in the same
  // bunch as a li_W.
  var bunch_out_index = 0;
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'label' ) {
      // If we encounter a label, output the current bunch.
      // This is because we need the label to be a valid jump address, so we
      // cannot compress together opcodes from before and after the label.
      // Basically, we need to prevent the label from being mid-bunch.
      push_bunch ( out, bunch_out_index, current_bunch );
      // We output 1 bunch.
      ++row_count;

      // We output the label.
      out[row_count] = row;
      ++row_count;

      bunch_out_index = row_count;
      current_bunch = [ ];

      // Output the label for later use in label expansion.
      out.push ( row );
      }
    else if ( row.type === 'cmd' ) {
      var size = 1 + row.val[1].length;
      // If the current_bunch would become to big from adding this opcode,
      // output the current bunch and begin a new one.
      if ( current_bunch.length + size > BUNCH_SIZE ) {
        push_bunch ( out, bunch_out_index, current_bunch );
        ++row_count;
        bunch_out_index = row_count;
        current_bunch = [ ];
        }
      current_bunch.push ( row.val[0] );
      current_bunch = current_bunch.concat ( row.val[1] );
      }
    else if ( row.type === 'literal' ) {
      // We can generate much better code here if we don't output a bunch.
      // This allows multiple variables to be initialized using one bunch of
      // li_w's, for example.
      out[ row_count + 1 ] = row;
      ++row_count;
      }
    }
  // Make sure to output the current_bunch (unless it's empty from a label at
  // the end of function). Note that actually *using* such a label would be
  // wrong, since there's no return opcode afterwards.
  // TODO(kzentner): Check the consistency of labels more thoroughly.
  if ( current_bunch.length > 0 ) {
    push_bunch ( out, bunch_out_index, current_bunch );
    }
  return out;
  }

function size_of_row ( row, literal_size ) {
  if ( row.type === 'label' ) {
    return 0;
    }
  else if ( row.type === 'literal' ) {
    return literal_size;
    }
  else if ( row.type === 'bunch' ) {
    return 1;
    }
  else {
    throw 'Could not determince precise size of row of type ' + row.type;
    }
  }

function find_label ( rows, start, label, literal_size ) {
  var dist = 0;
  var i;
  var row;

  for ( i = start; i < rows.length; i++ ) {
    row = rows[i];
    if ( row.type === 'label' && row.val === label ) {
      return dist - 1;
      }
    dist += size_of_row ( row, literal_size );
    }

  // Didn't find it going forwards, search backwards.
  dist = 0;
  for ( i = start; i >= 0; i-- ) {
    row = rows[i];
    if ( row.type === 'label' && row.val === label ) {
      return dist;
      }
    dist -= size_of_row ( row, literal_size );
    }

  throw 'Could not find label ' + label;
  }

function get_byte ( label, label_byte ) {
  // Checks that label is a string of the form .label_name[0], and returns
  // which byte of label it is.
  if ( typeof label_byte !== 'string' ) {
    return null;
    }
  var match = label_byte.match ( label );
  if ( match === null || match.index !== 0 ) {
    // This doesn't look like it's the label we were given, so return null.
    // This shouldn't ever occur in the usage below.
    return null;
    }
  var re = /\[([0-9])\]/;
  var res = re.exec ( label_byte.substr ( label.length ) );
  if ( res.index !== 0 ) {
    // We couldn't find the byte index, so return null.
    return null;
    }
  else {
    return parseInt ( res[1], 10 );
    }
  }

function expand_labels ( rows, literal_size ) {
  var out = [ ];
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'bunch' ) {
      // Replace label's which have been split up and placed in arguments and
      // compressed.
      // TODO(kzentner): Add a check that we have either only compressed code
      // (bunch) or uncompressed code (cmd).
      for ( var ib in row.val ) {
        var b = row.val[ib];
        // Check that it has the form of a label.
        if ( typeof b === 'string' && b[0] === '.' ) {
          // Extract the full label name from the byte label.
          var label = b.substr ( 0, b.length - '[0]'.length );
          var offset = find_label ( rows, r, label, literal_size );
          while ( true ) {
            var byt = get_byte ( label, row.val[ib] );
            if ( byt === null ) {
              // We couldn't determine the byte. Most likely, we hit an
              // argument besides the label and should exit here.
              break;
              }
            else {
              // Note that if offset it negative, this may result in large
              // positive values here.
              row.val[ib] = offset & ( 0xff << byt );
              }
            ++ib;
            }
          }
        }
      out.push ( row );
      }
    else if ( row.type === 'literal' ) {
      if ( row.val[0] === 'label' ) {
        // Also replace labels which have not been split up.
        row.val = [ 'integer', find_label ( rows, r, row.val[1], literal_size ) ];
        }
      out.push ( row );
      }
    }
  return out;
  }

function expand_ops ( rows ) {
  var out = [ ];
  for ( var r in rows ) {
    var row = rows[r];
    if ( row.type === 'bunch' ) {
      for ( var i in row.val ) {
        var byt = row.val[i];
        if ( typeof byt === 'string' ) {
          var code = opcodes.code[byt];
          if ( code === undefined ) {
            throw 'Could not expand opcode ' + byt;
            }
          else {
            row.val[i] = code;
            }
          }
        }
      }
      out.push ( row );
    }
  return out;
  }

var root = {
  write_bunch : function write_bunch ( buf, offset, bunch ) {
    misc.assert ( bunch.length === BUNCH_SIZE,
                  'Bunches should be ' + BUNCH_SIZE + ' bytes long.' );
    for ( var b in bunch ) {
      var byt = bunch[b];
      if ( byt >= 0 ) {
        buf.writeUInt8 ( byt, offset++ );
        }
      else {
        buf.writeInt8 ( byt, offset++ );
        }
      }
    },
  write_literal : function write_literal ( buf, offset, val ) {
    if ( val[0] === 'float' ) {
      if ( this.literal_size === 2 ) {
        buf.writeDoubleLE ( val[1], offset );
        }
      else if ( this.literal_size === 1 ) {
        buf.writeFloatLE ( val[1], offset );
        }
      else {
        throw 'Uknown target type!';
        }
      }
    else if ( val[0] === 'integer' ) {
      if ( this.literal_size === 2 ) {
        buf.writeInt32LE ( val[1] & 0xffffffff, offset );
        // Javascript seems to ignore shifts >= 32 bits.
        buf.writeInt32LE ( ( val[1] >>> 16 ) >> 16, offset + size_uint32_t );
        }
      else if ( this.literal_size === 1 ) {
        buf.writeInt32LE ( val[1], offset );
        }
      else {
        throw 'Uknown target type!';
        }
      }
    else if ( val[0] === 'uinteger' ) {
      if ( this.literal_size === 2 ) {
        buf.writeUInt32LE ( val[1] & 0xffffffff, offset );
        // Javascript seems to ignore shifts >= 32 bits.
        buf.writeUInt32LE ( ( val[1] >>> 16 ) >> 16, offset + size_uint32_t );
        }
      else if ( this.literal_size === 1 ) {
        buf.writeUInt32LE ( val[1], offset );
        }
      else {
        throw 'Uknown target type!';
        }
      }
    else if ( val[1] === 'lookup' ) {
      for ( var i = 0; i < BUNCH_SIZE * this.literal_size; i++ ) {
        buf.writeUInt8 ( 0xff, offset + i );
        }
      }
    },
  bytecode_size : function bytecode_size ( rows ) {
    var size = 0;
    for ( var r in rows ) {
      var row = rows[r];
      if ( row.type === 'literal' ) {
        size += BUNCH_SIZE * this.literal_size;
        }
      else if ( row.type === 'bunch' ) {
        size += BUNCH_SIZE;
        }
      else {
        throw 'Cannot determine the size of row with type ' + row.type;
        }
      }
    return size;
    },
  make_patch : function make_patch ( obj, rows ) {
    var header_size = this.factory.get_size ( 'ngl_buffer' );
    var buf = new buffer.Buffer ( header_size + this.bytecode_size ( rows ) );
    // This makes binary diffs cleaner.
    buf.fill ( 0 );
    var fixups = [];
    var offset = header_size;
    for ( var r in rows ) {
      var row = rows[r];
      if ( row.type === 'bunch' ) {
        this.write_bunch ( buf, offset, row.val );
        offset += BUNCH_SIZE;
        }
      if ( row.type === 'literal' ) {
        this.write_literal ( buf, offset, row.val );
        if ( row.val[0] === 'lookup' ) {
          fixups.push ( { offset : offset, val : row.val[1] } );
          }
        offset += BUNCH_SIZE * this.literal_size;
        }
      }
    return {
      kind : fixup_kinds.bytecode,
      buf : buf,
      fixups : fixups,
      offset : 0,
      id : obj.id,
      };
    },
  make_fn_header : function make_fn_header ( obj ) {
    var buf = new buffer.Buffer ( this.factory.get_size ( 'ngl_vm_func' ) );
    buf.fill ( 0 );
    var func = this.factory.wrap ( this.factory.get_type ( 'ngl_vm_func' ), {
        header : { refc : 0, type : 0xdeadbeef },
        code : 0,
        stack_space : 0
      } );
    func.set_offset ( 0 );
    func.write ( buf );
    var fixups = [];
    fixups.push ( {
      offset : func.get_slot ( 'header' ).get_slot ( 'type' ).offset,
      // TODO(kzentner): Don't hardcode 0, 3!
      val : { fixup_kind : 0, id : 3 },
      } );
    fixups.push ( {
      offset : func.get_slot ( 'code' ).offset,
      val : { fixup_kind : fixup_kinds.bytecode, id : obj.id },
      } );
    return {
      kind : fixup_kinds.vm_func,
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
    data = expand_labels ( data, this.literal_size );
    data = expand_ops ( data );

    var patch = this.make_patch ( obj, data );
    this.patches.push ( patch );
    var fn_header = this.make_fn_header ( obj );
    this.patches.push ( fn_header );
    },
  size_of_patches : function size_of_patches ( ) {
    size = 0;
    for ( var i in this.patches ) {
      var patch = this.patches[i];
      size += size_patch_header;
      size += patch.buf.length;
      }
    return size;
    },
  fixup_count : function fixup_count ( ) {
    var count = 0;
    for ( var i in this.patches ) {
      var patch = this.patches[i];
      count += patch.fixups.length;
      }
    return count;
    },
  output_size : function output_size ( ) {
    var size = 0;
    size += size_pkg_header;
    size += size_fixup_table_header;
    size += size_patch_table_header;
    size += this.size_of_patches ( );
    size += size_fixup * this.fixup_count ( );
    return size;
    },
  create_pkg : function create_pkg ( map, modules, target_type,
                                     common_defs_path ) {
    this.init ( this.factory );
    var self = this;
    this.target_type = target_type;
    if ( target_type === 'js' ) {
      this.literal_size = 1;
      }
    else if ( target_type === 'x86_64' ) {
      this.literal_size = 2;
      }
    else if ( target_type === 'ARM' ) {
      this.literal_size = 1;
      }

    this.factory.set_target_type ( target_type );
    this.factory.load_type_file ( common_defs_path + '/ngl_types.yaml' );

    map.each ( function ( name, obj ) {
      self.process_obj ( name, obj );
      } );


    var out_size = this.output_size ( );

    this.buffer = new buffer.Buffer ( out_size );
    // This makes binary diffs cleaner.
    this.buffer.fill ( 0 );

    this.sort_patches ( );

    this.write ( );

    return this.buffer;
    },
  sort_patches : function sort_patches ( ) {
    // TODO(kzentner): Sort patches! Required by the spec!
    },
  write_uint32 : function write_uint32 ( val ) {
    this.buffer.writeUInt32LE ( val, this.offset );
    this.offset += size_uint32_t;
    },
  write_header : function write_header ( ) {
    // Magic number found in ngl_vm/src/ngl_package.c
    this.write_uint32 ( 0x008fa5e5 );
    // Version
    this.write_uint32 ( 1 );
    // Location of fixup table
    this.write_uint32 ( size_pkg_header + size_patch_table_header + this.size_of_patches ( ) );
    // Location of patch table
    this.write_uint32 ( size_pkg_header );
    },
  write_patches : function write_patches ( ) {
    // Count
    this.write_uint32 ( this.patches.length );
    for ( var i in this.patches ) {
      var patch = this.patches[i];

      // Kind
      // TODO(kzentner): Make the compiler output objects besides bytecode.
      this.write_uint32 ( patch.kind );
      // ID
      this.write_uint32 ( patch.id );
      // TODO(kzentner): Make the compiler actually output patches, not just
      // new objects.
      // Offset
      this.write_uint32 ( 0 );

      var size = patch.buf.length;
      // to_delete
      this.write_uint32 ( size );
      // to_insert
      this.write_uint32 ( size );

      patch.offset = this.offset;
      patch.buf.copy ( this.buffer, this.offset, 0, patch.buf.length );
      this.offset += patch.buf.length;
      }
    },
  write_fixups : function write_fixups ( ) {
    var fixup_num = 0;
    // Count
    this.write_uint32 ( this.fixup_count ( ) );
    for ( var i in this.patches ) {
      var patch = this.patches[i];
      for ( var j in patch.fixups ) {
        var fix = patch.fixups[j];

        // Kind
        this.write_uint32 ( fix.val.fixup_kind );
        // ID
        this.write_uint32 ( fix.val.id );
        // Offset
        this.write_uint32 ( fix.offset + patch.offset );
        }
      }
    },
  write : function write ( ) {
    this.write_header ( );
    this.write_patches ( );
    this.write_fixups ( );
    },
  init : function init ( fact ) {
    this.patches = [];
    this.fixups = [];
    this.buffer = null;
    this.buffer_idx = 0;
    this.offset = 0;
    this.literal_size = 1;
    this.factory = fact;
    },
  };

function make ( ) {
  var out = Object.create ( root );
  out.init ( factory.make ( ) );
  return out;
  }

exports.make = make;
