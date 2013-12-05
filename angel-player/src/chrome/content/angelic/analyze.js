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

var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );
var module = require ( './module.js' );
var misc = require ( './misc.js' );
var varb = require ( './varb.js' );

function make_analyzer ( module ) {
  return {
    unknowns: [ scope.make ( ) ],
    scope_depth: 0,
    current_scope: module.globals,
    objects: [module.objects],
    get_objects: function ( ) {
      return this.objects [ this.objects.length - 1 ];
      },
    get_unknowns: function ( ) {
      return this.unknowns [ this.unknowns.length - 1 ];
      },
    add_object: function ( obj ) {
      //misc.print ( 'adding object to', this.get_objects ( ) );
      this.get_objects ( ).push ( obj );
      },
    push_object_list: function ( obj_list ) {
      this.objects.push ( obj_list );
      },
    pop_object_list: function ( ) {
      this.objects.pop ( );
      },
    get_text: function ( text, callback ) {
      var unknown = this.get_unknowns ( ).get_text ( text );
      if ( unknown === undefined ) {
        unknown = [];
        this.get_unknowns ( ).set_text ( text, unknown );
          
        }
      unknown.push ( { scope: this.current_scope,
                       callback: callback } );
      },
    set_variable: function ( text, val ) {
      var variable = this.current_scope.get_text ( text );
      if ( variable === undefined ) {
        // This is a variable declaration.
        variable = varb.make ( text );
        this.current_scope.set_text ( text, variable );
        }
      variable.assignments.push ( val );
      return variable;
      },
    push_scope: function ( new_scope ) {
      misc.assert ( new_scope.above ( ) === this.current_scope,
                    "New scope should be child of current scope." ); 
      this.current_scope = new_scope;
      this.scope_depth += 1;
      this.unknowns.push ( scope.make ( ) );
      },
    finalize_scope: function ( ) {
      var unknowns = this.get_unknowns ( );
      var parent_unknowns;
      if ( this.unknowns.length === 1 ) {
        parent_unknowns = module.imports;
        }
      else {
        parent_unknowns = this.unknowns[this.unknowns.length - 2];
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
          parent_unknowns.set_text ( key, vals );
          }
        } );
      },
    pop_scope: function ( ) {
      this.finalize_scope ( );
      this.unknowns.pop ( );
      this.current_scope = this.current_scope.above ( );
      this.scope_depth -= 1;
      },
    analyze: function ( node ) {
      if ( node.analyze !== undefined ) {
        return node.analyze ( this );
        }
      else {
        throw 'Could not analyze ' + JSON.stringify ( node, null, '  ' );
        }
      },
    finalize_module: function ( ) {
      // TODO(kzentner): Replace this hack.
      module.exports = module.globals;
      },
    };
  }

var sscope_text_methods = string_map.make ( {
  'fn' : {
    analyze : function ( analyzer ) {
      //misc.print ( 'analyzing', this );
      this.objects = [];
      analyzer.add_object ( this );
      analyzer.push_object_list ( this.objects );

      this.scope = scope.make ( analyzer.current_scope );
      analyzer.push_scope ( this.scope );

      this.args.forEach ( function ( arg ) {
        // TODO(kzentner): Add default argument value support.
        analyzer.set_variable ( arg.text, { type: 'argument' } );
        analyzer.analyze ( arg );
        } );

      this.body.analyze ( analyzer );
      analyzer.pop_scope ( );
      analyzer.pop_object_list ( );

      this.patch_class = 'fn';
      },
    },
  'if' : {
    analyze : function ( analyzer ) {
      var block_scope = scope.make ( analyzer.current_scope );
      analyzer.push_scope ( block_scope );
      this.block.analyze ( analyzer );
      analyzer.pop_scope ( );
      },
    },
  'while' : {
    analyze : function ( analyzer ) {
      var block_scope = scope.make ( analyzer.current_scope );
      analyzer.push_scope ( block_scope );
      this.block.analyze ( analyzer );
      analyzer.pop_scope ( );
      },
    },
  '=' : {
    analyze : function ( analyzer ) {
      var variable = analyzer.set_variable ( this.left.text, this.right );
      analyzer.analyze ( this.left );
      analyzer.analyze ( this.right );
      if ( analyzer.scope_depth === 0 ) {
        variable.location = 'global';
        }
      },
    },
  } );

var sscope_type_methods = string_map.make ( {
  'top_level' : {
    analyze : function ( analyzer ) {
      this.children.forEach ( function ( child ) {
        child.analyze ( analyzer );
        } );
      analyzer.finalize_scope ( );
      }
    },
  'block' : {
    analyze : function ( analyzer ) {
      this.children.forEach ( function ( child ) {
        child.analyze ( analyzer );
        } );
      }
    },
  } );

var magic_identifiers = string_map.make ( {
  'true' : { run_type: 'bool', text: 'true' },
  'false' : { run_type: 'bool', text: 'true' }
  } );

function infix ( text ) {
  return {
    analyze : function ( analyzer ) {
      analyzer.analyze ( this.left );
      analyzer.analyze ( this.right );
      }
    };
  } 

var escope_text_methods = string_map.make ( {
  '(' : {
    analyze : function ( analyzer ) {
      analyzer.analyze ( this.func );
      this.args.forEach ( function ( arg ) {
        analyzer.analyze ( arg );
        } );
      },
    },
  '-' : infix ( '-' ),
  '+' : infix ( '+' ),
  '!=' : infix ( '!=' ),
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

function analyze ( ast ) {
  var root_module = module.make ( '' );
  var core_module = make_core_module ( );
  root_module.ast = ast;
  var analyzer = make_analyzer ( root_module );
  analyzer.analyze ( ast );
  analyzer.finalize_module ( );
  // TODO(kzentner): Add support for modules besides the core module.
  root_module.imports.each_text ( function ( key, imprts ) {
    var res = core_module.exports.get_text ( key );
    if ( res !== undefined ) {
      imprts.forEach ( function ( imprt ) {
        imprt.callback ( res );
        } );
      }
    } );
  return {
    '': root_module,
    'core': core_module,
    };
  }

function make ( ) {
  return {
    analyze: analyze,
    setupScopes: setupScopes,
    };
  }

function make_core_module ( ) {
  var core = module.make ( 'core' );
  core.exports.load_text ( string_map.make ( {
    'print' : {
      type: 'external',
      obj: misc.print,
      },
    } ) );
  return core;
  }

function get_canonical_value ( node ) {
  if ( node.assignments && node.assignments.length == 1 ) {
    node.canonical_value = node.assignments[0];
    return node.assignments[0];
    }
  }

function generate_canonical_map ( module ) {
  var map = scope.make ( );
  function gen_text ( scop, path, obj ) {
    scop.set_text ( path, obj );
    obj.canonical_name = path;
    if ( obj.scope !== undefined ) {
      var named = [];
      misc.assert ( obj.objects !== undefined );
      obj.scope.each_text ( function ( key, child ) {
        child = get_canonical_value ( child );
        if ( obj.objects.indexOf ( child ) !== -1 ) {
          gen_text ( scop, path + '.' + key, child );
          named.push ( child );
          }
        } );
      for ( var i in obj.objects ) {
        var child = obj.objects[i];
        if ( named.indexOf ( child ) === -1 ) {
          // This object is not named.
          gen_text ( scop, path + '#' + i, child );
          }
        }
      }
    }
  module.globals.each_text ( function ( key, glob ) {
    gen_text ( map, key, get_canonical_value ( glob ) );
    } );
  return map;
  }

function generate_enumerations ( canonical_map ) {
  var enumerations = string_map.make ( );
  canonical_map.each_text ( function ( key, object ) {
    var object_id = enumerations.get ( object.patch_class );
    if ( object_id === undefined ) {
      object_id = 0;
      }
    object.object_id = object_id;
    enumerations.set ( object.patch_class, object_id + 1 );
    } );
  }

function test ( text ) {
  var parser = require ( './parser.js' );
  var scopes = string_map.make ( );
  var a_parser = parser.make ( );
  a_parser.setupScopes ( scopes );
  setupScopes ( scopes );
  parse_tree = a_parser.parse ( text );

  //misc.print ( parse_tree );
  var a_analyzer = make ( );
  var modules = a_analyzer.analyze ( parse_tree );
  var module = modules[''];
  var map = generate_canonical_map ( module );
  generate_enumerations ( map );
  misc.print ( map );
  //misc.print ( module.imports );
  //misc.print ( modules );
  }

var to_parse = '' +
'fib = fn (n):\n' +
'    n = n - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while n != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        n = n - 1\n' +
'    print (n)\n' +
'main = fn:\n' +
'    t = fn:\n' +
'        q = 1\n' +
'        wack = fn: 1\n' +
'    print(fn z: z = 12)\n' +
'    w = fn: q = 1\n' +
'    x = 50 - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while x != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        x = x - 1\n' +
'    print (b)\n' +
'test_fn = fn:\n' +
'    test = 0\n' +
'    if 0 != 0: test = 1\n' +
'    test = 0\n' +
'';


test ( to_parse );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
