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
var fn = require ( './fn.js' );
var varb = require ( './varb.js' );

function make_analyzer ( module ) {
  return {
    scope_depth: 0,
    current_scope: module.globals,
    unknown_scoped_variables: string_map.make ( ),
    add_object: function ( obj ) {
      module.objects.push ( obj );
      },
    get_variable_helper: function ( text ) {
      var variable = this.current_scope.get_text ( text );
      if ( variable === undefined ) {
        variable = this.unknown_scoped_variables.get ( text );
        }
      return variable;
      },
    get_variable: function ( text ) {
      var variable = this.get_variable_helper ( text );
      if ( variable === undefined ) {
        misc.print ( 'unknown', text );
        variable = varb.make ( text );
        this.unknown_scoped_variables.set ( text, [ this.current_scope, variable ] );
        }
      return variable;
      },
    set_variable: function ( text, val ) {
      var variable = this.current_scope.get_text ( text );
      if ( variable === undefined ) {
        var pair = this.unknown_scoped_variables.get ( text );
        if ( pair !== undefined && this.current_scope.is_above ( pair[0] ) ) {
          this.unknown_scoped_variables.delete ( text );
          variable = pair[1];
          misc.print ( 'found', text );
          }
        else {
          variable = varb.make ( text );
          }
        this.current_scope.set_text ( text, variable );
        }
      variable.assignments.push ( val );
      return variable;
      },
    add_to_scope: function ( str, obj ) {
      misc.assert ( this.current_scope.get_text ( str ) === undefined,
                    "Name collision: '" + str + "' is already defined in " +
                    this.current_scope );
      this.current_scope.set_text ( str, obj );
      },
    push_scope: function ( scope ) {
      misc.assert ( scope.above ( ) === this.current_scope,
                    "New scope should be child of current scope." ); 
      this.current_scope = scope;
      this.scope_depth += 1;
      },
    pop_scope: function ( ) {
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
    };
  }

var sscope_text_methods = string_map.make ( {
  'fn' : {
    analyze : function ( analyzer ) {
      this.scope = scope.make ( analyzer.current_scope );
      //var func = fn.make ( this, analyzer.current_scope );
      analyzer.add_object ( this );
      analyzer.push_scope ( this.scope );
      this.body.analyze ( analyzer );
      analyzer.pop_scope ( );
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
      for ( var idx in this.children ) {
        var child = this.children[idx];
        child.analyze ( analyzer );
        }
      misc.print ( analyzer.unknown_scoped_variables );
      }
    },
  'block' : {
    analyze : function ( analyzer ) {
      for ( var idx in this.children ) {
        var child = this.children[idx];
        analyzer.analyze ( child );
        }
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
      var i;
      analyzer.analyze ( this.func );
      for ( i in this.args ) {
        analyzer.analyze ( this.args[i] );
        }
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
        return res;
        }
      else {
        res = analyzer.get_variable ( this.text );
        }
      this.variable = res;
      return res;
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
  //root_module.ast = ast;
  var analyzer = make_analyzer ( root_module );
  ast.analyze ( analyzer );
  return {
    '': root_module,
    };
  }

function make ( ) {
  return {
    analyze: analyze,
    setupScopes: setupScopes,
    };
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
  misc.print ( modules );
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
'    x = 50 - 1\n' +
'    a = 0\n' +
'    b = 1\n' +
'    while x != 0:\n' +
'        temp = a + b\n' +
'        a = b\n' +
'        b = temp\n' +
'        x = x - 1\n' +
'    print (b)\n' +
'test = fn:\n' +
'    test = 0\n' +
'    if 0 != 0: test = 1\n';


test ( to_parse );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
