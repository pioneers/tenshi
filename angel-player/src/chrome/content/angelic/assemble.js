var string_map = require ( './string_map.js' );
var misc = require ( './misc.js' );

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
    var assembler = this.assembler;
    objs.forEach ( function ( obj ) {
      assemble ( obj, assembler );
      } );
    }
  }

function make ( assembler ) {
  return misc.obj_or ( {
    assembler : assembler,
    }, root );
  }

function assemble ( node, assembler ) {
  if ( typeof assembler !== 'object' ) {
    'lol' = 'nope';
    throw 'Assembler should be an object!';
    }
  if ( node.assemble !== undefined ) {
    node.assemble ( assembler );
    }
  else {
    misc.print ( node );
    throw 'Could not assemble node ' + node.text + '!';
    }
  }

function assemble_assignment ( assembler ) {
  if ( assembler.has_var ( this.left.text ) ) {
    assemble ( this.right, assembler );
    assembler.emit_cmd ( 'set', [ this.left.text ] );
    }
  else {
    assemble ( this.right, assembler );
    assembler.emit_result ( [ this.left.text ] );
    }
  }

function assemble_number ( assembler ) {
  return assembler.emit_li ( 'float', parseFloat ( this.text ) );
  }

function assemble_block ( assembler ) {
  var block = assembler.begin_block ( );
  this.children.forEach ( function ( child ) {
    assemble ( child, assembler );
    } );
  assembler.end_block ( block );
  }

function infix ( opseq ) {
  return function assemble_infix ( assembler ) {
    var left = assemble ( this.left, assembler );
    var right = assemble ( this.right, assembler );
    opseq.forEach ( function ( op ) {
      assembler.emit_cmd ( op );
      } );
    };
  }

function prefix ( postseq, prefunc ) {
  return function assemble ( assembler ) {
    if ( prefunc === undefined ) {
      prefunc ( assembler );
      }
    assemble ( this.right, assembler );
    postseq.forEach ( function ( op ) {
      assembler.emit_cmd ( op );
      } );
    };
  }

function assemble_sub ( assembler ) {
  if ( this.left === undefined ) {
    function load_0 ( assembler ) {
      assembler.emit_li ( 'float', 0 );
      }
    return prefix ( [ 'fsub' ], load_0 ).apply ( this, [ assembler ] );
    }
  else {
    return infix ( [ 'fsub' ] ).apply ( this, [ assembler ] );
    }
  }

function assemble_while ( assembler ) {
  var start = assembler.emit_label ( );
  assemble ( this.condition, assembler );
  var end = assembler.reserve_label ( );
  assembler.emit_bz ( end );
  assemble ( this.block, assembler );
  assembler.emit_j ( start );
  assembler.emit_label ( end );
  }

function assemble_if ( assembler ) {
  assemble ( this.condition, assembler );
  var end = assembler.reserve_label ( );
  assembler.emit_bz ( end );
  assemble ( this.block, assembler );
  assembler.emit_label ( end );
  }

function assemble_identifier ( assembler ) {
  if ( this.variable === undefined ||
       this.variable.location === undefined ) {
    misc.print ( this );
    throw 'Cannot assemble un-analyzed variable ' + this.text;
    }
  var loc = this.variable.location;
  if ( loc === 'stack' ) {
    assembler.emit_cmd ( 'dup1', [ this.text ] );
    }
  else if ( loc === 'global' ) {
    assembler.emit_li ( 'lookup', this.variable.canonical_value.canonical_name );
    }
  else if ( loc === 'external' ) {
    assembler.emit_li ( 'lookup', this.variable.canonical_value.name );
    }
  else {
    throw 'Could not find location of variable ' + this.text + '.';
    }
  }

function assemble_paren ( assembler ) {
  if ( this.type === 'call' ) {
    assemble ( this.func, assembler );
    this.args.forEach ( function ( arg ) {
      assemble ( arg, assembler );
      } );
    assembler.emit_cmd ( 'call1', [ this.args.length ] );
    }
  if ( this.ctxt === 'statement' ) {
    // Remove the function's return value from the stack, since it won't be needed.
    assembler.emit_cmd ( 'pop' );
    }
  }

function assemble_return ( assembler ) {
  assemble ( this.expr, assembler );
  assembler.emit_cmd ( 'ret' );
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
function assemble_fn ( assembler ) {
  assembler.begin_obj ( this.canonical_name );
  this.args.forEach ( function ( arg ) {
    assembler.add_var ( arg.text );
    } );
  assemble ( this.body, assembler );
  assembler.emit_cmd ( 'ret' );
  assembler.end_obj ( );
  }

exports.make = make;
