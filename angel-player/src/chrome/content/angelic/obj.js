var string_map = require ( './string_map.js' );

var make = function make ( name ) {
  if ( name === undefined ) {
    name = '#obj';
    }
  return {
    name : name,
    data : [],
    relocations : [],
    id : 0,
    last_label_id : 0,
    last_stack_id : 0,
    labels : string_map.make ( ),
    to_asm : function to_asm ( indent, mindent ) {
      return obj_to_asm ( this, indent, mindent );
      },
    };
  };

function get_label_end ( rows, start_index ) {
  var end_label;
  for ( end_label = start_index + 1; end_label < rows.length; end_label++ ) {
    if ( rows[end_label].type === 'label_end' &&
         rows[end_label].val === rows[start_index].val ) {
      return end_label + 1;
      }
    }
  if ( end_label === rows.length ) {
    for ( end_label = start_index + 1; end_label < rows.length; end_label++ ) {
      if ( rows[end_label].type === 'label' ) {
        // Don't skip the next label's start.
        return end_label - 1;
        }
      }
    }
  return end_label;
  }

function cmd_to_asm ( val ) {
  var out = val[0];
  if ( val[1] !== undefined ) {
    val[1].forEach ( function ( v ) {
      if ( typeof v === 'string' ) {
        out += ' $(' + v + ')';
        }
      else {
        out += ' ' + v;
        }
      } );
    }
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

function res_to_asm ( res ) {
  var out = '';
  for ( var i = 0; i < res.length - 1; i++ ) {
    out += '$(' + res[i] + '), ';
    }
  out += '$(' + res[res.length - 1] + ')';
  return out;
  }

function rows_to_asm ( rows, indent, mindent ) {
  if ( indent === undefined ) {
    indent = '  ';
    }
  if ( mindent === undefined ) {
    mindent = indent;
    }
  out = '';
  for ( var i = 0; i < rows.length; i++ ) {
    var row = rows[i];
    if ( row.type === 'label' ) {
      out += indent + row.val + ':\n';
      var end_label = get_label_end ( rows, i );
      out += rows_to_asm ( rows.slice ( i + 1, end_label ), indent + mindent, mindent );
      i = end_label;
      }
    else if ( row.type === 'end_label' ) {
      throw 'Label ended without having begun.';
      }
    else if ( row.type === 'res' ) {
      out += indent + ' -> ' + res_to_asm ( row.val ) + '\n';
      }
    else if ( row.type === 'cmd' ) {
      out += indent + cmd_to_asm ( row.val ) + '\n';
      }
    else if ( row.type === 'li' ) {
      out += indent + cmd_to_asm ( [ 'li.w' ] ) + '\n';
      if ( typeof row.val[1] === 'string' ) {
        out += indent + lookup_to_asm ( row.val[1] ) + '\n'; 
        }
      else {
        out += indent + literal_to_asm ( row.val ) + '\n';
        }
      }
    else if ( row.type === 'lookup' ) {
      out += indent + lookup_to_asm ( row.val[1] ) + '\n'; 
      }
    else if ( row.type === 'j' ) {
      out += indent + cmd_to_asm ( [ 'j.w' ] ) + '\n';
      out += indent + row.val + '\n';
      }
    else if ( row.type === 'bz' ) {
      out += indent + cmd_to_asm ( [ 'bz.w' ] ) + '\n';
      out += indent + row.val + '\n';
      }
    else {
      // This line is very useful for debugging this function.
      //out += indent + JSON.stringify ( row ) + '\n';
      }
    }
  return out;
  }

function obj_to_asm ( obj, indent, mindent ) {
  return rows_to_asm ( obj.data, indent, mindent );
  }

exports.make = make;
