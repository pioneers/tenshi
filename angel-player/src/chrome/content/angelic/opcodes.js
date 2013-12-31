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

function make_opcode ( code, name, argc, stack, func ) {
  return {
    code: code,
    name: name,
    argc: argc,
    stack: stack,
    func: func,
    };
  }

var opcodes = [
  make_opcode ( 0, 'noop', 0, 0,
  function noop ( state ) {
    // Since noop's are shifted into the bunch, or used to pad bunches, as soon
    // as a noop is encountered, the next bunch needs to be loaded.
    // It might make sense to refactor this, but this is also the must
    // efficient way to implement.
    if ( state.get_pc () === state.func.length ) {
      state.run = false;
      }
    else {
      state.check_bunch ( state.func[state.get_pc ( )] );
      state.bunch = state.func[state.get_pc ( ) ].slice ( 0 );
      state.move_pc ( 1 );
      }
    }),
  make_opcode ( 1, 'add', 1, -1,
  function add ( state ) {
    state.push ( state.pop ( ) + state.pop ( ) );
    }),
  make_opcode ( 2, 'li', 1, 1,
  function li ( state ) {
    state.push ( state.func[ state.get_pc ( ) ] );
    state.move_pc ( 1 );
    }),
  make_opcode ( 3, 'print', 2, 0,
  function print ( state ) {
    // This exists because we have not implemented external function calls yet.
    var arg_count = state.get_arg ( 1 );
    var i;
    var stack_length = state.stack.length;

    for ( i = 0; i < arg_count; i++ ) {
      misc.print ( 'VM:' + state.stack[stack_length - i - 1] );
      }
    }),
  make_opcode ( 4, 'end', 1, 0,
  function end ( state ) {
    state.run = false;
    }),
  make_opcode ( 5, 'mult', 1, -1,
  function add ( state ) {
    state.push ( state.pop ( ) * state.pop ( ) );
    }),
  make_opcode ( 6, 'div', 1, -1,
  function add ( state ) {
    state.push ( state.pop ( ) / state.pop ( ) );
    }),
  make_opcode ( 7, 'bn1', 2, -1,
  function bn1 ( state ) {
    if ( ! state.pop ( ) ) {
      state.move_pc ( state.get_arg ( 1 ) );
      }
    }),
  make_opcode ( 8, 'eq', 1, -1,
  function eq ( state ) {
    state.push ( state.pop ( ) === state.pop ( ) );
    }),
  make_opcode ( 9, 'dup', 2, 1,
    function dup ( state ) {
    state.push ( state.get_stack ( state.get_arg ( 1 ) ) );
    }),
  make_opcode ( 10, 'set', 2, -1,
    function set ( state ) {
    state.set_stack ( state.get_arg ( 1 ), state.pop ( ) );
    }),
  make_opcode ( 11, 'not', 1, 0,
  function not ( state ) {
    state.push ( ! state.pop ( ) );
    }),
  make_opcode ( 12, 'j1', 2, 0,
  function j1 ( state ) {
    state.move_pc ( state.get_arg ( 1 ) );
    }),
  make_opcode ( 13, 'pop', 1, -1,
  function pop ( state ) {
    state.pop ( );
    }),
  make_opcode ( 14, 'sub', 1, -1,
  function sub ( state ) {
    state.push ( -state.pop ( ) + state.pop ( ) );
    }),
  make_opcode ( 15, 'call', 2, 0,
  function call ( state ) {
    var arg_count = state.get_arg ( 1 );
    var func = state.get_stack ( arg_count );
    //misc.print ( 'func', func );
    if ( func.type === 'external' ) {
      var size = state.stack.length;
      var args = state.stack.splice ( size - arg_count, size );
      //var args = [];
      //misc.print ( 'args', args );
      func.func.apply ( null, args );
      for ( var i = 0; i < arg_count; i++ ) {
        state.pop ( );
        }
      // Pop the function.
      state.pop ( );
      }
    else if ( func.type === 'internal' ) {
      state.call_stack[state.call_stack_top++] = [state.func, state._pc, state.stack_top - ( arg_count + 1 ) ];
      state.func = func.code;
      state._pc = 0;
      }
    }),
  make_opcode ( 16, 'ret', 1, 0,
  function ret ( state ) {
    if ( state.call_stack_top === 0 ) {
      state.run = false;
      }
    else {
      var info = state.call_stack[--state.call_stack_top];
      state.func = info[0];
      state._pc = info[1];
      var ret = state.pop ( );
      while ( state.stack_top > info[2] ) {
        state.pop ( );
        }
      state.push ( ret );
      state.clear_bunch ( );
      }
    }),
  ];

function make_optables ( ops ) {
  var out = {
    op: {},
    code_to_obj: [],
    code: {},
    name: [],
    argc: [],
    func: [],
    };
  for ( var i in ops ) {
    out.op[ops[i].name] = ops[i];
    out.code_to_obj[ops[i].code] = ops[i];
    out.code[ops[i].name] = ops[i].code;
    out.argc[ops[i].code] = ops[i].argc;
    out.func[ops[i].code] = ops[i].func;
    out.name[ops[i].code] = ops[i].name;
    }
  return out;
  }

var tables = make_optables ( opcodes );

var code = tables.code;
var argc = tables.argc;
var func = tables.func;
var name = tables.name;
var code_to_obj = tables.code_to_obj;
var op = tables.op;

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['opcodes', 'op', 'code_to_obj', 'code', 'argc', 'func', 'name', 'tables'];
var exported_objects = [ opcodes ,  op ,  code_to_obj ,  code ,  argc ,  func ,  name ,  tables ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
