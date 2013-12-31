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

var number_type = make_type_op ( 'number' );
var bool_type = make_type_op ( 'bool' );

// Creates a typing environment. Basically, on of these should be used per
// module.
function make_env ( ) {
  var env = {
    // Holds which variables are not-generic.
    // This prevents the algorithm from becoming Turing complete.
    non_generic : [],
    functions : [],
    push_function : function ( func ) {
      this.functions.push ( func );
      },
    pop_function : function ( ) {
      this.functions.pop ( );
      },
    get_function : function ( ) {
      return this.functions[this.functions.length - 1];
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
    add_non_generic : function ( type ) {
      this.non_generic.push ( type );
      },
    remove_non_generic : function ( type ) {
      this.non_generic.splice ( this.non_generic.lastIndexOf ( type ), 1 );
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
    infer : function ( node ) {
      if ( node.infer !== undefined ) {
        var result = node.infer ( this );
        node.instance = result;
        return result;
        }
      else if ( node.recurse !== undefined ) {
        var self = this;
        node.recurse ( function ( child ) {
          self.infer ( child );
          } );
        }
      },
    get_instance : function ( obj ) {
      if ( obj.instance === undefined ) {
        obj.instance = this.make_type_var ( );
        }
      return obj.instance;
      },
    };
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
        env.unify ( ltype, type );
        env.unify ( rtype, type );
        return out_type;
        },
    };
  }

// Declares a prefix operator ast element.
function prefix ( type, text ) {
  return {
      infer: function ( env ) {
        var rtype = env.infer ( this.right );
        if ( rtype.name === type.name ) {
          return type;
          }
        else {
          throw 'argument to \'' + text + '\' was not a ' + text;
          }
        },
    };
  }

// Declares a constant ast element.
function constant ( type ) {
  return {
      infer: function ( env ) {
        return type;
        },
    };
  }

function infer_paren ( env ) {
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
    func_type = env.get_instance ( this.func );

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
  }

// Add infer methods to scopes.
function setupScopes ( scopes ) {
  var escope = scopes.get ( 'expression', scope.make ( ) );
  var sscope = scopes.get ( 'statement', scope.make ( escope ) );

  escope.load_type ( string_map.make ( {
    // Numbers are of type 'number'.
    'number' : {
      infer: function ( env ) {
        return number_type;
        },
      },
    // Identifiers are instances of whatever type they are in the function.
    'identifier' : {
      infer: function ( env ) {
        // In response to previous quandries about the use of fresh here:
        // The use of fresh here does not cause problems for e.g. local
        // variables of polymorphic function because of the non_generic list.

        return env.fresh ( env.get_instance ( this.variable ) );
        },
      },
    } ) );
  escope.load_text ( string_map.make ( {
    // These declaration should hopefully be self-explanatory.
    '-' : infix ( '-', number_type ),
    '+' : infix ( '+', number_type ),
    'or' : infix ( 'or', bool_type ),
    'and' : infix ( 'and', bool_type ),
    '!=' : infix (  '!=', number_type, bool_type ),
    '==' : infix ( '==', number_type, bool_type ),
    'not' : prefix ( 'not', bool_type ),
    'true': constant ( bool_type ),
    'false': constant ( bool_type ),
    'if': {
      infer: function ( env ) {
        var left_type;
        var right_type;
        env.unify ( bool_type, env.infer ( this.condition ) );
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
    '(' : { infer: infer_paren },
    'fn' : {
      infer: function ( env ) {
        var idx;
        var child;
        var fn_type;
        var in_types = [];
        var in_type;
        var out_type = env.make_type_var ( );
        var arg_idx;

        // Set up the function arguments.
        this.args.forEach ( function ( arg ) {
          in_types.push ( env.get_instance ( arg.variable ) );
          } );
        in_type = env.make_type_op ( 'args', in_types );

        fn_type = env.make_type_op ( 'fn', [in_type, out_type] );

        // Assigned early so that return statements in the body can use it.

        this.instance = fn_type;

        env.push_function ( this );
  
        // Type check the body of the function.
        env.infer ( this.body );

        env.pop_function ( this );

        return fn_type;
        },
      },
    } ) );

  sscope.load_text ( string_map.make ( {
    'while' : {
      infer : function ( env ) {
        env.unify ( bool_type, env.infer ( this.condition ) );
        env.infer ( this.block );
        },
      },
    '=' : {
      infer : function ( env ) {
        var old_type = env.get_instance ( this.left.variable );
        env.add_non_generic ( old_type );
        var def_type = env.infer ( this.right );
        env.remove_non_generic ( old_type );
        return env.unify ( old_type, def_type );
        },
      },
    'return' : {
      infer : function ( env ) {

        var type = env.infer ( this.expr );

        // Unify the return value's type with the current function's return
        // type.

        env.unify ( env.get_function ( ).instance.types [ 1 ], type );
        },
      },
    '(' : { infer: infer_paren },
    } ) );

  sscope.load_type ( string_map.make ( {
    } ) );
  }

function infer ( tree ) {
  var env = make_env ( );
  env.infer ( tree );
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
