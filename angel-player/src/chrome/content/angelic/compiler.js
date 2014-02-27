var misc = require ( './misc.js' );
var opcodes = require ( './opcodes.js' );
var string_map = require ( './string_map.js' );
var fn = require ( './fn.js' );
var scope = require ( './scope.js' );
var type = require ( './type.js' );

var tables = opcodes.tables;
var ops = tables.op;
var next = ops.next.code;

function make_bunch ( a, b, c, d ) {
  if ( a === undefined ) {
    a = next;
    }
  if ( b === undefined ) {
    b = next;
    }
  if ( c === undefined ) {
    c = next;
    }
  if ( d === undefined ) {
    d = next;
    }
  return [a, b, c, d];
  }

var make_cgen = function make_cgen ( target ) {
  return {
    bunch: [],
    code: [],
    lookups: [],
    // Number of temporary values on the stack.
    tempc: 0,
    vars: [],
    target: target,
    finalize: function finalize ( ) {
      if ( this.bunch.length > 0 ) {
        this.emit_pending_bunch ( );
        this.bunch = [];
        }
      this.target.code = this.code;
      this.target.lookups = this.lookups;
      },
    handle_error: function ( expected ) {
      throw expected;
      },
    emit_lookup: function ( lookup ) {
      this.lookups.push ( lookup );
      },
    emit: function ( a, b, c, d ) {
      // These if statements replace opcode objects with their code numbers.
      if ( a !== undefined && typeof a !== 'number' ) {
        a = a.code;
        }
      if ( b !== undefined && typeof b !== 'number' ) {
        b = b.code;
        }
      if ( c !== undefined && typeof c !== 'number' ) {
        c = c.code;
        }
      if ( d !== undefined && typeof d !== 'number' ) {
        d = d.code;
        }
      var count = 0;
      var bunch = [];
      if ( a !== undefined ) {
        ++count;
        bunch.push ( a );
        }
      if ( b !== undefined ) {
        ++count;
        bunch.push ( b );
        }
      if ( c !== undefined ) {
        ++count;
        bunch.push ( c );
        }
      if ( d !== undefined ) {
        ++count;
        bunch.push ( d );
        }
      if ( this.bunch.length + count > 4 ) {
        this.emit_pending_bunch ( );
        this.bunch = bunch;
        }
      else {
        this.bunch = this.bunch.concat ( bunch );
        }
      },
    emit_pending_bunch: function ( ) {
      if ( this.bunch.length > 0 ) {
        while ( this.bunch.length < 4 ) {
          this.bunch.push ( 0 );
          }
        misc.assert ( this.bunch.length === 4 );
        this.code.push ( this.bunch );
        this.bunch = [];
        }
      },
    emit_bunch: function ( bunch ) {
      var i;

      for ( i in bunch ) {
        if ( typeof bunch[i] !== 'number' ) {
          bunch[i] = bunch[i].code;
          }
        }
      this.emit_pending_bunch ( );
      this.code.push ( bunch );
      },
    add_temp: function ( count ) {
      this.tempc += count;
      },
    has_var: function has_var ( name ) {
      return this.vars.lastIndexOf ( name ) !== -1;
      },
    var_idx: function var_idx ( name ) {
      var index = this.vars.lastIndexOf( name );
      if ( index === -1 ) {
        throw 'Cannot get stack index of variable ' + name + '.';
        }
      var out = this.vars.length - index - 1 + this.tempc;
      return out;
      },
    add_var: function add_var ( name ) {
      this.vars.push ( name );
      },
    get_pc: function get_pc ( ) {
      return this.code.length - 1;
      },
    'set': function set ( idx, val ) {
      misc.assert ( this.code[idx] === 'reserved', 'Only reserved code should be overwritten' );
      this.code[idx] = val;
      },
    reserve_bunch: function ( ) {
      this.emit_pending_bunch ( );
      this.code.push ( 'reserved' );
      return this.get_pc ( );
      },
    // Scope snapshots are used to implement block local variables.
    // Basically, to compile a block, get a snapshot, and your code in the
    // block, then restore the snapshot.
    get_scope_snapshot: function ( ) {
      misc.assert ( this.tempc === 0, "There should be no temporary values at a scope snapshot." );
      return this.vars.slice ( 0 );
      },
    apply_scope_snapshot: function ( snapshot ) {
      // This one is not true in the case of if expressions.
      //misc.assert ( this.tempc === 0, "There should be no temporary values at a scope snapshot." );
      misc.assert ( snapshot.length <= this.vars.length, "Applied snapshot should have at most same number of vars." );
      var extra_now = false;
      for ( var i in this.vars ) {
        if ( ! extra_now && snapshot[i] === undefined ) {
          extra_now = true;
          }
        if ( ! extra_now ) {
          misc.assert ( snapshot[i] === this.vars[i], "Snapshot should have same prefix as current scope." );
          }
        else {
          misc.assert ( snapshot[i] === undefined, "There should be no more vars after the current scope ends." );
          this.emit ( ops.pop );
          }
        }
      this.vars = snapshot;
      },
    };
  };

function compile_assignment ( compiler ) {
  var cgen = compiler.cgen;
  if ( cgen.has_var ( this.left.text ) ) {
    compiler.compile ( this.right );
    cgen.emit ( ops.set1, cgen.var_idx ( this.left.text ) );
    cgen.add_temp ( -1 );
    }
  else {
    misc.assert ( cgen.tempc === 0, "There should be no temporaries when a variable is created." );
    compiler.compile ( this.right );
    cgen.add_var ( this.left.text );
    cgen.add_temp ( -1 );
    }
  }

function compile_number ( compiler ) {
  var cgen = compiler.cgen;
  cgen.emit ( ops.liw );
  cgen.emit_bunch ( parseInt ( this.text, 10 ) );
  cgen.add_temp ( 1 );
  }

function compile_block ( compiler ) {
  for ( var c in this.children ) {
    compiler.compile ( this.children[c] );
    }
  }

function compile_not_equal ( compiler ) {
  var cgen = compiler.cgen;
  compiler.compile ( this.left );
  compiler.compile ( this.right );
  cgen.emit ( ops.eq, ops.not );
  cgen.add_temp ( -1 );
  }
function compile_equal ( compiler ) {
  var cgen = compiler.cgen;
  compiler.compile ( this.left );
  compiler.compile ( this.right );
  cgen.emit ( ops.eq );
  cgen.add_temp ( -1 );
  }

function insert_jump ( cgen, difference ) {
  // TODO(kzentner): Use correct sized jump opcode.
  cgen.emit ( ops.j1, difference );
  }

function insert_bz ( cgen, opcode, target ) {
  // TODO(kzentner): Use correct sized branch.
  cgen.set ( opcode, make_bunch ( ops.bz1.code, target ) );
  }

function compile_while ( compiler ) {
  var cgen = compiler.cgen;
  var start = cgen.get_pc ( );
  compiler.compile ( this.condition );
  var branch = cgen.reserve_bunch ( );
  cgen.add_temp ( -1 );
  var scope_snapshot = cgen.get_scope_snapshot ( );
  for ( var c in this.block.children ) {
    compiler.compile ( this.block.children[c] );
    }
  cgen.apply_scope_snapshot ( scope_snapshot );
  cgen.emit_pending_bunch ( );
  insert_jump ( cgen, start - cgen.get_pc ( ) - 1 );
  cgen.emit ( ops.next );
  cgen.emit ( ops.next );
  cgen.emit ( ops.next );
  cgen.emit_pending_bunch ( );
  insert_bz ( cgen, branch, cgen.get_pc ( ) - branch );
  }

function compile_if ( compiler ) {
  var cgen = compiler.cgen;
  var start = cgen.get_pc ( );
  compiler.compile ( this.condition );
  var branch = cgen.reserve_bunch ( );
  cgen.add_temp ( -1 );
  var scope_snapshot = cgen.get_scope_snapshot ( );
  for ( var c in this.block.children ) {
    compiler.compile ( this.block.children[c] );
    }
  cgen.apply_scope_snapshot ( scope_snapshot );
  cgen.emit ( ops.next );
  cgen.emit_pending_bunch ( );
  insert_bz ( cgen, branch, cgen.get_pc ( ) - branch );
  }

function compile_add ( compiler ) {
  var cgen = compiler.cgen;
  compiler.compile ( this.left );
  compiler.compile ( this.right );
  cgen.emit ( ops.fadd );
  cgen.add_temp ( -1 );
  }

function compile_sub ( compiler ) {
  var cgen = compiler.cgen;
  compiler.compile ( this.left );
  compiler.compile ( this.right );
  cgen.emit ( ops.fsub );
  cgen.add_temp ( -1 );
  }

function make_lookup ( target, dest, index ) {
  return {
    target: target,
    dest: dest,
    index: index,
    };
  } 

function compile_identifier ( compiler ) {
  var cgen = compiler.cgen;
  if ( this.variable === undefined ) {
    misc.print ( this );
    }
  var location = this.variable.location;
  if ( location === 'stack' ) {
    var idx = cgen.var_idx ( this.text );
    cgen.emit ( ops.dup1, idx );
    cgen.add_temp ( 1 );
    }
  else if ( location === 'global' || 
            location === 'external' ) {
    cgen.emit ( ops.liw );
    cgen.add_temp ( 1 );
    cgen.emit_lookup ( make_lookup (
        this,
        cgen.target,
        cgen.reserve_bunch ( ) ) );
    }
  else {
    misc.print ( cgen.vars );
    misc.print ( this );
    throw 'Could not find location of variable ' + this.text + '.';
    }
  }

function compile_paren_expr ( compiler ) {
  var cgen = compiler.cgen;
  var k;
  if ( this.type == 'call' ) {
    compiler.compile ( this.func );
    for ( k in this.args ) {
      compiler.compile ( this.args[k] );
      }
    cgen.emit ( ops.call1, this.args.length );
    cgen.emit_pending_bunch ( );
    // + 1 for the function, -1 for the return value.
    cgen.add_temp ( -this.args.length );
    }
  }

function compile_paren_statement ( compiler ) {
  var cgen = compiler.cgen;
  var k;
  if ( this.type == 'call' ) {
    compiler.compile ( this.func );
    for ( k in this.args ) {
      compiler.compile ( this.args[k] );
      }
    cgen.emit ( ops.call1, this.args.length );

    // Remove the function's return value from the stack, since it won't be needed.
    cgen.emit ( ops.pop );
    cgen.emit_pending_bunch ( );
    // + 1 for the function, no return value (since this is a statement).
    cgen.add_temp ( - ( 1 + this.args.length ) );
    }
  }


function compile_return ( compiler ) {
  var cgen = compiler.cgen;
  compiler.compile ( this.expr );
  cgen.emit ( ops.ret );
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
function compile_fn ( compiler ) {
  var cgen = compiler.push_cgen ( this );
  for ( var a in this.args ) {
    cgen.add_var ( this.args[a].text );
    }
  for ( var k in this.body.children ) {
    compiler.compile ( this.body.children[k] );
    }
  cgen.emit ( ops.ret );
  compiler.pop_cgen ( );
  }

var root = {

  setupScopes: function setupScopes ( scopes ) {
    var statement_text_table = string_map.make ( {
      '=': compile_assignment,
      'while': compile_while,
      'return': compile_return,
      '(': compile_paren_statement,
      } );
    statement_text_table.each ( function ( key, val ) {
      scopes.get ( 'statement' ).field_text ( key, 'compile', val );
      } );
    var statement_type_table = string_map.make ( {
      'block': compile_block,
      } );
    statement_type_table.each ( function ( key, val ) {
      scopes.get ( 'statement' ).field_type ( key, 'compile', val );
      } );
    var expression_type_table = string_map.make ( {
      'number': compile_number,
      'identifier':  compile_identifier,
      } );
    expression_type_table.each ( function ( key, val ) {
      scopes.get ( 'expression' ).field_type ( key, 'compile', val );
      } );
    var expression_text_table = string_map.make ( {
      '!=': compile_not_equal,
      '+': compile_add,
      '-': compile_sub,
      '(': compile_paren_expr,
      'fn': compile_fn,
      'if': compile_if,
      } );
    expression_text_table.each ( function ( key, val ) {
      scopes.get ( 'expression' ).field_text ( key, 'compile', val );
      } );
    },

  compile: function ( node ) {
    if ( node.compile === undefined ) {
      misc.print ( node );
      throw 'Could not compile node.';
      }
    else {
      return node.compile ( this );
      }
    },

  compile_objs: function compile_objs ( objs ) {
    for ( var o in objs ) {
      var obj = objs[o];
      obj.compile ( this );
      }
    },

  push_cgen: function push_cgen ( target ) {
    this.cgen = make_cgen ( target );
    this.cgen_stack.push ( this.cgen );
    return this.cgen;
    },

  pop_cgen: function pop_cgen ( ) {
    this.cgen.finalize ( );
    this.cgen = this.cgen_stack.pop ( );
    },
  };

var make = function make ( ) {
  return misc.obj_or ( Object.create ( root ), {
    cgen_stack: [],
    cgen: null
    });
  };

exports.make = make;
