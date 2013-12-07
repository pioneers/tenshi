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
var compiler = require ( './compiler.js' );
var library = require ( './library.js' );
var executor = require ( './executor.js' );

function make ( ) {
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
      //misc.print ( 'adding object to', this.get_objects ( ) );
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
        //misc.print ( 'Got variable declaration for ' + text );
        //misc.print ( 'Variable = ', variable );
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
      //misc.print ( 'unknowns', unknowns );
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
    recurse : function ( node ) {
      if ( node.analyze !== undefined ) {
        return node.analyze ( this );
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
    analyze : function ( tree ) {
      var root_module = module.make ( '' );
      var core_module = make_core_module ( );
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

      this.body.analyze ( analyzer );
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
      this.block.analyze ( analyzer );
      analyzer.pop_scope ( );
      },
    },
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
      this.block.analyze ( analyzer );
      analyzer.pop_scope ( );
      },
    },
  '=' : {
    analyze : function ( analyzer ) {
      //misc.print ( 'Analyzing assignment to ' + this.left.text );
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
        child.analyze ( analyzer );
        } );
      analyzer.finalize_scope ( );
      }
    },
  'block' : {
    analyze : function ( analyzer ) {
      this.children.forEach ( function ( child ) {
        //misc.print ( 'Analyzing', child );
        analyzer.recurse ( child );
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
      analyzer.recurse ( this.left );
      analyzer.recurse ( this.right );
      }
    };
  } 

var escope_text_methods = string_map.make ( {
  '(' : {
    analyze : function ( analyzer ) {
      analyzer.recurse ( this.func );
      this.args.forEach ( function ( arg ) {
        analyzer.recurse ( arg );
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
        //misc.print ( 'Registering callback!' );
        var self = this;
        analyzer.get_text ( this.text, function ( val ) {
          //misc.print ( 'Calling callback!' );
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
    misc.print ( node );
    throw 'Could not get canonical value.';
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
        child.secondary_name = name;
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

function extract_all_objs ( module ) {
  var out = [];
  function extract ( obj ) {
    if ( obj.canonical_name === undefined ) {
      //obj = get_canonical_value ( obj );
      //misc.print ( obj );
      throw 'No canonical name!';
      }
    out.push ( obj );
    if ( obj.objects !== undefined ) {
      //misc.print ( obj.objects );
      obj.objects.forEach ( function ( child ) {
        //misc.print ( child );
        extract ( child );
        } );
      }
    }
  module.globals.each_text ( function ( key, glob ) {
    extract ( get_canonical_value ( glob ) );
    } );
  return out;
  }

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
