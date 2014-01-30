var misc = require ( './misc.js' );
var scope = require ( './scope.js' );

function heirarchies_work ( ) {
  var p = scope.make ( );
  var p2 = scope.make ( p );
  var c = scope.make ( p2 );
  p.set_text ( 'a', 1 );
  misc.assert ( c.get_text ( 'a' ) === 1 );

  c.set_text ( 'b', 2 );
  misc.assert ( p.get_text ( 'b' ) === undefined );

  p.set_text ( 'c', 3 );
  c.set_text ( 'c', 4 );
  misc.assert ( p.get_text ( 'c' ) === 3 );

  misc.assert ( p.is_above ( c ) );
  misc.assert ( p2.is_above ( c ) );
  }

heirarchies_work ();
