var kinds = require ( './kinds.js' );
var lookup = require ( './lookup.js' );
var misc = require ( './misc.js' );
var string_map = require ( './string_map.js' );

var root = {
  setupScopes: function setupScopes ( scopes ) {
    var statement_text_table = string_map.make ( {
      '=': assemble_assignment,
      'while': assemble_while,
      'return': assemble_return,
      '(': assemble_paren,
      } );
    statement_text_table.each ( function ( key, val ) {
      scopes.get ( 'statement' ).field_text ( key, 'assemble', val );
      } );
    var statement_type_table = string_map.make ( {
      'block': assemble_block,
      } );
    statement_type_table.each ( function ( key, val ) {
      scopes.get ( 'statement' ).field_type ( key, 'assemble', val );
      } );
    var expression_type_table = string_map.make ( {
      'number': assemble_number,
      'identifier':  assemble_identifier,
      } );
    expression_type_table.each ( function ( key, val ) {
      scopes.get ( 'expression' ).field_type ( key, 'assemble', val );
      } );
    var expression_text_table = string_map.make ( {
      '!=' : infix ( [ 'eq', 'not' ] ),
      '==' : infix ( [ 'eq' ] ),
      '+' : infix ( [ 'fadd' ] ),
      '-' : assemble_sub,
      '*' : infix ( [ 'fmul' ] ),
      '/' : infix ( [ 'fdiv' ] ),
      '%' : infix ( [ 'fmod' ] ),
      'not' : prefix ( [ 'not' ] ),
      '(': assemble_paren,
      'fn': assemble_fn,
      'if': assemble_if,
      } );
    expression_text_table.each ( function ( key, val ) {
      scopes.get ( 'expression' ).field_text ( key, 'assemble', val );
      } );
    },
  assemble_objs : function assemble_objs ( objs ) {
    var emitter = this.emitter;
    objs.forEach ( function ( obj ) {
      assemble ( obj, emitter );
      } );
    }
  };

function make ( emitter ) {
  return misc.obj_or ( {
    emitter : emitter,
    }, root );
  }

function assemble ( node, emitter ) {
  if ( typeof emitter !== 'object' ) {
    throw 'Assembler should be an object!';
    }
  if ( node.assemble !== undefined ) {
    node.assemble ( emitter );
    }
  else {
    misc.print ( node );
    throw 'Could not assemble node ' + node.text + '!';
    }
  }

function assemble_assignment ( emitter ) {
  if ( emitter.has_var ( this.left.text ) ) {
    assemble ( this.right, emitter );
    emitter.emit_cmd ( 'set_1', [ this.left.text ] );
    }
  else {
    assemble ( this.right, emitter );
    emitter.emit_result ( [ this.left.text ] );
    }
  }

function assemble_number ( emitter ) {
  return emitter.emit_li ( 'float', parseFloat ( this.text ) );
  }

function assemble_block ( emitter ) {
  var block = emitter.begin_block ( );
  this.children.forEach ( function ( child ) {
    assemble ( child, emitter );
    } );
  emitter.end_block ( block );
  }

function infix ( opseq ) {
  return function assemble_infix ( emitter ) {
    var left = assemble ( this.left, emitter );
    var right = assemble ( this.right, emitter );
    opseq.forEach ( function ( op ) {
      emitter.emit_cmd ( op );
      } );
    };
  }

function prefix ( postseq, prefunc ) {
  return function assemble ( emitter ) {
    if ( prefunc === undefined ) {
      prefunc ( emitter );
      }
    assemble ( this.right, emitter );
    postseq.forEach ( function ( op ) {
      emitter.emit_cmd ( op );
      } );
    };
  }

function assemble_sub ( emitter ) {
  function load_0 ( emitter ) {
    emitter.emit_li ( 'float', 0 );
    }
  if ( this.left === undefined ) {
    return prefix ( [ 'fsub' ], load_0 ).apply ( this, [ emitter ] );
    }
  else {
    return infix ( [ 'fsub' ] ).apply ( this, [ emitter ] );
    }
  }

function assemble_while ( emitter ) {
  var start = emitter.emit_label ( );
  assemble ( this.condition, emitter );
  var end = emitter.reserve_label ( );
  emitter.emit_bz ( end );
  assemble ( this.block, emitter );
  emitter.emit_j ( start );
  emitter.emit_label ( end );
  }

function assemble_if ( emitter ) {
  assemble ( this.condition, emitter );
  var end = emitter.reserve_label ( );
  emitter.emit_bz ( end );
  assemble ( this.block, emitter );
  emitter.emit_label ( end );
  }

function get_kind ( value ) {
  if ( value.text === 'fn' ) {
    return kinds.vm_func;
    }
  else {
    throw 'Could not determine kind of node!';
    }
  }

function assemble_identifier ( emitter ) {
  if ( this.variable === undefined ||
       this.variable.location === undefined ) {
    throw 'Cannot assemble un-analyzed variable ' + this.text;
    }
  var loc = this.variable.location;

  var value;
  var name;
  var kind;
  var id;

  if ( loc === 'stack' ) {
    emitter.emit_cmd ( 'dup_1', [ this.text ] );
    }
  else if ( loc === 'global' ) {
    value = this.variable.canonical_value;
    name = value.canonical_name;
    kind = get_kind ( value );
    id = value.object_id;
    emitter.emit_li ( 'lookup',
                      lookup.make ( name, kind, id ) );
    }
  else if ( loc === 'external' ) {
    value = this.variable.canonical_value;
    name = value.name;
    kind = kinds.external;
    id = value.object_id;
    emitter.emit_li ( 'lookup',
                      lookup.make ( name, kind, id ) );
    }
  else {
    throw 'Could not find location of variable ' + this.text + '.';
    }
  }

function assemble_paren ( emitter ) {
  if ( this.type === 'call' ) {
    assemble ( this.func, emitter );
    this.args.forEach ( function ( arg ) {
      assemble ( arg, emitter );
      } );
    emitter.emit_cmd ( 'call_1', [ this.args.length ] );
    }
  if ( this.ctxt === 'statement' ) {
    // Remove the function's return value from the stack, since it won't be needed.
    emitter.emit_cmd ( 'pop' );
    }
  }

function assemble_return ( emitter ) {
  assemble ( this.expr, emitter );
  emitter.emit_cmd ( 'ret' );
  }


// Function call stack interface:
// First, push the function.
// Then, push all the arguments from left to right.
// The call stack should look like:
// [ ..., func, arg0, arg1, ..., argk ]
// Then, perform the call opcode, with a single arg, the number of function
// arguments being used. The call opcode is responsible for removing the
// function from the stack, adding the return address to the call stack,
// growing the stack if necessary, and jumping into the function's code body.
//
// Note that the call opcode is also responsible for determining if the
// function on the top of the stack is an external function, in which case it
// passes the arguments from the stack to the external function.
// If this is in javascript, that involves using apply.
function assemble_fn ( emitter ) {
  emitter.begin_obj ( this.canonical_name, this.object_id );
  this.args.forEach ( function ( arg ) {
    emitter.add_var ( arg.text );
    } );
  assemble ( this.body, emitter );

  // TODO(kzentner): Is this the right place to put this logic?
  if ( this.canonical_name === 'main' ) {
    emitter.emit_cmd ( 'end' );
    }
  else {
    emitter.emit_cmd ( 'ret' );
    }
  emitter.end_obj ( );
  }

exports.make = make;
