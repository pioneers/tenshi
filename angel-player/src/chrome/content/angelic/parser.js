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

var xregexp = require ( './xregexp/xregexp-all.js' );
var string_map = require ( './string_map.js' );
var scope = require ( './scope.js' );
var misc = require ( './misc.js' );

//
// This module parses text into ASTs.
//

var lex = function ( ) {
  var token_reg = xregexp.XRegExp (
      ' (?<space>       \\p{Whitespace}+)                   |' +
      ' (?<number>      [0-9]+)                             |' +
      ' (?<identifier>  [\\p{Letter}_] [\\p{Letter}_0-9]*)  |' +
      ' (?<operator>    [^\\p{Letter}_0-9\\p{Whitespace}]+)  ' ,
      'x' );
  var types = ['space', 'number', 'identifier', 'operator'];
  return function lex ( text ) {
    var match = xregexp.exec ( text, token_reg );
    if ( !match ) {
      return null;
      }
    for ( var t in types ) {
      if ( match[types[t]] !== undefined ) {
        return { type: types[t], text: match[types[t]] };
        }
      }
    };
  } ( );

function infix ( lbp, rbp ) {
  rbp = rbp !== undefined ? rbp : lbp;
  return {
    led: function ( parser, left ) {
      this.left = left;
      this.right = parser.expr ( rbp );
      },
    lbp: lbp,
    };
  }

function prefix ( rbp ) {
  return {
    nud: function ( parser ) {
      this.right = parser.expr ( rbp );
      },
    };
  }

var make = function ( ) {
  var end_token = {
    'text': '(end)',
    'type': 'special',
    };

  var root_parser = {

    // First lookup token by text, then by type in <scope>, returning a clone
    // of the found prototype ored with the passed in token. If <scope> is
    // not supplied, use the default scope.
    //
    // This code should only be used by get_token, or when a particular
    // prototype is desired, and should not be used in most of the parser.
    lookup_token: function lookup_token ( token, scope ) {
      var to_clone;

      if ( scope === undefined ) {
        scope = this.current_scope;
        }

      if ( token === undefined ) {
        to_clone = end_token;
        }
      else {
        //misc.print ( token );
        to_clone = scope.get_text ( token.text );
        if ( to_clone === undefined ) {
          to_clone = scope.get_type ( token.type );
          }
        }

      if ( to_clone !== undefined && to_clone.make !== undefined ) {
        return to_clone.make ( token );
        }

      // Split operators apart, using the longest prefix.
      // Note, small O(n^2) performance here.
      if ( to_clone === undefined &&
          token.type === 'operator' &&
          token.text.length > 1 ) {
        this.text_idx -= 1;
        token.text = token.text.substr ( 0, token.text.length - 1 );
        return this.lookup_token ( token, scope );
        }
      if ( token.text === '(' ) {
        //misc.print ( to_clone );
        }

      return misc.obj_or ( Object.create ( to_clone || {} ), token );
      },

    // Get a token by index. This function should also not be used by most of
    // the parser. Note that this function, in combination with lex and
    // lookup_token, is responsible for lazily creating the array of tokens.
    //
    // The purpose of this lazy creation is to allow different scopes to have
    // different operators, etc.
    get_token: function get_token ( index ) {
      var token;

      if ( index === undefined ) {
        index = this.token_idx;
        }

      while ( index >= this.tokens.length ) {
        token = lex ( this.text.substr ( this.text_idx ) );
        if ( ! token ) {
          return end_token;
          }

        this.text_idx += token.text.length;
        this.tokens.push ( token );
        }
      return this.lookup_token ( this.tokens[index] );
      },

    // Receives error messages. Currently just throws and exception. Will be
    // fixed later.
    handle_error: function handle_error ( message ) {
      throw message;
      },

    // Returns a boolean indicating whether <matcher> matched a particular
    // <token>.
    // Examples:
    //   {} matches everything.
    //   null matches nothing.
    //   { type: 'number' } only matches numbers.
    //   { type: 'number', text: '1' } only matches numbers with text 1.
    //   { text: '1' } only matches tokens with text 1.
    match: function match ( matcher, token ) {

      if ( ! matcher ) {
        return false;
        }

      if ( matcher.type === undefined &&
           matcher.text === undefined ) {
        return true;
        }

      if ( matcher.type !== undefined  &&
           matcher.text !== undefined ) {
        return token.text === matcher.text &&
               token.type === matcher.type;
        }

      if ( matcher.type !== undefined ) {
        return matcher.type === token.type;
        }

      if ( matcher.text !== undefined ) {
        return matcher.text === token.text;
        }

      // Should not be reachable.
      throw 'Unreachable.';
      },

    // Moves the parser forward on token, returning the found token.
    // This is the function that should be used by most of the parser for
    // moving consuming tokens.
    //
    // <expected> is optional.
    // If <expected> is an object, then the following fields are used:
    //    type: the type of token to expect
    //    text: the text in the token to expect
    //    skip: a matcher of tokens to skip, defaults to { type: 'space' }
    //    peek: a boolean, prevents advance from erroring or moving forwards
    //    optional: a boolean, prevents advance from erroring
    // If <expected> is a string, it is equivalent to { text: <expected> }.
    advance: function advance ( expected ) {
      var token;
      var skip = { type: 'space' };
      var idx = this.token_idx;
      var peek = expected && expected.peek;
      var optional = expected && expected.optional;

      if ( ! ( expected instanceof Object ) ) {
        expected = { text: expected };
        }

      if ( expected.skip !== undefined ) {
        skip = expected.skip;
        }

      token = this.get_token ( idx + 1 );
      while ( this.match ( skip, token ) ) {
        idx += 1;
        token = this.get_token ( idx + 1 );
        }
      if ( expected && ! this.match ( expected, token ) ) {
        if ( peek || optional ) {
          return null;
          }
        else {
          this.handle_error ( 'expected: ' + JSON.stringify ( expected ) );
          }
        }
      else {
        if ( ! peek ) {
          this.token_idx = idx + 1;
          }
        }
      return this.get_token ( idx + 1 );
      },

    // Used to handle calling led and nud methods, which may not return
    // anything, which is semantically equivalent to returning their objects.
    get_res: function get_res ( obj, res ) {
      if ( res === undefined ) {
        return obj;
        }
      else {
        return res;
        }
      },

    // Parse an expression in <scope>, with right binding power <rbp>.
    // rbp is optional and defaults to 0.
    // scope is optional and defaults to 'expression'.
    // This function sets and restores this.current_scope.
    expr: function expr ( rbp, scope ) {
      var left;
      var t;
      var old_scope = this.current_scope;

      if ( rbp === undefined ) {
        rbp = 0;
        }
      if ( scope === undefined ) {
        scope = this.scopes.get ( 'expression' );
        }

      this.current_scope = scope;
      t = this.advance ( );

      // This is a very commonly hit failure point, so this line is here for
      // debugging purposes.
      //misc.print ( 't', t );
      left = this.get_res ( t, t.nud ( this ) );
      // This loop here is basically the standard Vaugn Pratt parsing
      // algorithm.
      while ( rbp < this.advance ( { peek: true } ).lbp ) {
        t = this.advance ( );
        left = this.get_res ( t, t.led ( this, left ) );
        }
      this.current_scope = old_scope;
      return left;
      },

    // Returns if there is any more text to parse.
    more: function more ( ) {
      return this.text_idx !== this.text.length;
      },

    // Parses a statement. Note that this method currently allows expression
    // statements.
    statement: function statement ( ) {
      return this.expr ( 0, this.scopes.get ( 'statement' ) );
      },

    // Parses a (indented) block of statements.
    // Can either be given an indent, which is expected to start with a
    // newline, or will use the indent which is next in the parser.
    block: function block ( indent, top_level ) {
      var type = top_level ? 'top_level' : 'block';
      var the_block = this.lookup_token ( { type: type },
          this.scopes.get ( 'statement' ) );
      var initial = this.advance ( { type: 'space',
                                     peek: true,
                                     skip: null } );
      var token;
      var child;
      var prev_block = this.current_block;

      this.current_block = the_block;

      if ( initial ) {
        if ( indent && indent !== initial.text ) {
          this.handle_error ( 'Wrong initial indent' );
          }
        }
      else {
        if ( indent ) {
          initial = { type: 'space', text: indent };
          }
        else {
          initial = { type: 'space', text: '' };
          }
        }

      if ( ! ( initial && initial.text[0] === '\n' ) ) {
        // There is a single statement on the same line as the colon.
        // In other words, it's something like the following.
        // if foo: bar
        the_block.children.push ( this.statement ( ) );
        }
      else {
        while ( this.more ( ) ) {
          token = this.advance ( { type: 'space', peek: true, skip: null } );
          if ( token && token.text !== initial.text ) {
            break;
            }
          the_block.children.push ( this.statement ( ) );
          }
        }

      this.current_block = prev_block;
      return the_block;
      },

    // Reads a tuple of expressions. Allows an extra trailing comma.
    tuple: function ( ) {
      var out = [];
      var ex;

      ex = this.expr ( );
      if ( ex ) {
        out.push ( ex );
        }
      while ( this.advance ( { text: ',', peek: true } ) ) {
        ex = this.expr ( );
        if ( ex ) {
          out.push ( ex );
          }
        }
      return out;
      },

    // Reads a tuple of lvalues. Allows an extra trailing comma.
    lvalue_tuple: function ( ) {
      var out = [];
      var ident;

      ident = this.advance ( { type: 'identifier', optional: true } );
      if ( ident ) {
        out.push ( ident );
        }
      while ( this.advance ( { text: ',', peek: true } ) ) {
        ident = this.advance ( { type: 'identifier', optional: true } );
        if ( ident ) {
          out.push ( ident );
          }
        }
      return out;
      },

    // Sets up the ast prototypes. Similar methods are used in all of the
    // compiler modules to attach appropriate methods to ast nodes.
    setupScopes: function setupScopes ( scopes ) {
      var escope = scopes.get ( 'expression', scope.make ( ) );
      var sscope = scopes.get ( 'statement', scope.make ( escope ) );

      this.scopes = scopes;

      var atom = {
        lbp: 0,
        nud: function ( parser ) { },
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
        '+' : 80,
        '-' : 80,
        '++' : 90,
        '--' : 90,
        'not': 90,
        } ).map ( function ( key, val ) { return prefix ( val ); } );

      var type_table = string_map.make ( {
        'identifier' : atom,
        'number' : atom,
        'space' : atom,
        } );

      function paren_led ( parser, left ) {
        this.type = 'call';
        this.func = left;
        this.args = parser.tuple ( );
        parser.advance ( ')' );
        return this;
        }

      escope.load_text ( infix_table );
      escope.load_text ( prefix_table );
      escope.load_type ( type_table );
      escope.load_text ( string_map.make ( {
        'true' : atom,
        'false': atom,
        'fn' : {
          // Parse a function declaration / expression.
          // A lot of the code complexity is due to the fact that both the
          // name and parentheses are optional.
          nud: function ( parser ) {
            var paren = parser.advance ( { text: '(', optional: true } );

            this.args = parser.lvalue_tuple ( );

            if ( paren ) {
              parser.advance ( ')' );
              }

            parser.advance ( ':' );
            this.body = parser.block ( );
            },
          },
        ',' : { lbp: 0 },
        '(' : { lbp: 100,
          // Note that this needs to be fixed, so that it intelligently is
          // either a tuple or parentheses around an expression.
          nud: function ( parser ) {
            this.children = parser.tuple ( );
            // TODO(kzentner): Fix this, because the above line eats the comma.
            if ( this.children.length != 1 ||
                parser.advance ( { text: ',', optional: true } ) ) {
              this.type = 'tuple';
              }
            else {
              this.type = 'expr';
              }
            parser.advance ( ')' );
            },
          led: paren_led,
          },
        ')' : { lbp: 0 },
        'if': {
          expr_nud: function ( parser ) {
            var else_token;
            this.type = 'expr';
            this.condition = parser.expr ( );
            parser.advance ( ':' );
            this.block = parser.block ( );
            parser.advance ( 'else' );
            parser.advance ( ':' );
            this.alt_block = parser.block ( );
            },
          statement_nud: function ( parser ) {
            this.type = 'statement';
            this.condition = parser.expr ( );
            parser.advance ( ':' );
            this.block = parser.block ( );
            },
          nud: function ( parser ) {
            this.alt_block = null;
            if ( parser.current_scope === escope ) {
              this.expr_nud ( parser );
              }
            else if ( parser.current_scope === sscope ) {
              this.statement_nud ( parser );
              }
            },
          },
        'else': {
          nud: function ( parser ) {
            var c = parser.current_block.children;
            if ( c[c.length - 1].text != 'if' ) {
              parser.handle_error ( 'Else statements should only follow if statements.' );
              }
            if ( parser.current_scope !== sscope ) {
              parser.handle_error ( 'Else statements should not be in expressions.' );
              }
            parser.advance ( ':' );
            this.block = c[c.length - 1].alt_block = parser.block ( );
            },
          },
        } ) );
      sscope.load_text ( string_map.make ( {
        '=' : infix ( 10 ),
        ':' : { lbp: 0 },
        'return': {
          nud: function ( parser ) {
            this.expr = parser.expr ( );
            },
          },
        'while': {
          nud: function ( parser ) {
            this.condition = parser.expr ( );
            parser.advance ( ':' );
            this.block = parser.block ( );
            },
          },
        '(' : {
          lbp: 100,
          led: paren_led,
          },
        } ) );
      sscope.load_type ( string_map.make ( {
        'block': { text: 'block',
          make: function make_block ( token ) {
            var out = misc.obj_or ( Object.create ( this ), token );
            out.children = [];
            return out;
            } },
        'top_level': { text: 'top_level',
          make: function make_block ( token ) {
            var out = misc.obj_or ( Object.create ( this ), token );
            out.children = [];
            return out;
            } },
         } ) );
      },

    // Parse a top-level string.
    parse: function parse ( text ) {
      this.text = text;
      this.current_scope = this.scopes.get ( 'statement' );
      return this.block ( '\n', true );
      },

    // Parse an expression. Meant for external use, not internal use.
    parse_expr: function parse_expr ( text ) {
      this.text = text;
      this.current_scope = this.scopes.get ( 'statement' );
      return this.expr ( );
      },
    };
  return function ( text ) {
    return misc.obj_or ( Object.create ( root_parser ), {
      tokens: [],
      text: '',
      token_idx: -1,
      text_idx: 0,
      current_scope: scope.make ( ),
      current_block: null,
      } );
    };
  } ( );

// Export Machinery to make node.js and xulrunner act the same.
var EXPORTED_SYMBOLS = ['make'];
var exported_objects = [ make ];
export_vals ( EXPORTED_SYMBOLS,
              exported_objects );
// End Export Machinery
