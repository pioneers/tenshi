var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var module = require ( './module.js' );
var misc = require ( './misc.js' );
var varb = require ( './varb.js' );
var compiler = require ( './compiler.js' );
var library = require ( './library.js' );
var executor = require ( './executor.js' );

//
// This module is responsible for finding all identifiers, and determining
// which variables those identifiers refer to.  It determines if variables have
// canonical values.  It finds all objects, giving them absolute (canonical)
// names.  It assigns numbers to those names.
//
// This probably seems like a lot of roles, but they're all related and
// relatively minor.
//
// The design of this modules use in the compiler is as follows.
// We start with an AST.
// Then, we traverse the AST, looking for identifiers used in expressions.
// For each of these identifiers, we assign their .variable field to a variable object.
// We also record assignments to a variable in the corresponding variable object.
// Finally, we create scope objects, storing our variables in their corresponding scope.
//
// We also record whenever we encounter an which should be part of the output of the compiler.
// For now, these objects only include functions, but in the future should also
// include vectors, strings, etc.
//
// Once we have walked the whole AST, we have a set of variables and a set of objects.
//
// First, we check all variables to see if that variable is only assigned to a
// single value, in which case we record the right hand side of that assignment
// as it's canonical value. 
//
// Canaonical values are useful for multiple things, but are particularly
// useful because they're the only way for functions to call each other.
// This is because Angelic doesn't have traditional lexical scoping, since that
// would result in mutable, non-linear objects.
//
// Basically, consider what happens in the following program
//
// Now, we want to give names to our objects. There are multiple reasons for
// this. One is so that we can find the main function, where the vm needs to
// start executing from. The other reason is that we need to have a way to find
// functions in order patch them.
//
// We give our objects canonical names using the scope objects, in a fairly straightforward process.

//
// Questions and answers about this module:
//
// What is the point of canonical values?
// Canonical values are useful for various reasons, but are necessary for one reason in particular.
// Since Angelic needs to avoid garbage collection, it needs to ensure that
// there is no way to create non-linear mutable objects.
// However, functions by design should be allowed to have arbitrary connections
// (any function should be allowed to call any other function in its scope).
// Consider what would happen in the evaluation of the following program using lexical scoping.
//
// obj = fn field:
//   return fn cmd, val:
//     if cmd == 'set':
//       field = val
//     return field
//
// main = fn:
//   while True:
//     o1 = obj(0)
//     o2 = obj(o1)
//     o1('set', o2)
//
// Now, please ignore the fact that this program does not type check ('o1' is
// non-generic when unifying 'obj(o1)').
// If we were to evaluate it anyways (since compiling it without type checking
// would work), using reference counting for this program would result in it
// using unbounded memory, since every iteration of the loop leaks a loop of
// two functions.
//
// Therefore, Angelic cannot allow inner functions to access their parent's
// scope. However, functions should still be able to access each other in
// arbitrary ways.
//
// Now, consider how we would evaluate the following program under lexical
// scoping.
//
// main = fn:
//   print_hello = fn name:
//     print("Hello, " + name)
//   print_excited = fn name:
//     print_hello(name + "!")
//   print_excited("Robert")
//
// When print_excited is called, it needs to somehow get access to print_hello
// in order to call it. One naive idea is to create the print_hello function,
// and place a reference to it on the stack. Then, when we create the
// print_excited function, we copy the reference and store it with
// print_excited. However, this only works if functions call each other in the
// opposite of the order they're defined in.
//
// Consider the following program, where this technique would not work.
//
// main = fn:
//   eval = fn expr, scope:
//     if isList(expr):
//        apply(scope, lookup(scope, expr[0]), map(eval, expr[1:]))
//     return expr
//   apply = fn scope, form, args:
//      if isFunc(form[1]):
//        return form[1](args)
//      else:
//        for i in enumerate(args):
//          scope = [[form[0][i], args[i]]] + scope
//        return eval(form[1], scope)
//   lookup = fn list, word:
//      if list[0][0] == word:
//        return list[0][1]
//      else:
//        return lookup(list[1:], word)
//   core = [['*', [['a', 'b'], [fn a, b: a * b]],
//           ['square', [['a'], ['*', 'a', 'a']]]]]
//   print(eval(['square', 10], core))
//
// Here we have a perfectly good (although dynamically scoped) Lisp evaluator.
// However, this will not work with the technique we currently have for constructing functions.
// Not only is there recursion, there is mutual recursion, which prevents our
// idea of creating functions one at a time. We could emit opcodes so that when
// apply and lookup are created eval is patched so that it can call them, but
// that seems rather hacky. Furthermore, there is not really an intuitive way
// that this should occur in more complex programs.
//
// Consider the following program, similar to the above, where only the way
// lookup is created has been changed.
//
// use_iterative = True
// main = fn:
//   lookup = fn list, word: return 0
//   eval = fn expr, scope:
//     if isList(expr):
//        apply(scope, lookup(scope, expr[0]), map(eval, expr[1:]))
//     return expr
//   apply = fn scope, form, args:
//      if isFunc(form[1]):
//        return form[1](args)
//      else:
//        for i in enumerate(args):
//          scope = [[form[0][i], args[i]]] + scope
//        return eval(form[1], scope)
//   if use_iterative:
//     lookup = fn list, word:
//       for key, val in list:
//         if key == word:
//           return val
//   else:
//     lookup = fn list, word:
//        if list[0][0] == word:
//          return list[0][1]
//        else:
//          return lookup(list[1:], word)
//   core = [['*', [['a', 'b'], [fn a, b: a * b]],
//           ['square', [['a'], ['*', 'a', 'a']]]]]
//   print(eval(['square', 10], core))
//
// Now, we either need significantly more advanced analysis to determine that
// only the iterative version of lookup is used, or we need to construct patch
// code in both paths of the if statement. Both of these is fairly complex.
// Generating patches seems to be the simplest solution at first glance, but it
// doesn't have very clear semantics. Consider the following program.
//
// main = fn:
//   y = fn:
//     return x
//   x = fn:
//     return y
//   while True:
//     y = fn:
//       return x
//     x = fn:
//       return y
//
// How to deal with this is not terribly clear. Presumably we would end up
// creating an infinitely long linked-list from these two functions. However, I
// would prefer to avoid having to figure out what the sematics of this program
// should be. 
//
// Instead, the compiler imposes some more restrictions, and performs some
// simple analysis.
//
// For a variable to be looked up lexically, it must have a canonical value.
//
// A variable has a canonical value if and only if it is assigned precisely
// once in the AST, and the right hand side of the assignment is an object
// (i.e. some static value which will be output by the compiler), such as a
// function or string.
//
// Then, the compiler can emit a load immediate opcode and a lookup for each
// time that variable is read (the one assignment can also be removed).
//

function make ( ) {

    //
    // Most of these fields and methods are involved in the variable
    // identification algorithm.
    // 
    //
    // The basic way this algorithm works is that as a scope is walked,
    // variables are collected into an 'unknown' array.
    //
    // Assigned variables are also accumulated.
    // When the scope ends, assigned variables are compared to unknown variables.
    // If the variable was assigned in this scope and not in any scope above
    // it (with except possibly the global scope), then the unknown's callback
    // is invoked. This callback is used to set the variable for the
    // corresponding identifiers.
    // Otherwise, if the variable's assignment is not found, the unkown is
    // bubbled up to the scope above.
    // This continues upward until we hit the global scope, where we then store
    // the remaining unknowns as imports. 
    //
    // TODO(kzentner): Improve imports and exports, possible using an entirely
    // seperate module.
    //

  return {
    unknownses : null,
    scope_depth : null,
    current_scope : null,
    objects : null,
    variables : null,
    get_objects : function ( ) {
      return this.objects [ this.objects.length - 1 ];
      },
    get_unknowns : function ( ) {
      return this.unknownses [ this.unknownses.length - 1 ];
      },
    add_object : function ( obj ) {
      this.get_objects ( ).push ( obj );
      },
    push_object_list : function ( obj_list ) {
      this.objects.push ( obj_list );
      },
    pop_object_list : function ( ) {
      this.objects.pop ( );
      },
    get_text : function ( text, callback ) {
      var unknown = this.get_unknowns ( ).get_text ( text );
      if ( unknown === undefined ) {
        unknown = [];
        this.get_unknowns ( ).set_text ( text, unknown );
        }
      unknown.push ( { scope : this.current_scope,
                       callback : callback } );
      },
    set_variable : function ( text, val ) {
      var variable = this.current_scope.get_text ( text );
      if ( variable === undefined || variable.location === 'global' ) {
        // This is a variable declaration.
        variable = varb.make ( text );
        this.variables.push ( variable );
        this.current_scope.set_text ( text, variable );
        }
      variable.assignments.push ( val );
      return variable;
      },
    push_scope : function ( new_scope ) {
      misc.assert ( new_scope.above ( ) === this.current_scope,
                    "New scope should be child of current scope." ); 
      if ( this.current_scope.children === undefined ) {
        this.current_scope.children = [];
        }
      this.current_scope.children.push ( new_scope );
      this.current_scope = new_scope;
      this.scope_depth += 1;
      this.unknownses.push ( scope.make ( ) );
      },
    finalize_scope : function ( ) {
      var unknowns = this.get_unknowns ( );
      var parent_unknowns;
      if ( this.unknownses.length === 1 ) {
        // TODO(kzentner): Remove this hack.
        parent_unknowns = this.root_module.imports;
        }
      else {
        parent_unknowns = this.unknownses[this.unknownses.length - 2];
        }
      var self = this;
      unknowns.each_text ( function ( key, vals ) {
        if ( self.current_scope.text_distance ( key ) === 0 ) {
          // Variable was declared in this scope.
          vals.forEach ( function ( val ) {
            val.callback ( self.current_scope.get_text ( key ) );
            } );
          }
        else {
          // Variable was not declared in this scope, bubble it up.
          var parent_list = parent_unknowns.get_text ( key );
          if ( parent_list === undefined ) {
            parent_unknowns.set_text ( key, vals );
            }
          else {
            parent_unknowns.set_text ( key, parent_list.concat ( vals ) );
            }
          }
        } );
      },
    pop_scope : function ( ) {
      this.finalize_scope ( );
      this.unknownses.pop ( );
      this.current_scope = this.current_scope.above ( );
      this.scope_depth -= 1;
      },
    //
    // Walk the AST.
    //
    recurse : function ( node ) {
      if ( node.analyze !== undefined ) {
        // If we can analyze this node, do so.
        return node.analyze ( this );
        }
      else if ( node.recurse !== undefined ) {
        var self = this;
        return node.recurse ( function ( child ) {
          self.recurse ( child );
          } );
        }
      else {
        throw 'Could not analyze ' + JSON.stringify ( node, null, '  ' );
        }
      },
    init : function ( root_module ) {
      this.unknownses = [ scope.make ( ) ];
      this.scope_depth = 0;
      this.current_scope = root_module.globals;
      this.objects = [ root_module.objects ];
      this.variables = [];
      this.root_module = root_module;
      },
    analyze : function ( tree, modules ) {
      var root_module = module.make ( '' );
      var core_module;
      // TODO(kzentner): Fix this hackery.
      if ( modules !== undefined ) {
        core_module = modules.get ( 'core' );
        }
      if ( core_module === undefined ) {
        core_module = make_core_module ( );
        }
      root_module.ast = tree;
      this.init ( root_module );
      this.recurse ( tree );
      this.finalize_module ( );
      // TODO(kzentner): Add support for modules besides the core module.
      root_module.imports.each_text ( function ( key, imprts ) {
        var res = core_module.exports.get_text ( key );
        if ( res !== undefined ) {
          imprts.forEach ( function ( imprt ) {
            imprt.callback ( res );
            } );
          }
        } );
      this.map = generate_canonical_map ( root_module );
      generate_enumerations ( this.map );
      this.all_objects = extract_all_objs ( root_module );
      },
    finalize_module : function ( ) {
      // TODO(kzentner): Replace this hack.
      module.exports = module.globals;
      },
    setupScopes : setupScopes,
    };
  }

var sscope_text_methods = string_map.make ( {
  'return' : {
    analyze : function ( analyzer ) {
      analyzer.recurse ( this.expr );
      },
    },
  'while' : {
    analyze : function ( analyzer ) {
      var block_scope = scope.make ( analyzer.current_scope );
      analyzer.recurse ( this.condition );
      analyzer.push_scope ( block_scope );
      analyzer.recurse ( this.block );
      analyzer.pop_scope ( );
      },
    },
  '=' : {
    analyze : function ( analyzer ) {
      var variable = analyzer.set_variable ( this.left.text, this.right );
      if ( analyzer.scope_depth === 0 ) {
        variable.location = 'global';
        }
      analyzer.recurse ( this.left );
      analyzer.recurse ( this.right );
      },
    },
  } );

var sscope_type_methods = string_map.make ( {
  'top_level' : {
    analyze : function ( analyzer ) {
      this.children.forEach ( function ( child ) {
        analyzer.recurse ( child );
        } );
      analyzer.finalize_scope ( );
      }
    },
  } );

var magic_identifiers = string_map.make ( {
  'true' : varb.make ( 'true' ),
  'false' : varb.make ( 'false' ),
  } );

var escope_text_methods = string_map.make ( {
  'fn' : {
    analyze : function ( analyzer ) {
      this.objects = [];
      analyzer.add_object ( this );
      analyzer.push_object_list ( this.objects );

      this.scope = scope.make ( analyzer.current_scope );
      analyzer.push_scope ( this.scope );

      this.args.forEach ( function ( arg ) {
        // TODO(kzentner): Add default argument value support.
        analyzer.set_variable ( arg.text, { type: 'argument' } );
        analyzer.recurse ( arg );
        } );

      analyzer.recurse ( this.body );
      analyzer.pop_scope ( );
      analyzer.pop_object_list ( );

      this.patch_class = 'fn';
      },
    },
  'if' : {
    analyze : function ( analyzer ) {
      var block_scope = scope.make ( analyzer.current_scope );
      analyzer.recurse ( this.condition );
      analyzer.push_scope ( block_scope );
      analyzer.recurse ( this.block );
      analyzer.pop_scope ( );
      },
    },
  } );

var escope_type_methods = string_map.make ( {
  'number' : {
    analyze : function ( analyzer ) {
      return this;
      },
    },
  'identifier' : {
    analyze : function ( analyzer ) {
      var res = magic_identifiers.get ( this.text );

      if ( res !== undefined ) {
        this.variable = res;
        }
      else {
        var self = this;
        analyzer.get_text ( this.text, function ( val ) {
          self.variable = val;
          } );
        }
      },
    },
  } );

function setupScopes ( scopes ) {
  var escope = scopes.get ( 'expression', scope.make ( ) );
  var sscope = scopes.get ( 'statement', scope.make ( escope ) );

  sscope.load_text ( sscope_text_methods );
  sscope.load_type ( sscope_type_methods );

  escope.load_text ( escope_text_methods );
  escope.load_type ( escope_type_methods );
  }

var next_external_obj_id = 0;

//
// Theses external functions will be replaced.
//
function make_external_obj ( name, value ) {
  return {
    canonical_value: {
      patch_class: 'external',
      canonical_name: name,
      object_id: next_external_obj_id++,
      obj: value,
      },
    location: 'external',
    };
  }

function make_core_module ( ) {
  var core = module.make ( 'core' );
  core.exports.load_text ( string_map.make ( {
    'print' : make_external_obj ( 'core.print', misc.print ),
    } ) );
  return core;
  }


function get_canonical_value ( node ) {
  if ( node.assignments && node.assignments.length == 1 ) {
    node.canonical_value = node.assignments[0];
    return node.assignments[0];
    }
  else {
    throw 'Could not get canonical value.';
    }
  }
//
// Generate a mapping from absolute names to objects.
// If one or more variables has the object as its canonical value, the first
// alphabetical variable will be become the canonical name.
// All other names, and the number of the object in its scope, will be added as
// secondary names.
//
function generate_canonical_map ( module ) {
  var map = scope.make ( );
  function add_secondary_name ( obj, name ) {
    if ( obj.secondary_names !== undefined ) {
      obj.secondary_names = [];
      }
    obj.secondary_names.push ( name );
    }
  // We want the alphabetically first ("smallest") name.
  function add_new_name ( obj, name ) {
    if ( obj.canonical_name !== undefined ) {
      if ( obj.canonical_name > name ) {
        add_secondary_name ( obj, obj.canonical_name );
        obj.canonical_name = name;
        }
      else {
        add_secondary_name ( obj, name );
        }
      }
    else {
      obj.canonical_name = name;
      }
    }
  function gen_text ( scop, path, obj ) {
    scop.set_text ( path, obj );
    add_new_name ( obj, path );
    if ( obj.scope !== undefined ) {
      var named = [];
      misc.assert ( obj.objects !== undefined );
      obj.scope.each_text ( function ( key, child ) {
        if ( child.assignments.length === 1 ) {
          child = get_canonical_value ( child );
          if ( obj.objects.indexOf ( child ) !== -1 ) {
            gen_text ( scop, path + '.' + key, child );
            named.push ( child );
            }
          }
        } );
      for ( var i in obj.objects ) {
        var child = obj.objects[i];
        var name = path + '#' + i;
        add_secondary_name ( child, name );
        scop.set_text ( name, child );
        if ( named.indexOf ( child ) === -1 ) {
          // This object is not named.
          gen_text ( scop, path + '#' + i, child );
          child.canonical_name = name;
          }
        }
      }
    }
  module.globals.each_text ( function ( key, glob ) {
    gen_text ( map, key, get_canonical_value ( glob ) );
    } );
  return map;
  }

//
// Gives each object a number, and stores it in object.object_id.
// This function will be completely overhauled when patching support is added,
// since object_id is how the object to be patched is recorded in the patch
// format.
//
function generate_enumerations ( canonical_map ) {
  var enumerations = string_map.make ( );
  canonical_map.each_text ( function ( key, object ) {
    if ( key === object.canonical_name ) {
      // Only process by canonical name.
      var object_id = enumerations.get ( object.patch_class );
      if ( object_id === undefined ) {
        object_id = 0;
        }
      object.object_id = object_id;
      enumerations.set ( object.patch_class, object_id + 1 );
      }
    } );
  }

//
// Used to extract all the objects from the module.
// This is used to generate the set of objects which actually need to be output
// by the compiler.
//
function extract_all_objs ( module ) {
  var out = [];
  function extract ( obj ) {
    if ( obj.canonical_name === undefined ) {
      throw 'No canonical name!';
      }
    out.push ( obj );
    if ( obj.objects !== undefined ) {
      obj.objects.forEach ( function ( child ) {
        extract ( child );
        } );
      }
    }
  module.globals.each_text ( function ( key, glob ) {
    extract ( get_canonical_value ( glob ) );
    } );
  return out;
  }

exports.make = make;
