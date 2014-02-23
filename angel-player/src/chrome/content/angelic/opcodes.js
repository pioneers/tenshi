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
  make_opcode ( 0x00, 'next', 0, 0,
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
    } ),
  make_opcode ( 0x01, 'dup1', 2, 1,
    function dup1 ( state ) {
    state.push ( state.get_stack ( state.get_arg ( 1 ) ) );
    } ),
  make_opcode ( 0x02, 'li4', 1, 1,
  function li ( state ) {
    state.push ( state.func[ state.get_pc ( ) ] );
    state.move_pc ( 1 );
    } ),
  make_opcode ( 0x03, 'set1', 2, -1,
    function set ( state ) {
    state.set_stack ( state.get_arg ( 1 ), state.pop ( ) );
    } ),
  make_opcode ( 0x04, 'pop', 1, -1,
  function pop ( state ) {
    state.pop ( );
    } ),
  make_opcode ( 0x05, 'call1', 2, 0,
  function call ( state ) {
    var arg_count = state.get_arg ( 1 );
    var func = state.get_stack ( arg_count );
    if ( func.type === 'external' ) {
      var size = state.stack.length;
      var args = state.stack.splice ( size - arg_count, size );
      var res = func.func.apply ( null, args );

      // Pop the args.
      for ( var i = 0; i < arg_count; i++ ) {
        state.pop ( );
        }

      // Pop the function.
      state.pop ( );

      // Push the result
      state.push ( res );
      }
    else if ( func.type === 'internal' ) {
      state.call_stack[state.call_stack_top++] = [state.func, state._pc, state.stack_top - ( arg_count + 1 ) ];
      state.func = func.code;
      state._pc = 0;
      }
    } ),
  make_opcode ( 0x06, 'ret', 1, 0,
  function ret ( state ) {
    if ( state.call_stack_top === 0 ) {
      state.run = false;
      }
    else {
      var info = state.call_stack[--state.call_stack_top];
      state.func = info[0];
      state._pc = info[1];
      var out = state.pop ( );
      while ( state.stack_top > info[2] ) {
        state.pop ( );
        }
      state.push ( out );
      state.clear_bunch ( );
      }
    } ),
  make_opcode ( 0x07, 'eq', 1, -1,
  function eq ( state ) {
    state.push ( state.pop ( ) === state.pop ( ) );
    } ),
  make_opcode ( 0x08, 'j1', 2, 0,
  function j1 ( state ) {
    state.move_pc ( state.get_arg ( 1 ) );
    } ),
  make_opcode ( 0x09, 'j2', 3, 0,
  function j2 ( state ) {
    state.move_pc ( state.get_arg ( 1 ) << 8 + state.get_arg ( 2 ) );
    } ),
  make_opcode ( 0x0a, 'j3', 4, 0,
  function j3 ( state ) {
    state.move_pc ( ( state.get_arg ( 1 ) << 8
                    + state.get_arg ( 2 ) << 8 ) 
                    + state.get_arg ( 3 ) );
    } ),
  make_opcode ( 0x0b, 'j4', 1, 0,
  function j4 ( state ) {
    state.move_pc ( state.func[ state.get_pc ( ) ] );
    } ),
  make_opcode ( 0x0c, 'bz1', 2, -1,
  function bz1 ( state ) {
    if ( ! state.pop ( ) ) {
      state.move_pc ( state.get_arg ( 1 ) );
      }
    } ),
  make_opcode ( 0x0d, 'bz2', 3, -1,
  function bz2 ( state ) {
    if ( ! state.pop ( ) ) {
      state.move_pc ( state.get_arg ( 1 ) << 8 + state.get_arg ( 2 ) );
      }
    } ),
  make_opcode ( 0x0e, 'bz3', 4, -1,
  function bz3 ( state ) {
    if ( ! state.pop ( ) ) {
      state.move_pc ( ( state.get_arg ( 1 ) << 8
                      + state.get_arg ( 2 ) << 8 ) 
                      + state.get_arg ( 3 ) );
      }
    } ),
  make_opcode ( 0x0f, 'bz4', 1, -1,
  function bz4 ( state ) {
    if ( ! state.pop ( ) ) {
      state.move_pc ( state.func[ state.get_pc ( ) ] );
      }
    } ),
  make_opcode ( 0x10, 'not', 1, 0,
  function not ( state ) {
    state.push ( ! state.pop ( ) );
    } ),

  make_opcode ( 0x11, 'fadd', 1, -1,
  function fadd ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left + right );
    } ),
  make_opcode ( 0x12, 'fsub', 1, -1,
  function fsub ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left - right );
    } ),
  make_opcode ( 0x13, 'fmul', 1, -1,
  function fmul ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left * right );
    } ),
  make_opcode ( 0x14, 'fdiv', 1, -1,
  function fdiv ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left / right );
    } ),
  make_opcode ( 0x15, 'fmod', 1, -1,
  function fmod ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left % right );
    } ),

  make_opcode ( 0x16, 'iadd', 1, -1,
  function iadd ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left + right );
    } ),
  make_opcode ( 0x17, 'isub', 1, -1,
  function isub ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left - right );
    } ),
  make_opcode ( 0x18, 'imul', 1, -1,
  function imul ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left * right );
    } ),
  make_opcode ( 0x19, 'idiv', 1, -1,
  function idiv ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left / right );
    } ),
  make_opcode ( 0x1a, 'imod', 1, -1,
  function imod ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left % right );
    } ),

  make_opcode ( 0x1b, 'uadd', 1, -1,
  function uadd ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left + right );
    } ),
  make_opcode ( 0x1c, 'usub', 1, -1,
  function usub ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left - right );
    } ),
  make_opcode ( 0x1d, 'umul', 1, -1,
  function umul ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left * right );
    } ),
  make_opcode ( 0x1e, 'udiv', 1, -1,
  function udiv ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left / right );
    } ),
  make_opcode ( 0x1f, 'umod', 1, -1,
  function umod ( state ) {
    var right = state.pop ( );
    var left = state.pop ( );
    state.push ( left % right );
    } ),

  make_opcode ( 0x20, 'refi', 1, 0,
  function refi ( state ) {
    // TODO(kzentner): Do we want to emulate reference counting in javascript.
    } ),
  make_opcode ( 0x21, 'refd', 1, 0,
  function refd ( state ) {
    // TODO(kzentner): Do we want to emulate reference counting in javascript.
    state.pop ( );
    } ),

  make_opcode ( 0x22, 'make1', 2, 0,
  function make1 ( state ) {
    var field_count = state.get_arg ( 1 );
    var size = state.stack.length;
    var obj = state.stack.splice ( size - field_count - 1, size );

    // Pop the fields.
    for ( var i = 0; i < field_count; i++ ) {
      state.pop ( );
      }

    // Pop the type tag.
    state.pop ( );

    state.push ( obj );
    } ),

  make_opcode ( 0x23, 'pushfree1', 1, 0,
  function pushfree1 ( state ) {
    // TODO(kzentner): Do we want to emulate reference counting in javascript?
    } ),

  make_opcode ( 0x24, 'popfree1', 1, 0,
  function popfree1 ( state ) {
    // TODO(kzentner): Do we want to emulate reference counting in javascript?
    } ),

  make_opcode ( 0x25, 'clone', 1, 0,
  function pushfree1 ( state ) {
    function copy ( obj ) {
      if ( typeof ( obj ) !== 'object' ) {
        return obj;
        }
      out = [];
      obj.forEach ( function ( val ) {
        // TODO(kzentner): Only recurse on mutable objects.
        out.push ( copy ( val ) );
        } );
      return out;
      }
    state.push ( copy ( state.pop ( ) ) );
    } ),

  make_opcode ( 0x26, 'safe', 1, 0,
  function safe ( state ) {
    // TODO(kzentner): Implement safe point instrumentation.
    } ),

  make_opcode ( 0x27, 'read1', 2, 0,
  function read1 ( state ) {
    state.push ( state.pop ( )[ state.get_arg ( 1 ) - 1 ] );
    } ),

  make_opcode ( 0x28, 'write1', 2, 0,
  function write1 ( state ) {
    var val = state.pop ( );
    var obj = state.pop ( );
    obj[ state.get_arg ( 1 ) - 1 ] = val;
    } ),

  make_opcode ( 0x29, 'stack1', 2, 0,
  function stack1 ( state ) {
    state.stack_top += state.get_arg ( 1 );
    } ),

  make_opcode ( 0x2a, 'noop', 1, 0,
  function noop ( state ) {
    } ),

  make_opcode ( 0x2b, 'end', 1, 0,
  function end ( state ) {
    state.run = false;
    } ),

  make_opcode ( 0x2c, 'debug', 1, 0,
  function debug ( state ) {
    state.debug = ! state.debug;
    } ),
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

exports.opcodes = opcodes;
exports.op = op;
exports.code_to_obj = code_to_obj;
exports.code = code;
exports.argc = argc;
exports.func = func;
exports.name = name;
exports.tables = tables;
