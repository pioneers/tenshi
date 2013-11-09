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
// This module provides type inference on an Angelic AST.
//

var misc = require ( './misc.js' );
var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );

var type_var_idx = 0;

// Represents the types of unknown variables or expressions.
function make_type_var ( ) {
  type_var_idx += 1;
  return {
    kind : 'var',
    name : '#var' + type_var_idx,
    instance : null,
    };
  }

// Represents both final, concrete types and composite types (including
// functions).
function make_type_op ( name, types ) {
  return {
    kind : 'op',
    name : name,
    types: types || [],
    };
  }

// Creates a typing environment. Basically, on of these should be used per
// module.
function make_env ( table ) {
  var env = {
    // Represents known variables and types.
    table : scope.make ( ),
    // Holds which variables are not-generic.
    // This prevents the algorithm from becoming Turing complete.
    non_generic : [],
    // Maps from variables we have encountered to pairs of which scope they
    // appeared in and their type variable.
    unknown : string_map.make ( ),
    // Adds a variable.
    load_text : function ( table ) {
      this.table.load_text ( table );
      },
    // Adds a type.
    load_type : function ( table ) {
      this.table.load_type ( table );
      },
    // Gets a type.
    get_type : function ( type ) {
      return this.table.get_type ( type );
      },
    // Gets a variable which is guaranteed to exist.
    get_text : function  ( text ) {
      var retrieved = this.table.get_text ( text );
      if ( retrieved === undefined ) {
        return this.get_unknown ( text );
        }
      else {
        return retrieved;
        }
      },
    // Gets a variable which may not have been declared yet.
    get_unknown : function  ( text ) {
      var type = this.make_type_var ( );
      this.unknown.set ( text, [this.table, type] );
      return type;
      },
    // Sets a variables type. Used to determine the scope of previously unknown
    // variables.
    set_text : function ( text, val ) {
      var pair;
      if ( this.unknown.has ( text ) ) {
        pair = this.unknown.get ( text );
        if ( this.table.is_above ( pair[0] ) ) {
          this.table.set_text ( text, pair[1] );
          this.unknown.delete ( text );
          return pair[1];
          }
        }
      this.table.set_text ( text, val );
      return val;
      },
    // Set a type name to a type.
    set_type : function ( type, val ) {
      return this.table.set_type ( type, val );
      },
    // Create a type variable.
    make_type_var : function ( ) {
      return make_type_var ( );
      },
    // Checks whether a type in part of a composite type.
    // Also used to prevent Turing completeness.
    occurs_in_type : function ( type_var, other_var ) {
      var pruned_other = this.prune ( other_var );
      if ( pruned_other === type_var ) {
        return true;
        }
      else if ( pruned_other.kind === 'op' ) {
        return this.occurs_in ( type_var, pruned_other.types );
        }
      return false;
      },
    // Convenience function which calls occurs_in_type for multiple types.
    occurs_in : function ( type_var, type_var_array ) {
      var idx;
      for ( idx in type_var_array ) {
        if ( this.occurs_in_type ( type_var, type_var_array[idx] ) ) {
          return true;
          }
        }
      return false;
      },
    // Checks whether a variable is generic in this context.
    // Prevents Turing completeness.
    is_generic : function ( type_var ) {
      return ! this.occurs_in ( type_var, this.non_generic );
      },
    // Creates a root or composite type.
    make_type_op : function ( name, types ) {
      return make_type_op ( name, types );
      },
    // Collapses a chain of type variables.
    prune : function ( type_var ) {
      if ( type_var.kind === 'var' ) {
        if ( type_var.instance ) {
          type_var.instance = this.prune ( type_var.instance );
          return type_var.instance;
          }
        }
      return type_var;
      },
    // Creates a semi-copy of a type.
    // Used to prevent mutating abstract composite types.
    fresh : function ( type ) {
      var mappings = string_map.make ( );
      var pruned = this.prune ( type );
      var fresh_types = [];
      var idx;

      if ( pruned.kind === 'var' ) {
        if ( this.is_generic ( pruned ) ) {
          if ( ! mappings.get ( pruned.name ) ) {
            mappings.set ( pruned.name, env.make_type_var ( ) );
            }
          return mappings.get ( pruned.name );
          }
        else {
          return pruned;
          }
        }
      else if ( pruned.kind === 'op' ) {
        for ( idx in pruned.types ) {
          fresh_types.push ( this.fresh ( pruned.types[idx] ) );
          }
        return this.make_type_op ( pruned.name, fresh_types );
        }
      else {
        throw 'Could not contruct fresh kind: ' + type.kind;
        }
      },
    // Attempts to make two types the same.
    // This is where type errors are actually checked.
    unify : function ( A, B ) {
      var a = this.prune ( A );
      var b = this.prune ( B );
      var idx;

      if ( a.kind === 'var' ) {
        if ( this.occurs_in_type ( a, b ) ) {
          throw "Recursive unification.";
          }
        // If a is a variable, is hasn't been unified before, because we just
        // pruned it.
        a.instance = b;
        }
      else if ( a.kind === 'op' && b.kind === 'var' ) {
        // Reverse the order.
        this.unify ( b, a );
        }
      else if ( a.kind === 'op' && b.kind === 'op' ) {
        // Type operators should have the same name and size.
        if ( a.name !== b.name ) {
          throw a.name + ' does not match ' + b.name;
          }
        else if ( a.types.length !== b.types.length ) {
          throw a.name + ' of different sizes: ' + a.types.length + ', ' + b.types.length;
          }
        for ( idx in a.types ) {
          this.unify ( a.types[idx], b.types[idx] );
          }
        }
      else {
        throw 'Cannot unify ' + a.name + ', ' + b.name;
        }
      },
    // Create a tuple type.
    make_tuple : function ( children ) {
      return make_tuple_type ( children );
      },
    // Used to implement scoping.
    push_block : function ( ) {
      this.table = scope.make ( this.table );
      },
    // Used to implement scoping.
    pop_block : function ( ) {
      this.table = this.table.above ( );
      },
    infer : function ( node ) {
      var result = node.infer ( this );
      node.run_type = result;
      return result;
      },
    };
  // This is where built in types and objects are currently initialized.
  // TODO(kzentner): Refactor this.
  env.set_type ( 'number', make_type_op ( 'number' ) );
  env.set_type ( 'bool', make_type_op ( 'bool' ) );
  return env;
  }

// Declares an infix operator ast element.
function infix ( text, type, out_type ) {
  if ( out_type === undefined ) {
    out_type = type;
    }
  return {
      infer: function ( env ) {
        var ltype = env.infer ( this.left );
        var rtype = env.infer ( this.right );
        env.unify ( ltype, env.get_type ( type ) );
        env.unify ( rtype, env.get_type ( type ) );
        return env.get_type ( out_type );
        },
    };
  }

// Declares a prefix operator ast element.
function prefix ( type, text ) {
  return {
      infer: function ( env ) {
        var rtype = env.infer ( this.right );
        if ( rtype.name === type ) {
          return env.get_type ( type );
          }
        else {
          throw 'argument to \'' + text + '\' was not a ' + text;
          }
        },
    };
  }

// Declares a constant ast element.
function constant ( text ) {
  return {
      infer: function ( env ) {
        return env.get_type ( text );
        },
    };
  }

// Add infer methods to scopes.
function setupScopes ( scopes ) {
  var escope = scopes.get ( 'expression', scope.make ( ) );
  var sscope = scopes.get ( 'statement', scope.make ( escope ) );

  escope.load_type ( string_map.make ( {
    // Numbers are of type 'number'.
    'number' : {
      infer: function ( env ) {
        return env.get_type ( 'number' );
        },
      },
    // Identifiers are instances of whatever type they are in the function.
    'identifier' : {
      infer: function ( env ) {
        var to_prune = env.get_text ( this.text );
        // The use of fresh here has a few odd effects. In particular,
        // variables pointing to polymorphic types are not restricted to a
        // single concrete type.
        // TODO(kzentner): Do we want this behavior.
        return env.fresh ( to_prune );
        }
      },
    } ) );
  escope.load_text ( string_map.make ( {
    // These declaration should hopefully be self-explanatory.
    '-' : infix ( '-', 'number'),
    '+' : infix ( '+', 'number'),
    'or' : infix ( 'or', 'bool' ),
    'and' : infix ( 'and', 'bool' ),
    '!=' : infix (  '!=', 'number', 'bool' ),
    '==' : infix ( '==', 'number', 'bool' ),
    'not' : prefix ( 'not', 'bool' ),
    'true': constant ( 'bool' ),
    'false': constant ( 'bool' ),
    'if': {
      infer: function ( env ) {
        var left_type;
        var right_type;
        env.unify ( env.get_type ( 'bool' ), env.infer ( this.condition ) );
        left_type = env.infer ( this.block );
        // If we are in an expression, we must have an alt-block and should
        // return the common type of both branches.
        if ( this.type === 'expr' ) {
          right_type = env.infer ( this.alt_block );
          env.unify ( left_type, right_type );
          return left_type;
          }
        },
      },
    'else': {
      // This method is only called on else statements, not in expressions, so
      // it just needs to typecheck its block.
      infer: function ( env ) {
        env.infer ( this.block );
        },
      },
    '(' : {
      infer: function ( env ) {
        var tuple_types = [];
        var arg_types = [];
        var arg_type;
        var idx;
        var func_type;

        if ( this.type === 'call' ) {
          for ( idx in this.args ) {
            // Analyze all the arguments.
            arg_types.push ( env.infer ( this.args[idx] ) );
            }
          // Function type operators ('fn') are operators of two types, the
          // first of which is a different type operator ('args').
          arg_type = env.make_type_op ( 'args', arg_types );
          func_type = env.get_text ( this.func.text );
          //if ( func_type === undefined ) {
            //func_type = env.get_text ( this.func.text );
            //}
          // If we're calling a type-var, make that type var point to a new
          // function type-operator.
          if ( func_type.kind === 'var' &&
               func_type.instance === null ) {
            func_type.instance = env.make_type_op ( 'fn', 
                [ env.make_type_var ( ),
                  env.make_type_var ( ) ] );
            func_type = func_type.instance;
            }
          env.unify ( arg_type, func_type.types[0] );
          // The type of a call is the return type of the function for that call.
          return func_type.types[1];
          }
        else if ( this.type === 'expr' ) {
          // The parentheses are just around some other expression.
          return env.infer ( this.children[0] );
          }
        else if ( this.type === 'tuple' ) {
          // The parentheses are around a tuple.
          for ( idx in this.children ) {
            tuple_types.push ( env.infer ( this.children ) );
            }
          return env.make_type_op ( 'tuple', tuple_types );
          }
        },
      },
    'fn' : {
      infer: function ( env ) {
        var idx;
        var child;
        var fn_type;
        var in_types = [];
        var in_type;
        var out_type = env.make_type_var ( );
        var arg_idx;

        // Set up the scope.
        env.push_block ( );

        // Set up the function arguments.
        for ( arg_idx in this.args ) {
          in_type = env.make_type_var ( );
          env.set_text ( this.args[arg_idx].text, in_type );
          in_types.push ( in_type );
          }
        in_type = env.make_type_op ( 'args', in_types );

        fn_type = env.make_type_op ( 'fn', [in_type, out_type] );
  
        // Type check the body of the function.
        for ( idx in this.body.children ) {
          child = this.body.children[idx];
          if ( child.text === 'return' ) {
            // return statements determine the return type of the function.
            // TODO(kzentner): Support expression functions with single expression bodies.
            // TODO(kzentner): Support return statements in sub-blocks.
            env.unify ( out_type, env.infer ( child.expr ) );
            }
          else {
            child.infer ( env );
            }
          }
        // Leave the scope.
        env.pop_block ( );

        // If we are named, add ourself to the current environment.
        // Note that this can be added late, because the inferencer can handle
        // late-delcared variables.
        if ( this.named ) {
          env.set_text ( this.name, fn_type );
          }
        return fn_type;
        },
      },
    } ) );

  sscope.load_text ( string_map.make ( {
    // This should be self-explanatory.
    'while': {
      infer: function ( env ) {
        env.unify ( env.get_type ( 'bool' ), env.infer ( this.condition ) );
        env.infer ( this.block );
        },
      },
    '=' : {
      infer: function ( env ) {
        var old_type = env.get_text ( this.left.text ) || env.make_type_var ( );
        var definition_type;

        old_type = env.set_text ( this.left.text, old_type );
        // The variable should not have generic type in the body of its definition.
        // This is one of the trickiest parts of Hindley-Milner.
        env.non_generic.push ( old_type );
        definition_type = env.infer ( this.right );
        env.non_generic.pop ( );

        // The actual variables type should be the most general of the old type
        // and the new definition.
        env.unify ( old_type, definition_type );
        },
      },
    } ) );

  sscope.load_type ( string_map.make ( {
    // These act the same, except that top_level doesn't create a new scope.
    'top_level' : {
      infer: function ( env ) {
       var idx;
  
        for ( idx in this.children ) {
          env.infer ( this.children[idx] );
          }
        },
      },
    'block' : {
      // Blocks return the value of the last statement in them.
      infer: function ( env ) {
       var out;
       var idx;

       env.push_block ( );
  
        for ( idx in this.children ) {
          out = env.infer ( this.children[idx] );
          }

        env.pop_block ( );
        return out;
        },
      },
    } ) );
  }

function infer ( tree ) {
  var env = make_env ( );
  var ret = env.infer ( tree );
  misc.print(env.table.toString());
  misc.print(env.unknown.toString());
  return ret;
  }

function make ( ) {
  return {
    setupScopes: setupScopes,
    infer: infer,
    };
  }

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
