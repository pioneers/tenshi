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

//
// This module provides the executor object, which is created to execute a
// library. It passes itself to opcodes, which call methods on it to modify the
// vm state.
//

var opcodes = require( './opcodes.js' );
var misc = require ( './misc.js' );


var tables = opcodes.tables;
function shift_bunch ( bunch, amount ) {
  for ( var i = 0; i < amount; i++ ) {
    var removed = bunch.shift ( );
    bunch[3] = 0;
    }
  }


function make ( ) {
  return {
    func: [],
    stack_top: 0,
    call_stack_top: 0,
    frame: 0,
    _pc: 0,
    run: true,
    debug: false,
    stack: [],
    call_stack: [],
    bunch: [0, 0, 0, 0],
    execute: function ( lib ) {
      var main = lib.get_obj ( 'fn', 'main' );
      this.run_code ( main.data );
      },
    run_code: function ( code ) {
      //misc.print ( 'Executing code', code );
      //this.debug = true;
      this.func = code;
      while ( this.run ) {
        var op = this.bunch[0];
        if ( this.debug ) {
          this.debug_print ( );
          }
        // this is the state object.
        tables.func[op] ( this );
        shift_bunch ( this.bunch, tables.argc[op] );
        }
      },
    push: function ( val ) {
      this.stack[this.stack_top] = val;
      this.stack_top += 1;
      },
    get_pc: function ( ) {
      return this._pc;
      },
    get_arg: function ( idx ) {
      return this.bunch [ idx ];
      },
    move_pc: function ( amount ) {
      this._pc += amount;
      },
    pop: function ( ) {
      this.stack_top -= 1;
      var out = this.stack[this.stack_top];
      this.stack = this.stack.slice (0, this.stack_top);
      return out;
      },
    get_stack: function get_stack ( index ) {
      return this.stack[this.stack_top - index - 1];
      },
    set_stack: function set_stack ( index, val ) {
      this.stack[this.stack_top - index] = val;
      },
    check_bunch: function ( bunch ) {
      if ( bunch === undefined ) {
        misc.print ( 'Invalid opcode bunch:', bunch );
        }
      },
    clear_bunch: function ( ) {
      this.bunch = [ 0, 0, 0, 0 ];
      },
    debug_print: function ( ) {
      var op = this.bunch[0];
      if ( op === 0 ) {
        return;
        }
      misc.print ( this.stack );
      for ( var s in this.stack ) {
        if (this.stack[s] === undefined) {
          throw 'Undefined on stack!';
          }
        }
      var args = ' ';
      for ( var i = 1; i < tables.argc[op]; i++) {
        args += this.get_arg ( i ) + ' ';
        }
      misc.print ( tables.name[op] + args );
      },
    };
  }


// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
