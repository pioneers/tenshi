// Import Machinery used to make xulrunner and node.js act the same.
var require = require;
var export_vals;

if ( require === undefined ) {
  export_vals = function ( names, vals ) {
    };
  require = function ( filename ) {
    var module = {};
    Components.utils.import ( 'chrome://angel-player/content/angelic/' + filename, module );
    return module;
    };
  }
else {
  export_vals = function ( names, vals ) {
    var n;

    for ( n in names ) {
      exports[names[n]] = vals[n];
      }
    };
  }
// End Import Machinery

var misc = require ( './misc.js' );
var opcodes = require ( './opcodes.js' );
var string_map = require ( './string_map.js' );
var library = require ( './library.js' );
var fn = require ( './fn.js' );
var scope = require ( './scope.js' );
var type = require ( './type.js' );

var tables = opcodes.tables;
var ops = tables.op;
var noop = ops.noop.code;

function make_bunch ( a, b, c, d ) {
  if ( a === undefined ) {
    a = noop;
    }
  if ( b === undefined ) {
    b = noop;
    }
  if ( c === undefined ) {
    c = noop;
    }
  if ( d === undefined ) {
    d = noop;
    }
  return [a, b, c, d];
  }

var make_cgen = function make_cgen ( target ) {
  return {
    code: [],
    // Number of temporary values on the stack.
    tempc: 0,
    vars: [],
    target: target,
    finalize: function finalize ( ) {
      this.target.data = this.code;
      },
    handle_error: function ( expected ) {
      throw expected;
      },
    emit: function ( a, b, c, d ) {
      // These if statements replace opcode objects with their code numbers.
      if ( a instanceof Object ) {
        a = a.code;
        }
      if ( b instanceof Object ) {
        b = b.code;
        }
      if ( c instanceof Object ) {
        c = c.code;
        }
      if ( d instanceof Object ) {
        d = d.code;
        }
      this.emit_bunch ( make_bunch ( a, b, c, d ) );
      },
    emit_bunch: function ( bunch ) {
      var i;

      for (i in bunch) {
        if (bunch[i] instanceof Object) {
          bunch[i] = bunch[i].code;
          }
        }
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
      var out = this.vars.length - index - 1 + this.tempc;
      return out;
      },
    add_var: function add_var ( name ) {
      misc.assert ( this.tempc === 1, "There should be precisely one temporary when a variable is added." );
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

function compile_assignment ( cgen ) {
   if ( cgen.has_var ( this.left.text ) ) {
     this.right.compile ( cgen );
     cgen.emit ( ops.set, cgen.var_idx ( this.left.text ) );
     cgen.add_temp ( -1 );
     }
   else {
     misc.assert ( cgen.tempc === 0, "There should be no temporaries when a variable is created." );
     this.right.compile ( cgen );
     cgen.add_var ( this.left.text );
     cgen.add_temp ( -1 );
     }
  }

function compile_number ( cgen ) {
    cgen.emit ( ops.li );
    cgen.emit_bunch ( parseInt ( this.text, 10 ) );
    cgen.add_temp ( 1 );
  }

function compile_block ( compiler ) {
  for ( var c in this.children ) {
    this.children[c].compile ( compiler );
    }
  }

function compile_not_equal ( cgen ) {
  this.left.compile ( cgen );
  this.right.compile ( cgen );
  cgen.emit ( ops.eq, ops.not );
  cgen.add_temp ( -1 );
  }

function compile_while ( cgen ) {
  var start = cgen.get_pc ( );
  this.condition.compile ( cgen );
  var branch = cgen.reserve_bunch ( );
  cgen.add_temp ( -1 );
  var scope_snapshot = cgen.get_scope_snapshot ( );
  for ( var c in this.block.children ) {
    this.block.children[c].compile ( cgen );
    }
  cgen.apply_scope_snapshot ( scope_snapshot );
  cgen.emit ( ops.j1, start - cgen.get_pc ( ) - 1 );
  cgen.emit ( ops.noop );
  cgen.set ( branch, make_bunch ( ops.bn1.code, cgen.get_pc ( ) - branch ) );
  }

function compile_if ( cgen ) {
  var start = cgen.get_pc ( );
  this.condition.compile ( cgen );
  var branch = cgen.reserve_bunch ( );
  cgen.add_temp ( -1 );
  var scope_snapshot = cgen.get_scope_snapshot ( );
  for ( var c in this.block.children ) {
    this.block.children[c].compile ( cgen );
    }
  cgen.apply_scope_snapshot ( scope_snapshot );
  cgen.emit ( ops.noop );
  cgen.set ( branch, make_bunch ( ops.bn1.code, cgen.get_pc ( ) - branch ) );
  }

function compile_add ( cgen ) {
  this.left.compile ( cgen );
  this.right.compile ( cgen );
  cgen.emit ( ops.add );
  cgen.add_temp ( -1 );
  }

function compile_sub ( cgen ) {
  this.left.compile ( cgen );
  this.right.compile ( cgen );
  cgen.emit ( ops.sub );
  cgen.add_temp ( -1 );
  }


function compile_identifier ( cgen ) {
  var idx = cgen.var_idx ( this.text );
  cgen.emit ( ops.dup, idx );
  cgen.add_temp ( 1 );
  }

function compile_paren ( cgen ) {
  var k;
  if ( this.type == 'call' ) {
    for ( k in this.args ) {
      this.args[k].compile ( cgen );
      }
    // TODO(kzentner): replace this hack with actual external function calls.
    if ( this.func.text === 'print' ) {
      cgen.emit ( ops.print, this.args.length );
      }
    else {
      cgen.emit ( ops.call, this.args.length );
      }
    }
  }

function compile_return ( cgen ) {
  // TODO(kzentner): Implement function returns.
  }

function compile_fn ( compiler ) {
  var a_fn = fn.make ( );
  var cgen = compiler.push_cgen ( a_fn );
  var k;
  for ( k in this.body.children ) {
    this.body.children[k].compile ( cgen );
    }
  compiler.lib.add_obj ( this.name, compiler.lib.name_type_idx ( 'fn' ), a_fn );
  compiler.pop_cgen ( );
  }

var root = {

  setupScopes: function setupScopes ( scopes ) {
    var statement_text_table = string_map.make ( {
      '=': compile_assignment,
      'while': compile_while,
      'return': compile_return,
      } );
    statement_text_table.each ( function ( key, val ) {
      scopes.get ( 'statement' ).field_text ( key, 'compile', val );
      } );
    var statement_type_table = string_map.make ( {
      'block': compile_block,
      'top_level': compile_block,
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
      '(': compile_paren,
      'fn': compile_fn,
      'if': compile_if,
      } );
    expression_text_table.each ( function ( key, val ) {
      scopes.get ( 'expression' ).field_text ( key, 'compile', val );
      } );
    },

  compile: function compile ( ast ) {
    var fn_t = type.make ( 'fn' );
    var num_t = type.make ( 'num' );

    // Set up the initial types.
    // This should probably be refactored.

    this.lib.add_type ( 'fn', fn_t );
    this.lib.add_type ( 'num', num_t );
    ast.compile ( this );
    return this.lib;
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
    lib: library.make ( ),
    cgen_stack: [],
    cgen: null
    });
  };

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
