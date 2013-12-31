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
// This module adds the recurse method to all ast prototypes that need it.
//

var scope = require ( './scope.js' );
var string_map = require ( './string_map.js' );

function infix ( ) {
  return {
    recurse: function ( func ) {
      func ( this.left );
      func ( this.right );
      },
    };
  }

function prefix ( ) {
  return {
    recurse: function ( func ) {
      func ( this.right );
      },
    };
  }

var make = function ( ) {
  var end_token = {
    'text': '(end)',
    'type': 'special',
    };

  var root_recurser = {
    setupScopes: function setupScopes ( scopes ) {
      var escope = scopes.get ( 'expression', scope.make ( ) );
      var sscope = scopes.get ( 'statement', scope.make ( escope ) );

      this.scopes = scopes;

      var atom = {
        recurse: function ( func ) {}
        };

      var infix_table = string_map.make ( {
        '+' : 60,
        '-' : 60,
        '*' : 70,
        '/' : 70,
        '%' : 70,
        '==': 40,
        'and': 30,
        'or': 30,
        '!=': 40,
        } ).map ( function ( key, val ) { return infix ( val ); } );

      var prefix_table = string_map.make ( {
        '++' : 90,
        '--' : 90,
        'not': 90,
        } ).map ( function ( key, val ) { return prefix ( val ); } );

      var type_table = string_map.make ( {
        'identifier' : atom,
        'number' : atom,
        'space' : atom,
        } );

      function paren_recurse ( func ) {
        if ( this.children ) {
          this.children.forEach ( func );
          }
        if ( this.func ) {
          func ( this.func );
          }
        if ( this.args ) {
          this.args.forEach ( func );
          }
        }

      escope.load_text ( infix_table );
      escope.load_text ( prefix_table );
      escope.load_type ( type_table );
      escope.load_text ( string_map.make ( {
        'fn' : {
          recurse: function ( func ) {
            this.args.forEach ( func );
            func ( this.body );
            },
          },
        '(' : { recurse: paren_recurse, },
        'if': {
          recurse: function ( func ) {
            func ( this.condition );
            func ( this.block );
            if ( this.alt_block ) {
              func ( this.alt_block );
              }
            },
          },
        'else': {
          recurse: function ( func ) {
            func ( this.block );
            }
          },
        } ) );
      sscope.load_text ( string_map.make ( {
        '=' : infix ( 10 ),
        'return': {
          recurse: function ( func ) {
            func ( this.expr );
            }
          },
        'while': {
          recurse: function ( func ) {
            func ( this.condition );
            func ( this.block );
            }
          },
        '(' : { recurse: paren_recurse, },
        } ) );
      sscope.load_type ( string_map.make ( {
        'block': {
          recurse: function ( func ) {
            this.children.forEach ( func );
            }
          },
        'top_level': {
          recurse: function ( func ) {
            this.children.forEach ( func );
            }
          },
         } ) );
      },
    };
  return function ( text ) {
    return Object.create ( root_recurser );
    };
  } ( );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
